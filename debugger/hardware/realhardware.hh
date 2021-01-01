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
    
    void reset() override;
    void step() override;
    void upload(std::function<void(double)> on_progress) override;
    
    void update_registers() override;
    
    void register_keypress(uint8_t key) override;

private:
    void open_serial_port(std::string const& serial_port);
    bool send_expect(uint8_t data, uint8_t expected) const;
    std::vector<uint8_t> send(std::vector<uint8_t> const& data, size_t expect) const;
    
    int fd = 0;
    uint16_t pc_ = 0x0;
    mutable std::optional<std::ofstream> logfile_;
    
    static uint16_t calculate_checksum(std::vector<uint8_t> const& data);
};

#endif
