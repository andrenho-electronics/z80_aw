#include "compiler.h"

#include <stdio.h>
#include <limits.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <toml.h>
#include <sys/param.h>
#include <ctype.h>

#include "../contrib/map.h"
#include "z80aw_priv.h"
#include "../../../../../../usr/local/include/toml.h"

typedef struct {
    uint8_t* data;
    size_t   sz;
} Bytes;

typedef struct {
    char*     name;
    size_t    size_mb;
    uint8_t   fat_type;
    map_str_t filenames;
    char*     bootloader;
} DiskInfo;

typedef struct DebugInformation {
    DebugProjectType project_type;
    char**           filenames;
    size_t           n_filenames;
    map_str_t        source_map;
    map_int_t        location_map;     // key: address, value: source location
    map_int_t        rlocation_map;    // key: source location, value: address
    map_void_t       bytes_map;        // key: source location, value: Bytes
    DebugSymbol*     symbols;
    size_t           n_symbols;
    bool             success;
    char*            output;
    Binary*          binary;
    size_t           n_binary;
    uint16_t         checksum1, checksum2;
    DiskInfo         disk_info;
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

int debug_sourcebytes(DebugInformation const* di, SourceLocation sl, uint8_t* buf, size_t buf_sz)
{
    int hash = (int) ((sl.file << 16) | (sl.line));
    char key[16];
    snprintf(key, sizeof key, "%d", hash);
    Bytes** bytes = (Bytes**) map_get(&((DebugInformation*)di)->bytes_map, key);
    if (!bytes)
        return 0;
    memcpy(buf, (*bytes)->data, MIN(buf_sz, (*bytes)->sz));
    return (*bytes)->sz;
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

DebugProjectType debug_project_type(DebugInformation const* di)
{
    return di->project_type;
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

static SourceFile* read_config_file_vasm(toml_table_t* conf)
{
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
    return sf;
}

static SourceFile* read_config_file_vasm_disk(toml_table_t* conf, DiskInfo* disk_info)
{
    toml_table_t* disk = toml_table_in(conf, "disk");
    if (!disk)
        ERROR_N("Invalid file format: key 'disk' not found.");
    
    toml_datum_t name = toml_string_in(disk, "name");
    if (name.ok)
        disk_info->name = name.u.s;
    
    toml_datum_t size_mb = toml_int_in(disk, "size_MB");
    if (!size_mb.ok)
        ERROR_N("Invalid file format: key 'disk.size_MB' not found.");
    disk_info->size_mb = size_mb.u.i;
    
    toml_datum_t fat = toml_int_in(disk, "FAT");
    if (!fat.ok)
        ERROR_N("Invalid file format: key 'disk.FAT' not found.");
    if (fat.u.i != 16 && fat.u.i != 32)
        ERROR_N("Only FAT16 and FAT32 are supported.");
    disk_info->fat_type = fat.u.i;
    
    struct toml_datum_t bootloader = toml_string_in(disk, "bootloader");
    if (!bootloader.ok)
        ERROR_N("Invalid file format: key 'disk.bootloader' not found.");
    
    toml_array_t* files = toml_array_in(disk, "files");
    if (!files)
        ERROR_N("Invalid file format: key 'files' not found.");
    
    int nelem = toml_array_nelem(files);
    SourceFile* sf = calloc(nelem + 2, sizeof(SourceFile));
    
    sf[0] = (SourceFile) {
        .source_file = bootloader.u.s,
        .address = 0,
    };
    disk_info->bootloader = strdup(bootloader.u.s);
    
    for (int i = 0; i < nelem; ++i) {
        toml_table_t* tbl = toml_table_at(files, i);
        sf[i+1] = (SourceFile) {
                .source_file = toml_string_in(tbl, "source").u.s,
                .address = toml_int_in(tbl, "address").u.i,
        };
        map_set(&disk_info->filenames, sf[i+1].source_file, toml_string_in(tbl, "output").u.s);
    }
    sf[nelem+1] = (SourceFile) { .source_file = NULL, .address = -1 };
    return sf;
}

static SourceFile* load_project_file(char const* project_file, DebugProjectType project_type, DebugInformation* di)
{
    FILE* fp = fopen(project_file, "r");
    if (!fp)
        ERROR_N("Project file '%s' could not be opened.", project_file);
    
    char errbuf[512];
    toml_table_t* conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);
    
    if (!conf)
        ERROR_N("Error loading project file '%s': %s", project_file, errbuf);
    
    SourceFile* sf;
    switch (project_type) {
        case PT_VASM:
            sf = read_config_file_vasm(conf);
            break;
        case PT_VASM_DISK:
            sf = read_config_file_vasm_disk(conf, &di->disk_info);
            break;
    }
    
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

static char* strdup_adjust_tabs(const char* original)
{
    char buf[4096] = { 0 };
    size_t ib = 0;
    for (size_t io = 0; io < strlen(original); ++io) {
        if (original[io] != '\t') {
            buf[ib++] = original[io];
        } else {
            size_t f = 8 - (ib % 8);
            for (size_t i = 0; i < f; ++i)
                buf[ib++] = ' ';
        }
    }
    return strdup(buf);
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
            map_set(&di->source_map, key, strdup_adjust_tabs(source));
    
        } else if (section == SOURCE && line[0] == ' ') {  // address
            SourceLocation sl = { .file = file_number, .line = file_line };
    
            // read address
            char buf[10];
            strncpy(buf, &line[23], 4); buf[4] = '\0';
            size_t addr = strtoul(buf, NULL, 16);
            if (addr == ULONG_MAX)
                ERROR("Invalid listing file format.");
            
            // store address in location and rlocation
            int hash = (int) ((sl.file << 16) | (sl.line));
            char hash_str[16], addr_str[16];
            snprintf(hash_str, sizeof hash_str, "%d", hash);
            snprintf(addr_str, sizeof addr_str, "%zu", addr);
            map_set(&di->location_map, addr_str, hash);
            map_set(&di->rlocation_map, hash_str, addr);
            
            // add bytes
            size_t pos = 30;
            uint8_t data[16];
            size_t sz = 0;
            while (sz < sizeof data) {
                if (strlen(line) < pos + 2)
                    break;
                char hbuf[3] = {0};
                strncpy(hbuf, &line[pos], 2);
                if (!isxdigit(hbuf[0]))
                    break;
                size_t byte = strtoul(hbuf, NULL, 16);
                if (byte == ULONG_MAX)
                    break;
                data[sz++] = byte;
                pos += 3;
            }
            Bytes* bytes = malloc(sizeof(Bytes));
            bytes->sz = sz;
            bytes->data = malloc(sz);
            memcpy(bytes->data, data, sz);
            map_set(&di->bytes_map, hash_str, bytes);
        
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

static DebugInformation* compile(DebugProjectType project_type, const char* project_file)
{
    DebugInformation* di = calloc(1, sizeof(DebugInformation));
    map_init(&di->source_map);
    map_init(&di->location_map);
    map_init(&di->rlocation_map);
    map_init(&di->bytes_map);
    map_init(&di->disk_info.filenames);
    di->output = strdup("");
    di->success = true;
    
    char file_path[512];
    find_project_path(project_file, file_path, sizeof file_path);
    cleanup(file_path);
    
    int file_offset = 0;
    
    SourceFile* source_files = load_project_file(project_file, project_type, di);
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

DebugInformation* compile_vasm(const char* project_file)
{
    return compile(PT_VASM, project_file);
}

DebugInformation* compile_vasm_disk(const char* project_file)
{
    return compile(PT_VASM_DISK, project_file);
}

int debug_generate_image(DebugInformation* di, const char* image_file)
{
    char call[4096];
    
    // create image
    snprintf(call, sizeof call, "mkfs.vfat -F %d -n %s -C %s %zu",
            di->disk_info.fat_type, di->disk_info.name ? di->disk_info.name : "UNNAMED", image_file,
            di->disk_info.size_mb * 1024);
    system(call);
    
    // create files
    const char* origin;
    map_iter_t iter = map_iter(&di->disk_info.filenames);
    while ((origin = map_next(&di->disk_info.filenames, &iter))) {
        // find binary
        size_t file_number = 0;
        char* fn;
        while ((fn = debug_filename(di, file_number))) {
            if (strcmp(fn, origin) == 0)
                goto file_found;
            ++file_number;
        }
        ERROR("Could not find origin file '%s' when generating image.");
file_found:;
        Binary const* bin = debug_binary(di, file_number);
        
        // create file
        char* dest = *map_get(&di->disk_info.filenames, origin);
        char* filename = tmpnam(NULL);
        FILE* f = fopen(filename, "wb");
        if (!f)
            ERROR("When generating files to add to image, could not create file '%s' from '%s'.", filename, debug_filename(di, file_number));
        if (fwrite(bin->data, bin->sz, 1, f) != 1)
            ERROR("When generating files to add to image, could not add data to file '%s' from '%s'.", filename, debug_filename(di, file_number));
        fclose(f);
        
        // copy file to partition
        char buf[1024];
        snprintf(buf, sizeof buf, "mcopy -i %s %s ::%s", image_file, filename, dest);
        system(buf);
        unlink(filename);
    }
    
    // find bootstrap
    size_t file_number = 0;
    char* fn;
    while ((fn = debug_filename(di, file_number))) {
        if (strcmp(fn, di->disk_info.bootloader) == 0)
            goto file_found2;
        ++file_number;
    }
    ERROR("Could not find bootloader when generating image file.");
file_found2:;
    Binary const* bin = debug_binary(di, file_number);
    
    // add bootstrap
    FILE* f = fopen(image_file, "r+b");
    uint16_t boot_location = 0;
    size_t boot_size = 0;
    if (di->disk_info.fat_type == 16) {
        boot_location = 0x3e;
        boot_size = 448;
    } else if (di->disk_info.fat_type == 32) {
        boot_location = 0x5a;
        boot_size = 420;
    }
    // initial jump
    fseek(f, 0, SEEK_SET);
    char jp[] = { 0xc3, boot_location, 0x00 };  // z80: jp BOOTSECTOR
    fwrite(jp, sizeof jp, 1, f);
    // clear boot area
    char empty[512] = { 0 };
    fseek(f, boot_location, SEEK_SET);
    fwrite(empty, boot_size, 1, f);
    // boot code
    fseek(f, boot_location, SEEK_SET);
    if (bin->sz >= boot_size)
        ERROR("Bootloader is too large (max %zu bytes)", boot_size);
    fwrite(bin->data, bin->sz, 1, f);
    // set image as bootable
    fseek(f, 0x1fe, SEEK_SET);
    char bootable[] = { 0x55, 0xaa };
    fwrite(bootable, 2, 1, f);
    fclose(f);
    
    return 0;
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
    
    // bytes
    iter = map_iter(&di->source_map);
    while ((key = map_next(&di->bytes_map, &iter))) {
        Bytes* bs = *map_get(&di->bytes_map, key);
        free(bs->data);
        free(bs);
    }
    map_deinit(&di->bytes_map);
    
    // filenames
    iter = map_iter(&di->disk_info.filenames);
    while ((key = map_next(&di->disk_info.filenames, &iter)))
        free(*map_get(&di->disk_info.filenames, key));
    map_deinit(&di->disk_info.filenames);
    
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
    free(di->disk_info.name);
    free(di->disk_info.bootloader);
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
        char* line;
        SourceLocation sl = { .file = i, .line = 1 };
        while ((line = debug_sourceline(di, sl))) {
            printf("      { line: \"%s\"", line);
            int addr = debug_rlocation(di, sl);
            if (addr != -1)
                printf(", addr: 0x%x", addr);
            uint8_t buf[10];
            int r = debug_sourcebytes(di, sl, buf, sizeof buf);
            if (r > 0) {
                printf(", bytes: [ ");
                for (int k = 0; k < r; ++k)
                    printf("%02X ", buf[k]);
                printf("] ");
            }
            printf(" },\n");
            ++sl.line;
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
