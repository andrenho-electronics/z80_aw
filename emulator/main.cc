#include "ui/ui.hh"

#include <curses.h>

int main()
{
    initscr();
    resize_term(40, 130);
    noecho();
    keypad(stdscr, true);
    refresh();

    UI ui;

    while (ui.active())
        ui.execute();
}