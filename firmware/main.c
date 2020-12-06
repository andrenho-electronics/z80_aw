#include "serial.h"
#include "ansi.h"
#include "lowlevel.h"

int main()
{
    serial_init();
    serial_print(ANSI_CLRSCR);

    set_OE_595(1);
    serial_printhex8(get_OE_595());

    for (;;) {
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
