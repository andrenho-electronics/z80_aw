#include "ui.hh"

#include <curses.h>

UI::UI()
{
    redraw();
}

UI::~UI()
{
    endwin();
}

void UI::execute()
{
    int ch = getch();
    if (ch == ' ')
        active_ = false;
}

void UI::redraw() const
{
    status.redraw();
}
