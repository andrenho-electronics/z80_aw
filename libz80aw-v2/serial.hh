#ifndef SERIAL_HH_
#define SERIAL_HH_

#include "proto/messages.pb.h"

class Serial {
public:
    explicit Serial(const char* port);
    ~Serial();

    Reply request(Request const& request) const;

    void set_log_bytes(bool v) { log_bytes_ = v; }
    void set_log_message(bool v) { log_message_ = v; }

private:
    int  fd = -1;
    bool log_bytes_ = false;
    bool log_message_ = false;
    
    void send_request(Request const& request) const;
    Reply receive_reply() const;
    
    static std::string length_string_16(std::string const& data);
    static std::string checksum_string(std::string const& data);
    static std::pair<uint8_t, uint8_t> checksum(std::string const& data);
};

#endif
