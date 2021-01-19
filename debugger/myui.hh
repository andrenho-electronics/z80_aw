#ifndef DEBUGGER_MYUI_HH
#define DEBUGGER_MYUI_HH

#include <optional>

#include "ui.hh"
#include "options.hh"
#include "../libz80aw/pres/z80pres.hh"

struct ErrorMessage {
    std::string title;
    std::string message;
};

class MyUI : public UI {
public:
    explicit MyUI(Window const& window, Options const& options);
    void draw() override;
    
private:
    std::optional<Z80Presentation> presentation {};
    
    bool show_demo_window;
    
    // start window
    char emulator_path[1024] = "../libz80aw";
    char project_file[2048] { 0 };
    char serial_port[256] = "/dev/ttyUSB0";
    bool emulator_mode = true;
    std::optional<ErrorMessage> error_message;
    
    Z80Presentation& p() { return presentation.value(); }
    
    void draw_start();
    void draw_code();
    void draw_memory();
    void draw_cpu();
    
    void error(std::string const& title, std::string const& message);
    
    void draw_error_modal();
};

#endif
