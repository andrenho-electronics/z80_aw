#ifndef DEBUGGER_MYUI_HH
#define DEBUGGER_MYUI_HH

#include "ui.hh"
#include "options.hh"

class MyUI : public UI {
public:
    explicit MyUI(Window const& window, Options const& options)
        : UI(window), show_demo_window(options.show_demo_window()) {}
    
    void draw() override;
    
private:
    bool show_demo_window;
    
    // start window
    bool show_start_window = true;
    char project_file[2048] { 0 };
    char serial_port[256] { 0 };
    bool emulator_mode = true;
    
    void draw_demo();
    
    void draw_start();
};

#endif
