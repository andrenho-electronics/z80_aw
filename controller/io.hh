#ifndef IO_HH_
#define IO_HH_

#include <stdint.h>

struct Status {
    uint16_t addr;
    uint8_t  data;
};

class IO {
public:
    IO();

    Status get_status() const;

private:
    enum class State { Input, Output, HighImpedance };
    void   set_state(State state) const;

    uint16_t get_addr() const;
};

#endif

// vim:ts=4:sts=4:sw=4:expandtab
