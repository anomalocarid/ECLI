/**
 * Definitions for the ECL interpreter state
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
#ifndef __ECLI_STATE_H__
#define __ECLI_STATE_H__

#include "ecli.h"

typedef enum {
    ECL_INVALID=0,
    ECL_INT,
    ECL_FLOAT
} ecl_type_t;

typedef struct {
    ecl_type_t type;
    union {
        float f;
        uint32_t u;
    };
} ecl_value_t;

typedef struct {
    size_t stack_size;
    uint32_t stackp;
    uint32_t basep;
    ecl_value_t* stack;
    th10_instr_t** callstack;
    th10_ecl_t* ecl;
    uint32_t csp;
} ecl_state_t;

/* state.c */
extern ecli_result_t initialize_ecl_state(ecl_state_t* state, th10_ecl_t* ecl, unsigned int stack_size);
extern void free_ecl_state(ecl_state_t* state);

/* interpreter.c */
extern ecli_result_t run_th10_instruction(ecl_state_t* state, th10_instr_t* ins, th10_instr_t** next);

#endif 
