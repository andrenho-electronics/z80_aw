#include "memory.hh"

void Memory::redraw() const
{
    mvwprintw(window_, 1, 2, "Page: [< PgDown]      [PgUp >]");
    mvwprintw(window_, 2, 9, "_0 _1 _2 _3 _4 _5 _6 _7   _8 _9 _A _B _C _D _E _F");
    for (int i = 0; i < 16; ++i)
        mvwprintw(window_, i + 3, 5, "%X_", i);
    Window::redraw();
}
