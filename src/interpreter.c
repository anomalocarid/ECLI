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

/**
 * Run a single ECL instruction
 **/
ecli_result_t
run_th10_instruction(ecl_state_t* state)
{
    static ecl_value_t params[32];
    static ecl_value_t values[32];
    ecl_value_t* value;
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
            int32_t slot = (params[i].type == ECL_FLOAT32) ? (int32_t)params[i].f : params[i].i;
            retval = state_get_variable(state, slot, &values[i]);
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
            state->bp = state_pop(state)->u;
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
            value = state_pop(state);
            if(value->i == 0) {
                uint32_t at_time = params[1].u;
                
                next = (th10_instr_t*)(((uint8_t*)ins) + params[0].i);
            }
        }   break;
        
        case INS_UNKNOWN21: { // unknown21 - use it to print the top of the stack
            if(state->sp > 0) {
                ecl_value_t* val = state_pop(state);
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

        case INS_PUSH: 
        case INS_PUSHF: { // push
            retval = state_push(state, &values[0]);
        }   break;

        case INS_SET: { // set
            value = state_pop(state);
            value->type = ECL_INT32;
            retval = state_set_variable(state, params[0].i, value);
        }   break;
        
        case INS_SETF: {
            value = state_pop(state);
            value->type = ECL_FLOAT32;
            retval = state_set_variable(state, params[0].i, value);
        }   break;
        
        case INS_ADDI: {
            value = state_pop(state);
            ecl_value_t* top = state_peek(state);
            top->i += value->i;
            top->type = ECL_INT32;
        }   break;

        case INS_ADDF: {
            value = state_pop(state);
            ecl_value_t* top = state_peek(state);
            top->f += value->f;
            top->type = ECL_FLOAT32;
        }   break;
        
        case INS_DECI: { // deci
            values[0].type = ECL_INT32;
            retval = state_push(state, &values[0]);
            values[0].i = values[0].i - 1;
            retval = state_set_variable(state, params[0].i, &values[0]);
        }   break;
        
        case INS_FLAGSET: { // flagSet
            state->flags = values[0].i;
            break;
        }
        
        case INS_SETCHAPTER: { // setChapter
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
