#include "ui.hh"

#include <curses.h>

void UI::init_curses()
{
    initscr();
#if WIN32
    resize_term(44, 130);
#endif
    noecho();
    keypad(stdscr, true);
    refresh();

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLUE);
    }
}


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

void UI::redraw()
{
    status.resize(LINES - 20, COLS - 25);
    memory.resize(LINES - 20, 0, 0, COLS - 25);
    source.resize(0, 0, LINES - 20, COLS / 2);
    terminal.resize(0, COLS / 2, LINES - 20, 0);

    status.redraw();
    memory.redraw();
    source.redraw();
    terminal.redraw();
}
