#include "ansi.h"
#include "bus.h"
#include "serial.h"
#include "repl.h"
#include "z80.h"

#include <avr/io.h>
#include "lowlevel.h"

int main()
{
    serial_init();
    bus_init();
    z80_clock_cycle();

    serial_print(ANSI_CLRSCR);
    serial_puts("Welcome to Z80-AW debugger. Type 'h' for help.");

    for (;;)
        repl_exec();
}

// vim:ts=4:sts=4:sw=4:expandtab
