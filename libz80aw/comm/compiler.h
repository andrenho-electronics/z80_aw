#ifndef LIBZ80AW_COMPILER_H
#define LIBZ80AW_COMPILER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

typedef struct DebugInformation DebugInformation;

typedef struct {
    uint8_t* data;
    size_t   sz;
    uint16_t addr;
} Binary;

typedef struct {
    char*    symbol;
    uint16_t addr;
} DebugSymbol;

typedef struct {
    ssize_t file;
    size_t line;
} SourceLocation;

typedef enum { PT_VASM, PT_VASM_DISK } DebugProjectType;

DebugInformation* compile_vasm(const char* project_file);
void              debug_free(DebugInformation* di);

// all queries below are null terminated
char*               debug_filename(DebugInformation const* di, size_t i);
size_t              debug_file_count(DebugInformation const* di);
char*               debug_sourceline(DebugInformation const* di, SourceLocation sl);
int                 debug_sourcebytes(DebugInformation const* di, SourceLocation sl, uint8_t* buf, size_t buf_sz);  // return -1 if not found, else return size of struct
SourceLocation      debug_location(DebugInformation const* di, uint16_t addr);        // file = -1 if location is not found
int                 debug_rlocation(DebugInformation const* di, SourceLocation sl);   // -1 if not found
DebugSymbol const*  debug_symbol(DebugInformation const* di, size_t i);
Binary const*       debug_binary(DebugInformation const* di, size_t i);
size_t              debug_binary_count(DebugInformation const* di);
uint16_t            debug_binary_checksum(DebugInformation const* di);
DebugProjectType    debug_project_type(DebugInformation const* di);

bool           debug_output(DebugInformation const* di, char* buf, size_t bufsz);
int            debug_generate_image(DebugInformation* di, const char* file);

void           debug_print(DebugInformation const* di);

#endif //LIBZ80AW_COMPILER_H
