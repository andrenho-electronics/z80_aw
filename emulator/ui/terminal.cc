#include "terminal.hh"

Terminal::Terminal()
{
    background_color_ = COLOR_TERMINAL;
    subwindow_ = newwin(1, 1, 0, 0);
    scrollok(subwindow_, TRUE);
}

Terminal::~Terminal()
{
    delwin(subwindow_);
}

void Terminal::resize(int line, int col, int lines, int cols)
{
    Window::resize(line, col, lines, cols);
    wresize(subwindow_, this->lines_ - 1, this->cols_ - 1);
    mvwin(subwindow_, 1, this->col_ + 1);
}

void Terminal::update() const
{
    mvwchgat(subwindow_, cursor_line_, cursor_col_, ' ', 0, 5, nullptr);
    wrefresh(subwindow_);
}

