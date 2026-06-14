// src/engines/make.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../minifier.h"

static int is_pure_echo_line(const char* str) {
    int i = 0;
    if (str[i] == '@') i++;
    if (strncmp(&str[i], "echo", 4) == 0) {
        if (str[i+4] == ' ' || str[i+4] == '\t' || str[i+4] == '"' || str[i+4] == '\'') {
            return 1;
        }
    }
    return 0;
}

static int ends_with_backslash(const char* str) {
    int i = 0;
    int last_non_space = -1;
    while (str[i] != '\n' && str[i] != '\r' && str[i] != '\0') {
        if (str[i] != ' ' && str[i] != '\t') {
            last_non_space = i;
        }
        i++;
    }
    if (last_non_space >= 0 && str[last_non_space] == '\\') {
        return 1;
    }
    return 0;
}

char* minify_makefile_code(char* code) {
    char* new_code = (char*)malloc(strlen(code) + 1);
    if (new_code == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(1);
    }

    int i = 0, j = 0;
    int line_start = 1;
    int paren_depth = 0;
    int brace_depth = 0;

    while (code[i] != '\0') {
        if (line_start) {
            while (code[i] == '\n' || code[i] == '\r') {
                i++;
            }
            if (code[i] == '\0') break;
            line_start = 0;

            int offset = (code[i] == '\t') ? 1 : 0;
            if (is_pure_echo_line(&code[i + offset])) {
                if (!ends_with_backslash(&code[i + offset])) {
                    while (code[i] != '\n' && code[i] != '\0') {
                        i++;
                    }
                    line_start = 1;
                    continue;
                }
            }
            
            if (code[i] == '@') {
                i++;
            }
            else if (code[i] == '\t' && code[i+1] == '@') {
                new_code[j++] = code[i++];
                i++;
            }
        }

        if (code[i] == '$') {
            new_code[j++] = code[i++];
            if (code[i] == '(') { paren_depth++; new_code[j++] = code[i++]; }
            else if (code[i] == '{') { brace_depth++; new_code[j++] = code[i++]; }
            continue;
        }
        if (code[i] == ')' && paren_depth > 0) { paren_depth--; }
        if (code[i] == '}' && brace_depth > 0) { brace_depth--; }

        if (code[i] == '#') {
            int escaped = (i > 0 && code[i-1] == '\\');
            if (!escaped && paren_depth == 0 && brace_depth == 0) {
                while (code[i] != '\n' && code[i] != '\0') i++;
                continue;
            }
        }

        // --- Target Colon and Assignment Whitespace Compression ---
        if (paren_depth == 0 && brace_depth == 0) {
            int op_len = 0;
            
            // Match operators: :=, +=, ?=, or standard target/static pattern colons ':'
            if (strncmp(&code[i], ":=", 2) == 0 || strncmp(&code[i], "+=", 2) == 0 || strncmp(&code[i], "?=", 2) == 0) {
                op_len = 2;
            } else if (code[i] == '=' || code[i] == ':') {
                op_len = 1;
            }

            if (op_len > 0) {
                // 1. Strip padding spaces immediately preceding the token
                while (j > 0 && (new_code[j-1] == ' ' || new_code[j-1] == '\t')) {
                    j--;
                }
                // 2. Transmit the character token
                for (int k = 0; k < op_len; k++) {
                    new_code[j++] = code[i++];
                }
                // 3. Skip remaining whitespace right after the token
                while (code[i] == ' ' || code[i] == '\t') {
                    i++;
                }
                continue;
            }
        }

        if (code[i] == '\n') {
            while (j > 0 && (new_code[j-1] == ' ' || new_code[j-1] == '\t')) {
                j--;
            }
            if (j > 0 && new_code[j-1] != '\n') {
                new_code[j++] = '\n';
            }
            i++;
            line_start = 1;
            paren_depth = 0;
            brace_depth = 0;
            continue;
        }

        new_code[j++] = code[i++];
    }

    while (j > 0 && (new_code[j-1] == ' ' || new_code[j-1] == '\t' || new_code[j-1] == '\n')) {
        j--;
    }
    if (j > 0) {
        new_code[j++] = '\n';
    }
    new_code[j] = '\0';

    return new_code;
}

