#include "ansi.h"
#include "bus.h"
#include "serial.h"
#include "z80.h"

int main()
{
    bus_init();
    z80_clock_cycle();

    serial_init();
    serial_print(ANSI_CLRSCR);

    for (;;) {
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
