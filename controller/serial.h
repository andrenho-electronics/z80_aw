#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdint.h>

void serial_init();

void    serial_send(uint8_t byte);
uint8_t serial_recv();

#endif

// vim:ts=4:sts=4:sw=4:expandtab
