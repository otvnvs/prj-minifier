// src/utils.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minifier.h"

char* file_read(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: File not found\n");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(1);
    }

    size_t read_bytes = fread(buffer, 1, size, file);
    buffer[read_bytes] = '\0';

    fclose(file);
    return buffer;
}

int is_c_keyword(const char* str) {
    const char* keywords[] = {
        "auto", "break", "case", "char", "const", "continue", "default", "do",
        "double", "else", "enum", "extern", "float", "for", "goto", "if",
        "int", "long", "register", "return", "short", "signed", "sizeof", "static",
        "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while"
    };
    int num_keywords = sizeof(keywords) / sizeof(keywords[0]);
    for (int i = 0; i < num_keywords; i++) {
        if (strcmp(str, keywords[i]) == 0) return 1;
    }
    return 0;
}

