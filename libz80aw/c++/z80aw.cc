#include "z80aw.hh"

#include <stdexcept>

#define CHECKED(value) if ((value) < 0) { throw std::runtime_error(z80aw_last_error()); }

namespace z80aw {

std::string initialize_emulator(std::string const& emulator_path, bool z80_registers)
{
    char serial_port_buf[255];
    CHECKED(z80aw_initialize_emulator(emulator_path.c_str(), serial_port_buf, sizeof serial_port_buf, z80_registers));
    return serial_port_buf;
}

void init(Config const& config)
{
    Z80AW_Config cfg = {
            config.serial_port.c_str(),
            config.log_to_stdout,
            config.assert_empty_buffer,
    };
    CHECKED(z80aw_init(&cfg));
}

void close()
{
    CHECKED(z80aw_close());
}

void set_error_callback(void (*error_cb)(const char* description, void* data), void* data)
{
    CHECKED(z80aw_set_error_callback(error_cb, data));
}

void write_byte(uint16_t addr, uint8_t data)
{
    CHECKED(z80aw_write_byte(addr, data));
}

uint8_t read_byte(uint16_t addr)
{
    int r = z80aw_read_byte(addr);
    CHECKED(r);
    return r;
}

void write_block(uint16_t addr, std::vector<uint8_t> const& data)
{
    CHECKED(z80aw_write_block(addr, data.size(), data.data()));
}

std::vector<uint8_t> read_block(uint16_t addr, uint16_t sz)
{

}

void upload_compiled(z80aw::DebugInformation const& di, std::function<void(void*, float)> const& f, void* data)
{
}

bool is_uploaded(z80aw::DebugInformation const& di)
{
}

void reset()
{
}

void powerdown()
{
}

uint16_t pc()
{
}

uint8_t step()
{
}

Registers step_debug()
{
}

void add_breakpoint(uint16_t addr)
{
}

void remove_breakpoint(uint16_t addr)
{
}

void remove_all_breakpoints()
{
}

std::vector<uint16_t> query_breakpoints()
{
}

void keypress(uint8_t)
{
}

void continue_()
{
}

void stop()
{
}

Event last_event()
{
}

std::string last_error()
{
}

DebugInformation::~DebugInformation()
{

}

std::string DebugInformation::sourceline(SourceLocation sl) const
{
    return std::__cxx11::string();
}

SourceLocation DebugInformation::location(uint16_t addr) const
{
    return SourceLocation();
}

uint16_t DebugInformation::rlocation(SourceLocation sl) const
{
    return 0;
}

DebugInformation DebugInformation::compile_vasm(std::string const& project_file)
{
    return DebugInformation();
}

}