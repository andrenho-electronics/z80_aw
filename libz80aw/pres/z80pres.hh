#ifndef LIBZ80PRES_Z80PRES_HH
#define LIBZ80PRES_Z80PRES_HH

#include <optional>
#include <string>
#include "z80aw.hh"

#include "codeview.hh"
#include "z80state.hh"

class Z80Presentation {
public:
    static Z80Presentation initialize_with_emulator(std::string const& emulator_path);
    
    explicit Z80Presentation(std::string const& serial_port);
    ~Z80Presentation();
    
    Z80Presentation(Z80Presentation const&) = delete;
    Z80Presentation& operator=(Z80Presentation const&) = delete;
    Z80Presentation(Z80Presentation&&) = default;
    Z80Presentation& operator=(Z80Presentation&&) = default;
    
    void compile_project_vasm(std::string const& project_path);
    void update();
    
    CodeView& codeview() { return codeview_; }
    
    void reset();
    void step();
    
    void remove_all_breakpoints();

private:
    std::optional<z80aw::DebugInformation> debug_information {};
    
    Z80State z80_state_;
    CodeView codeview_;
};

#endif //LIBZ80PRES_Z80PRES_HH
