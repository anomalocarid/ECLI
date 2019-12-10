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

// Rank masks
#define RANK_EASY (0xF0 | (1 << 0))
#define RANK_NORMAL (0xF0 | (1 << 1))
#define RANK_HARD (0xF0 | (1 << 2))
#define RANK_LUNATIC (0xF0 | (1 << 3))
#define RANK_EN (RANK_EASY | RANK_NORMAL)
#define RANK_HL (RANK_HARD | RANK_LUNATIC)
#define RANK_ALL (RANK_EN | RANK_HL)

/**
 * Represents the header of a MoF-and-later ECL file
 **/
typedef struct {
PACK_BEGIN
    char magic[4]; /* Always 'SCPT' */
    uint16_t unknown1; /* 1 */
    uint16_t include_length; /* include_offset + ANIM+ECLI length */
    uint32_t include_offset; /* sizeof(th10_header_t) */
    uint32_t zero1; /* always zero */
    uint32_t sub_count; /* number of subroutines */
    uint32_t zero2[4]; /* always zero */
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

typedef struct {
PACK_BEGIN
	char name[4]; /* 'ECLI' or 'ANIM' */
	uint32_t count; /* number of files */
	char data[]; /* start of the first string */
PACK_END
} PACK_ATTRIBUTE th10_include_list_t;

typedef struct {
    char* name;
    th10_instr_t* start;
} th10_ecl_sub_t;

// Represents an ECL file loaded in memory
typedef struct {
    th10_header_t* header;
    th10_include_list_t* anims;
    th10_include_list_t* eclis;
    th10_ecl_sub_t* subs;
} th10_ecl_t;

// The kinds of includes allowed in ECL files
typedef enum {
    INCLUDE_ANIM=0,
    INCLUDE_ECLI,
    INCLUDE_MAX
} include_t;

/* General ECL functions */
extern ecli_result_t load_th10_ecl_from_file_object(th10_ecl_t* ecl, FILE* f);
extern void free_th10_ecl(th10_ecl_t* ecl);

/* ECL Header Functions */
extern int verify_th10_ecl_header(th10_ecl_t* ecl);
extern void print_th10_ecl_header(th10_ecl_t* ecl);

/* ECL Include List Functions */
extern th10_include_list_t* th10_ecl_get_include_list(th10_ecl_t* ecl, include_t include);
extern char* th10_ecl_get_include(th10_include_list_t* list, unsigned int idx);

#endif
