#ifndef SERIAL_HH_
#define SERIAL_HH_

#include <stddef.h>
#include <stdint.h>

class Serial {
public:
    Serial();

    void putc(char c) const;
    char getc() const;

    void getline(char* buf, size_t sz, bool echo) const;
    void puts(const char* text = "") const;

    void print(const char* text) const;
    void printhex(uint16_t value, uint8_t sz) const;
};

void waitk();

#endif

// vim:ts=4:sts=4:sw=4:expandtab
