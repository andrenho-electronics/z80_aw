#ifndef SERIAL_HH_
#define SERIAL_HH_

#include "proto/messages.pb.h"

class Serial {
public:
    Serial(const char* port);
    ~Serial();

    Reply request(Request const& request) const;

    void set_log_bytes(bool v) { log_bytes_ = v; }

private:
    int  fd = -1;
    bool log_bytes_ = false;

    std::string talk(std::string const& data) const;
};

#endif
