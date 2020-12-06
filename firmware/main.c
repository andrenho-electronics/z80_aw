#include "serial.h"
#include "ansi.h"

int main()
{
    serial_init();
    serial_print(ANSI_CLRSCR);

    for (;;) {
        serial_print("? ");
        uint16_t data = serial_inputhex();
        serial_printhex16(data);
        serial_puts();
        // serial_send(serial_recv());
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
