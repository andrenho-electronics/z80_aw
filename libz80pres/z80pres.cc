#include "z80pres.hh"

#include <stdexcept>

#include "../libz80aw/c++/z80aw.hh"

static bool initialized = false;

Z80Presentation Z80Presentation::initialize_with_emulator(std::string const& emulator_path)
{
    std::string serial_port = z80aw::initialize_emulator(emulator_path, false);
    return Z80Presentation(serial_port);
}

Z80Presentation::Z80Presentation(std::string const& serial_port)
{
    z80aw::init({ serial_port, false, false });
}

Z80Presentation::~Z80Presentation()
{
    z80aw::finalize_emulator();
    z80aw::close();
}

void Z80Presentation::compile_project_vasm(std::string const& project_path)
{

}
