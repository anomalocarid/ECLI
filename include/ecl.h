/**
 * Definitions for ECL files
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
#ifndef __ECL_H__
#define __ECL_H__

#include "config.h"
#include <stdint.h>

typedef struct {
PACK_BEGIN
    char magic[4];
    uint16_t unknown1; /* 1 */
    uint16_t include_length; /* include_offset + ANIM+ECLI length */
    uint32_t include_offset; /* sizeof(th10_header_t) */
    uint32_t zero1;
    uint32_t sub_count;
    uint32_t zero2[4];
PACK_END
} PACK_ATTRIBUTE th10_header_t;

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

extern ecli_result_t read_th10_ecl_header(th10_header_t* header, FILE* f);
extern int verify_th10_ecl_header(th10_header_t* header);
extern void print_th10_ecl_header(th10_header_t* header);

#endif
