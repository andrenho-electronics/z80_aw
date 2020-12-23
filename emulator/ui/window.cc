#include "window.hh"
#include "colors.hh"

Window::Window() {
    window_ = newwin(0, 0, 0, 0);
}

Window::~Window()
{
    delwin(window_);
}

void Window::redraw() const
{
    box(window_, 0, 0);
    std::string name_ = std::string(" ") + name() + " ";
    mvwprintw(window_, 0, cols_ / 2 - name_.length() / 2, name_.c_str());
    wbkgd(window_, background_color_);
    wrefresh(window_);
}

void Window::resize(int line, int col, int lines, int cols)
{
    this->line_ = line;
    this->col_ = col;
    this->lines_ = lines == 0 ? LINES - line: lines;
    this->cols_ = cols == 0 ? COLS - col : cols;
    wresize(window_, this->lines_, this->cols_);
    mvwin(window_, line, col);
}
