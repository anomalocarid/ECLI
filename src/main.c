#include <stdio.h>
#include <stdlib.h>

#include "ecli.h"

int
main(int argc, char** argv)
{
    int verbose = 0;

    if(argc <= 1) {
        exit(EXIT_SUCCESS);
    }
    
    char* fname = argv[1];
    
    FILE* f = fopen(fname, "rb");
    if(f == NULL) {
        fprintf(stderr, "Failed to open file %s\n", fname);
        return EXIT_FAILURE;
    }
    
    /* Read in ECL file */
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
    
    /* Dump some information about the file */
    if(verbose) {
        print_th10_ecl_header(&ecl);
        
        for(include_t i = INCLUDE_ANIM; i < INCLUDE_MAX; i++) {
            th10_include_list_t* list = th10_ecl_get_include_list(&ecl, i);
            printf("Include type: %s\n", &list->name[0]);
            if(list->count < 1) {
                continue;
            }
            
            char* s = th10_ecl_get_include(list, 0);
            printf("Include list: %s", s);
            
            for(unsigned int j = 1; j < list->count; j++) {
                s = th10_ecl_get_include(list, j);
                printf(", %s", s);
            }
            printf("\n");
        }
        
        printf("Subs: %s", ecl.subs[0].name);
        for(unsigned int i = 1; i < ecl.header->sub_count; i++) {
            printf(", %s", ecl.subs[i].name);
        }
        printf("\n");
    }
    
    /* Initialize interpreter */
    ecl_state_t state;
    result = initialize_ecl_state(&state, &ecl, 2048);
    if(result != ECLI_SUCCESS) {
        fprintf(stderr, "Failed to initialize interpreter state.\n");
        free_th10_ecl(&ecl);
        return EXIT_FAILURE;
    }
    
    /* Find main sub and execute */
    th10_ecl_sub_t* sub = get_th10_ecl_sub_by_name(&ecl, "main");
    if(sub == NULL) {
        fprintf(stderr, "ECL file has no main sub.\n");
        free_ecl_state(&state);
        free_th10_ecl(&ecl);
        return EXIT_FAILURE;
    }
    
    th10_instr_t* ins = sub->start;
    while(ins != NULL) {
        if(verbose) {
            print_th10_instruction(ins);
        }
        run_th10_instruction(&state, ins, &ins);
    }

    free_ecl_state(&state);
    free_th10_ecl(&ecl);

    return EXIT_SUCCESS;
}
