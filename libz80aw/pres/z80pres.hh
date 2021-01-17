#ifndef LIBZ80PRES_Z80PRES_HH
#define LIBZ80PRES_Z80PRES_HH

#include <optional>
#include <string>
#include "z80aw.hh"

#include "codeview.hh"
#include "z80state.hh"

using RegisterFetchMode = z80aw::RegisterFetchMode;

class Z80Presentation {
public:
    static Z80Presentation initialize_with_emulator(std::string const& emulator_path);
    
    explicit Z80Presentation(std::string const& serial_port);
    ~Z80Presentation();
    
    Z80Presentation(Z80Presentation const&) = delete;
    Z80Presentation& operator=(Z80Presentation const&) = delete;
    Z80Presentation(Z80Presentation&&) = delete;
    Z80Presentation& operator=(Z80Presentation&&) = delete;
    
    void compile_project_vasm(std::string const& project_path);
    void simple_compilation_vasm(std::string const& code);
    void update();
    
    CodeView& codeview() { return codeview_; }
    
    void set_logging_to_stdout(bool v);
    void set_assert_empty_buffer(bool v);
    void set_register_fetch_mode(RegisterFetchMode mode);
    
    void reset();
    void step();
    void continue_();
    void stop();
    void check_events();
    
    void remove_all_breakpoints();
    
    uint16_t                 pc() const { return z80_state_.pc; }
    std::optional<Registers> registers() const { return z80_state_.registers; }
    Z80State::Mode           mode() const { return z80_state_.mode; }
    
private:
    std::optional<z80aw::DebugInformation> debug_information {};
    
    Z80State z80_state_;
    CodeView codeview_;
};

#endif //LIBZ80PRES_Z80PRES_HH
