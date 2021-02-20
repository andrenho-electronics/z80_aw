#ifndef Z80AW_HH_
#define Z80AW_HH_

#include <cstdint>

#include "serial.hh"

class Z80AW {
public:
    virtual ~Z80AW() = default;
    virtual uint16_t free_mem() const = 0;
    
    virtual void set_log_bytes(bool) {}
    virtual void set_log_messages(bool) {}
};

#endif
