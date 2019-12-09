/**
 * Functions for reading in and parsing binary ECL files
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
#include <stdio.h>

#include "ecli.h"

/**
 * Load in a MoF-and-later ECL header from a FILE object
 **/
ecli_result_t
read_th10_ecl_header(th10_header_t* header, FILE* f)
{
    if(0 != fseek(f, 0, SEEK_SET)) {
        return ECLI_FAILURE;
    }
    
    size_t amt = fread(header, sizeof(th10_header_t), 1, f);
    if(amt != 1) {
        return ECLI_FAILURE;
    }

    return ECLI_SUCCESS;
}

/**
 * Verify the magic number in a MoF-and-later ECL header
 **/
int
verify_th10_ecl_header(th10_header_t* header)
{
    return (int)((*(uint32_t*)&header->magic) == (*(uint32_t*)"SCPT"));
}

/**
 * Dump an ECL header to STDOUT
 **/
void
print_th10_ecl_header(th10_header_t* header)
{
    char magic[5];
    memcpy(&magic[0], &header->magic[0], 4*sizeof(char));
    printf("magic: %s\nunknown1: %x\ninclude_length: %d\n", 
           &magic[0], header->unknown1, header->include_length);
    printf("include_offset: %d\nzero1: %x\nsub_count: %d\n",
           header->include_offset, header->zero1, 
           header->sub_count);
    for(unsigned int i = 0; i < 4; i++) {
        printf("zero2 %d: %x\n", i, header->zero2[i]);
    }
}
