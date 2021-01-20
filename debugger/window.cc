#include "window.hh"

#include <iostream>
#include <stdexcept>

Window::Window()
{
    glfwSetErrorCallback([](int, const char* description) { std::cerr << description << "\n"; });
    if (!glfwInit())
        throw std::runtime_error("glfwInit");
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    window = glfwCreateWindow(800, 600, "Z80AW Debugger", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("glfwCreateWindow");
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Window::running() const
{
    return !glfwWindowShouldClose(window);
}

void Window::do_events(bool wait)
{
    if (wait)
        glfwWaitEventsTimeout(0.05);
    else
        glfwPollEvents();
}

void Window::render()
{
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.3f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Window::swap_buffers()
{
    glfwSwapBuffers(window);
}
