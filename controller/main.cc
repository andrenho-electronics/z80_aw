#include "io.hh"
#include "serial.hh"
#include "repl.hh"

#include <util/delay.h>

int main()
{
    IO       io;
    Serial   serial;
    Repl     repl(serial, io);

    io.set_addr(0x0);
    io.set_high_impedance();

    serial.init();
    repl.welcome();

    while (1)
        repl.execute();
}

// vim:ts=4:sts=4:sw=4:expandtab
