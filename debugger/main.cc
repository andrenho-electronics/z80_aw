#include <cstdlib>
#include <iostream>

#include "window.hh"
#include "myui.hh"

int main()
{
    Window window;
    MyUI ui(window);
    
    while (window.running()) {
        window.do_events();
        
        ui.render();
        window.render();
        ui.render_draw();
        
        window.swap_buffers();
    }
    
    return 0;
}