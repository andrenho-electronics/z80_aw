#ifndef COMPUTER_HH_
#define COMPUTER_HH_

#include "io.hh"

class Computer {
public:
    Computer(IO const& io) : io(io) {}

    void    clear_bus() const;

    uint8_t read_ram(uint16_t addr) const;
    void    write_ram(uint16_t addr, uint8_t data) const;

private:
    IO const& io;
};

#endif

// vim:ts=4:sts=4:sw=4:expandtab
