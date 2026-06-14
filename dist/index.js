import { minifyC, removeComments, minifyMakefile, minifyJS, getBuildNumber } from './sqminify/index.js';

// Define boilerplate samples to help use the application right away
const sampleCodeBlocks = {
    js: `/**
 * TEST CASE 1: Multi-line block comments containing syntax traps
 * function nestedTrap() { return "this should be deleted"; }
 * let x = /regex_inside_comment/;
 */
function test() {
    // TEST CASE 2: Inline comment immediately followed by code expressions
    let compressionRatio = 0.85; // trailing comment here

    // TEST CASE 3: Standard Division Operators (Must NOT be treated as Regex)
    let quotientA = 100 / 10 / 2; 
    let quotientB = (quotientA + 50) / 2; // division following parenthesis closing token

    // TEST CASE 4: True Regular Expression Literals (Must NOT be treated as Division)
    let inlineRegex = /=/g;
    let assignmentRegex = /=|\\(|\\)|,|:/;
    let returnedRegex = (() => {
        return /^[a-zA-Z_][a-zA-Z0-9_]*$/g; // regex immediately following return keyword
    })();

    // TEST CASE 5: Literal Strings containing hidden comment characters (Must remain fully intact)
    let urlString = "https://github.com"; 
    let blockString = 'This is a /* fake block comment */ inside single quotes';
    let escapedQuote = "The minifier said: \\"Don't panic!\\" and it worked.";

    // TEST CASE 6: Advanced Template Literals with Backticks and internal spacing configurations
    let multiLineTemplate = \`
        Line 1: \${quotientA}
        Line 2: // This fake comment line must survive inside the template string context!
        Line 3: /* Another fake block comment */
    \`;

    // TEST CASE 7: Aggressive Symbol Whitespace Compression
    // Heavy unneeded spacing surrounding structural brackets, logic gates, and math operators
    if ( quotientA   >   5   &&   quotientB   !==   0 ) {
        const matrixResult = [ 1 , 2 , 3 ] . map ( ( element ) => {
            return element * 2 ;
        } ) ;
        console.log ( "Result payload matrix processed successfully!" ) ;
    }

    return {
        regexValid: inlineRegex.test("="),
        templateString: multiLineTemplate,
        mathCheck: quotientB
    };
}
test();
`,
    c: `#include <stdio.h>\n/* Global framework configuration entry\n   to compute native state processes */\nint main() {\n    // Forward message stream directly to hardware buffer\n    printf("Running inside isolation sandboxes!\\n");\n    return 0;\n}`,
    c_no_comments: `#include <stdio.h>\n/* Keep the structure but wipe out details */\nvoid debug_log() {\n    // Diagnostic tracker flag\n    char *msg = "WASM active";\n}`,
    make: `# Set compiler platform overrides configurations\nCC = gcc\nCFLAGS = -Wall -Wextra\n\nall: build_target\n\nbuild_target: main.o\n\t$(CC) $(CFLAGS) main.o -o build_target # Link logic output`
};

async function initAppWorkbench() {
    const versionTag   = document.getElementById('versionTag');
    const langSelect   = document.getElementById('langSelect');
    const srcInput     = document.getElementById('srcInput');
    const minifyOutput = document.getElementById('minifyOutput');
    const minifyBtn    = document.getElementById('minifyBtn');
    const inputStats   = document.getElementById('inputStats');
    const outputStats  = document.getElementById('outputStats');

    // 🚀 FETCH BUILD NUMBER DIRECTLY FROM THE EXPOSED WASM COMPONENT
    try {
        const buildId = await getBuildNumber();
        versionTag.textContent = `(WASM Build: ${buildId})`;
    } catch (err) {
        console.error("Failed to read version metadata out of WASM heap:", err);
        versionTag.textContent = `(WASM Build: Error)`;
    }

    // Update stats counters UI tracking
    function updateMetrics(element, text) {
        element.textContent = `${text.length} chars | ${text.split('\n').length} lines`;
    }

    // Swaps templates automatically when the user alters choices
    langSelect.addEventListener('change', () => {
        const mode = langSelect.value;
        srcInput.value = sampleCodeBlocks[mode] || '';
        minifyOutput.textContent = 'Awaiting generation run configuration pass...';
        updateMetrics(inputStats, srcInput.value);
        updateMetrics(outputStats, '');
    });

    // Execute core translation bindings on interaction step
    minifyBtn.addEventListener('click', async () => {
        const activeLanguageMode = langSelect.value;
        const rawContentInput = srcInput.value;
        
        if (!rawContentInput.trim()) {
            minifyOutput.textContent = '// Please supply source data content before processing.';
            return;
        }

        minifyOutput.textContent = 'Compiling and minimizing stream context assets via C memory layers...';
        
        try {
            let processedOutputString = '';

            // Map runtime calls down to individual modular engine wrappers
            switch (activeLanguageMode) {
                case 'js':
                    processedOutputString = await minifyJS(rawContentInput);
                    break;
                case 'c':
                    processedOutputString = await minifyC(rawContentInput);
                    break;
                case 'c_no_comments':
                    processedOutputString = await removeComments(rawContentInput);
                    break;
                case 'make':
                    processedOutputString = await minifyMakefile(rawContentInput);
                    break;
                default:
                    processedOutputString = '// Error: Unsupported target platform parsing type.';
            }

            minifyOutput.textContent = processedOutputString;
            updateMetrics(outputStats, processedOutputString);

        } catch (runtimeException) {
            console.error("WASM Core Execution Stack Exception: ", runtimeException);
            minifyOutput.textContent = `/* Runtime Exception Encountered:\n${runtimeException.message} */`;
        }
    });

    // Load original template default option text layout on launch
    srcInput.value = sampleCodeBlocks[langSelect.value];
    updateMetrics(inputStats, srcInput.value);

const copyBtn = document.getElementById('copyBtn');

copyBtn.addEventListener('click', async () => {
    const minifiedText = minifyOutput.textContent;

    // Ignore placeholder errors or ungenerated default streams
    if (!minifiedText || minifiedText.startsWith('Output file') || minifiedText.startsWith('// Please')) {
        return;
    }

    try {
        copyBtn.textContent = 'Copying...';
        await navigator.clipboard.writeText(minifiedText);
        copyBtn.textContent = 'Copied!';
        copyBtn.style.borderColor = '#a6e3a1'; // Turn border green momentarily
        copyBtn.style.color = '#a6e3a1';
    } catch (err) {
        console.error('Clipboard injection failure:', err);
        copyBtn.textContent = 'Failed!';
        copyBtn.style.borderColor = '#f38ba8'; // Turn border red on crash
    }

    // Restore standard UI button definitions after delay frame passes
    setTimeout(() => {
        copyBtn.textContent = 'Copy Code';
        copyBtn.style.borderColor = '';
        copyBtn.style.color = '';
    }, 1500);
});
}

// Attach listeners on asset loading pass
window.addEventListener('DOMContentLoaded', initAppWorkbench);

