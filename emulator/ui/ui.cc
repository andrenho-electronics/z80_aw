#include "ui.hh"

#include <curses.h>

void UI::init_curses()
{
    initscr();
#if WIN32
    resize_term(44, 150);
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
        init_pair(8, COLOR_GREEN, COLOR_BLUE);
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
        case KEY_NPAGE: case 60498:
            memory.change_page(-1);
            break;
        case KEY_PPAGE: case 60499:
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
        case 'k':
            terminal.ask_keypress();
            redraw();
            update();
            break;
        case 's':
            step();
            break;
        case 'c':
            run();
            break;
        case 'n':
            if (hardware->next_is_subroutine()) {
                hardware->add_breakpoint_next();
                run();
            } else {
                step();
            }
            break;
        case 'q':
            active_ = false;
            break;
    }
}

void UI::redraw()
{
    draw_status_bar();

    source.resize(0, 0, LINES - 20, COLS - 60);
    terminal.resize(0, COLS - 60, LINES - 20, 0);
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
    attrset(COLOR_FIELD); printw("Add/remove breakpoint ");
    attrset(COLOR_TERMINAL); printw(" k ");
    attrset(COLOR_FIELD); printw("Keypress ");
    attrset(COLOR_TERMINAL); printw(" s ");
    attrset(COLOR_FIELD); printw("Step ");
    attrset(COLOR_TERMINAL); printw(" n ");
    attrset(COLOR_FIELD); printw("Next ");
    attrset(COLOR_TERMINAL); printw(" c ");
    attrset(COLOR_FIELD); printw("Continue ");
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

void UI::step()
{
    hardware->step();
    source.pc_updated();
    update();
}

void UI::run()
{
    source.set_running();
    memory.set_running();
    status.set_running();

    // redraw status bar
    move(LINES - 1, 0);
    clrtoeol();
    attrset(COLOR_TERMINAL); printw("Ctrl+C ");
    attrset(COLOR_FIELD); printw(" Stop execution ");

    // run
    nodelay(stdscr, TRUE);
    hardware->step();  // skip current breakpoint
    while (!hardware->is_breakpoint(hardware->PC())) {
        hardware->step();
        int ch = getch();
        if (ch == 3) { // CTRL + C
            break;
        } else if (ch != EOF) {
            terminal.keypress(translate_char(ch));
            for (int i = 0; i < 100; ++i)  // allow time for the keypress to be registered
                hardware->step();
        }
    }

    // execution stopped, restore everything
    nodelay(stdscr, FALSE);
    redraw();
    source.pc_updated();
    update();
    draw_status_bar();
}

uint8_t UI::translate_char(int ch)
{
    // printf("%0X\n", ch);
    switch (ch) {
        case KEY_ENTER:
        case 10:
            return 13;
        default:
            return ch;
    }
}
