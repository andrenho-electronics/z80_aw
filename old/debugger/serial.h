#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdint.h>

void ser_init();
void set_printchar(char c);
void ser_printstr(const char* text);
void ser_printhex(unsigned value, int digits);
char ser_input(unsigned* data1, unsigned* data2, int* pars);
uint8_t ser_inputhex();

#endif

// vim:ts=4:sts=4:sw=4:expandtab
