#include "emulatedhardware.hh"

EmulatedHardware::EmulatedHardware()
    : z80_({}), memory_()
{
    z80_.TrapBadOps = 1;
    z80_.User = this;
    ResetZ80(&z80_);
}

void EmulatedHardware::step()
{
    RunZ80(&z80_);
}

void EmulatedHardware::reset()
{
    z80_ = {};
    z80_.TrapBadOps = 1;
    z80_.User = this;
    ResetZ80(&z80_);
}

std::vector<uint8_t> EmulatedHardware::get_memory(uint16_t addr, uint16_t sz)
{
    std::vector<uint8_t> r;
    std::copy(&memory_[addr], &memory_[addr + sz], std::back_inserter(r));
    return r;
}
