/**
 * Functions for running ECL instructions
 *
 * Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that the
 * following conditions are met:
 *
 * 1. Redistributions of source code must retain this list
 *    of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce this
 *    list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 **/
#include "ecli.h"
#include "ecl.h"
#include "state.h"

static void
pushi(ecl_state_t* s, int32_t i)
{
    s->stack[s->sp].type = ECL_INT32;
    s->stack[s->sp++].i = i;
}

static int32_t
popi(ecl_state_t* s)
{
    return s->stack[--s->sp].i;
}

// get int variable
static int32_t
getvi(ecl_state_t* s, uint32_t idx)
{
    return s->stack[s->bp+idx].i;
}

// get int param
static int32_t
geti(ecl_state_t* s, uint8_t slot)
{
    th10_instr_t* ins = s->ip;
    uint16_t mask = ins->param_mask;
    uint32_t val = *(((uint32_t*)&ins->data[0]) + slot);
    if((mask >> slot) & 1) {
        return getvi(s, val);
    }
    
    return (int32_t)val;
}

/**
 * Run a single ECL instruction
 **/
ecli_result_t
run_th10_instruction(ecl_state_t* state)
{
    static ecl_value_t params[32];
    static ecl_value_t values[32];
    ecl_value_t value;
    unsigned int nparam;
    
    ecli_result_t retval = ECLI_SUCCESS;
    
    th10_instr_t* ins = state->ip;
    th10_instr_t* next = (th10_instr_t*)(((uint8_t*)ins) + ins->size);

    if(!(state->difficulty & ins->rank_mask)) {
        state->ip = next;
        return ECLI_SUCCESS;
    }
    
    retval = get_ins_params(ins, params, &nparam);

    if(!SUCCESS(retval)) {
        return retval;
    }
    
    // Replace variable references with the corresponding values
    for(unsigned int i = 0; i < nparam; i++) {
        if(ins->param_mask & (1 << i)) {
            retval = state_get_variable(state, params[i].i, &values[i]);
            if(!SUCCESS(retval)) {
                return retval;
            }
        } else {
            values[i] = params[i];
        }
    }

    // Execute the instruction
    switch(ins->id) {
        case INS_RET: // return
            state->sp = state->bp;
            state->bp = popi(state);
            if(state->csp == 0) {
                retval = ECLI_DONE;
            } else {
                next = state->callstack[--state->csp];
            }
            break;

        case INS_CALL: { // call
            state->callstack[state->csp++] = next;
            th10_ecl_sub_t* sub = get_th10_ecl_sub_by_name(state->ecl, params[0].s);
            if(sub == NULL) {
                fprintf(stderr, "call: sub \"%s\" does not exist\n", params[0].s);
                retval = ECLI_FAILURE;
            }
            next = sub->start;
        }   break;
        
        case INS_JMP: { // jmp (unconditional goto)
            uint32_t at_time = params[1].u;
            
            next = (th10_instr_t*)(((uint8_t*)ins) + params[0].i);
        }   break;
        
        case INS_JMPEQ: { // jmpEq
            if(popi(state) == 0) {
                uint32_t at_time = params[1].u;
                
                next = (th10_instr_t*)(((uint8_t*)ins) + params[0].i);
            }
        }   break;
        
        case INS_UNKNOWN21: { // unknown21 - use it to print the top of the stack
            if(state->sp > 0) {
                ecl_value_t* val = &state->stack[state->sp-1];
                if(val->type == ECL_INT32) {
                    printf("%d\n", val->i);
                } else if(val->type == ECL_FLOAT32) {
                    printf("%f\n", val->f);
                }
            }
        }   break;
        
        case INS_WAIT: { // wait 
            int32_t amt = values[0].i;
            // TODO: actually wait
        }   break;
        
        case INS_UNKNOWN30: { // unknown30 - we're using this as a string print statement
            printf("%s\n", values[0].s);
        }   break;

        case INS_STACKALLOC: { // stackAlloc
            retval = state_setup_frame(state, params[0].u >> 2);
        }   break;

        case INS_PUSH: { // push
            pushi(state, geti(state, 0));
        }   break;

        case INS_SET: { // set
            value.type = ECL_INT32;
            value.i = popi(state);
            retval = state_set_variable(state, params[0].i, &value);
        }   break;
        
        case INS_DECI: { // deci
            pushi(state, values[0].i);
            value.type = ECL_INT32;
            value.i = values[0].i - 1;
            retval = state_set_variable(state, params[0].i, &value);
        }   break;
        
        case 502: { // flagSet
            state->flags = values[0].i;
            break;
        }
        
        case 524: { // setChapter
            state->chapter = values[0].i;
        }   break;
        
        default:
            fprintf(stderr, "Unknown instruction id: %d\n", ins->id);
            retval = ECLI_FAILURE;
            break;
    }
    
    state->ip = next;
    return retval;
}
