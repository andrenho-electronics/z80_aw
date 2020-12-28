#include <climits>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <regex>
#include "compiler.hh"

CompiledCode compiled_code;

static std::string execute_compiler(std::string const& filename)
{
    char buffer[1024];

#ifdef _WIN32
    std::string executable = "bin\\vasmz80_oldstyle.exe";
#else
    std::string executable = "/usr/local/bin/vasmz80_oldstyle";
#endif
    std::string commandline = executable + " -chklabels -L listing.txt -Fbin -autoexp -o rom.bin " + filename + " 2>&1";

    FILE* pipe = popen(commandline.c_str(), "r");
    if (!pipe)
        throw std::runtime_error("Could not open executable " + executable + "!");

    std::string result;
    while (fgets(buffer, sizeof buffer, pipe) != nullptr)
        result += buffer;
    if (pclose(pipe) != 0)
        throw std::runtime_error(result);
    return result;
}

static size_t load_listing(std::string const& filename, int file_offset, CompiledCode& cc)
{
    std::ifstream f("listing.txt");
    if (f.fail()) {
        std::cerr << "File listing.txt does not exist or could not be opened.\n";
        exit(1);
    }

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
}

static void load_binary_into_memory(uint16_t addr)
{
    std::ifstream f("rom.bin", std::ios_base::binary);
    if (f.fail()) {
        std::cerr << "File listing.txt does not exist or could not be opened.\n";
        exit(1);
    }

    f.seekg(0, std::ios::end);
    size_t length = f.tellg();
    f.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer;
    buffer.resize(length);
    f.read(reinterpret_cast<char *>(&buffer[0]), length);

    for (uint8_t b: buffer)
        hardware->set_memory(addr++, b);
}

static void cleanup()
{
    unlink("listing.txt");
    unlink("rom.bin");
}

Result compile_assembly_code(Config cf)
{
    CompiledCode cc;

    Result result;
    int file_offset = 0;
    cleanup();
    for (auto const& c: cf.config_file()) {
        result[c.filename] = execute_compiler(c.filename);
        file_offset += load_listing(c.filename, file_offset, compiled_code);
        if (cf.hardware_type() == Emulated)
            load_binary_into_memory(c.memory_location);
        // TODO - match binary on real hardware (?)
        cleanup();
    }

    return result;
}