#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdbool.h>
#include <stdint.h>
#include <avr/pgmspace.h>

void     serial_init();

void     serial_send(uint8_t byte);
uint8_t  serial_recv();
uint16_t serial_recv16();

void     serial_spaces(int n);
void     serial_printstr(PGM_P s);
void     serial_puts();
void     serial_putsstr(PGM_P s);
void     serial_printint(int data);
void     serial_printhex4(uint8_t data);
void     serial_printhex8(uint8_t data);
void     serial_printhex16(uint16_t data);
void     serial_printbit(bool v);

uint16_t serial_inputhex(unsigned num_digits);

#endif

// vim:ts=4:sts=4:sw=4:expandtab
