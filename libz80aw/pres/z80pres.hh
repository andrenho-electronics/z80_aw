#ifndef LIBZ80PRES_Z80PRES_HH
#define LIBZ80PRES_Z80PRES_HH

#include <optional>
#include <string>
#include "z80aw.hh"

#include "codeview.hh"
#include "z80state.hh"
#include "memoryview.hh"
#include "terminalview.hh"
#include "symbol.hh"
#include "diskview.hh"

using RegisterFetchMode = z80aw::RegisterFetchMode;
using CompilerType = z80aw::DebugInformation::CompilerType;

class Z80Presentation {
public:
    Z80Presentation(std::string const& serial_port_or_emulator_path, bool initialize_with_emulator);
    ~Z80Presentation();
    
    Z80Presentation(Z80Presentation const&) = delete;
    Z80Presentation& operator=(Z80Presentation const&) = delete;
    Z80Presentation(Z80Presentation&&) = delete;
    Z80Presentation& operator=(Z80Presentation&&) = delete;
    
    void compile_project(CompilerType compiler_type, std::string const& text);
    void recompile_project();
    void upload_compiled(void (*upload_callback)(void* data, float perc) = nullptr, void* data = nullptr);
    bool is_uploaded() const { return is_uploaded_; }
    void update();
    
    CodeView& codeview() { return codeview_; }
    MemoryView& memoryview() { return memoryview_; }
    TerminalView const& terminalview() const { return terminalview_; }
    DiskView& diskview() { return disk_view_; }
    
    bool logging_to_stdout();
    bool assert_empty_buffer();
    RegisterFetchMode register_fetch_mode();
    void set_logging_to_stdout(bool v);
    void set_assert_empty_buffer(bool v);
    void set_register_fetch_mode(RegisterFetchMode mode);
    
    void powerdown();
    void reset();
    void step();
    void continue_();
    void stop();
    void next();
    void check_events();
    void nmi();
    
    void keypress(uint8_t key);
    
    void remove_all_breakpoints();
    
    uint16_t                 pc() const { return z80_state_.pc; }
    std::optional<Registers> registers() const { return z80_state_.registers; }
    Z80State::Mode           mode() const { return z80_state_.mode; }
    
    std::vector<std::string> const& file_list() const { return file_list_; }
    std::vector<Symbol> const& symbol_list() const { return symbol_list_; }
    
    bool show_disk_window() const;
    void update_upload_status();

private:
    std::optional<z80aw::DebugInformation> debug_information {};
    
    struct LastCompilation {
        z80aw::DebugInformation::CompilerType mode;
        std::string                           project_name;
    };
    
    std::optional<LastCompilation> last_compilation_ {};
    Z80State z80_state_;
    CodeView codeview_;
    MemoryView memoryview_;
    TerminalView terminalview_;
    DiskView disk_view_;
    std::vector<std::string> file_list_;
    std::vector<Symbol> symbol_list_;
    bool is_uploaded_ = false;
    bool running_with_emulator_;
    std::string disk_image_path_;
    
    void create_file_symbol_list();
    void generate_disk_image(std::string const& path, bool update_emulator);
    void delete_disk_image(std::string const& path);
};

#endif //LIBZ80PRES_Z80PRES_HH
