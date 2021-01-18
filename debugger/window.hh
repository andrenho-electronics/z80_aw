#ifndef DEBUGGER_WINDOW_HH
#define DEBUGGER_WINDOW_HH

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "glad/include/glad/glad.h"

class Window {
public:
    Window();
    ~Window();
    
    Window(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;
    
    bool running() const;
    void do_events();
    void render();
    void swap_buffers();
    
    GLFWwindow* ptr() const { return window; }

private:
    GLFWwindow* window;
};

#endif
