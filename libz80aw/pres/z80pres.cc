#include "z80pres.hh"

#include <stdexcept>

Z80Presentation::Z80Presentation(std::string const& serial_port_or_emulator_path, bool initialize_with_emulator)
    : codeview_(z80_state_), memoryview_(z80_state_), terminalview_(25, 80)
{
    std::string serial_port;
    if (initialize_with_emulator) {
        serial_port = z80aw::initialize_emulator(serial_port_or_emulator_path);
    } else {
        serial_port = serial_port_or_emulator_path;
    }
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
    create_file_symbol_list();
    is_uploaded_ = false;
    update();
}

void Z80Presentation::upload_compiled(void (* upload_callback)(void*, float), void* data)
{
    if (!debug_information.has_value())
        throw std::runtime_error("There's no compiled project to upload.");
    z80aw::upload_compiled(debug_information.value(), upload_callback, data);
    is_uploaded_ = true;
    memoryview().update();
}

void Z80Presentation::recompile_project()
{
    if (last_compilation_) {
        compile_project(last_compilation_.value().mode, last_compilation_.value().project_name);
        create_file_symbol_list();
        is_uploaded_ = false;
    }
}

void Z80Presentation::update()
{
    z80_state_.pc = z80aw::pc();
    codeview_.update();
    memoryview_.update();
}

void Z80Presentation::step()
{
    z80aw::StepResult sr = z80aw::step();
    if (sr.printed_char != 0)
        terminalview_.add_char(sr.printed_char);
    if (sr.registers.valid)
        z80_state_.registers = sr.registers;
    else
        z80_state_.registers = {};
    update();
}

void Z80Presentation::reset()
{
    z80aw::reset();
    terminalview_.reset();
    update();
}

void Z80Presentation::nmi()
{
    z80aw::nmi();
}

void Z80Presentation::remove_all_breakpoints()
{
    z80aw::remove_all_breakpoints();
    update();
}

bool Z80Presentation::logging_to_stdout()
{
    return z80aw::logging_to_stdout();
}

bool Z80Presentation::assert_empty_buffer()
{
    return z80aw::assert_empty_buffer();
}

RegisterFetchMode Z80Presentation::register_fetch_mode()
{
    return z80aw::register_fetch_mode();
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
    bool cont = z80_state_.mode == Z80State::Running;
    auto le = z80aw::last_event();
    if (le.bkp_reached) {
        z80_state_.mode = Z80State::Stopped;
        try {
            z80_state_.registers = z80aw::registers();
        } catch(...) {}
        update();
    }
    if (le.char_printed) {
        terminalview_.add_char(le.char_printed);
        if (cont)
            z80aw_cpu_continue();
    }
}

void Z80Presentation::next()
{
    z80aw::next();
    update();
}

void Z80Presentation::keypress(uint8_t key)
{
    z80aw::keypress(key);
}

void Z80Presentation::powerdown()
{
    z80aw::powerdown();
}

void Z80Presentation::create_file_symbol_list()
{
    file_list_ = debug_information->filenames();
    
    symbol_list_.clear();
    for (auto const& s: debug_information->symbols()) {
        auto osl = debug_information->location(s.addr);
        if (osl.has_value()) {
            symbol_list_.emplace_back(
                s.symbol,
                debug_information->filenames().at(osl->file),
                osl->line,
                s.addr
            );
        }
    }
}
