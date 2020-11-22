#include "repl.hh"

#define BUFSZ 256

Repl::Repl(Serial const& serial, IO const& io) 
    : serial(serial), io(io)
{
}

void
Repl::welcome() const
{
    serial.clear_screen();
    serial.puts("Welcome to Z80 controller.");
    serial.puts();
    serial.puts("Keys: [C]ycle");
    serial.puts();
    serial.puts("CYCLE    DATA ADDR MREQ WR RD M1 IORQ HALT BUSACK");
}

void
Repl::execute()
{
    print_input();
    serial.getc();
}

void
Repl::print_input() const
{
    serial.puts("00000000  00  0000   0   0  0  0   0    0     0");
}

// vim:ts=4:sts=4:sw=4:expandtab
