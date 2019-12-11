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
 * Load an entire ECL file into memory and set up pointers
 **/
ecli_result_t
load_th10_ecl_from_file_object(th10_ecl_t* ecl, FILE* f)
{
    memset(ecl, 0, sizeof(th10_ecl_t));

    if(0 != fseek(f, 0, SEEK_END)) {
        return ECLI_FAILURE;
    }
    
    long size = ftell(f);
    
    if(0 != fseek(f, 0, SEEK_SET)) {
        return ECLI_FAILURE;
    }
    
    ecl->header = (th10_header_t*)xmalloc(size);
    
    size_t amt = fread(ecl->header, size, 1, f);
    if(amt != 1) {
        xfree(ecl->header);
        return ECLI_FAILURE;
    }
    
    // Make sure the magic number is present
    if(!verify_th10_ecl_header(ecl)) {
        uint8_t magic[5];
        memcpy(&magic[0], &ecl->header->magic[0], 4*sizeof(uint8_t));
        magic[4] = '\0';
        fprintf(stderr, "Invalid magic number: %s\n", magic);
        free_th10_ecl(ecl);
        return ECLI_FAILURE;
    }
    
    // ECL includes
    uint8_t* include_base = (uint8_t*)ecl->header + ecl->header->include_offset;
    uint8_t* include_end = include_base + ecl->header->include_length;
    uint8_t* p = include_base;

    // Get pointers to the start of the includes
    while(p < include_end) {
        th10_include_list_t* list = (th10_include_list_t*)p;
        uint32_t name = *(uint32_t*)&list->name[0];
        if(name == *(uint32_t*)"ANIM") {
            ecl->anims = list;
        } else if(name == *(uint32_t*)"ECLI") {
            ecl->eclis = list;
        } else {
            fprintf(stderr, "Unknown include type: %s\n", list->name);
            free_th10_ecl(ecl);
            return ECLI_FAILURE;
        }
        
        p = &list->data[0];
        for(unsigned int i = 0; i < list->count; i++) {
            while(*p) { p++; }
            p++;
        }
        p = p + (((uintptr_t)p) & 0x03);
    }
    
    // Here p points to subs
    uint32_t* sub_offsets = (uint32_t*)p;
    uint8_t* names = (uint8_t*)(sub_offsets + ecl->header->sub_count);
    
    ecl->subs = xmalloc(sizeof(th10_ecl_sub_t) * ecl->header->sub_count);
    
    for(unsigned int i = 0; i < ecl->header->sub_count; i++) {
        th10_sub_t* sub = (th10_sub_t*)(((uint8_t*)ecl->header) + sub_offsets[i]);
        if(*(uint32_t*)&sub->magic[0] != *(uint32_t*)"ECLH") { 
            fprintf(stderr, "Invalid sub start.\n");
            free_th10_ecl(ecl);
            return ECLI_FAILURE;
        }
        
        ecl->subs[i].name = (char*)names;
        ecl->subs[i].sub = sub;
        ecl->subs[i].start = (th10_instr_t*)&sub->data[0];
        
        while(*names) { names++; }
        names++;
    }
    
    return ECLI_SUCCESS;
}

/**
 * Free a loaded ECL file
 **/
void
free_th10_ecl(th10_ecl_t* ecl)
{
    xfree(ecl->header);
    xfree(ecl->subs);
    memset(ecl, 0, sizeof(th10_ecl_t));
}

/**
 * Verify the magic number in a MoF-and-later ECL header
 **/
int
verify_th10_ecl_header(th10_ecl_t* ecl)
{
    return (int)((*(uint32_t*)&ecl->header->magic) == (*(uint32_t*)"SCPT"));
}

/**
 * Dump an ECL header to STDOUT
 **/
void
print_th10_ecl_header(th10_ecl_t* ecl)
{
    char magic[5];
    memcpy(&magic[0], &ecl->header->magic[0], 4*sizeof(char));
    magic[4] = '\0';
    printf("magic: %s\nunknown1: %x\ninclude_length: %d\n", 
           &magic[0], ecl->header->unknown1, ecl->header->include_length);
    printf("include_offset: %d\nzero1: %x\nsub_count: %d\n",
           ecl->header->include_offset, ecl->header->zero1, 
           ecl->header->sub_count);
    for(unsigned int i = 0; i < 4; i++) {
        printf("zero2 %d: %x\n", i, ecl->header->zero2[i]);
    }
}

/**
 * Get an include list by type
 **/
th10_include_list_t*
th10_ecl_get_include_list(th10_ecl_t* ecl, include_t include)
{
    th10_include_list_t* list;
    switch(include) {
        case INCLUDE_ANIM:
            list = ecl->anims;
            break;
        case INCLUDE_ECLI:
            list = ecl->eclis;
            break;
    }
    return list;
}

/**
 * Get a pointer to an include name in an include list
 **/
char*
th10_ecl_get_include(th10_include_list_t* list, unsigned int idx)
{
    if(idx > list->count) {
        return NULL;
    }
    
    uint8_t* p = (uint8_t*)&list->data[0];
    unsigned int i = 0;
    while((i != idx) && (i < list->count)) {
        while(*p) { p++; }
        p++;
        i++;
    }
    
    return p;
}
