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
    sf[nelem - 1] = (SourceFile) { .source_file = NULL, .address = -1 };
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
    return pclose(pipe) == 0 ? 1 : 0;
}

static int load_listing(DebugInformation* di, const char* path, int file_offset)
{
    char filename[512];
    snprintf(filename, sizeof filename, "%s/listing.txt", path);
    
    FILE* fp = fopen(filename, "rb");
    if (!fp)
        ERROR("File '%s' could not be open.", filename);
    
    typedef enum { SOURCE, FILENAMES, OTHER } Section;
    Section section = SOURCE;
    
    /*
    enum Section { Source, Filenames, Other };

    std::string line;
    size_t max_file_number = 0;
    size_t file_number, file_line;
    Section section = Source;
    while (std::getline(f, line)) {
        if (line.empty())
            continue;
        if (line == "Sections:" || line == "Symbols:") {
            section = Other;
        } else if (line == "Sources:") {
            section = Filenames;
        } else if (section == Source && line[0] == 'F') {   // regular source line
            // read line
            std::string file_number_s = line.substr(1, 2);
            std::string file_line_s = line.substr(4, 4);
            std::string source = line.substr(15);
            file_number = strtoul(file_number_s.c_str(), nullptr, 10);
            file_line = strtoul(file_line_s.c_str(), nullptr, 10);
            if (file_number == ULONG_MAX || file_line == ULONG_MAX)
                throw std::runtime_error("Invalid listing file format.");

            // adjust file offset (to permit reading multiple files)
            file_number += file_offset;
            max_file_number = std::max(max_file_number, file_number);

            // adjust source count
            while (cc.source.size() < (file_number + 1))
                cc.source.emplace_back();

            cc.source[file_number].push_back(source);

        } else if (section == Source && line[0] == ' ') {  // address
            std::string addr_s = line.substr(23, 4);
            unsigned long addr = strtoul(addr_s.c_str(), nullptr, 16);
            if (addr == ULONG_MAX)
                throw std::runtime_error("Invalid listing file format.");
            SourceLocation sl = { file_number, file_line };
            cc.locations[addr] = sl;
            cc.rlocations[sl] = addr;

        } else if (section == Filenames && line[0] == 'F') {
            std::string file_number_s = line.substr(1, 2);
            file_number = strtoul(file_number_s.c_str(), nullptr, 10);
            file_number += file_offset;
            while (cc.filename.size() <= file_number)
                cc.filename.emplace_back("-");
            cc.filename[file_number] = line.substr(5);
        }
    }

    return max_file_number + 1;
     */
    return 0;
}

static int load_binary(DebugInformation* di, char* path)
{
    char filename[512];
    snprintf(filename, sizeof filename, "%s/listing.txt", path);
    
    FILE* fp = fopen(filename, "rb");
    if (!fp)
        ERROR("File '%s' could not be open.", filename);
    
    fseek(fp, 0, SEEK_END);
    size_t length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    di->binary_sz = length;
    di->binary = malloc(length);
    fread(di->binary, 1, length, fp);
    fclose(fp);
}

DebugInformation* compile_vasm(const char* project_file)
{
    DebugInformation* di = calloc(1, sizeof(struct DebugInformation));
    map_init(&di->location_map);
    map_init(&di->rlocation_map);
    di->output = strdup("");
    
    char file_path[512];
    find_project_path(project_file, file_path, sizeof file_path);
    cleanup(file_path);
    
    int file_offset = 0;
    
    SourceFile* source_files = load_project_file(project_file, file_path);
    if (!source_files)
        return NULL;
    bool error_found = false;
    for (SourceFile* source_file = source_files; source_file->source_file; ++source_file) {
        if (!error_found) {
            int result = execute_compiler(di, file_path, source_file);
            if (result == 1) {
                file_offset += load_listing(di, file_path, file_offset);
                load_binary(di, file_path);
            } else if (result == 0) {
                error_found = true;
            } else {
                free(di);
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


