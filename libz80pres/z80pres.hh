#ifndef LIBZ80PRES_Z80PRES_HH
#define LIBZ80PRES_Z80PRES_HH

#include <string>

namespace z80pres {

void initialize_real_hardware(std::string const& serial_port);
void initialize_emulator(std::string const& emulator_path);

void finalize();

}

#endif //LIBZ80PRES_Z80PRES_HH
