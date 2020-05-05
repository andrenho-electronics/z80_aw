#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "io.h"
#include "repl.h"
#include "serial.h"

int
main()
{
    io_init();
    ser_init();

    ser_printstr("\x1b[2J\x1b[HType 'h' for help'.\r\n");
    for (;;) {
        repl_do();
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
