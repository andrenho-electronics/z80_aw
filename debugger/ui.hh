#ifndef DEBUGGER_UI_HH
#define DEBUGGER_UI_HH

#include "imgui/imgui.h"
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

protected:
    virtual void draw() = 0;
    
    ImGuiIO& io;
    ImGuiContext* context = nullptr;
    
private:
    ImGuiIO& init();
};

#endif
