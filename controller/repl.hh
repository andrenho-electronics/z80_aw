#ifndef REPL_HH_
#define REPL_HH_

#include <stdint.h>

#include "io.hh"
#include "serial.hh"

class Repl {
public:
    Repl(Serial const& serial, IO const& io) : serial(serial), io(io) {}
    void execute();

private:
    Serial const& serial;
    IO     const& io;

    struct Command {
        char      cmd;
        size_t    n_params = 0;
        uint16_t* params   = nullptr;
    };

    Command parse_command(const char* str) const;

    void print_help() const;
    void print_cpu_status() const;
};

#endif

// vim:ts=4:sts=4:sw=4:expandtab
