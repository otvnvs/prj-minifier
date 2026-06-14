NATIVE_CC?=gcc
WIN_CC?=x86_64-w64-mingw32-gcc
WASM_CC?=emcc

PROFILE?=dev
SERVE_PORT?=8080

SRC_DIR:=src
OBJ_BASE:=obj
BIN_BASE:=bin
DIST_DIR:=dist

WASM_WEB_DIR:=src/platform/wasm/web
#WASM_WEB_SRCS:=$(shell find $(WASM_WEB_DIR) -type f 2>/dev/null)
WASM_WEB_SRCS:=$(shell find $(WASM_WEB_DIR) -type f ! -name '.*.sw?' ! -name '.*.swo')
WASM_WEB_DIST:=$(patsubst $(WASM_WEB_DIR)/%, $(DIST_DIR)/%, $(WASM_WEB_SRCS))

HAS_WINE:=$(shell which wine64 2>/dev/null || which wine 2>/dev/null)
WINE_EXEC:=$(if $(shell which wine64 2>/dev/null),wine64,wine)

BUILD_NUMBER := $(shell \
	if [ ! -f .build_no ]; then echo 0 > .build_no; fi; \
	NUM=$$(($$(cat .build_no) + 1)); \
	echo $$NUM > .build_no; \
	echo $$NUM)

ifdef WSL_DISTRO_NAME
    IS_WSL:=1
endif

ifdef IS_WSL
    WIN_RUNNER=./$(TARGET)
    TEST_RUNNER=$(TEST_DIR)/$(TARGET)
else
    WIN_RUNNER=$(WINE_EXEC) ./$(TARGET)
    TEST_RUNNER=$(WINE_EXEC) $(TEST_DIR)/$(TARGET)
endif

ifeq ($(PROFILE),prod)
    WASM_OPT_FLAGS=-O3 --closure 1
    WASM_EXPORTS=-s EXPORTED_FUNCTIONS="['_get_build_number', '_remove_comments', '_minify_c_code', '_minify_makefile_code', '_minify_js_code', '_malloc', '_free']"
    NATIVE_OPT_FLAGS=-O3 -flto -s
    WIN_OPT_FLAGS=-O3 -flto -s
else
    WASM_OPT_FLAGS=-O2
    WASM_EXPORTS=-s EXPORTED_FUNCTIONS="['_get_build_number', '_remove_comments', '_minify_c_code', '_minify_makefile_code', '_minify_js_code', '_malloc', '_free']"
    NATIVE_OPT_FLAGS=-O2 -g
    WIN_OPT_FLAGS=-O2 -g
endif

# --- Platform Selection Pass ---
ifeq ($(firstword $(MAKECMDGOALS)),windows)
    CC:=$(WIN_CC)
    PLATFORM:=x86_64-w64-mingw32
    EXT:=.exe
    OPT_FLAGS:=$(WIN_OPT_FLAGS)
else ifeq ($(firstword $(MAKECMDGOALS)),test_windows)
    CC:=$(WIN_CC)
    PLATFORM:=x86_64-w64-mingw32
    EXT:=.exe
    OPT_FLAGS:=$(WIN_OPT_FLAGS)
else ifeq ($(firstword $(MAKECMDGOALS)),wasm)
    CC:=$(WASM_CC)
    PLATFORM:=wasm32-emscripten
    EXT:=.js
    OPT_FLAGS:=$(WASM_OPT_FLAGS)
else ifeq ($(firstword $(MAKECMDGOALS)),serve)
    CC:=$(WASM_CC)
    PLATFORM:=wasm32-emscripten
    EXT:=.js
    OPT_FLAGS:=$(WASM_OPT_FLAGS)
else
    CC:=$(NATIVE_CC)
    PLATFORM:=x86_64-linux-gnu
    EXT:=
    OPT_FLAGS:=$(NATIVE_OPT_FLAGS)
endif

# Discover ALL .c files recursively across the entire src tree
ALL_SRCS:=$(shell find $(SRC_DIR) -type f -name '*.c')

# Strict Conditional Code Filtering
ifeq ($(PLATFORM),wasm32-emscripten)
    SRCS:=$(filter-out $(SRC_DIR)/main.c, $(ALL_SRCS))
else
    SRCS:=$(filter-out $(SRC_DIR)/platform/wasm/%, $(ALL_SRCS))
endif

CFLAGS   := -Wall -Wextra $(OPT_FLAGS) -I$(SRC_DIR) -DBUILD_NUMBER=$(BUILD_NUMBER)
DEPFLAGS:=-MMD -MP
OBJ_DIR:=$(OBJ_BASE)/$(PLATFORM)

# FIX: Map the WebAssembly binary generation path cleanly to the subfolder structure
ifeq ($(PLATFORM),wasm32-emscripten)
    TARGET:=$(DIST_DIR)/sqminify/sqminify$(EXT)
else
    TARGET:=$(BIN_BASE)/sqminify-$(PLATFORM)$(EXT)
endif

OBJS:=$(patsubst %.c, $(OBJ_DIR)/%.o, $(SRCS))
DEPS:=$(OBJS:.o=.d)

WASM_RUN_MAIN=0
WASM_HOST_FLAGS=$(WASM_OPT_FLAGS) \
                  $(WASM_EXPORTS) \
                  -s ALLOW_MEMORY_GROWTH=1 \
                  -s MODULARIZE=1 \
                  -s EXPORT_ES6=1 \
                  -s ASYNCIFY=1 \
                  -s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap', 'stringToUTF8', 'UTF8ToString']" \
                  -s INVOKE_RUN=$(WASM_RUN_MAIN)

.PHONY: all native windows wasm clean test test_windows serve

all: native

native: $(TARGET)

windows: $(TARGET)

# Copy web files recursively into dist preserving path layout
$(DIST_DIR)/%: $(WASM_WEB_DIR)/%
	@mkdir -p $(dir $@)
	cp $< $@

wasm: $(OBJS) $(WASM_WEB_DIST)
	@mkdir -p $(DIST_DIR)/sqminify
	$(WASM_CC) $(WASM_HOST_FLAGS) -I$(SRC_DIR) $(OBJS) -o $(TARGET)
	@echo "=============================================="
	@echo "SUCCESS: WebAssembly module package built inside ./$(DIST_DIR)"
	@echo "=============================================="

serve: wasm
	@echo "=============================================="
	@echo "Fire up local web server preview engine on http://localhost:$(SERVE_PORT)"
	@echo "=============================================="
	@python3 -c "import http.server, os; os.chdir('./$(DIST_DIR)'); http.server.SimpleHTTPRequestHandler.extensions_map['.wasm'] = 'application/wasm'; http.server.test(HandlerClass=http.server.SimpleHTTPRequestHandler, port=$(SERVE_PORT))"

ifeq ($(PLATFORM),wasm32-emscripten)
# Linked natively by emcc above
else
$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@
	@echo "Successfully linked: $@"
endif

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

# --- Self-Minification Test Pass (Linux / Native) ---
test: native
	@echo "=============================================="
	@echo "Starting Self-Minification Test Pass ($(PLATFORM))..."
	@echo "=============================================="
	$(eval TEST_DIR := $(shell mktemp -d /tmp/minifier_test.XXXXXX))
	@mkdir -p $(TEST_DIR)
	@echo "Created sandbox directory: $(TEST_DIR)"
	@cp -r $(SRC_DIR) $(TEST_DIR)/
	@for file in $(SRCS); do \
		rel_path=$${file#$(SRC_DIR)/}; \
		mkdir -p $(TEST_DIR)/$(SRC_DIR)/$$(dirname $$rel_path); \
		echo "Minifying: $$file -> $(TEST_DIR)/$(SRC_DIR)/$$rel_path"; \
		./$(TARGET) -l c $$file $(TEST_DIR)/$(SRC_DIR)/$$rel_path || exit 1; \
	done
	@echo "Copying Makefile"
	@cp Makefile $(TEST_DIR)/Makefile
	@echo "----------------------------------------------"
	@echo "Compiling the minified source code..."
	@echo "----------------------------------------------"
	@$(MAKE) -C $(TEST_DIR) native NATIVE_CC=$(CC)
	@echo "Testing minified executable output..."
	@$(TEST_DIR)/$(TARGET) --help > /dev/null 2>&1 || [ $$? -eq 1 ]
	@rm -rf $(TEST_DIR)
	@echo "=============================================="
	@echo "SUCCESS: The minified codebase compiled perfectly!"
	@echo "=============================================="

# --- Self-Minification Test Pass (Windows Cross-Pass) ---
test_windows: windows
	@# Fail early if we are on pure Linux and Wine is completely missing
	@if [ -z "$(IS_WSL)" ] && [ -z "$(HAS_WINE)" ]; then \
		echo "Error: wine or wine64 is required to run Windows tests on this Linux host." >&2; \
		exit 1; \
	fi
	@echo "=============================================="
	@echo "Starting Windows Self-Minification Test Pass..."
	@echo "Host Environment: $(if $(IS_WSL),WSL (Windows Native Interop),Linux ($(WINE_EXEC)))"
	@echo "=============================================="
	$(eval TEST_DIR := $(shell if [ -n "$(IS_WSL)" ]; then \
		win_tmp=$$(wslpath -u $$(cmd.exe /c "echo %TEMP%" 2>/dev/null | tr -d '\r')); \
		mktemp -d "$$win_tmp/minifier_win_test.XXXXXX"; \
	else \
		mktemp -d /tmp/minifier_win_test.XXXXXX; \
	fi))
	@mkdir -p $(TEST_DIR)/$(SRC_DIR)
	@echo "Created sandbox directory: $(TEST_DIR)"
	@cp $(SRC_DIR)/*.h $(TEST_DIR)/$(SRC_DIR)/
	@for file in $(SRCS); do \
		rel_path=$${file#$(SRC_DIR)/}; \
		mkdir -p $(TEST_DIR)/$(SRC_DIR)/$$(dirname $$rel_path); \
		echo "Minifying via Runner: $$file -> $(TEST_DIR)/$(SRC_DIR)/$$rel_path"; \
		if [ -n "$(IS_WSL)" ]; then \
			win_in=$$(wslpath -w "$$file"); \
			win_out=$$(wslpath -w "$(TEST_DIR)/$(SRC_DIR)/$$rel_path"); \
			$(WIN_RUNNER) -l c "$$win_in" "$$win_out" || exit 1; \
		else \
			$(WIN_RUNNER) -l c "$$file" "$(TEST_DIR)/$(SRC_DIR)/$$rel_path" || exit 1; \
		fi; \
	done
	#@echo "Minifying root configurations: Makefile"
	#@if [ -n "$(IS_WSL)" ]; then \
	#	win_in=$$(wslpath -w "Makefile"); \
	#	win_out=$$(wslpath -w "$(TEST_DIR)/Makefile"); \
	#	$(WIN_RUNNER) -l make "$$win_in" "$$win_out" || exit 1; \
	#else \
	#	$(WIN_RUNNER) -l make Makefile "$(TEST_DIR)/Makefile" || exit 1; \
	#fi
	@echo "Copying Makefile"
	@cp Makefile $(TEST_DIR)/Makefile
	@echo "----------------------------------------------"
	@echo "Cross-compiling the minified source code for Windows..."
	@echo "----------------------------------------------"
	@$(MAKE) -C $(TEST_DIR) windows WIN_CC=$(WIN_CC)
	@echo "Testing minified Windows executable output..."
	@if [ -n "$(IS_WSL)" ]; then \
		$(TEST_RUNNER) --help > /dev/null 2>&1 || [ $$? -eq 1 ]; \
	else \
		$(TEST_RUNNER) --help > /dev/null 2>&1 || [ $$? -eq 1 ]; \
	fi
	@rm -rf $(TEST_DIR)
	@echo "=============================================="
	@echo "SUCCESS: The minified Windows codebase compiled perfectly!"
	@echo "=============================================="

clean:
	rm -rf $(OBJ_BASE) $(BIN_BASE) $(DIST_DIR)

-include $(DEPS)

