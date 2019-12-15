/**
 * ECL instructions
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
#ifndef __ECLI_INS_H__
#define __ECLI_INS_H__

#include "config.h"
#include <stdint.h>

//th17 ins IDs
typedef enum {
    //system instructions
    INS_NOP=0,
    INS_DELETE=1,
    INS_RET=10,
    INS_CALL=11,
    INS_JMP=12,
    INS_JMPEQ=13,
    INS_JMPNEQ=14,
    INS_CALLASYNC=15,
    INS_UNKNOWN21=21,
    INS_WAIT=23,
    INS_UNKNOWN30=30,
    INS_STACKALLOC=40,
    INS_PUSH=42,
    INS_SET=43,
    INS_PUSHF=44,
    INS_SETF=45,
    INS_ADDF=51,
    INS_DECI=78,
    // Enemy property management and other miscellaneous things
    INS_FLAGSET=502,
    INS_SETCHAPTER=524,
    
    INS_INVALID=0xFFFF
} ecl_ins_id;

typedef struct {
PACK_BEGIN
    uint32_t time;
    uint16_t id;
    uint16_t size;
    uint16_t param_mask;
    /* The rank bitmask.
     *   1111LHNE
     * Bits mean: easy, normal, hard, lunatic. The rest are always set to 1. */
    uint8_t rank_mask;
    /* There doesn't seem to be a way of telling how many parameters there are
     * from the additional data. */
    uint8_t param_count;
    /* From TH13 on, this field stores the number of current stack references
     * in the parameter list. */
    uint32_t zero;
    unsigned char data[];
PACK_END
} PACK_ATTRIBUTE th10_instr_t;

#endif
