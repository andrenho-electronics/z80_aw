#include <string.h>

#include <util/delay.h>

#include "io.hh"
#include "serial.hh"
#include "repl.hh"

int main()
{
    IO     io;
    Serial serial;
    Repl   repl(serial, io);

    while (1)
        repl.execute();
}

// vim:ts=4:sts=4:sw=4:expandtab
