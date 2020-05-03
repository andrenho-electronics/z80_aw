#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "serial.h"

int main()
{
    ser_init();
    ser_printstr("\x1b[2J\x1b[HType 'h' for help'.");

    for (;;) {
        ser_printstr("\n\r? ");
        ser_input();
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
