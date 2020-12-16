#include "ansi.h"
#include "bus.h"
#include "lowlevel.h"
#include "serial.h"
#include "repl.h"
#include "run.h"
#include "z80.h"

#include <avr/io.h>

int main()
{
    serial_init();
    bus_init();
    z80_init();
    z80_init();
    z80_powerdown();

    run();

    /*
    serial_printstr(PSTR(ANSI_CLRSCR));
    serial_putsstr(PSTR("Welcome to Z80-AW debugger. Type 'h' for help."));

    for (;;)
        repl_exec();
    */
}

// vim:ts=4:sts=4:sw=4:expandtab
