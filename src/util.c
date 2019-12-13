/**
 * Utility functions for ECLI
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
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int argc;
static int cur;
static char** argv;
static char* param;

void*
xmalloc(size_t amt)
{
    void* p = malloc(amt);
    if(p == NULL) {
        fprintf(stderr, "allocation of %d bytes failed!\n", amt);
        exit(EXIT_FAILURE);
    }
    
    return p;
}

/**
 * Command-line argument parsing
 **/
void
args_set(int _argc, char** _argv)
{
    argc = _argc;
    argv = _argv;
    cur = 1;
}

int
arg_get(param_t* params)
{
    if(cur >= argc) {
        return 0;
    }
    
    char* arg = argv[cur++];
    char c = arg[0];
    
    int is_long = 0;
    if(c != '\0') {
        if(c == '-') { /* option */
            if(arg[1] == '-') { /* long option */
                is_long = 1;
            }
        } else { /* positional argument */
            param = arg;
            return 1;
        }
        
        // Look for the option
        char found = 0;
        for(param_t* p = params; p->shortname != '\0'; p++) {
            
            if(is_long) {
                if(strcmp(&arg[2], p->longname) == 0) {
                    found = p->shortname;
                }
            } else {
                if(arg[1] == p->shortname) {
                    found = p->shortname;
                }
            }
            
            // Look for the option's parameter, if it needs one
            if(found) {
                if(p->flag != NULL) {
                    *(p->flag) = 1;
                }

                if(p->has_arg) {
                    if((cur >= argc) || (argv[cur][0] == '-')) {
                        fprintf(stderr, "No argument given for %s\n", arg);
                    }
                    param = argv[cur++];
                }
                break;
            }
        }
        
        if(!found) {
            fprintf(stderr, "Unrecognized option: %s\n", arg);
            return -1;
        }
        
        return found;
    }

    return 0;
}

const char*
arg_get_param()
{
    return param;
}
