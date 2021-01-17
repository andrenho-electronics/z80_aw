#ifndef LIBZ80AW_Z80STATE_HH
#define LIBZ80AW_Z80STATE_HH

#include <cstdint>

#include "z80aw.hh"

using Registers = z80aw::Registers;

struct Z80State {
    enum Mode { Stopped, Running };
    
    uint16_t                 pc = 0;
    Mode                     mode = Stopped;
    std::optional<Registers> registers {};
};

#endif
