#ifndef IO_HH_
#define IO_HH_

#include <stdint.h>

struct Inputs {
    uint8_t  data;
    uint16_t addr;
    bool     mreq;
    bool     wr;
    bool     rd;
    bool     m1;
    bool     iorq;
    bool     halt;
    bool     busack;
};

class IO {
public:
    Inputs   read_inputs() const;
    
    void     set_high_impedance() const;

    void     set_addr(uint16_t) const;
    uint16_t read_addr() const;

    void     set_rom(uint16_t addr, uint8_t data) const;
    uint8_t  read_rom(uint16_t addr) const;

private:
    uint8_t  read_data() const;
    void     set_data(uint8_t data) const;
};

#endif

// vim:ts=4:sts=4:sw=4:expandtab
