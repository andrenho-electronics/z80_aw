#include "status.hh"

void Status::redraw() const
{
    mvwprintw(window_, 1, 2, "AF:");
    mvwprintw(window_, 2, 2, "BC:");
    mvwprintw(window_, 3, 2, "DE:");
    mvwprintw(window_, 4, 2, "HL:");
    mvwprintw(window_, 1, 10, "AF':");
    mvwprintw(window_, 2, 10, "BC':");
    mvwprintw(window_, 3, 10, "DE':");
    mvwprintw(window_, 4, 10, "HL':");
    mvwprintw(window_, 6, 2, "IX:");
    mvwprintw(window_, 7, 2, "IY:");
    mvwprintw(window_, 8, 2, "SP:");
    mvwprintw(window_, 9, 2, "PC:");
    mvwprintw(window_, 6, 10, "I:");
    mvwprintw(window_, 7, 10, "R:");
    mvwprintw(window_, 11, 2, "Flags:");
    Window::redraw();
}
