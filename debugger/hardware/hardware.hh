#ifndef EMULATOR_HARDWARE_HH
#define EMULATOR_HARDWARE_HH

#include <cstdint>
#include <memory>
#include <unordered_set>

class Hardware {
public:
    virtual ~Hardware() = default;
    virtual void    set_memory(uint16_t addr, uint8_t data) = 0;
    virtual uint8_t get_memory(uint16_t addr) = 0;

    virtual uint16_t AF() const = 0;
    virtual uint16_t BC() const = 0;
    virtual uint16_t DE() const = 0;
    virtual uint16_t HL() const = 0;
    virtual uint16_t AFx() const = 0;
    virtual uint16_t BCx() const = 0;
    virtual uint16_t DEx() const = 0;
    virtual uint16_t HLx() const = 0;
    virtual uint16_t IX() const = 0;
    virtual uint16_t IY() const = 0;
    virtual uint16_t PC() const = 0;
    virtual uint16_t SP() const = 0;
    virtual uint8_t I() const = 0;
    virtual uint8_t R() const = 0;
    virtual bool HALT() const = 0;

    virtual void reset() = 0;

    bool next_is_subroutine() const;

    void add_breakpoint(uint16_t addr);
    void add_breakpoint_next();
    void remove_breakpoint(uint16_t addr);
    bool is_breakpoint(uint16_t addr) const;

    virtual void step() = 0;

protected:
    Hardware() = default;
    std::unordered_set<uint16_t> breakpoints_;
};

extern std::unique_ptr<Hardware> hardware;

#endif //EMULATOR_HARDWARE_HH