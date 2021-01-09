#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdbool.h>
#include <stdint.h>
#if !SIMULATOR
#  include <avr/pgmspace.h>
#else
#  define PGM_P const char*
#endif

void     serial_init();

void     serial_send(uint8_t byte);
void     serial_send16(uint16_t word);
uint8_t  serial_recv();
uint16_t serial_recv16();

#endif

// vim:ts=4:sts=4:sw=4:expandtab
