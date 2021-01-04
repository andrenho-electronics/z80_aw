#include "compiler.h"

#include <stdlib.h>
#include <stdio.h>

#include "contrib/map.h"

typedef struct DebugInformation {
    char**       filenames;
    size_t       n_filenames;
    char***      source;
    size_t*      source_nlines;
    map_int_t    location_map;     // key: address, value: source location
    map_int_t    rlocation_map;    // key: source location, value: address
    DebugSymbol* symbols;
    size_t       n_symbols;
    bool         success;
    char*        output;
} DebugInformation;

void debug_free(DebugInformation* di)
{
    // filenames
    for (char** f_ptr = di->filenames; f_ptr; ++f_ptr)
        free(*f_ptr);
    free(di->filenames);
    
    // sources
    for (char*** sl_ptr = di->source; sl_ptr; ++sl_ptr) {
        for (char** src_ptr = *sl_ptr; src_ptr; ++src_ptr)
            free(*src_ptr);
        free(*sl_ptr);
    }
    free(di->source_nlines);
    free(di->source);
    
    // maps
    map_deinit(&di->location_map);
    map_deinit(&di->rlocation_map);
    
    // symbols
    free(di->symbols);
    
    // output
    free(di->output);
    
    free(di);
}

char* debug_filename(DebugInformation* di, size_t i)
{
    if (i < di->n_filenames)
        return di->filenames[i];
    else
        return NULL;
}

char* debug_sourceline(DebugInformation* di, SourceLocation sl)
{
    if (sl.file >= (ssize_t) di->n_filenames)
        return NULL;
    if (sl.line >= di->source_nlines[sl.file])
        return NULL;
    return di->source[sl.file][sl.line];
}

SourceLocation debug_location(DebugInformation* di, uint16_t addr)
{
    char key[16];
    snprintf(key, sizeof key, "%d", addr);
    int* hash = map_get(&di->location_map, key);
    if (!hash)
        return (SourceLocation) { .file = -1, .line = 0 };
    return (SourceLocation) { .file = *hash >> 16, .line = *hash & 0xffff };
}

int debug_rlocation(DebugInformation* di, SourceLocation sl)
{
    int hash = (int) ((sl.file << 16) | (sl.line));
    char key[16];
    snprintf(key, sizeof key, "%d", hash);
    int* addr = map_get(&di->rlocation_map, key);
    if (!addr)
        return -1;
    return *addr;
}

DebugSymbol* debug_symbol(DebugInformation* di, size_t i)
{
    if (i >= di->n_symbols)
        return NULL;
    return &di->symbols[i];
}

bool debug_output(DebugInformation* di, char* buf, size_t bufsz)
{
    snprintf(buf, bufsz, "%s", di->output);
    return di->success;
}

//
// VASM compilation
//

DebugInformation* compile_vasm(const char* project_file)
{
    return 0;
}

