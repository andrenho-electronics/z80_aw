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
