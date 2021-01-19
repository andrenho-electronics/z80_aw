#ifndef DEBUGGER_MYUI_HH
#define DEBUGGER_MYUI_HH

#include <optional>

#include "ui.hh"
#include "options.hh"
#include "../libz80aw/pres/z80pres.hh"
#include "config.hh"

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
    bool show_advanced_window = false;
    
    // start window
    Config config;
    std::optional<ErrorMessage> error_message {};
    
    Z80Presentation& p() { return presentation.value(); }
    
    // draw
    void draw_start();
    void draw_code();
    void draw_memory();
    void draw_cpu();
    
    // errors
    void error(std::string const& title, std::string const& message);
    void draw_error_modal();
    
    // actions
    void start_execution();
    
    void draw_advanced();
    
    void draw_code_view();
};

#endif
