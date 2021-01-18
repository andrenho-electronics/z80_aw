#ifndef DEBUGGER_MYUI_HH
#define DEBUGGER_MYUI_HH

#include "ui.hh"

class MyUI : public UI {
public:
    explicit MyUI(Window const& window) : UI(window) {}
    
    void draw() override;
    
private:
    bool show_demo_window = true;
};

#endif
