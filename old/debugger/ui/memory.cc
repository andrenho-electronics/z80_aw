#include "memory.hh"
#include "../hardware/hardware.hh"

void Memory::redraw() const
{
    mvwprintw(window_, 1, 9, "_0 _1 _2 _3 _4 _5 _6 _7   _8 _9 _A _B _C _D _E _F");
    for (int i = 0; i < 16; ++i)
        mvwprintw(window_, i + 2, 5, "%X_", i);
    mvwprintw(window_, 1, 82, "Stack:");
    Window::redraw();
}

void Memory::update() const
{
    wattr_on(window_, COLOR_MEMORY, nullptr);

    // memory
    std::vector<uint8_t> data = hardware->get_memory(page * 0x100, 256);
    size_t i = 0;
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            uint16_t addr = x + (y * 0x10) + (page * 0x100);
            uint8_t c = data[i];
            if (addr == hardware->registers().PC)
                wattr_on(window_, COLOR_FIELD, nullptr);
            mvwprintw(window_, y + 2, 9 + (x * 3) + (x > 7 ? 2 : 0), "%02X", c);
            if (addr == hardware->registers().PC)
                wattr_on(window_, COLOR_MEMORY, nullptr);
            mvwprintw(window_, y + 2, 62 + x + (x > 7 ? 1 : 0), "%c", (c >= 32 && c < 127) ? c : '.');
            ++i;
        }
    }

    // stack
    constexpr size_t STACK_ITEMS = 12;
    std::vector<uint8_t> stack = hardware->get_memory(hardware->registers().SP, STACK_ITEMS * 2);
    for (size_t i = 0; i < 12; ++i) {
        uint8_t a = stack.at(i * 2);
        uint8_t b = stack.at(i * 2 + 1);
        mvwprintw(window_, i + 3, 83, "%04X", (b << 8) | a);
    }
    wattr_off(window_, COLOR_MEMORY, nullptr);
    
    // memory areas
    if (!hardware->matching_upload_checksum()) {
        wattr_on(window_, COLOR_WARNING, nullptr);
        wattr_on(window_, A_BLINK, nullptr);
        mvwprintw(window_, 1, 62, "Upload required!");
        wattr_off(window_, COLOR_WARNING, nullptr);
        wattr_off(window_, A_BLINK, nullptr);
    } else {
        mvwprintw(window_, 1, 62, "                ");
    }

    for (int i = 0; i < 16; ++i)
        mvwprintw(window_, i + 2, 3, "%02X", page);

    wrefresh(window_);
}

void Memory::change_page(int rel)
{
    page += rel;
    update();
}

void Memory::update_page(uint8_t page_)
{
    this->page = page_;
    update();
}

void Memory::set_running() const
{
    werase(window_);
    Window::redraw();
    wrefresh(window_);
}
