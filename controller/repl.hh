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

    uint16_t cycle = 0;

    void print_instructions() const;
    void print_input() const;

    void test_addr() const;
    void test_rom() const;
};

#endif

// vim:ts=4:sts=4:sw=4:expandtab
