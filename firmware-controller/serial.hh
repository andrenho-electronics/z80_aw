#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdint.h>

class Serial {
public:
    static Serial init();

    void    send(uint8_t byte) const;
    uint8_t recv() const;
    uint8_t recv_noblock() const;

    void    clrscr() const;
};

#endif
