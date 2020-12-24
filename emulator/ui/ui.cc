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
        init_pair(2, COLOR_BLACK, COLOR_CYAN);
        init_pair(3, COLOR_CYAN, COLOR_BLUE);
        init_pair(4, COLOR_YELLOW, COLOR_BLUE);
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
    status.resize(LINES - 19, COLS - 25);
    memory.resize(LINES - 19, 0, 0, COLS - 25);
    source.resize(0, 0, LINES - 19, COLS / 2);
    terminal.resize(0, COLS / 2, LINES - 19, 0);

    status.redraw();
    memory.redraw();
    source.redraw();
    terminal.redraw();
}

void UI::update()
{
    status.update();
    memory.update();
    source.update();
    terminal.update();
}
