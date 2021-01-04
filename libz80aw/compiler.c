#include "compiler.h"

#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <toml.h>

#include "contrib/map.h"
#include "z80aw_priv.h"

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
    uint8_t*     binary;
    size_t       binary_sz;
} DebugInformation;

void debug_free(DebugInformation* di)
{
    // filenames
    if (di->filenames) {
        for (char** f_ptr = di->filenames; f_ptr; ++f_ptr)
            free(*f_ptr);
        free(di->filenames);
    }
    
    // sources
    if (di->source) {
        for (char*** sl_ptr = di->source; sl_ptr; ++sl_ptr) {
            for (char** src_ptr = *sl_ptr; src_ptr; ++src_ptr)
                free(*src_ptr);
            free(*sl_ptr);
        }
        free(di->source_nlines);
        free(di->source);
    }
    
    // maps
    map_deinit(&di->location_map);
    map_deinit(&di->rlocation_map);
    
    // other
    free(di->symbols);
    free(di->output);
    free(di->binary);
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

typedef struct {
    char*    source_file;
    uint16_t address;
} SourceFile;

static void find_project_path(char const* filename, char* path, size_t path_sz)
{
    char filename_copy[512];
    strncpy(filename_copy, filename, sizeof filename_copy);
    strncpy(path, dirname(filename_copy), path_sz);
}

static void cleanup(const char* path)
{
    char filename[512];
    snprintf(filename, sizeof filename, "%s/listing.txt", path);
    unlink(filename);
    snprintf(filename, sizeof filename, "%s/rom.bin", path);
    unlink(filename);
}

static SourceFile* load_project_file(char const* project_file, const char* path)
{
    FILE* fp = fopen(project_file, "r");
    if (!fp) {
        z80aw_set_error("Project file '%s' could not be opened.", project_file);
        return NULL;
    }
    
    const char errbuf[512];
    toml_table_t* conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);
    
    if (!conf) {
        z80aw_set_error("Error loading project file '%s': %s", project_file, errbuf);
        return NULL;
    }
    
    // ....
    
    return NULL;
}

static bool execute_compiler(DebugInformation* di, const char* file_path, SourceFile source_file)
{
    return 0;
}

static int load_listing(DebugInformation* di, const char* path, int file_offset)
{
    return 0;
}

static void load_binary(DebugInformation* di, char* path)
{
}

DebugInformation* compile_vasm(const char* project_file)
{
    DebugInformation* di = calloc(1, sizeof(struct DebugInformation));
    map_init(&di->location_map);
    map_init(&di->rlocation_map);
    
    char file_path[512];
    find_project_path(project_file, file_path, sizeof file_path);
    cleanup(file_path);
    
    int file_offset = 0;
    
    SourceFile* source_files = load_project_file(project_file, file_path);
    if (!source_files)
        return NULL;
    bool error_found = false;
    for (SourceFile* source_file = source_files; source_file; ++source_file) {
        if (!error_found) {
            bool result = execute_compiler(di, file_path, *source_file);
            if (result) {
                file_offset += load_listing(di, file_path, file_offset);
                load_binary(di, file_path);
            } else {
                error_found = true;
            }
            cleanup(file_path);
        }
        free(source_file->source_file);
    }
    free(source_files);
    
    return di;
}


