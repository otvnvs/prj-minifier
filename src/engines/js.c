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

// Helper to determine if a character is a JS operator or delimiter symbol
static int is_js_symbol(char c) {
    return (c == '=' || c == '+' || c == '-' || c == '*' || c == '/' || 
            c == '%' || c == '(' || c == ')' || c == '{' || c == '}' || 
            c == '[' || c == ']' || c == ',' || c == ';' || c == ':' || 
            c == '?' || c == '!' || c == '&' || c == '|' || c == '<' || c == '>' || c == '.');
}

// Helper to check if a character is a valid alphanumeric identifier token
static int is_alphanumeric(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
            (c >= '0' && c <= '9') || c == '_' || c == '$');
}

EMSCRIPTEN_EXPORT char *minify_js_code(const char *source) {
    size_t len = strlen(source);
    
    char *result = malloc(len + 1);
    if (!result) return NULL;

    size_t j = 0;
    size_t i = 0;

    int in_dbl_quote = 0;
    int in_sgl_quote = 0;
    int in_template  = 0;

    while (i < len) {
        int is_escaped = (i > 0 && source[i - 1] == '\\');

        // --- LITERAL STRINGS BOUNDARY TRACKING ---
        if (source[i] == '"' && !is_escaped && !in_sgl_quote && !in_template) {
            in_dbl_quote = !in_dbl_quote;
        } else if (source[i] == '\'' && !is_escaped && !in_dbl_quote && !in_template) {
            in_sgl_quote = !in_sgl_quote;
        } else if (source[i] == '`' && !is_escaped && !in_dbl_quote && !in_sgl_quote) {
            in_template = !in_template;
        }

        // Handle minification adjustments only when outside explicit literal strings
        if (!in_dbl_quote && !in_sgl_quote && !in_template) {
            
            // 1. Handle Block Comments: /* ... */
            if (i < len - 1 && source[i] == '/' && source[i + 1] == '*') {
                i += 2; 
                while (i < len - 1 && !(source[i] == '*' && source[i + 1] == '/')) {
                    i++;
                }
                if (i < len - 1) i += 2; 
                else i = len; 
                continue;
            }

            // 2. Handle Single-Line Comments: // ...
            if (i < len - 1 && source[i] == '/' && source[i + 1] == '/') {
                i += 2; 
                while (i < len && source[i] != '\n' && source[i] != '\r') {
                    i++;
                }
                continue; // FIX: Force immediate re-evaluation cycle on the newline token
            }

            // 3. Robust Multi-Line and Empty Line Collapsing Pass
            if (source[i] == '\n' || source[i] == '\r') {
                size_t peek = i;
                int newline_count = 0;

                while (peek < len) {
                    if (source[peek] == '\n') {
                        newline_count++;
                        peek++;
                    } else if (source[peek] == '\r' || source[peek] == ' ' || source[peek] == '\t') {
                        peek++;
                    } else if (peek < len - 1 && source[peek] == '/' && source[peek + 1] == '/') {
                        peek += 2;
                        while (peek < len && source[peek] != '\n' && source[peek] != '\r') peek++;
                    } else if (peek < len - 1 && source[peek] == '/' && source[peek + 1] == '*') {
                        peek += 2;
                        while (peek < len - 1 && !(source[peek] == '*' && source[peek + 1] == '/')) peek++;
                        if (peek < len - 1) peek += 2;
                        else peek = len;
                    } else {
                        break;
                    }
                }

                if (newline_count > 0) {
                    // If the previous written token was a semicolon or brace, 
                    // skip emitting the newline entirely to flatten the code out!
                    if (j > 0 && result[j - 1] != ';' && result[j - 1] != '{' && result[j - 1] != '}' && result[j - 1] != '\n') {
                        result[j++] = '\n';
                    }
                    i = peek; 
                    continue;
                }
            }

            // 4. Horizontal Inline Spacing Compression Pass
            if (source[i] == ' ' || source[i] == '\t') {
                size_t peek = i + 1;
                while (peek < len && (source[peek] == ' ' || source[peek] == '\t')) {
                    peek++;
                }

                if (j == 0 || result[j - 1] == '\n' || peek >= len || source[peek] == '\n' || source[peek] == '\r') {
                    i = peek;
                    continue;
                }

                if (is_js_symbol(result[j - 1]) || is_js_symbol(source[peek])) {
                    i = peek;
                    continue;
                }

                if (is_alphanumeric(result[j - 1]) && is_alphanumeric(source[peek])) {
                    result[j++] = ' ';
                }
                
                i = peek;
                continue;
            }
        }

        // Transfer valid code frames safely
        if (i < len) {
            result[j++] = source[i++];
        }
    }

    result[j] = '\0';
    return result;
}
