#ifndef DEBUGGER_REALHARDWARE_HH
#define DEBUGGER_REALHARDWARE_HH

#include "hardware.hh"

#include <fstream>

class RealHardware : public Hardware {
public:
    explicit RealHardware(std::string const& serial_port, std::optional<std::string> const& log_file);
    
    void                 set_memory(uint16_t addr, uint8_t data) override;
    uint8_t              get_memory(uint16_t addr) const override;
    std::vector<uint8_t> get_memory(uint16_t addr, uint16_t sz) const override;
    
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
    uint16_t PC() const override { return pc_; }
    uint16_t SP() const override { return 0; }
    uint8_t I() const override { return 0; }
    uint8_t R() const override { return 0; }
    bool HALT() const override { return false; }
    
    void reset() override;
    void step() override;
    void upload() override;

private:
    void open_serial_port(std::string const& serial_port);
    bool send_expect(uint8_t data, uint8_t expected) const;
    std::vector<uint8_t> send(std::vector<uint8_t> const& data, size_t expect) const;
    
    int fd = 0;
    uint16_t pc_ = 0x0;
    mutable std::optional<std::ofstream> logfile_;
};

#endif
