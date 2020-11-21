#include "io.hh"
#include "serial.hh"
#include "repl.hh"

int main()
{
    IO     io;
    Serial serial;
    Repl   repl(serial, io);

    io.write_addr(0xabcd);

    while (1)
        ;//repl.execute();
}

// vim:ts=4:sts=4:sw=4:expandtab
