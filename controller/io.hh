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

struct Status {
    uint16_t    addr;
    uint8_t     data;
    CpuFlagsOut flags;
};


class IO {
public:
    IO();

    Status read_status() const;

private:
    enum class State { Input, Output, HighImpedance };
    void   set_state(State state) const;

    uint16_t    read_addr() const;
    CpuFlagsOut read_parallel() const;
};

#endif

// vim:ts=4:sts=4:sw=4:expandtab
