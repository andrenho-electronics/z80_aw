#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdbool.h>
#include <stdint.h>

void     serial_init();

void     serial_send(uint8_t byte);
void     serial_send16(uint16_t word);
uint8_t  serial_recv();
uint8_t  serial_recv_noblock();
uint16_t serial_recv16();
uint32_t serial_recv24();

#endif

// vim:ts=4:sts=4:sw=4:expandtab
