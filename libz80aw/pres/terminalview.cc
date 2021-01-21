#include "terminalview.hh"

TerminalView::TerminalView(size_t lines, size_t columns)
    : lines_(lines), columns_(columns)
{
    reset();
}

void TerminalView::add_char(uint8_t c)
{
    if (c == 10) {   // line feed
        if (cursor_y_ == (lines_ - 1))
            pull_screen_up();
        else
            ++cursor_y_;
    } else if (c == 13) {   // carriage return
        cursor_x_ = 0;
    } else {
        if (cursor_x_ == (columns_ - 1) && cursor_y_ == (lines_ - 1)) {
            pull_screen_up();
            cursor_x_ = 0;
        }
        text_.at(cursor_y_).at(cursor_x_) = c;
        ++cursor_x_;
    }
}

void TerminalView::pull_screen_up()
{
    text_.erase(text_.begin());
    text_.emplace_back(columns_, ' ');
}

void TerminalView::reset()
{
    text_.clear();
    for (size_t i = 0; i < lines_; ++i)
        text_.emplace_back(columns_, ' ');
    cursor_x_ = cursor_y_ = 0;
    text_.at(lines_ - 1).replace(columns_ - 1, 1, "X");
}
