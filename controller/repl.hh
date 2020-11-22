#ifndef REPL_HH_
#define REPL_HH_

#include <stdint.h>

#include "io.hh"
#include "serial.hh"

class Repl {
public:
    Repl(Serial const& serial, IO const& io);
    void welcome() const;
    void execute();

private:
    Serial const& serial;
    IO     const& io;

    void print_input() const;
};

#endif

// vim:ts=4:sts=4:sw=4:expandtab
