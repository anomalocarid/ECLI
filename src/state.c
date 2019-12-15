/**
 * Functions for handling the ECL interpreter state
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

#define STACK_SIZE 1024

/**
 * Initialize the ECL interpreter state
 **/
ecli_result_t 
initialize_ecl_state(ecl_state_t* state, th10_ecl_t* ecl)
{
    memset(state, 0, sizeof(ecl_state_t));
    state->stack_size = STACK_SIZE;
    state->ecl = ecl;
    state->stack = xmalloc(sizeof(ecl_value_t)*STACK_SIZE);
    state->callstack = xmalloc(sizeof(th10_instr_t*)*STACK_SIZE);
    
    memset(state->stack, 0, sizeof(ecl_value_t)*STACK_SIZE);
    memset(state->callstack, 0, sizeof(th10_instr_t*)*STACK_SIZE);
    
    return ECLI_SUCCESS;
}

/**
 * Free the ECL interpreter state
 **/
void
free_ecl_state(ecl_state_t* state)
{
    xfree(state->stack);
    xfree(state->callstack);
    memset(state, 0, sizeof(ecl_state_t));
}

/**
 * Setup a stack frame
 **/
ecli_result_t
state_setup_frame(ecl_state_t* state, uint32_t nvars)
{
    state->stack[state->sp].type = ECL_UINT32;
    state->stack[state->sp++].u = state->bp;
    state->bp = state->sp;
    state->sp += nvars;
    for(unsigned int i = 0; i < nvars; i++) {
        state->stack[state->bp + i].type = ECL_INT32;
    }
    
    return ECLI_SUCCESS;
}

ecli_result_t
state_push(ecl_state_t* state, ecl_value_t* value)
{
    state->stack[state->sp++] = *value;
    return ECLI_SUCCESS;
}

ecl_value_t*
state_pop(ecl_state_t* state)
{
    return &state->stack[--state->sp];
}

ecl_value_t*
state_peek(ecl_state_t* state)
{
    return &state->stack[state->sp - 1];
}

ecli_result_t
state_get_variable(ecl_state_t* state, int32_t slot, ecl_value_t* result)
{
    if(slot >= 0) { // stack
        ecl_value_t* v = &state->stack[state->bp + (slot >> 2)];
        result->type = v->type;

        switch(v->type) {
            case ECL_INT32:
                result->i = v->i;
                break;
            case ECL_FLOAT32:
                result->f = v->f;
                break;
            default:
                return ECLI_FAILURE; // unsupported type
                break;
        }
    } else { // global/local
    }
    return ECLI_SUCCESS;
}

ecli_result_t
state_set_variable(ecl_state_t* state, int32_t slot, ecl_value_t* value)
{
    if(slot >= 0) { // stack
        slot = slot >> 2;
        ecl_value_t* v = &state->stack[state->bp + slot];
        v->type = value->type;
        
        switch(v->type) {
            case ECL_INT32:
                v->i = value->i;
                break;
            case ECL_FLOAT32:
                v->f = value->f;
                break;
            default:
                return ECLI_FAILURE;
                break;
        }
    } else { // global/local
    }
    return ECLI_SUCCESS;
}
