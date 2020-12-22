#include <curses.h>

int main()
{
    initscr();
    printw("Hello world!");
    refresh();
    getch();
    endwin();
}