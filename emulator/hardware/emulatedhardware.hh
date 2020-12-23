#ifndef EMULATOR_EMULATEDHARDWARE_HH
#define EMULATOR_EMULATEDHARDWARE_HH

#include <stdint-gcc.h>
#include <Z80.h>
#include "hardware.hh"

class EmulatedHardware : public Hardware {
public:
    EmulatedHardware();

    void    set_memory(uint16_t addr, uint8_t data) override { memory_[addr] = data; }
    uint8_t get_memory(uint16_t addr) override { return memory_[addr]; }

private:
    Z80 z80_;
    uint32_t memory_[64 * 1024];
};

#endif //EMULATOR_EMULATEDHARDWARE_HH
