#ifndef EMULATOR_HARDWARE_HH
#define EMULATOR_HARDWARE_HH

#include <cstdint>
#include <memory>

class Hardware {
public:
    virtual ~Hardware() = default;
    virtual void set_memory(uint16_t addr, uint8_t data) = 0;

protected:
    Hardware() = default;
};

extern std::unique_ptr<Hardware> hardware;

#endif //EMULATOR_HARDWARE_HH