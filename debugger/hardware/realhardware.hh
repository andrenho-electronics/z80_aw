#ifndef DEBUGGER_REALHARDWARE_HH
#define DEBUGGER_REALHARDWARE_HH

#include "hardware.hh"

class RealHardware : public Hardware {
public:
    explicit RealHardware(std::string const& serial_port);
    
    void                 set_memory(uint16_t addr, uint8_t data) override;
    uint8_t              get_memory(uint16_t addr) override;
    std::vector<uint8_t> get_memory(uint16_t addr, uint16_t sz) override;
    
    void reset() override;
    void step() override;
    
    uint16_t AF() const override { return 0; }
    uint16_t BC() const override { return 0; }
    uint16_t DE() const override { return 0; }
    uint16_t HL() const override { return 0; }
    uint16_t AFx() const override { return 0; }
    uint16_t BCx() const override { return 0; }
    uint16_t DEx() const override { return 0; }
    uint16_t HLx() const override { return 0; }
    uint16_t IX() const override { return 0; }
    uint16_t IY() const override { return 0; }
    uint16_t PC() const override { return 0; }
    uint16_t SP() const override { return 0; }
    uint8_t I() const override { return 0; }
    uint8_t R() const override { return 0; }
    bool HALT() const override { return false; }

private:
    int fd = 0;
    
    void open_serial_port(std::string const& serial_port);
    bool send_expect(uint8_t data, uint8_t expected) const;
    std::vector<uint8_t> send(std::vector<uint8_t> const& data, size_t expect) const;
};

#endif
