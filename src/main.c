#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minifier.h"

void print_usage(const char* prog_name) {
    printf("Usage: %s [-l c|make] <input_file> [output_file]\n", prog_name);
    printf(" sqminify buidld %s\n", get_build_number());
}

int main(int argc, char** argv)
{
    LangType selected_lang = LANG_C; // Default language context
    char* input_file = NULL;
    char* output_file = NULL;
    int positional_idx = 0;

    // 1. Walk arguments to filter flags out from files
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--lang") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: Missing argument value for flag %s\n", argv[i]);
                print_usage(argv[0]);
                return 1;
            }
            i++; // Advance parser to extract option value
            if (strcmp(argv[i], "c") == 0) {
                selected_lang = LANG_C;
            } else if (strcmp(argv[i], "make") == 0) {
                selected_lang = LANG_MAKEFILE;
} else if (strcmp(argv[i], "js") == 0) {   // <-- Add this block
    selected_lang = LANG_JS;
            } else {
                fprintf(stderr, "Error: Unsupported source language validation key '%s'\n", argv[i]);
                return 1;
            }
        } else {
            // Positional handling allocations
            if (positional_idx == 0) {
                input_file = argv[i];
            } else if (positional_idx == 1) {
                output_file = argv[i];
            } else {
                fprintf(stderr, "Error: Extraneous parameter detected '%s'\n", argv[i]);
                print_usage(argv[0]);
                return 1;
            }
            positional_idx++;
        }
    }

    if (input_file == NULL) {
        print_usage(argv[0]);
        return 1;
    }

    // 2. Load the payload file contents
    char* orig_buffer = file_read(input_file);
    char* processed_buffer = NULL;

    // 3. Dispatch data passes via active language selector switches
    if (selected_lang == LANG_C) {
        char* no_comments = remove_comments(orig_buffer);
        processed_buffer = minify_c_code(no_comments);
        free(no_comments); // Keep memory leak-free
    } 
    else if (selected_lang == LANG_MAKEFILE) {
        // GNU Make has explicit commenting structures (#), so it calls its own router block directly
        processed_buffer = minify_makefile_code(orig_buffer);
} else if (selected_lang == LANG_JS) {     // <-- Add this block
    processed_buffer = minify_js_code(orig_buffer);
}

    // 4. Output processing pipelines
    if (output_file != NULL) {
        FILE* file = fopen(output_file, "w");
        if (file == NULL) {
            fprintf(stderr, "Error: Cannot create '%s' output target file!\n", output_file);
            free(orig_buffer);
            free(processed_buffer);
            return 1;
        }
        fprintf(file, "%s", processed_buffer);
        fclose(file);
    } else {
        printf("%s", processed_buffer);
    }

    free(orig_buffer);
    free(processed_buffer);
    return 0;
}

