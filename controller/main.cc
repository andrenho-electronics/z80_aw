#include "io.hh"
#include "serial.hh"
#include "repl.hh"

int main()
{
    IO     io;
    Serial serial;
    Repl   repl(serial, io);

    CpuFlagIn cfi;
    uint8_t f = 0xf1;
    memcpy(&cfi, &f, 1);
    io.write_flags(cfi);

    while (1)
        ;//repl.execute();
}

// vim:ts=4:sts=4:sw=4:expandtab
