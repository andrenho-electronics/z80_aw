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

    uint16_t AF() const override { return z80_.AF.W; }
    uint16_t BC() const override { return z80_.BC.W; }
    uint16_t DE() const override { return z80_.DE.W; }
    uint16_t HL() const override { return z80_.HL.W; }
    uint16_t AFx() const override { return z80_.AF1.W; }
    uint16_t BCx() const override { return z80_.BC1.W; }
    uint16_t DEx() const override { return z80_.DE1.W; }
    uint16_t HLx() const override { return z80_.HL1.W; }
    uint16_t IX() const override { return z80_.IX.W; }
    uint16_t IY() const override { return z80_.IY.W; }
    uint16_t PC() const override { return z80_.PC.W; }
    uint16_t SP() const override { return z80_.SP.W; }
    uint8_t I() const override { return z80_.I; }
    uint8_t R() const override { return z80_.R; }
    bool    HALT() const override { return z80_.IFF & IFF_HALT; }

    void step() override;

    void reset() override;

private:
    Z80 z80_;
    uint32_t memory_[64 * 1024];
};

#endif //EMULATOR_EMULATEDHARDWARE_HH
