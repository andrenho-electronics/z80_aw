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

    bool operator==(SourceLocation const& other) const {
        return file == other.file && line == other.line;
    }

    struct HashFunction {
        size_t operator()(const SourceLocation& sl) const {
            return std::hash<size_t>()(sl.file * 10000 + sl.line);
        }
    };
};

struct CompiledCode {
    std::vector<std::string>                     filename;
    std::vector<std::vector<std::string>>        source;    // index = filename index
    std::unordered_map<uint16_t, SourceLocation> locations;
    std::unordered_map<SourceLocation, uint16_t, SourceLocation::HashFunction> rlocations;

    void reset() {
        filename.clear();
        source.clear();
        locations.clear();
        rlocations.clear();
    }
};

extern CompiledCode compiled_code;

Result compile_assembly_code(ConfigFile const& cf);

#endif //EMULATOR_COMPILER_HH
