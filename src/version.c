#include <stdio.h>
#include "minifier.h"

// Macro stringification helpers
#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define EMSCRIPTEN_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define EMSCRIPTEN_EXPORT
#endif
/**
 * Returns the compile-time incremented build number as a string.
 * Works perfectly on Linux, Windows, and WebAssembly.
 */
const char *get_build_number(void) {
#ifdef BUILD_NUMBER
    return STRINGIFY(BUILD_NUMBER);
#else
    return "DEVELOPMENT_UNVERSIONED";
#endif
}

