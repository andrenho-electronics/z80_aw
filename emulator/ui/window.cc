#include "window.hh"

Window::Window(int x, int y, int w, int h)
        : x(x), y(y), w(w), h(h) {
    window_ = newwin(h, w, y, x);
    if (w == 0)
        this->w = COLS - x;
}

Window::~Window()
{
    delwin(window_);
}

void Window::redraw() const
{
    box(window_, 0, 0);
    std::string name_ = std::string(" ") + name() + " ";
    mvwprintw(window_, 0, w / 2 - name_.length() / 2, name_.c_str());
    wrefresh(window_);
}

