#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdbool.h>
#include <stdint.h>

void     serial_init();

void     serial_send(uint8_t byte);
uint8_t  serial_recv();

void     serial_spaces(int n);
void     serial_print(const char* s);
void     serial_printhex8(uint8_t data);
void     serial_printhex16(uint16_t data);
void     serial_printbit(bool v);

uint16_t serial_inputhex();

#define  serial_puts(V) serial_print(V "\r\n")

#endif

// vim:ts=4:sts=4:sw=4:expandtab
