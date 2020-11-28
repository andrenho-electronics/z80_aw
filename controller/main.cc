#include "io.hh"
#include "serial.hh"
#include "repl.hh"

#include <util/delay.h>

int main()
{
    IO       io;
    Serial   serial;
    Repl     repl(serial, io);

    serial.init();
    repl.welcome();

    for (uint16_t i = 0; i < 0xffff; ++i) {
        io.set_addr(i);
        _delay_ms(10);
    }

    while (1)
        repl.execute();
}

// vim:ts=4:sts=4:sw=4:expandtab
