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
    s->stack[s->sp].type = ECL_INT;
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
    ecli_result_t retval = ECLI_SUCCESS;
    th10_instr_t* ins = state->ip;
    th10_instr_t* next = (th10_instr_t*)(((uint8_t*)ins) + ins->size);
    if(!(state->difficulty & ins->rank_mask)) {
        state->ip = next;
        return ECLI_SUCCESS;
    }

    switch(ins->id) {
        case 10: // return
            state->sp = state->bp;
            state->bp = popi(state);
            if(state->csp == 0) {
                retval = ECLI_DONE;
            } else {
                next = state->callstack[--state->csp];
            }
            break;

        case 11: { // call
            uint32_t len = *(uint32_t*)&ins->data[0];
            char* name = &ins->data[4];
            
            state->callstack[state->csp++] = next;
            th10_ecl_sub_t* sub = get_th10_ecl_sub_by_name(state->ecl, name);
            if(sub == NULL) {
                fprintf(stderr, "call: sub \"%s\" does not exist\n", name);
                retval = ECLI_FAILURE;
            }
            next = sub->start;
        }   break;
        
        case 12: { // jmp (unconditional goto)
            int32_t offset = *(int32_t*)&ins->data[0];
            uint32_t at_time = *(uint32_t*)&ins->data[4];
            
            next = (th10_instr_t*)(((uint8_t*)ins) + offset);
        }   break;
        
        case 13: { // jmpEq
            if(popi(state) == 0) {
                int32_t offset = *(int32_t*)&ins->data[0];
                uint32_t at_time = *(uint32_t*)&ins->data[4];
                
                next = (th10_instr_t*)(((uint8_t*)ins) + offset);
            }
        }   break;
        
        case 21: { // unknown21 - use it to print the top of the stack
            if(state->sp > 0) {
                ecl_value_t* val = &state->stack[state->sp-1];
                if(val->type == ECL_INT) {
                    printf("%d\n", val->i);
                } else if(val->type == ECL_FLOAT) {
                    printf("%f\n", val->f);
                }
            }
        }   break;
        
        case 23: { // wait 
            uint32_t amt = geti(state, 0);
            // TODO: actually wait
        }   break;
        
        case 30: { // unknown30 - we're using this as a string print statement
            uint32_t len = *(uint32_t*)&ins->data[0];
            char* s = &ins->data[4];
            
            printf("%s\n", s);
        }   break;

        case 40: { // stackAlloc
            uint32_t amt = geti(state, 0) >> 2;
            
            pushi(state, state->bp);
            state->bp = state->sp;
            state->sp += amt;
        }   break;

        case 42: { // push
            pushi(state, geti(state, 0));
        }   break;

        case 43: { // set
            uint32_t var = *(uint32_t*)&ins->data[0];
            state->stack[state->bp+var].type = ECL_INT;
            state->stack[state->bp+var].i = popi(state);
        }   break;
        
        case 78: { // deci
            uint32_t var = *(uint32_t*)&ins->data[0];
            int32_t value = getvi(state, var);
            pushi(state, value);
            state->stack[state->bp+var].type = ECL_INT;
            state->stack[state->bp+var].i = value - 1;
        }   break;
        
        case 502: { // flagSet
            state->flags = geti(state, 0);
            break;
        }
        
        case 524: { // setChapter
            state->chapter = geti(state, 0);
        }   break;
        
        default:
            fprintf(stderr, "Unknown instruction id: %d\n", ins->id);
            retval = ECLI_FAILURE;
            break;
    }
    
    state->ip = next;
    return retval;
}
