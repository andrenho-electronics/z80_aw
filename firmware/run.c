#include "run.h"

#include <stdbool.h>
#include <avr/io.h>

#include "lowlevel.h"
#include "memory.h"
#include "serial.h"
#include "z80.h"

#define CLK_UP   0b00001011
#define CLK_DOWN 0b00000011

void run()
{
    serial_putsstr(PSTR("Welcome to the Z80-AW computer.\r\n"));

    z80_init();
    bool last_iorq = true;

    for (;;) {
        PORTB = CLK_UP;
        PORTB = CLK_DOWN;

        // check for video interrupt (TODO - move this to AVR interrupt)
        if (last_iorq == 1 && get_IORQ() == 0) {
            uint16_t addr = memory_read_addr();
            if ((addr & 0xff) == 0x00) {   // video device
                uint8_t data = addr >> 8;
                serial_send(data);
            }
        }

        last_iorq = get_IORQ();
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
