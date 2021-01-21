#include "z80aw.hh"

#include <stdexcept>

#define CHECKED(value) if ((value) < 0) { throw std::runtime_error(z80aw_last_error()); }

namespace z80aw {

std::string initialize_emulator(std::string const& emulator_path)
{
    char serial_port_buf[255];
    CHECKED(z80aw_initialize_emulator(emulator_path.c_str(), serial_port_buf, sizeof serial_port_buf));
    return serial_port_buf;
}

void init(std::string const& serial_port)
{
    CHECKED(z80aw_init(serial_port.c_str()));
}

void close()
{
    CHECKED(z80aw_close());
}

bool logging_to_stdout()
{
    return z80aw_get_logging_to_stdout();
}

bool assert_empty_buffer()
{
    return z80aw_get_assert_empty_buffer();
}

RegisterFetchMode register_fetch_mode()
{
    return static_cast<RegisterFetchMode>(z80aw_get_register_fetch_mode());
}

void set_logging_to_stdout(bool v)
{
    CHECKED(z80aw_set_logging_to_stdout(v));
}

void set_assert_empty_buffer(bool v)
{
    CHECKED(z80aw_set_assert_empty_buffer(v));
}

void set_error_callback(void (*error_cb)(const char* description, void* data), void* data)
{
    CHECKED(z80aw_set_error_callback(error_cb, data));
}

void set_register_fetch_mode(RegisterFetchMode mode)
{
    CHECKED(z80aw_set_register_fetch_mode(static_cast<Z80AW_RegisterFetchMode>(mode)));
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
    std::vector<uint8_t> data(sz);
    CHECKED(z80aw_read_block(addr, sz, data.data()));
    return data;
}

void upload_compiled(z80aw::DebugInformation const& di, void (*upload_callback)(void* data, float perc), void* data)
{
    CHECKED(z80aw_upload_compiled(di.raw_ptr(), upload_callback, data));
    CHECKED(z80aw_cpu_reset());
}

void simple_compilation(std::string const& code)
{
    char err_buf[4096];
    if (z80aw_simple_compilation(code.c_str(), err_buf, sizeof err_buf) < 0)
        throw std::runtime_error(err_buf);
}

bool is_uploaded(z80aw::DebugInformation const& di)
{
    return z80aw_is_uploaded(di.raw_ptr());
}

void reset()
{
    CHECKED(z80aw_cpu_reset());
}

void powerdown()
{
    CHECKED(z80aw_cpu_powerdown());
}

uint16_t pc()
{
    int p = z80aw_cpu_pc();
    CHECKED(p);
    return p;
}

StepResult step()
{
    StepResult sr {};
    uint8_t printed_char;
    CHECKED(z80aw_cpu_step(&sr.registers, &printed_char));
    sr.printed_char = printed_char;
    return sr;
}

Registers registers()
{
    Z80AW_Registers reg;
    CHECKED(z80aw_cpu_registers(&reg));
    return reg;
}

void add_breakpoint(uint16_t addr)
{
    CHECKED(z80aw_add_breakpoint(addr));
}

void remove_breakpoint(uint16_t addr)
{
    CHECKED(z80aw_remove_breakpoint(addr));
}

void remove_all_breakpoints()
{
    CHECKED(z80aw_remove_all_breakpoints());
}

std::vector<uint16_t> query_breakpoints()
{
    uint16_t addr[512];
    int sz = z80aw_query_breakpoints(addr, sizeof addr);
    CHECKED(sz);
    return std::vector<uint16_t>(addr, addr + sz);
}

void keypress(uint8_t k)
{
    CHECKED(z80aw_keypress(k));
}

void next()
{
    CHECKED(z80aw_cpu_next());
}

void continue_()
{
    CHECKED(z80aw_cpu_continue());
}

void stop()
{
    CHECKED(z80aw_cpu_stop());
}

void nmi()
{
    CHECKED(z80aw_cpu_nmi());
}

Event last_event()
{
    return z80aw_last_event();
}

std::string last_error()
{
    return z80aw_last_error();
}

void finalize_emulator()
{
    CHECKED(z80aw_finalize_emulator());
}

//
// DEBUG INFORMATION
//

DebugInformation::DebugInformation(CompilerType compiler_type, std::string const& project_file)
{
    char error_msg[4096];
    
    switch (compiler_type) {
        case Vasm:
            raw_ptr_ = ::compile_vasm(project_file.c_str());
            if (raw_ptr_ == nullptr)
                throw std::runtime_error(std::string("Compilation error:\n") + z80aw_last_error());
            if (!debug_output(raw_ptr_, error_msg, sizeof error_msg)) {
                debug_free(raw_ptr_);
                throw std::runtime_error(std::string("Compilation error:\n") + error_msg);
            }
            break;
        case VasmCode:
            raw_ptr_ = z80aw_simple_compilation_debug(project_file.c_str(), error_msg, sizeof error_msg);
            if (!raw_ptr_)
                throw std::runtime_error(std::string("Compilation error:\n") + error_msg);
            break;
        default:
            throw std::runtime_error("Compiler unsupported.");
    }
    
    size_t n = debug_file_count(raw_ptr_);
    for (size_t i = 0; i < n; ++i)
        filenames_.emplace_back(debug_filename(raw_ptr_, i));
    
    n = debug_binary_count(raw_ptr_);
    for (size_t i = 0; i < n; ++i) {
        ::Binary const* bin = debug_binary(raw_ptr_, i);
        binaries_.push_back({ std::vector<uint8_t>(bin->data, bin->data + bin->sz), bin->addr });
    }
}

DebugInformation::~DebugInformation()
{
    debug_free(raw_ptr_);
}

std::optional<std::string> DebugInformation::sourceline(SourceLocation sl) const
{
    char* s = debug_sourceline(raw_ptr_, sl);
    if (!s)
        return {};
    else
        return s;
}

std::optional<SourceLocation> DebugInformation::location(uint16_t addr) const
{
    SourceLocation sl = debug_location(raw_ptr_, addr);
    if (sl.file == -1)
        return {};
    else
        return sl;
}

std::optional<uint16_t> DebugInformation::rlocation(SourceLocation sl) const
{
    int n = debug_rlocation(raw_ptr_, sl);
    if (n < 0)
        return {};
    else
        return static_cast<uint16_t>(n);
}

std::string DebugInformation::compiler_output() const
{
    char error_msg[4096];
    debug_output(raw_ptr_, error_msg, sizeof error_msg);
    return error_msg;
}

void DebugInformation::print() const
{
    debug_print(raw_ptr_);
}

std::vector<uint8_t> DebugInformation::bytes(SourceLocation sl) const
{
    uint8_t buf[16];
    int r = debug_sourcebytes(raw_ptr_, sl, buf, sizeof buf);
    CHECKED(r);
    if (r == 0)
        return {};
    return std::vector(buf, buf + r);
}

std::vector<DebugInformation::Symbol> DebugInformation::symbols() const
{
    std::vector<DebugInformation::Symbol> ss;
    size_t i = 0;
    for (;;) {
        DebugSymbol const* sym = debug_symbol(raw_ptr_, i++);
        if (!sym)
            break;
        ss.push_back({ sym->symbol, sym->addr });
    }
    return ss;
}

}