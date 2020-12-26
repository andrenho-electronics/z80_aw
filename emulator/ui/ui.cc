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
        init_pair(0, COLOR_WHITE, COLOR_BLACK);
        init_pair(1, COLOR_WHITE, COLOR_BLUE);
        init_pair(2, COLOR_BLACK, COLOR_CYAN);
        init_pair(3, COLOR_CYAN, COLOR_BLUE);
        init_pair(4, COLOR_YELLOW, COLOR_BLUE);
        init_pair(5, COLOR_BLACK, COLOR_RED);
        init_pair(6, COLOR_BLACK, COLOR_WHITE);
        init_pair(7, COLOR_WHITE, COLOR_RED);
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
    // printf("%d\n", ch);
    switch (ch) {
        case KEY_PPAGE: case 60499:
            memory.change_page(-1);
            break;
        case KEY_NPAGE: case 60498:
            memory.change_page(1);
            break;
        case KEY_UP: case 60419:
            source.move_cursor(-1);
            break;
        case KEY_DOWN: case 60418:
            source.move_cursor(1);
            break;
        case 'g':
            memory.update_page(ask("New page?"));
            draw_status_bar();
            break;
        case 'f':
            source.choose_file();
            redraw();
            update();
            break;
        case 'b':
            source.swap_breakpoint();
            break;
        case 'q':
            active_ = false;
            break;
    }
}

void UI::redraw()
{
    draw_status_bar();

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

void UI::draw_status_bar()
{
    move(LINES - 1, 0);
    clrtoeol();
    attrset(COLOR_TERMINAL); printw("PgUp/Down ");
    attrset(COLOR_FIELD); printw("Memory page ");
    attrset(COLOR_TERMINAL); printw(" g ");
    attrset(COLOR_FIELD); printw("Go to page ");
    attrset(COLOR_TERMINAL); printw(" f ");
    attrset(COLOR_FIELD); printw("Choose file ");
    attrset(COLOR_TERMINAL); printw(" b ");
    attrset(COLOR_FIELD); printw("Add/remove breakpoint");
}

long UI::ask(std::string const &question)
{
    move(LINES - 1, 0);
    clrtoeol();
    attrset(COLOR_TERMINAL);
    printw((question + " ").c_str());
    char buf[1024];
    echo();
    getnstr(buf, sizeof buf);
    noecho();
    return strtol(buf, nullptr, 0);
}
