#include "compiler.h"

#include <limits.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <toml.h>
#include <sys/param.h>

#include "contrib/map.h"
#include "z80aw_priv.h"

typedef struct DebugInformation {
    char**       filenames;
    size_t       n_filenames;
    map_str_t    source_map;
    map_int_t    location_map;     // key: address, value: source location
    map_int_t    rlocation_map;    // key: source location, value: address
    DebugSymbol* symbols;
    size_t       n_symbols;
    bool         success;
    char*        output;
    Binary*      binary;
    size_t       n_binary;
    uint16_t     checksum1, checksum2;
} DebugInformation;

char* debug_filename(DebugInformation const* di, size_t i)
{
    if (i < di->n_filenames)
        return di->filenames[i];
    else
        return NULL;
}

size_t debug_file_count(DebugInformation const* di)
{
    return di->n_filenames;
}

char* debug_sourceline(DebugInformation const* di, SourceLocation sl)
{
    char key[16];
    snprintf(key, sizeof key, "%zd:%zu", sl.file, sl.line);
    char** value = map_get(&((DebugInformation*)di)->source_map, key);
    return value ? *value : NULL;
}

SourceLocation debug_location(DebugInformation const* di, uint16_t addr)
{
    char key[16];
    snprintf(key, sizeof key, "%d", addr);
    int* hash = map_get(&((DebugInformation*)di)->location_map, key);
    if (!hash)
        return (SourceLocation) { .file = -1, .line = 0 };
    return (SourceLocation) { .file = *hash >> 16, .line = *hash & 0xffff };
}

int debug_rlocation(DebugInformation const* di, SourceLocation sl)
{
    int hash = (int) ((sl.file << 16) | (sl.line));
    char key[16];
    snprintf(key, sizeof key, "%d", hash);
    int* addr = map_get(&((DebugInformation*)di)->rlocation_map, key);
    if (!addr)
        return -1;
    return *addr;
}

DebugSymbol const* debug_symbol(DebugInformation const* di, size_t i)
{
    if (i >= di->n_symbols)
        return NULL;
    return &di->symbols[i];
}

bool debug_output(DebugInformation const* di, char* buf, size_t bufsz)
{
    snprintf(buf, bufsz, "%s", di->output);
    return di->success;
}

Binary const* debug_binary(DebugInformation const* di, size_t i)
{
    if (i >= di->n_binary)
        return NULL;
    return &di->binary[i];
}

size_t debug_binary_count(DebugInformation const* di)
{
    return di->n_binary;
}

uint16_t debug_binary_checksum(DebugInformation const* di)
{
    return di->checksum1 | (di->checksum2 << 8);
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

static SourceFile* load_project_file(char const* project_file)
{
    FILE* fp = fopen(project_file, "r");
    if (!fp)
        ERROR_N("Project file '%s' could not be opened.", project_file);
    
    char errbuf[512];
    toml_table_t* conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);
    
    if (!conf)
        ERROR_N("Error loading project file '%s': %s", project_file, errbuf);
    
    toml_array_t* sources = toml_array_in(conf, "sources");
    if (!sources)
        ERROR_N("Invalid file format: key 'sources' not found.");
    
    int nelem = toml_array_nelem(sources);
    SourceFile* sf = calloc(nelem + 1, sizeof(SourceFile));
    for (int i = 0; i < nelem; ++i) {
        toml_table_t* tbl = toml_table_at(sources, i);
        sf[i] = (SourceFile) {
            .source_file = toml_string_in(tbl, "source").u.s,
            .address = toml_int_in(tbl, "address").u.i,
        };
    }
    sf[nelem] = (SourceFile) { .source_file = NULL, .address = -1 };
    toml_free(conf);
    
    return sf;
}

static int execute_compiler(DebugInformation* di, const char* file_path, SourceFile* source_file)
{
#ifdef _WIN32
    const char* executable = "bin\\vasmz80_oldstyle.exe";
#else
    const char* executable = "/usr/local/bin/vasmz80_oldstyle";
#endif
    char commandline[1024];
    snprintf(commandline, sizeof commandline, "cd %s && %s  -chklabels -L listing.txt -Fbin -autoexp -o rom.bin %s 2>&1",
             file_path, executable, source_file->source_file);
    
    FILE* pipe = popen(commandline, "r");
    if (!pipe)
        ERROR("Could not execute compiler. The compiler was expected to be found at '%s'. Maybe you need to run 'install_vasm.sh' at the root of this distibution?", executable);
    
    char buffer[4096];
    
    while (fgets(buffer, sizeof buffer, pipe) != NULL) {
        di->output = realloc(di->output, strlen(di->output) + strlen(buffer) + 1);
        strcat(di->output, buffer);
    }
    return pclose(pipe) == 0 ? 0 : 1;
}

static void ensure_file_count(DebugInformation* di, size_t file_number)
{
    while (di->n_filenames < (file_number + 1)) {
        ++di->n_filenames;
        di->filenames = realloc(di->filenames, sizeof(di->filenames) * di->n_filenames);
    }
}

static int load_listing(DebugInformation* di, const char* path, int file_offset)
{
    char filename[512];
    snprintf(filename, sizeof filename, "%s/listing.txt", path);
    
    FILE* fp = fopen(filename, "rb");
    if (!fp)
        ERROR("File '%s' could not be open.", filename);
    
    regex_t regex1, regex2;
    int rx1 = regcomp(&regex1, "^([^\\s][A-z0-9_]+)\\s*EXPR\\s*\\([[:digit:]]+=((0x)?[[:xdigit:]]+)\\).*ABS", REG_EXTENDED); // name in group 1, address in group 2
    if (rx1 != 0)
        ERROR("Error compiling regex 1");
    int rx2 = regcomp(&regex2, "^([^\\s][A-z0-9_]+)\\s+LAB\\s*\\(((0x)?[[:xdigit:]]+)\\)", REG_EXTENDED);                    // name in group 1, address in group 2
    if (rx2 != 0)
        ERROR("Error compiling regex 2");
    
    int max_file_number = 0;
    size_t file_number = 0, file_line = 0;
    char line[2048];
    
    typedef enum { SOURCE, FILENAMES, SYMBOLS, OTHER } Section;
    Section section = SOURCE;
    
    while (fgets(line, sizeof line, fp)) {
        size_t line_len = strlen(line);
        if (line_len == 0)
            continue;
        
        // chomp enter
        while (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')
            line[line_len - 1] = '\0';
        
        // parse line
        if (strlen(line) == 0)
            continue;
        if (strcmp(line, "Sections:") == 0) {
            section = OTHER;
        } else if (strcmp(line, "Symbols:") == 0) {
            section = SYMBOLS;
        } else if (strcmp(line, "Sources:") == 0) {
            section = FILENAMES;
        } else if (section == SOURCE && line[0] == 'F') {  // regular source file
            char buf[10];
            strncpy(buf, &line[1], 2); buf[2] = '\0';
            file_number = strtoul(buf, NULL, 10);
            strncpy(buf, &line[4], 4); buf[4] = '\0';
            file_line = strtoul(buf, NULL, 10);
            const char* source = &line[15];   // TODO - remove ENTER
            if (file_number == ULONG_MAX || file_line == ULONG_MAX)
                ERROR("Invalid listing file format.");
            
            // adjust file offset (to permit reading multiple files)
            file_number += file_offset;
            max_file_number = MAX(max_file_number, file_number);
    
            // adjust source count
            ensure_file_count(di, file_number);
    
            // add source line
            char key[16];
            snprintf(key, sizeof key, "%zd:%zu", file_number, file_line);
            map_set(&di->source_map, key, strdup(source));
    
        } else if (section == SOURCE && line[0] == ' ') {  // address
            char buf[10];
            strncpy(buf, &line[23], 4); buf[4] = '\0';
            size_t addr = strtoul(buf, NULL, 16);
            if (addr == ULONG_MAX)
                ERROR("Invalid listing file format.");
            SourceLocation sl = { .file = file_number, .line = file_line };
            int hash = (int) ((sl.file << 16) | (sl.line));
            char hash_str[16], addr_str[16];
            snprintf(hash_str, sizeof hash_str, "%d", hash);
            snprintf(addr_str, sizeof addr_str, "%zu", addr);
            map_set(&di->location_map, addr_str, hash);
            map_set(&di->rlocation_map, hash_str, addr);
        
        } else if (section == FILENAMES && line[0] == 'F') {
            char bf[10];
            strncpy(bf, &line[1], 2);
            bf[2] = '\0';
            file_number = strtoul(bf, NULL, 10) + file_offset;
            ensure_file_count(di, file_number);
            di->filenames[file_number] = strdup(&line[5]);
            
        } else if (section == SYMBOLS) {
            regmatch_t m[5] = { 0 };
            int r = regexec(&regex1, line, sizeof m, m, 0);
            if (r == REG_NOMATCH)
                r = regexec(&regex2, line, sizeof m, m, 0);
            if (r == 0 && m[1].rm_so != -1 && m[2].rm_so != -1) {  // match
                char symbol_name[512] = { 0 };
                strncat(symbol_name, &line[m[1].rm_so], m[1].rm_eo - m[1].rm_so);
                char addr_s[16] = { 0 };
                strncat(addr_s, &line[m[2].rm_so], m[2].rm_eo - m[2].rm_so);
                unsigned long addr = strtoul(addr_s, NULL, 16);
                int n_symbols = ++di->n_symbols;
                di->symbols = realloc(di->symbols, n_symbols * sizeof(DebugSymbol));
                di->symbols[n_symbols - 1].symbol = strdup(symbol_name);
                di->symbols[n_symbols - 1].addr = addr;
            }
        }
    }
    
    regfree(&regex1);
    regfree(&regex2);
    
    fclose(fp);
    return max_file_number + 1;
}

static int load_binary(DebugInformation* di, char* path, uint16_t address)
{
    char filename[512];
    snprintf(filename, sizeof filename, "%s/rom.bin", path);
    
    // open file
    FILE* fp = fopen(filename, "rb");
    if (!fp)
        ERROR("File '%s' could not be open.", filename);
    
    // find file size
    fseek(fp, 0, SEEK_END);
    size_t length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
   
    // increment binary count
    di->binary = realloc(di->binary, sizeof(Binary) * ++di->n_binary);
    Binary* b = &di->binary[di->n_binary - 1];
    
    // add to structure
    b->addr = address;
    b->sz = length;
    b->data = malloc(length);
    fread(b->data, 1, length, fp);
    
    // calculate checksum
    for (size_t i = 0; i < length; ++i) {
        di->checksum1 = (di->checksum1 + b->data[i]) % 255;
        di->checksum2 = (di->checksum2 + di->checksum1) % 255;
    }
    
    fclose(fp);
    return 0;
}

DebugInformation* compile_vasm(const char* project_file)
{
    DebugInformation* di = calloc(1, sizeof(DebugInformation));
    map_init(&di->source_map);
    map_init(&di->location_map);
    map_init(&di->rlocation_map);
    di->output = strdup("");
    di->success = true;
    
    char file_path[512];
    find_project_path(project_file, file_path, sizeof file_path);
    cleanup(file_path);
    
    int file_offset = 0;
    
    SourceFile* source_files = load_project_file(project_file);
    if (!source_files) {
        debug_free(di);
        return NULL;
    }
    bool error_found = false;
    for (SourceFile* source_file = source_files; source_file->source_file; ++source_file) {
        if (!error_found) {
            int result = execute_compiler(di, file_path, source_file);
            if (result == 0) {
                file_offset += load_listing(di, file_path, file_offset);
                load_binary(di, file_path, source_file->address);
            } else if (result == 1) {
                di->success = false;
                error_found = true;
            } else {
                debug_free(di);
                di = NULL;
                error_found = true;   // will return null
            }
            cleanup(file_path);
        }
        free(source_file->source_file);
    }
    free(source_files);
    
    return di;
}

void debug_free(DebugInformation* di)
{
    // filenames
    for (size_t i = 0; i < di->n_filenames; ++i)
        free(di->filenames[i]);
    free(di->filenames);
    
    // source
    const char *key;
    map_iter_t iter = map_iter(&di->source_map);
    while ((key = map_next(&di->source_map, &iter)))
        free(*map_get(&di->source_map, key));
    map_deinit(&di->source_map);
    
    // maps
    map_deinit(&di->location_map);
    map_deinit(&di->rlocation_map);
    
    // binaries
    for (size_t i = 0; i < di->n_binary; ++i)
        free(di->binary[i].data);
    
    // symbols
    for (size_t i = 0; i < di->n_symbols; ++i)
        free(di->symbols[i].symbol);
    
    // other
    free(di->symbols);
    free(di->output);
    free(di->binary);
    free(di);
}


void debug_print(DebugInformation const* di)
{
    printf("{\n");
    
    printf("  filenames: [ ");
    for (size_t i = 0; i < debug_file_count(di); ++i)
        printf("\"%s\", ", debug_filename(di, i));
    printf("],\n");
    
    printf("  sources: {\n");
    for (size_t i = 0; i < debug_file_count(di); ++i) {
        printf("    { \"%s\" : [\n", debug_filename(di, i));
        size_t j = 1;
        char* line;
        while ((line = debug_sourceline(di, (SourceLocation) { .file = i, .line = j }))) {
            printf("      { line: \"%s\"", line);
            int addr = debug_rlocation(di, (SourceLocation) { .file = i, .line = j });
            if (addr != -1)
                printf(", addr: 0x%x", addr);
            printf(" },\n");
            ++j;
        }
        printf("    ] },\n");
    }
    printf("  },\n");
    
    printf("  symbols: [\n");
    size_t i = 0;
    for (DebugSymbol const* sym = debug_symbol(di, i); sym; sym = debug_symbol(di, ++i)) {
        printf("    { addr: 0x%x, name: \"%s\" },\n", sym->addr, sym->symbol);
    }
    printf("  ],\n");
    
    printf("  binaries: [\n");
    i = 0;
    for (Binary const* bin = debug_binary(di, i); bin; bin = debug_binary(di, ++i)) {
        printf("    { addr: 0x%x, data: [ ", bin->addr);
        for (size_t j = 0; j < bin->sz; ++j)
            printf("%02X ", bin->data[j]);
        printf("] },\n");
    }
    printf("  ],\n");
    
    printf("}\n");
}
