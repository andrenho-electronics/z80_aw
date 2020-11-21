#ifndef IO_HH_
#define IO_HH_

#include <stdint.h>

struct CpuFlagsOut {
    bool halt   : 1;
    bool mreq   : 1;
    bool ioreq  : 1;
    bool m1     : 1;
    bool busack : 1;
    bool wr     : 1;
    bool rd     : 1;
};

struct CpuFlagsIn {
    bool int_   : 1;
    bool nmi    : 1;
    bool reset  : 1;
    bool busreq : 1;
    bool wait   : 1;
    bool wr     : 1;
    bool rd     : 1;
    bool mreq   : 1;
};

struct Status {
    uint16_t    addr;
    uint8_t     data;
    CpuFlagsOut flags;
};


class IO {
public:
    IO();

    Status read_status() const;

    void write_data(uint8_t data) const;
    void write_addr(uint16_t addr) const;
    void write_flags(CpuFlagsIn flags) const;
    
    void set_high_impedance() const;
    void disable_flags_output() const;

    uint8_t     read_data() const;
    uint16_t    read_addr() const;
    CpuFlagsOut read_parallel() const;
};

#endif

// vim:ts=4:sts=4:sw=4:expandtab
