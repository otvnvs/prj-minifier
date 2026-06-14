// Import the Emscripten-generated WebAssembly wrapper module
import createModule from './sqminify.js';

let wasmInstance = null;

/**
 * Internal helper to guarantee the WebAssembly instance is loaded 
 * and instantiated asynchronously exactly once.
 */
async function getModuleInstance() {
    if (!wasmInstance) {
        wasmInstance = await createModule();
    }
    return wasmInstance;
}

/**
 * Minifies C Source Code (Removes comments and shrinks extra spaces/newlines)
 * @param {string} sourceCode - Raw unminified C code
 * @returns {Promise<string>} Minified C output code string
 */
export async function minifyC(sourceCode) {
    const Module = await getModuleInstance();
    
    // cwrap binds the underlying C function pointer signatures
    const removeComments = Module.cwrap('remove_comments', 'number', ['string']);
    const minifyCCode    = Module.cwrap('minify_c_code', 'number', ['string']);

    // 1. Strip Comments
    const commentsRemovedPtr = removeComments(sourceCode);
    const cleanSource = Module.UTF8ToString(commentsRemovedPtr);

    // 2. Perform Whitespace Compression pass
    const minifiedPtr = minifyCCode(cleanSource);
    const result = Module.UTF8ToString(minifiedPtr);

    // 3. Clean up heap allocations to prevent browser-side memory leaks
    Module._free(commentsRemovedPtr);
    Module._free(minifiedPtr);

    return result;
}

/**
 * Removes comments from C Source Code without compacting whitespaces
 * @param {string} sourceCode - Raw C code
 * @returns {Promise<string>} Code string with comments stripped out
 */
export async function removeComments(sourceCode) {
    const Module = await getModuleInstance();
    const removeCommentsSymbol = Module.cwrap('remove_comments', 'number', ['string']);

    const heapPointer = removeCommentsSymbol(sourceCode);
    const result = Module.UTF8ToString(heapPointer);
    
    Module._free(heapPointer);
    return result;
}

/**
 * Minifies Makefile Code (Safely strips comments while preserving structural tabs)
 * @param {string} sourceCode - Raw Makefile text content
 * @returns {Promise<string>} Compressed Makefile output
 */
export async function minifyMakefile(sourceCode) {
    const Module = await getModuleInstance();
    const minifyMakefileSymbol = Module.cwrap('minify_makefile_code', 'number', ['string']);

    const heapPointer = minifyMakefileSymbol(sourceCode);
    const result = Module.UTF8ToString(heapPointer);
    
    Module._free(heapPointer);
    return result;
}

/**
 * Minifies JavaScript Code (Protects regex, templates, single/double strings)
 * @param {string} sourceCode - Raw unminified JavaScript code
 * @returns {Promise<string>} Condensed production-ready JavaScript code string
 */
export async function minifyJS(sourceCode) {
    const Module = await getModuleInstance();
    const minifyJSSymbol = Module.cwrap('minify_js_code', 'number', ['string']);

    const heapPointer = minifyJSSymbol(sourceCode);
    const result = Module.UTF8ToString(heapPointer);
    
    Module._free(heapPointer);
    return result;
}

/**
 * Retrieves the compile-time build version universally shared across all target builds.
 * @returns {Promise<string>} The active compiler build number string
 */
export async function getBuildNumber() {
    const Module = await getModuleInstance();
    const getBuildSymbol = Module.cwrap('get_build_number', 'number', []);
    
    const heapPointer = getBuildSymbol();
    return Module.UTF8ToString(heapPointer);
}
