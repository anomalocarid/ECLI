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
    const char* opcode;
} ins_format_t;

static const ins_format_t instruction_formats[] = {
    //system instructions
    {INS_NOP, "", "nop"},
    {INS_DELETE, "", "delete"},
    {INS_RET, "", "return"},
    {INS_CALL, "s", "call"},
    {INS_JMP, "iu", "jmp"},
    {INS_JMPEQ, "iu", "jmpEq"},
    {INS_JMPNEQ, "iu", "jmpNeq"},
    {INS_CALLASYNC, "s", "callAsync"},
    {INS_UNKNOWN21, "", "unknown21"},
    {INS_WAIT, "i", "wait"},
    {INS_UNKNOWN30, "s", "unknown30"},
    {INS_STACKALLOC, "u", "stackAlloc"},
    {INS_PUSH, "i", "push"},
    {INS_SET, "i", "set"},
    {INS_DECI, "i", "deci"},
    // Enemy property management and other miscellaneous things
    {INS_FLAGSET, "i", "flagSet"},
    {INS_SETCHAPTER, "i", "setChapter"}
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

static void
print_param(ecl_value_t* params, unsigned int i, uint16_t mask)
{
    if(mask & (1 << i)) {
        if(params[i].type == ECL_FLOAT32) {
            putchar('%');
        } else if(params[i].type == ECL_INT32) {
            putchar('$');
        }
        //TODO: handle global/local variable references
        printf("%c", 'A' + params[i].i);
    } else {
        value_print(&params[i]);
    }
}

void
print_th10_instruction(th10_instr_t* ins)
{
    uint8_t rank_mask = ins->rank_mask & 0x0F;
    static ecl_value_t params[32];

    // Display difficulty options
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

    // Time label
    int amt = 6;
    if(ins->time != 0) {
        amt -= printf("%d:", ins->time);
    }
    for(unsigned int i = 0; i < amt; i++) {
        putchar(' ');
    }
    
    for(unsigned int i = 0; i < sizeof(instruction_formats) / sizeof(ins_format_t); i++) {
        if(instruction_formats[i].id == ins->id) {
            printf("%s(", instruction_formats[i].opcode);
            
            size_t num = strlen(instruction_formats[i].format);
            ecli_result_t result = value_get_parameters(params, instruction_formats[i].format, &ins->data[0]);
            // display parameter list
            if(SUCCESS(result) && (num > 0)) {
                print_param(params, 0, ins->param_mask);
                for(unsigned int j = 1; j < num; j++) {
                    printf(", ");
                    print_param(params, j, ins->param_mask);
                }
            }
            
            printf(");\n");
            return;
        }
    }
    
    printf("ins_%d;\n", ins->id);
}
