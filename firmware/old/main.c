#include "bus.h"
#include "dbg.h"
#include "lowlevel.h"
#include "serial.h"
#include "z80.h"

#include <avr/io.h>

int main()
{
    serial_init();
    bus_init();
    z80_init();
    z80_init();
    z80_powerdown();

    for (;;)
        dbg_exec();
}

// vim:ts=4:sts=4:sw=4:expandtab
