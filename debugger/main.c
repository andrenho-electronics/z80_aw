#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "serial.h"

void
print_help()
{
    ser_printstr("h: help\r\n");
    ser_printstr("w ADDR DATA: write byte to memory\r\n");
    ser_printstr("r ADDR: read byte from memory\r\n");
    ser_printstr("data format is uppercase hexa (ex. 0C AF 12)\r\n");
}

int
main()
{
    ser_init();
    ser_printstr("\x1b[2J\x1b[HType 'h' for help'.\r\n");

    for (;;) {
        unsigned data1, data2;
        ser_printstr("? ");
        switch (ser_input(&data1, &data2)) {
            case 'h':
                print_help();
                break;
            default:
                ser_printstr("Syntax error.\r\n");
                break;
        }
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
