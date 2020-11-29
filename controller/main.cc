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
    serial.printhex(io.read_rom(0), 2); serial.puts();
    io.set_rom(0, 0x12);
    serial.printhex(io.read_rom(0), 2); serial.puts();

    while (1)
        repl.execute();
}

// vim:ts=4:sts=4:sw=4:expandtab
