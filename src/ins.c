/**
 * Functions for dealing with ECL instructions
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

#include <stdio.h>
#include <stdint.h>

static uint8_t last_mask = 0x0F;

// Instruction parameter format strings
typedef struct {
    uint16_t id;
    const char* format;
} ins_format_t;

static const ins_format_t instruction_formats[] = {
    //system instructions
    {INS_NOP, ""},
    {INS_DELETE, ""},
    {INS_RET, ""},
    {INS_CALL, "s"},
    {INS_JMP, "iu"},
    {INS_JMPEQ, "iu"},
    {INS_JMPNEQ, "iu"},
    {INS_CALLASYNC, "s"},
    {INS_UNKNOWN21, ""},
    {INS_WAIT, "i"},
    {INS_UNKNOWN30, "s"},
    {INS_STACKALLOC, "u"},
    {INS_PUSH, "i"},
    {INS_SET, "i"},
    {INS_DECI, "i"},
    // Enemy property management and other miscellaneous things
    {INS_FLAGSET, "i"},
    {INS_SETCHAPTER, "i"}
};

ecli_result_t
get_ins_params(th10_instr_t* ins, ecl_value_t* values, unsigned int* num)
{
    for(unsigned int i = 0; i < sizeof(instruction_formats) / sizeof(ins_format_t); i++) {
        if(instruction_formats[i].id == ins->id) {
            if(num) {
                *num = strlen(instruction_formats[i].format);
            }
            return value_get_parameters(values, instruction_formats[i].format, &ins->data[0]);
        }
    }
    return ECLI_FAILURE;
}

void
print_th10_instruction_raw(th10_instr_t* ins)
{
    printf("time: %d, id: %d, size: %d, ", ins->time, ins->id, ins->size);
    printf("param_mask: %d, rank_mask: %x, param_count: %d\n",
           ins->param_mask, ins->rank_mask, ins->param_count);
}

void
print_th10_instruction(th10_instr_t* ins)
{
    uint8_t rank_mask = ins->rank_mask & 0x0F;
    static ecl_value_t params[32];

    if(rank_mask != last_mask) {
        putchar('!');
        if(rank_mask == 0x0F) {
            putchar('*');
        } else {
            if(rank_mask & 0x08) putchar('L');
            if(rank_mask & 0x04) putchar('H');
            if(rank_mask & 0x02) putchar('N');
            if(rank_mask & 0x01) putchar('E');
        }
        putchar('\n');
        last_mask = rank_mask;
    }

    int amt = 6;
    if(ins->time != 0) {
        amt -= printf("%d:", ins->time);
    }
    for(unsigned int i = 0; i < amt; i++) {
        putchar(' ');
    }
     
    get_ins_params(ins, params, NULL);

    switch(ins->id) {
        //system instructions
        case INS_RET: {
            printf("return");
        }   break;
        case INS_CALL: {
            char* name = params[0].s;
            printf("call(\"%s\")", name);
        }   break;
        case INS_JMP: {
            printf("goto %d @ %u", params[0].i, params[1].u);
        }   break;
        case INS_JMPEQ: {
            printf("jmpEq(%d, %u)", params[0].i, params[1].u);
        }   break;
        case INS_WAIT: {
            printf("wait(%d)", params[0].u);
        }   break;
        case INS_UNKNOWN30: {
            printf("unknown30(\"%s\")", params[0].s);
        }   break;
        case INS_STACKALLOC: {
            uint32_t amt = params[0].u >> 2;
            if(amt > 0) {
                printf("var A");
                for(unsigned int i = 1; i < amt; i++) {
                    printf(", %c", 'A'+i);
                }
            } else {
                printf("stackAlloc(%d)", amt);
            }
        }   break;
        case INS_PUSH: {
            int32_t value = params[0].i;
            printf("push(");
            if(ins->param_mask & 1) {
                printf("$%c", 'A' + (value >> 2));
            } else {
                printf("%d", value);
            }
            putchar(')');
        }   break;
        case INS_SET: {
            int32_t value = params[0].i;
            printf("set(");
            if(ins->param_mask & 1) {
                printf("$%c", 'A' + (value >> 2));
            } else {
                printf("%d", value);
            }
            putchar(')');
            break;
        }
        case INS_DECI: {
            uint32_t var = params[0].u >> 2;
            printf("deci($%c)", 'A' + var);
        }   break;
        // Enemy property management and other miscellaneous things
        case INS_FLAGSET:
            printf("flagSet(%d)", params[0].i);
            break;
        case INS_SETCHAPTER:
            printf("setChapter(%d)", params[0].i);
            break;
            
        default:
            printf("ins_%d()", ins->id);
            break;
    }

    printf(";\n");
}
