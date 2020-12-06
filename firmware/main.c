#include "serial.h"
#include "ansi.h"
#include "lowlevel.h"

int main()
{
    serial_init();
    serial_print(ANSI_CLRSCR);

    for (;;) {
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
