#ifndef EMULATOR_EMULATEDHARDWARE_HH
#define EMULATOR_EMULATEDHARDWARE_HH

#include <stdint-gcc.h>
#include "hardware.hh"

class EmulatedHardware : public Hardware {
public:
    EmulatedHardware() {}

    void set_memory(uint16_t addr, uint8_t data) override { }
};

#endif //EMULATOR_EMULATEDHARDWARE_HH
