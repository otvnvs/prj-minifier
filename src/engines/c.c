#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../minifier.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define EMSCRIPTEN_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define EMSCRIPTEN_EXPORT
#endif

static int is_c_symbol(char c) {
    return (c == '=' || c == '+' || c == '-' || c == '*' || c == '/' || 
            c == '%' || c == '(' || c == ')' || c == '{' || c == '}' || 
            c == '[' || c == ']' || c == ',' || c == ';' || c == ':' || 
            c == '?' || c == '!' || c == '&' || c == '|' || c == '<' || c == '>');
}

static int is_alphanumeric(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
            (c >= '0' && c <= '9') || c == '_' || c == '$');
}

// Robust helper to track if a quote symbol is actually escaped,
// taking consecutive backslashes into account (e.g., '\\' means the quote is NOT escaped)
static int is_char_escaped(const char *source, size_t i) {
    if (i == 0) return 0;
    int count = 0;
    size_t back = i - 1;
    while (back > 0 && source[back] == '\\') {
        count++;
        back--;
    }
    if (back == 0 && source[back] == '\\') {
        count++;
    }
    return (count % 2 != 0);
}

EMSCRIPTEN_EXPORT char *remove_comments(const char *source) {
    size_t len = strlen(source);
    char *result = malloc(len + 1);
    if (!result) return NULL;
    
    size_t j = 0;
    int in_string = 0;
    int in_char = 0;
    
    for (size_t i = 0; i < len; i++) {
        // FIX 1: Use the robust backslash tracker helper
        int is_escaped = is_char_escaped(source, i);
        
        if (source[i] == '"' && !is_escaped && !in_char) {
            in_string = !in_string;
        } else if (source[i] == '\'' && !is_escaped && !in_string) {
            in_char = !in_char;
        }
        
        if (!in_string && !in_char) {
            if (source[i] == '/' && source[i + 1] == '/') {
                while (i < len && source[i] != '\n') {
                    i++;
                }
                if (i < len) {
                    result[j++] = source[i];
                }
                continue;
            }
            if (source[i] == '/' && source[i + 1] == '*') {
                i += 2;
                while (i < len - 1 && !(source[i] == '*' && source[i + 1] == '/')) {
                    i++;
                }
                if (i < len) i++; 
                continue;
            }
        }
        result[j++] = source[i];
    }
    result[j] = '\0';
    return result;
}

EMSCRIPTEN_EXPORT char *minify_c_code(const char *source) {
    size_t len = strlen(source);
    char *result = malloc(len + 1);
    if (!result) return NULL;

    size_t j = 0;
    int in_string = 0;
    int in_char = 0;

    for (size_t i = 0; i < len; i++) {
        // FIX 2: Use the robust backslash tracker helper here too
        int is_escaped = is_char_escaped(source, i);

        int old_string = in_string;
        int old_char = in_char;

        if (source[i] == '"' && !is_escaped && !in_char) {
            in_string = !in_string;
        } else if (source[i] == '\'' && !is_escaped && !in_string) {
            in_char = !in_char;
        }

        if (old_string || old_char || source[i] == '"' || source[i] == '\'') {
            result[j++] = source[i];
            continue;
        }

        // Protect Preprocessor Directives (#define, #ifdef, etc.)
        if (source[i] == '#' && (j == 0 || result[j - 1] == '\n')) {
            while (i < len) {
                if (source[i] == '\n' || source[i] == '\r') {
                    size_t back = i;
                    while (back > 0 && (source[back - 1] == ' ' || source[back - 1] == '\t')) {
                        back--;
                    }
                    if (back > 0 && source[back - 1] == '\\') {
                        result[j++] = source[i++];
                        continue;
                    }
                    break; 
                }
                result[j++] = source[i++];
            }
            result[j++] = '\n'; 
            if (i >= len) break;
            continue;   
        }

        // High-Compression Spacing Engine Block
        if (source[i] == ' ' || source[i] == '\t' || source[i] == '\n' || source[i] == '\r') {
            size_t next = i + 1;
            while (next < len && (source[next] == ' ' || source[next] == '\t' || source[next] == '\n' || source[next] == '\r')) {
                next++;
            }
            
            if (next < len && source[next] == '#') {
                if (j > 0 && result[j - 1] != '\n') {
                    result[j++] = '\n';
                }
                i = next - 1;
                continue;
            }
            
            if (j == 0 || result[j - 1] == '\n' || next >= len || source[next] == '\n' || source[next] == '\r' || source[next] == '#') {
                i = next - 1;
                continue;
            }
            if (is_c_symbol(result[j - 1]) || is_c_symbol(source[next])) {
                i = next - 1;
                continue;
            }
            if (is_alphanumeric(result[j - 1]) && is_alphanumeric(source[next])) {
                result[j++] = ' ';
            }
            i = next - 1;
            continue;
        }

        result[j++] = source[i];
    }
    result[j] = '\0';
    return result;
}

