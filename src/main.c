#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ecli.h"

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

static int show_header, show_includes;

param_t params[] = {
    {'h', "help", NULL, 0, "Print this message."},
    {'d', "difficulty", NULL, 1, "Set the difficulty (easy, normal, hard, lunatic)"},
    {'H', "dump-header", &show_header, 0, "Dump the ECL header."},
    {'I', "dump-includes", &show_includes, 0, "Dump the ECL ANIM/ECLI includes."},
    {'v', "verbose", &global.verbose, 0, "Print a lot of useful debug information."},
    {0, NULL, NULL, 0, NULL}
};

const char* desc = "ECL Interpreter for the newest Touhou games";
const char* pos = "eclfile";
const char* longdesc = NULL;

int
main(int argc, char** argv)
{
    srand(time(0));

    /* Parse command-line arguments */
    args_set(argc, argv);
    const char* fname = NULL;
    int c;
    
    global.difficulty = DIFF_LUNATIC;

    while((c = arg_get(params)) != 0) {
        fflush(stdout);
        switch(c) {
            case -1:
                arg_print_usage(desc, pos, params, longdesc);
                return EXIT_FAILURE;
                break;
            // difficulty setting
            case 'd': {
                const char* arg = arg_get_param();
                if(strcmp(arg, "easy") == 0) {
                    global.difficulty = DIFF_EASY;
                } else if(strcmp(arg, "normal") == 0) {
                    global.difficulty = DIFF_NORMAL;
                } else if(strcmp(arg, "hard") == 0) {
                    global.difficulty = DIFF_HARD;
                } else if(strcmp(arg, "lunatic") == 0) {
                    global.difficulty = DIFF_LUNATIC;
                } else {
                    fprintf(stderr, "Unknown difficulty: %s\n\n", arg);
                    arg_print_usage(desc, pos, params, longdesc);
                }
            }   break;

            case 'h':
                arg_print_usage(desc, pos, params, longdesc);
                return EXIT_SUCCESS;
                break;

            case 1: // ECL file (positional arg)
                if(fname != NULL) {
                    fprintf(stderr, "Multiple files given on command line.\n");
                    return EXIT_FAILURE;
                }
                
                fname = arg_get_param();
                break;

            default:
                break;
        }
    }
    
    if(fname == NULL) {
        fprintf(stderr, "No ECL file given.\n");
        return EXIT_FAILURE;
    }
    
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
    if(show_header) {
        print_th10_ecl_header(&ecl);
    }
    
    if(show_includes) {
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
    if(!SUCCESS(initialize_globals())) {
        fprintf(stderr, "Failed to initialize global variables.\n");
        free_th10_ecl(&ecl);
        return EXIT_FAILURE;
    }

    ecl_state_t* main;
    result = allocate_ecl_state(&main, &ecl);
    if(result != ECLI_SUCCESS) {
        fprintf(stderr, "Failed to initialize interpreter state.\n");
        free_th10_ecl(&ecl);
        return EXIT_FAILURE;
    }
    
    /* Find main sub and execute */
    th10_ecl_sub_t* sub = get_th10_ecl_sub_by_name(&ecl, "main");
    if(sub == NULL) {
        fprintf(stderr, "ECL file has no main sub.\n");
        free_ecl_state(main);
        free_th10_ecl(&ecl);
        return EXIT_FAILURE;
    }
    
    /* Current interpeter loop - get next instruction and execute */
    main->ip = sub->start;
    
    while(1) {
        result = run_all_ecl_instances(main);
        if(result == ECLI_DONE) {
            break;
        } else if(result == ECLI_FAILURE) {
            fprintf(stderr, "Interpretation failed.\n");
            break;
        }
        
        for(ecl_state_t* p = main; p != NULL; p = p->next) {
            p->wait = max(p->wait - 1, 0);
            if(p->wait == 0) {
                p->time++;
            }
        }
    }

    free_ecl_state(main);
    free_th10_ecl(&ecl);

    return EXIT_SUCCESS;
}
