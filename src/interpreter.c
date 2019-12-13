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
pushi(ecl_state_t* s, uint32_t i)
{
    s->stack[s->sp].type = ECL_INT;
    s->stack[s->sp++].u = i;
}

static uint32_t
popi(ecl_state_t* s)
{
    return s->stack[--s->sp].u;
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
        
        case 30: { // unknown30 - we're using this as a string print statement
            uint32_t len = *(uint32_t*)&ins->data[0];
            char* s = &ins->data[4];
            
            printf("%s\n", s);
        }   break;

        case 40: { // stackAlloc
            uint32_t amt = (*(uint32_t*)&ins->data[0]) >> 2;
            
            pushi(state, state->bp);
            state->bp = state->sp;
            state->sp += amt;
        }   break;

        case 42: { // push
            uint32_t value = *(uint32_t*)&ins->data[0];
            
            pushi(state, value);
        }   break;

        case 43: { // set
            uint32_t var = *(uint32_t*)&ins->data[0];
            state->stack[state->bp+var].type = ECL_INT;
            state->stack[state->bp+var].u = popi(state);
        }   break;
        
        default:
            fprintf(stderr, "Unknown instruction id: %d\n", ins->id);
            retval = ECLI_FAILURE;
            break;
    }
    
    state->ip = next;
    return retval;
}
