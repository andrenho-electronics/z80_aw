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
        init_pair(5, COLOR_BLACK, COLOR_RED);
    }
}


UI::UI()
{
    redraw();
    source.pc_updated();
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
    source.resize(0, 0, LINES - 20, COLS / 2);
    terminal.resize(0, COLS / 2, LINES - 20, 0);
    status.resize(LINES - 20, COLS - 25, 19);
    memory.resize(LINES - 20, 0, 19, COLS - 25);

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
