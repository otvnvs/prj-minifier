// src/engines/c.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../minifier.h"

char* remove_comments(char* buffer) {
    char* new_buffer = (char*)malloc(strlen(buffer) + 1);
    if (new_buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(1);
    }

    int i = 0, j = 0;
    while (buffer[i] != '\0') {
        if (buffer[i] == '/' && buffer[i + 1] == '/') {
            while (buffer[i] != '\n' && buffer[i] != '\0') i++;
        }
        else if (buffer[i] == '/' && buffer[i + 1] == '*') {
            i += 2;
            while (buffer[i] != '\0' && (buffer[i] != '*' || buffer[i + 1] != '/')) i++;
            if (buffer[i] != '\0') i += 2;
        }
        else {
            new_buffer[j++] = buffer[i++];
        }
    }
    new_buffer[j] = '\0';
    return new_buffer;
}

char* minify_c_code(char* code) {
    char* new_code = (char*)malloc(strlen(code) + 1);
    if (new_code == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(1);
    }

    int i = 0, j = 0;
    while (code[i] != '\0') {
        if (isalpha((unsigned char)code[i]) || code[i] == '_') {
            int start = i;
            while (isalnum((unsigned char)code[i]) || code[i] == '_') i++;
            int len = i - start;
            
            char* temp = (char*)malloc(len + 1);
            memcpy(temp, &code[start], len);
            temp[len] = '\0';

            memcpy(&new_code[j], temp, len);
            j += len;

            if (is_c_keyword(temp)) {
                while (isspace((unsigned char)code[i])) i++;
                if (code[i] != '(') new_code[j++] = ' ';
            } else {
                while (isspace((unsigned char)code[i])) i++;
                if (isalnum((unsigned char)code[i]) || code[i] == '_') new_code[j++] = ' ';
            }
            free(temp);
        }
        else if (code[i] == '#') {
            while (code[i] != '\n' && code[i] != '\0') new_code[j++] = code[i++];
            if (code[i] == '\n') new_code[j++] = code[i++];
        }
        else if (isspace((unsigned char)code[i])) {
            i++;
        }
        else if (code[i] == ';' || code[i] == ',' || code[i] == '{' || code[i] == '}' || code[i] == '(' || code[i] == ')') {
            new_code[j++] = code[i++];
            while (isspace((unsigned char)code[i])) i++;
        }
        else if (code[i] == '\'') {
            i++; 
            int num = -1;
            if (code[i] != '\\') {
                num = code[i++];
                if (code[i] == '\'') i++;
            } else {
                i++;
                switch(code[i]) {
                    case 'a':  num = '\a'; break;
                    case 'b':  num = '\b'; break;
                    case 'f':  num = '\f'; break;
                    case 'n':  num = '\n'; break;
                    case 'r':  num = '\r'; break;
                    case 't':  num = '\t'; break;
                    case 'v':  num = '\v'; break;
                    case '\\': num = '\\'; break;
                    case '\'': num = '\''; break;
                    case '"':  num = '\"'; break;
                    case '?':  num = '\?'; break;
                    default:   num = code[i]; break; 
                }
                i++;
                if (code[i] == '\'') i++;
            }
            j += sprintf(new_code + j, "%d", num);
        }
        else if (code[i] == '"') {
            new_code[j++] = code[i++];
            while (code[i] != '"' && code[i] != '\0') {
                if (code[i] == '\\' && code[i+1] != '\0') new_code[j++] = code[i++];
                new_code[j++] = code[i++];
            }
            if (code[i] == '"') new_code[j++] = code[i++];
        }
        else {
            new_code[j++] = code[i++];
        }
    }
    new_code[j] = '\0';
    return new_code;
}

