#include "compiler.hh"

static std::string execute_compiler(std::string const& filename)
{
    char buffer[1024];

#ifdef _WIN32
    std::string executable = "bin\\vasmz80_oldstyle.exe";
#else
#error  Implement this!
#endif
    executable += " -chklabels -L listing.txt -Fbin -autoexp -o rom.bin " + filename + " 2>&1";

    FILE* pipe = popen(executable.c_str(), "r");
    if (!pipe)
        throw std::runtime_error("Could not open executable!");

    std::string result;
    while (fgets(buffer, sizeof buffer, pipe) != nullptr)
        result += buffer;
    if (pclose(pipe) != 0)
        throw std::runtime_error(result);
    return result;
}

static void load_listing(std::string const& filename)
{
}

static void load_binary_into_memory(uint16_t addr)
{
}

static void cleanup()
{
}

std::string compile_assembly_code(ConfigFile const& cf)
{
    std::string result;
    for (auto const& c: cf) {
        result += "Compiling " + c.filename + "...\n";
        result += execute_compiler(c.filename) + "\n";
        load_listing(c.filename);
        load_binary_into_memory(c.memory_location);
        cleanup();
    }
    return result;
}