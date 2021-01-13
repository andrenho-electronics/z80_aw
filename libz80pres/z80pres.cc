#include "z80pres.hh"

#include "../libz80aw/c++/z80aw.hh"

namespace z80pres {

void initialize_real_hardware(std::string const& serial_port)
{
    z80aw::init({ serial_port, true, false });
}

void initialize_emulator(std::string const& emulator_path)
{
    std::string serial_port = z80aw::initialize_emulator(emulator_path, false);
    z80aw::init({ serial_port, false, false });
}

void finalize()
{
    z80aw::finalize_emulator();
    z80aw::close();
}

}
