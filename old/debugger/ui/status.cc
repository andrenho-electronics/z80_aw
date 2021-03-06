#include "status.hh"
#include "../hardware/hardware.hh"

void Status::redraw() const
{
    mvwprintw(window_, 1, 2, "AF:");
    mvwprintw(window_, 2, 2, "BC:");
    mvwprintw(window_, 3, 2, "DE:");
    mvwprintw(window_, 4, 2, "HL:");
    mvwprintw(window_, 1, 13, "AF':");
    mvwprintw(window_, 2, 13, "BC':");
    mvwprintw(window_, 3, 13, "DE':");
    mvwprintw(window_, 4, 13, "HL':");
    mvwprintw(window_, 6, 2, "IX:");
    mvwprintw(window_, 7, 2, "IY:");
    mvwprintw(window_, 8, 2, "SP:");
    mvwprintw(window_, 9, 2, "PC:");
    mvwprintw(window_, 6, 13, "I:");
    mvwprintw(window_, 7, 13, "R:");
    mvwprintw(window_, 8, 13, "Halt: ");
    mvwprintw(window_, 11, 2, "Flags:");
    mvwprintw(window_, 12, 4, "Sign:");
    mvwprintw(window_, 13, 4, "Zero:");
    mvwprintw(window_, 14, 4, "Half-carry:");
    mvwprintw(window_, 15, 4, "Parity/Ovf:");
    mvwprintw(window_, 16, 4, "Add/Sub:");
    mvwprintw(window_, 17, 4, "Carry:");
    Window::redraw();
}

void Status::update() const
{
#define BIT(line, col, value) { if (value) wattr_on(window_, COLOR_FIELD, nullptr); else wattr_on(window_, COLOR_DISABLED, nullptr); mvwprintw(window_, line, col, "%d", value); }
    wattr_on(window_, COLOR_FIELD, nullptr);
    mvwprintw(window_, 1, 6, "%02X %02X", hardware->registers().AF >> 8, hardware->registers().AF & 0xff);
    mvwprintw(window_, 2, 6, "%02X %02X", hardware->registers().BC >> 8, hardware->registers().BC & 0xff);
    mvwprintw(window_, 3, 6, "%02X %02X", hardware->registers().DE >> 8, hardware->registers().DE & 0xff);
    mvwprintw(window_, 4, 6, "%02X %02X", hardware->registers().HL >> 8, hardware->registers().HL & 0xff);
    mvwprintw(window_, 1, 18, "%02X %02X", hardware->registers().AFx >> 8, hardware->registers().AFx & 0xff);
    mvwprintw(window_, 2, 18, "%02X %02X", hardware->registers().BCx >> 8, hardware->registers().BCx & 0xff);
    mvwprintw(window_, 3, 18, "%02X %02X", hardware->registers().DEx >> 8, hardware->registers().DEx & 0xff);
    mvwprintw(window_, 4, 18, "%02X %02X", hardware->registers().HLx >> 8, hardware->registers().HLx & 0xff);
    mvwprintw(window_, 6, 6, "%04X", hardware->registers().IX);
    mvwprintw(window_, 7, 6, "%04X", hardware->registers().IY);
    mvwprintw(window_, 8, 6, "%04X", hardware->registers().SP);
    mvwprintw(window_, 9, 6, "%04X", hardware->registers().PC);
    mvwprintw(window_, 6, 16, "%02X", hardware->registers().I);
    mvwprintw(window_, 7, 16, "%02X", hardware->registers().R);
    BIT(12, 16, (hardware->registers().SP >> 7) & 1);
    BIT(13, 16, (hardware->registers().SP >> 6) & 1);
    BIT(14, 16, (hardware->registers().SP >> 4) & 1);
    BIT(15, 16, (hardware->registers().SP >> 2) & 1);
    BIT(16, 16, (hardware->registers().SP >> 1) & 1);
    BIT(17, 16, (hardware->registers().SP >> 0) & 1);
    BIT(8, 19, hardware->registers().HALT);
    wattr_off(window_, COLOR_FIELD, nullptr);
    wrefresh(window_);
#undef BIT
}

void Status::set_running() const
{
    werase(window_);
    Window::redraw();
    wrefresh(window_);
}
