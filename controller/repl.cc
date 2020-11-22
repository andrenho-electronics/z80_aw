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

    while (true) {
        switch (serial.getc()) {
            case 'C': case 'c':
                ++cycle;
                return;
            default:
                serial.putc('\a');
        }
    }
}

void
Repl::print_input() const
{
    Inputs in = io.read_inputs();
    serial.printhex(cycle, 8);
    serial.print("  ");
    serial.printhex(in.data, 2);
    serial.print("  ");
    serial.printhex(in.addr, 4);
    serial.print("   ");
    serial.printbit(in.mreq);
    serial.print("   ");
    serial.printbit(in.wr);
    serial.print("  ");
    serial.printbit(in.rd);
    serial.print("  ");
    serial.printbit(in.m1);
    serial.print("   ");
    serial.printbit(in.iorq);
    serial.print("    ");
    serial.printbit(in.halt);
    serial.print("     ");
    serial.printbit(in.busack);
    serial.puts();
}

// vim:ts=4:sts=4:sw=4:expandtab
