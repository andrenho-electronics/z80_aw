#ifndef Z80AW_HH_
#define Z80AW_HH_

#include "serial.hh"

class Z80AW {
public:
    Z80AW(const char* port);

private:
    Serial serial_;
};

#endif
