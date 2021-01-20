#include "window.hh"
#include "myui.hh"
#include "options.hh"

#include <chrono>
#include <thread>
using namespace std::chrono_literals;

int main(int argc, char* argv[])
{
    Options options(argc, argv);
    Window window;
    MyUI ui(window, options);
    
    while (window.running()) {
        window.do_events(ui.stopped());
        
        ui.render();
        window.render();
        ui.render_draw();
        
        window.swap_buffers();
        std::this_thread::sleep_for(1ms);
    }
    
    return 0;
}