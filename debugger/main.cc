#include "window.hh"
#include "myui.hh"
#include "options.hh"

int main(int argc, char* argv[])
{
    Options options(argc, argv);
    Window window;
    MyUI ui(window, options);
    
    while (window.running()) {
        window.do_events();
        
        ui.render();
        window.render();
        ui.render_draw();
        
        window.swap_buffers();
    }
    
    return 0;
}