#include "status.hh"
#include "../hardware/hardware.hh"

void Status::redraw() const
{
    mvwprintw(window_, 1, 2, "AF:");
    mvwprintw(window_, 2, 2, "BC:");
    mvwprintw(window_, 3, 2, "DE:");
    mvwprintw(window_, 4, 2, "HL:");
    mvwprintw(window_, 1, 13, "AF':");
    mvwprintw(window_, 2, 13, "BC':");
    mvwprintw(window_, 3, 13, "DE':");
    mvwprintw(window_, 4, 13, "HL':");
    mvwprintw(window_, 6, 2, "IX:");
    mvwprintw(window_, 7, 2, "IY:");
    mvwprintw(window_, 8, 2, "SP:");
    mvwprintw(window_, 9, 2, "PC:");
    mvwprintw(window_, 6, 13, "I:");
    mvwprintw(window_, 7, 13, "R:");
    mvwprintw(window_, 11, 2, "Flags:");
    Window::redraw();
}

void Status::update() const
{
    wattr_on(window_, COLOR_FIELD, nullptr);
    mvwprintw(window_, 1, 6, "%02X %02X", hardware->AF() >> 8, hardware->AF() & 0xff);
    wattr_off(window_, COLOR_FIELD, nullptr);
    wrefresh(window_);
}
