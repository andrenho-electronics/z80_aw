#include "run.h"

#include <avr/io.h>

#include "z80.h"

#define CLK_UP   0b00001011
#define CLK_DOWN 0b00000011

void run()
{
    z80_init();

    for (;;) {
        PORTB = CLK_UP;
        PORTB = CLK_DOWN;
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
