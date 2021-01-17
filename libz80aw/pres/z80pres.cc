#include "z80pres.hh"

#include <stdexcept>

Z80Presentation Z80Presentation::initialize_with_emulator(std::string const& emulator_path)
{
    std::string serial_port = z80aw::initialize_emulator(emulator_path, false);
    return Z80Presentation(serial_port);
}

Z80Presentation::Z80Presentation(std::string const& serial_port)
    : codeview_(z80_state_)
{
    z80aw::init(serial_port);
}

Z80Presentation::~Z80Presentation()
{
    z80aw::finalize_emulator();
    z80aw::close();
}

void Z80Presentation::compile_project_vasm(std::string const& project_path)
{
    debug_information.emplace(z80aw::DebugInformation::CompilerType::Vasm, project_path);
    codeview_.set_debug_information(*debug_information);
    update();
}

void Z80Presentation::update()
{
    z80_state_.pc = z80aw::pc();
    codeview_.update();
}

void Z80Presentation::step()
{
    z80aw::step_debug();
    update();
}

void Z80Presentation::reset()
{
    z80aw::reset();
}

void Z80Presentation::remove_all_breakpoints()
{
    z80aw::remove_all_breakpoints();
    update();
}

void Z80Presentation::set_logging_to_stdout(bool v)
{
    z80aw::set_logging_to_stdout(v);
}

void Z80Presentation::set_assert_empty_buffer(bool v)
{
    z80aw::set_assert_empty_buffer(v);
}

void Z80Presentation::continue_()
{
    z80aw::continue_();
    z80_state_.mode = Z80State::Running;
}

void Z80Presentation::stop()
{
    z80aw::stop();
    z80_state_.mode = Z80State::Stopped;
}
