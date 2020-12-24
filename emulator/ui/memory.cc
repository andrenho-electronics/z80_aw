#include "memory.hh"
#include "../hardware/hardware.hh"

void Memory::redraw() const
{
    mvwprintw(window_, 1, 9, "_0 _1 _2 _3 _4 _5 _6 _7   _8 _9 _A _B _C _D _E _F");
    for (int i = 0; i < 16; ++i)
        mvwprintw(window_, i + 2, 5, "%X_", i);
    Window::redraw();
}

void Memory::update() const
{
    wattr_on(window_, COLOR_MEMORY, nullptr);
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            uint16_t addr = x + (y * 0x10) + (page * 0x100);
            uint8_t c = hardware->get_memory(addr);
            if (addr == hardware->PC())
                wattr_on(window_, COLOR_FIELD, nullptr);
            mvwprintw(window_, y + 2, 9 + (x * 3) + (x > 7 ? 2 : 0), "%02X", c);
            if (addr == hardware->PC())
                wattr_on(window_, COLOR_MEMORY, nullptr);
            mvwprintw(window_, y + 2, 62 + x + (x > 7 ? 1 : 0), "%c", (c >= 32 && c < 127) ? c : '.');
        }
    }

    wattr_off(window_, COLOR_MEMORY, nullptr);
    for (int i = 0; i < 16; ++i)
        mvwprintw(window_, i + 2, 3, "%02X", page);

    wrefresh(window_);
}
