#ifndef Z80AW_HH_
#define Z80AW_HH_

#include <cstdint>

#include "serial.hh"

class Z80AW {
public:
    Z80AW(const char* port);
    ~Z80AW();

    uint16_t free_mem() const;

private:
    Serial serial_;
};

#endif
