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
#include <stdlib.h>

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

/**
 * Read the include lists after an ECL header
 **/
ecli_result_t
load_th10_includes(th10_header_t* header, FILE* f)
{
    if(0 != fseek(f, header->include_offset, SEEK_SET)) {
        return ECLI_FAILURE;
    }
    
    // Buffer to hold the lists temporarily
    size_t include_size = header->include_length - header->include_offset;
    unsigned char* buf = malloc(include_size);
    if(buf == NULL) {
        return ECLI_FAILURE;
    }
    
    // Load the lists into memory in their binary format
    size_t amt = fread(buf, include_size, 1, f);
    if(amt != 1) {
        free(buf);
        return ECLI_FAILURE;
    }
    
    // Convert the first list to a list of char*s
    th10_include_list_t* list = (th10_include_list_t*)buf;
    unsigned int num_strings = 0;
    char* basep = &list->data[0];
    char* p = basep;
    
    char** strings = malloc(list->count * sizeof(char*));
    
    if(strings == NULL) {
        free(buf);
        return ECLI_FAILURE;
    }
    
    memset(strings, 0, list->count * sizeof(char*));
    
    while(num_strings < list->count) {
        while(*p) { p++; }
        p++; // to include NULL character
        char* str = malloc((size_t)(p - basep));
        if(str == NULL) {
            free(buf);
            free(strings);
            return ECLI_FAILURE;
        }
        
        memcpy(str, basep, (size_t)(p - basep));
        strings[num_strings] = str;
        num_strings++;
        basep = p;
    }
    
    char name[5];
    memcpy(&name[0], &list->name[0], 4*sizeof(char));
    name[4] = '\0';
    printf("Include type: %s\n", name);
    if(num_strings > 0) {
        printf("Strings: `%s'", strings[0]);
        for(unsigned int i = 1; i < num_strings; i++) {
            printf(", `%s'", strings[i]);
        }
        printf("\n");
    }
    
    for(unsigned int i = 0; i < num_strings; i++)
    {
        free(strings[i]);
    }
    free(strings);
    free(buf);
    return ECLI_SUCCESS;
}
