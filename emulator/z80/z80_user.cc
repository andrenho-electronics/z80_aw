#include "Z80.h"

#include "../hardware/hardware.hh"

void WrZ80(word Addr,byte Value)
{
    hardware->set_memory(Addr, Value);
}

byte RdZ80(word Addr)
{
    return hardware->get_memory(Addr);
}

void OutZ80(word Port,byte Value)
{
}

byte InZ80(word Port)
{
    return 0;
}

word LoopZ80(Z80 *R)
{
    return INT_QUIT;
}

void PatchZ80(Z80 *R)
{
}
