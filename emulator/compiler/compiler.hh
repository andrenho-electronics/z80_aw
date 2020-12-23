#ifndef EMULATOR_COMPILER_HH
#define EMULATOR_COMPILER_HH

#include <unordered_map>
#include <memory>
#include <string>
#include "../hardware/hardware.hh"
#include "../config/config.hh"

using Result = std::unordered_map<std::string, std::string>;

struct SourceLocation {
    size_t file;
    size_t line;
};

struct CompiledCode {
    std::unordered_map<size_t, std::string>      filename;
    std::vector<std::vector<std::string>>        source;    // index = filename index
    std::unordered_map<uint16_t, SourceLocation> locations;
};

std::pair<Result, CompiledCode> compile_assembly_code(ConfigFile const& cf);

#endif //EMULATOR_COMPILER_HH
