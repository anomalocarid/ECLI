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
