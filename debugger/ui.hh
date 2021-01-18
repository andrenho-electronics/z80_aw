#ifndef DEBUGGER_UI_HH
#define DEBUGGER_UI_HH

#include "window.hh"

class UI {
public:
    explicit UI(Window const& window);
    virtual ~UI();
    
    UI(const UI&) = delete;
    UI(UI&&) = delete;
    UI& operator=(const UI&) = delete;
    UI& operator=(UI&&) = delete;
    
    void render();
    void render_draw();
    
    virtual void draw() = 0;
};

#endif
