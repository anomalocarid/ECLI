#include <stdio.h>
#include <stdlib.h>

#include "ecli.h"

int
main(int argc, char** argv)
{
    if(argc <= 2) {
        exit(EXIT_SUCCESS);
    }
    
    char* ver = argv[1];
    char* fname = argv[2];
    
    FILE* f = fopen(fname, "rb");
    if(f == NULL) {
        fprintf(stderr, "Failed to open file %s\n", fname);
        return EXIT_FAILURE;
    }
    
    th10_ecl_t ecl;
    ecli_result_t result = load_th10_ecl_from_file_object(&ecl, f);
    fclose(f);
    
    if(result != ECLI_SUCCESS) {
        fprintf(stderr, "Failed to load ECL file %s\n", fname);
        return EXIT_FAILURE;
    }
    
    if(!verify_th10_ecl_header(&ecl)) {
        fprintf(stderr, "Invalid magic number.\n");
        return EXIT_FAILURE;
    }
    
    print_th10_ecl_header(&ecl);
    
    for(include_t i = INCLUDE_ANIM; i < INCLUDE_MAX; i++) {
        th10_include_list_t* list = th10_ecl_get_include_list(&ecl, i);
        printf("Include type: %s\n", &list->name[0]);
        
        char* s = th10_ecl_get_include(list, 0);
        printf("Include list: %s", s);
        
        for(unsigned int j = 1; j < list->count; j++) {
            s = th10_ecl_get_include(list, j);
            printf(", %s", s);
        }
        printf("\n");
    }
    free_th10_ecl(&ecl);

    return EXIT_SUCCESS;
}
