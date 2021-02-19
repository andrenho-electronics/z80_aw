#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdint.h>

void    serial_init();
void    serial_send(uint8_t byte);
uint8_t serial_recv();
uint8_t serial_recv_noblock();

#endif
