#ifndef EMULATOR_EMULATEDHARDWARE_HH
#define EMULATOR_EMULATEDHARDWARE_HH

#include <stdint-gcc.h>
#include <Z80.h>
#include "hardware.hh"

class EmulatedHardware : public Hardware {
public:
    EmulatedHardware();

    void                 set_memory(uint16_t addr, uint8_t data) override { memory_[addr] = data; }
    uint8_t              get_memory(uint16_t addr) const override { return memory_[addr]; }
    std::vector<uint8_t> get_memory(uint16_t addr, uint16_t sz) const override;
    
    void step() override;
    void reset() override;
    void upload(std::function<void(double)> on_progress) override;
    
    void update_registers() override;
    
    void register_keypress(uint8_t key) override;
    
    bool keyboard_interrupt() const { return keyboard_interrupt_; }
    void clear_keyboard_interrupt() { keyboard_interrupt_ = false; }
    uint8_t last_keypress() const { return last_keypress_; }
    
    void start_running() override;
    void stop_running() override;
    void evaluate_events() override;

private:
    Z80      z80_;
    uint32_t memory_[64 * 1024];
    bool     keyboard_interrupt_;
    uint8_t  last_keypress_;
};

#endif //EMULATOR_EMULATEDHARDWARE_HH
