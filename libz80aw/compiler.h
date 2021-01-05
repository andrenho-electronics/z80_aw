#ifndef LIBZ80AW_COMPILER_H
#define LIBZ80AW_COMPILER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct DebugInformation DebugInformation;

typedef struct {
    char*    symbol;
    uint16_t addr;
} DebugSymbol;

typedef struct {
    ssize_t file;
    size_t line;
} SourceLocation;

DebugInformation* compile_vasm(const char* project_file);
void              debug_free(DebugInformation* di);

// all queries below are null terminated
char*          debug_filename(DebugInformation* di, size_t i);
size_t         debug_file_count(DebugInformation* di);
char*          debug_sourceline(DebugInformation* di, SourceLocation sl);
SourceLocation debug_location(DebugInformation* di, uint16_t addr);        // file = -1 if location is not found
int            debug_rlocation(DebugInformation* di, SourceLocation sl);   // -1 if not found
DebugSymbol*   debug_symbol(DebugInformation* di, size_t i);

bool           debug_output(DebugInformation* di, char* buf, size_t bufsz);

void           debug_print(DebugInformation* di);


#endif //LIBZ80AW_COMPILER_H
