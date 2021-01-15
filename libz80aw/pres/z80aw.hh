#ifndef LIBZ80AW_Z80AW_HH
#define LIBZ80AW_Z80AW_HH

#include <functional>
#include <optional>
#include <string>
#include <vector>

extern "C" {
#include "../comm/z80aw.h"
}

namespace z80aw {

class DebugInformation;

struct Config {
    std::string serial_port;
    bool        log_to_stdout = false;
    bool        assert_empty_buffer = false;
};

using Registers = Z80AW_Registers;
using Event = Z80AW_Event;

std::string initialize_emulator(std::string const& emulator_path, bool z80_registers=false);  // return serial port

void init(Config const& config);
void close();

void set_error_callback(void (*error_cb)(const char* description, void* data), void* data);

void    write_byte(uint16_t addr, uint8_t data);
uint8_t read_byte(uint16_t addr);

void                 write_block(uint16_t addr, std::vector<uint8_t> const& data);
std::vector<uint8_t> read_block(uint16_t addr, uint16_t sz);

void upload_compiled(z80aw::DebugInformation const& di, void (*upload_callback)(void* data, float perc) = nullptr, void* data = nullptr);
bool is_uploaded(z80aw::DebugInformation const& di);

void      reset();
void      powerdown();
uint16_t  pc();
uint8_t   step();   //  return printed char

struct StepResult {
    Registers registers;
    uint8_t   printed_char;
};
StepResult step_debug();

void                  add_breakpoint(uint16_t addr);
void                  remove_breakpoint(uint16_t addr);
void                  remove_all_breakpoints();
std::vector<uint16_t> query_breakpoints();

void keypress(uint8_t);

void continue_();
void stop();

Event       last_event();
std::string last_error();

void finalize_emulator();

class DebugInformation {
public:
    struct Binary {
        std::vector<uint8_t> data;
        uint16_t             addr;
    };
    
    struct Symbol {
        std::string symbol;
        uint16_t    addr;
    };
    
    enum CompilerType { Vasm };
    
    DebugInformation(CompilerType compiler_type, std::string const& project_file);
    ~DebugInformation();
    
    std::vector<std::string> const& filenames() const { return filenames_; }
    std::vector<Binary> const&      binaries() const { return binaries_; }
    std::optional<std::string>      sourceline(SourceLocation sl) const;
    std::optional<SourceLocation>   location(uint16_t addr) const;
    std::optional<uint16_t>         rlocation(SourceLocation sl) const;
    ::DebugInformation const*       raw_ptr() const { return raw_ptr_; }
    
    std::string                     compiler_output() const;
    void                            print() const;
    
    DebugInformation(DebugInformation const&) = delete;
    DebugInformation& operator=(DebugInformation const&) = delete;
    DebugInformation(DebugInformation&&) = default;
    DebugInformation& operator=(DebugInformation&&) = default;
    
private:
    explicit DebugInformation(::DebugInformation* raw_ptr) : raw_ptr_(raw_ptr) {}
    
    ::DebugInformation*      raw_ptr_;
    std::vector<std::string> filenames_;
    std::vector<Binary>      binaries_;
};

}

#endif //LIBZ80AW_Z80AW_HH