#include "Z80.h"

#include "../hardware/hardware.hh"
#include "../hardware/emulatedhardware.hh"

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
    if ((Port & 0xff) == 0x0)
        hardware->print_char(Value);
}

byte InZ80(word Port)
{
    if ((Port & 0xff) == 0x1) {
        auto* eh = dynamic_cast<EmulatedHardware*>(hardware.get());
        return eh->last_keypress();
    }
    return 0;
}

word LoopZ80(Z80 *R)
{
    (void) R;
    
    auto* eh = dynamic_cast<EmulatedHardware*>(hardware.get());
    if (eh->keyboard_interrupt()) {
        eh->clear_keyboard_interrupt();
        return 0xcf;
    }
    return INT_QUIT;
}

void PatchZ80(Z80 *R)
{
    (void) R;
}
