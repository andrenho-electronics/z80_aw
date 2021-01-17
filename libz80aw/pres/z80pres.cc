#include "z80pres.hh"

#include <stdexcept>

Z80Presentation Z80Presentation::initialize_with_emulator(std::string const& emulator_path)
{
    std::string serial_port = z80aw::initialize_emulator(emulator_path);
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

void Z80Presentation::compile_project(CompilerType compiler_type, std::string const& text)
{
    debug_information.emplace(compiler_type, text);
    last_compilation_ = { compiler_type, text };
    codeview_.set_debug_information(*debug_information);
    update();
}


void Z80Presentation::recompile_project()
{
    if (last_compilation_)
        compile_project(last_compilation_.value().mode, last_compilation_.value().project_name);
}

void Z80Presentation::update()
{
    z80_state_.pc = z80aw::pc();
    codeview_.update();
}

void Z80Presentation::step()
{
    z80aw::StepResult sr = z80aw::step();
    // TODO - send character to terminal
    if (sr.registers.valid)
        z80_state_.registers = sr.registers;
    else
        z80_state_.registers = {};
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


void Z80Presentation::set_register_fetch_mode(RegisterFetchMode mode)
{
    z80aw::set_register_fetch_mode(mode);
}

void Z80Presentation::continue_()
{
    z80aw::continue_();
    z80_state_.mode = Z80State::Running;
    update();
}

void Z80Presentation::stop()
{
    z80aw::stop();
    z80_state_.mode = Z80State::Stopped;
    step();
    update();
}

void Z80Presentation::check_events()
{
    auto le = z80aw::last_event();
    switch (le.type) {
        case Z80AW_PRINT_CHAR:
            // TODO - print char in terminal
            update();
            break;
        case Z80AW_BREAKPOINT:
            z80_state_.mode = Z80State::Stopped;
            try {
                z80_state_.registers = z80aw::registers();
            } catch(...) {}
            update();
            break;
        case Z80AW_ERROR:
            throw std::runtime_error("There was an error fetch last event.");
        case Z80AW_NO_EVENT:
            break;
    }
}

void Z80Presentation::next()
{
    z80aw::next();
    update();
}

