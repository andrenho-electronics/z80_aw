#include "terminal.hh"

Terminal* global_terminal = nullptr;

Terminal::Terminal()
{
    background_color_ = COLOR_TERMINAL;
    subwindow_ = newwin(1, 1, 0, 0);
    scrollok(subwindow_, TRUE);
    global_terminal = this;
}

Terminal::~Terminal()
{
    delwin(subwindow_);
}

void Terminal::resize(int line, int col, int lines, int cols)
{
    Window::resize(line, col, lines, cols);
    wresize(subwindow_, this->lines_ - 2, this->cols_ - 2);
    mvwin(subwindow_, line + 1, col + 1);
}

void Terminal::update() const
{
    wmove(subwindow_, cursor_line_, cursor_col_);
    wchgat(subwindow_, 1, 0, 5, nullptr);
    wrefresh(subwindow_);
}

void Terminal::print_char(uint8_t ch)
{
    wmove(subwindow_, cursor_line_, cursor_col_);
    wchgat(subwindow_, 1, 0, COLOR_TERMINAL, nullptr);
    switch (ch) {
        case 10:
            break;
        case 13:
            wprintw(subwindow_, "\n");
            break;
        default:
            wprintw(subwindow_, "%c", ch);
    }
    getyx(subwindow_, cursor_line_, cursor_col_);
    update();
}

