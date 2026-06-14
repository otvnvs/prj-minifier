// src/minifier.h
#ifndef MINIFIER_H
#define MINIFIER_H

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define WASM_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define WASM_EXPORT
#endif

// Supported language definitions
typedef enum {
    LANG_C,
    LANG_MAKEFILE,
    LANG_JS
} LangType;

// Utilities
char* file_read(const char* filepath);
int is_c_keyword(const char* str);

// Language Engines
char* remove_comments(const char* buffer);
char* minify_c_code(const char* code);
char* minify_makefile_code(char* code);
char* minify_js_code(const char* code);
const char *get_build_number(void);
#endif

