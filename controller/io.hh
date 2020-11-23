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
    void   set_high_impedance() const;
    Inputs read_inputs() const;

private:
    uint8_t  read_data() const;
    uint16_t read_addr() const;
};

#endif

// vim:ts=4:sts=4:sw=4:expandtab
