#include <avr/interrupt.h>
#include <avr/io.h>

#include "dbg.h"
#include "io.h"
#include "memory.h"
#include "serial.h"
#include "sdcard.h"
#include "z80.h"

int main()
{
    io_init();

    // setup RX interrupt
    serial_init();

    // main loop
    for (;;) {
        if (z80_mode() == M_CONTINUE)
            z80_step();
        debugger_cycle();
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
