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
    z80_clock_cycle(false);

    serial_printstr(PSTR(ANSI_CLRSCR));
    serial_putsstr(PSTR("Welcome to Z80-AW debugger. Type 'h' for help."));

    for (;;)
        repl_exec();
}

// vim:ts=4:sts=4:sw=4:expandtab
