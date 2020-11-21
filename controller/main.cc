#include <string.h>

#include <util/delay.h>

#include "io.hh"
#include "computer.hh"
#include "serial.hh"
#include "repl.hh"

int main()
{
    IO       io;
    Serial   serial;
    Computer computer(io);
    Repl     repl(serial, io);

    computer.write_ram(0, 0xab);
    serial.printhex(computer.read_ram(0), 2); serial.puts();
    /*
    computer.write_ram(1, 0xcd);
    serial.printhex(computer.read_ram(0), 2); serial.puts();
    serial.printhex(computer.read_ram(1), 2); serial.puts();
    */

    while (1)
        ; //repl.execute();
}

// vim:ts=4:sts=4:sw=4:expandtab
