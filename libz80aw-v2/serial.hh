#ifndef SERIAL_HH_
#define SERIAL_HH_

class Serial {
public:
    Serial(const char* port);
    ~Serial();

private:
    int fd = -1;
};

#endif
