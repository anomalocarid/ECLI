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
        int32_t i;
    };
} ecl_value_t;

enum {
    DIFF_EASY=1,
    DIFF_NORMAL=2,
    DIFF_HARD=4,
    DIFF_LUNATIC=8
};

typedef struct {
    // Data stack
    size_t stack_size;
    uint32_t sp; // Stack pointer
    uint32_t bp; // Base pointer
    ecl_value_t* stack;
    // Call stack
    th10_instr_t** callstack;
    uint32_t csp;
    
    th10_ecl_t* ecl; // ECL data
    th10_instr_t* ip; // Instruction pointer
    
    //Internal state
    uint8_t difficulty;
    uint32_t flags;
    uint32_t chapter;
} ecl_state_t;

/* state.c */
extern ecli_result_t initialize_ecl_state(ecl_state_t* state, th10_ecl_t* ecl);
extern void free_ecl_state(ecl_state_t* state);

/* interpreter.c */
extern ecli_result_t run_th10_instruction(ecl_state_t* state);

#endif 
