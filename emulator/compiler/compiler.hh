#ifndef EMULATOR_COMPILER_HH
#define EMULATOR_COMPILER_HH

#include <memory>
#include <string>
#include "../hardware/hardware.hh"
#include "../config/config.hh"

std::string compile_assembly_code(ConfigFile const& cf);

#endif //EMULATOR_COMPILER_HH
