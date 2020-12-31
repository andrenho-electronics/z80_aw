#include "terminal.hh"
#include "../hardware/hardware.hh"

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

void Terminal::ask_keypress()
{
    int cols = 50;
    WINDOW* wkey = newwin(5, cols, LINES / 2 - 3, COLS / 2 - cols / 2);
    std::string name = " Keypress ";
    std::string message = "Press a key to cause a keypress event";
    wbkgd(wkey, COLOR_DIALOG);
    box(wkey, 0, 0);
    mvwprintw(wkey, 0, cols / 2 - name.length() / 2, name.c_str());
    mvwprintw(wkey, 2, cols / 2 - message.length() / 2, message.c_str());
    wrefresh(wkey);
    int ch = getch();
    keypress(ch);
    delwin(wkey);
}

void Terminal::keypress(uint8_t key)
{
    if (key == 10)
        return;
    hardware->register_keypress(key);
}

