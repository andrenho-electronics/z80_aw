#ifndef LIBZ80AW_V2_REALHARDWARE_HH
#define LIBZ80AW_V2_REALHARDWARE_HH

#include <string>

#include "z80aw.hh"

class RealHardware : public Z80AW {
public:
    explicit RealHardware(std::string const& serial_port);
    ~RealHardware() override;
    
    uint16_t free_mem() const override;
    
    void set_log_bytes(bool v) override { serial_.set_log_bytes(v); }
    void set_log_messages(bool v) override { serial_.set_log_message(v); }

private:
    Serial serial_;
};

#endif
