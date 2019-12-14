/**
 * ECL value handling for instruction parameters
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

ecli_result_t
value_get_parameters(ecl_value_t* values, const char* format, uint8_t* data)
{
    size_t len = strlen(format);
    
    for(unsigned int i = 0; i < len; i++) {
        switch(format[i]) {
            case 'f':
                values[i].type = ECL_FLOAT32;
                values[i].f = *(float*)data;
                data += 4;
                break;

            case 'i':
                values[i].type = ECL_INT32;
                values[i].i = *(int32_t*)data;
                data += 4;
                break;
            
            case 's':
                values[i].type = ECL_STRING;
                uint32_t len = *(uint32_t*)data;
                values[i].s = (char*)(data+4);
                data = data + 4 + len;
                break;
            
            case 'u':
                values[i].type = ECL_UINT32;
                values[i].u = *(uint32_t*)data;
                data += 4;
                break;
                
            default:
                fprintf(stderr, "Unrecognized format char: %c\n", format[i]);
                return ECLI_FAILURE;
                break;
        }
    }
    
    return ECLI_SUCCESS;
}
