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

void
print_th10_instruction_raw(th10_instr_t* ins)
{
    printf("time: %d, id: %d, size: %d, ", ins->time, ins->id, ins->size);
    printf("param_mask: %d, rank_mask: %x, param_count: %d\n",
           ins->param_mask, ins->rank_mask, ins->param_count);
}

static uint8_t last_mask = 0x0F;

void
print_th10_instruction(th10_instr_t* ins)
{
    uint8_t rank_mask = ins->rank_mask & 0x0F;
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

    switch(ins->id) {
        case 10: {
            printf("return");
        }   break;
        case 11: {
            char* name = &ins->data[4];
            printf("call(\"%s\")", name);
        }   break;
        case 30: {
            char* s = &ins->data[4];
            printf("unknown30(\"%s\")", s);
        }   break;
        case 40: {
            uint32_t amt = (*(uint32_t*)&ins->data[0]) >> 2;
            if(amt > 0) {
                printf("var A");
                for(unsigned int i = 1; i < amt; i++) {
                    printf(", %c", 'A'+i);
                }
            } else {
                printf("stackAlloc(%d)", amt);
            }
        }   break;
        case 42: {
            uint32_t value = *(uint32_t*)&ins->data[0];
            printf("push(%d)", value);
        }   break;
        case 43: {
            uint32_t value = *(uint32_t*)&ins->data[0];
            printf("set(%d)", value);
            break;
        }
        default:
            printf("ins_%d()", ins->id);
            break;
    }

    printf(";\n");
}
