# Code Minifier

Code minifier in C. [Live Preview](https://otvnvs.github.io/prj-minifier/)

## Project Structure

* `src/main.c` — cli
* `src/minifier.h` — global declarations, engine interface, enums
* `src/version.c` — versioning
* `src/utils.c` — file reading, allocation
* `src/engines/` — minification processors
* `src/platform/wasm/web/` — web application and es module

## Building

```bash
make native
make windows
make wasm
```

## Running

```bash
./bin/sqminify ./src/main.c
./bin/sqminify -l make Makefile
```

To run the web application, use `make serve`
