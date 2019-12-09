#include <stdio.h>
#include <stdlib.h>

#include "ecli.h"

int
main(int argc, char** argv)
{
    char* ver;
    char* fname;
    
    if(argc <= 2) {
        exit(EXIT_SUCCESS);
    }
    
    ver = argv[1];
    fname = argv[2];
    
    FILE* f = fopen(fname, "rb");
    if(f == NULL) {
        fprintf(stderr, "Failed to open file %s\n", fname);
        exit(EXIT_FAILURE);
    }
    
    th10_header_t header;
    ecli_result_t result;
    
    result = read_th10_ecl_header(&header, f);
    if(result != ECLI_SUCCESS) {
        fprintf(stderr, "Failed to read in ECL header from %s\n", fname);
        return EXIT_FAILURE;
    }
    
    if(!verify_th10_ecl_header(&header)) {
        char magic[5];
        memcpy(&magic[0], &header.magic[0], 4*sizeof(char));
        magic[4] = '\0';
        
        fprintf(stderr, "Invalid magic number: `%s'.\n", &magic[0]);
        return EXIT_FAILURE;
    }
    
    print_th10_ecl_header(&header);

    load_th10_includes(&header, f);
    fclose(f);
    return EXIT_SUCCESS;
}
