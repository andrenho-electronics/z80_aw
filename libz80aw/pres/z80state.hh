#ifndef LIBZ80AW_Z80STATE_HH
#define LIBZ80AW_Z80STATE_HH

#include <cstdint>

struct Z80State {
    enum Mode { Stopped, Running };
    
    uint16_t pc = 0;
    Mode     mode = Stopped;
};

#endif
