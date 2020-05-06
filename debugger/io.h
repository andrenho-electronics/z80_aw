#ifndef IO_H_
#define IO_H_

#include <stdint.h>

void    io_init();
uint8_t io_read(uint16_t addr);
void    io_write(uint16_t addr, uint8_t data);

void    io_test();  // TODO - temporary

#endif

// vim:ts=4:sts=4:sw=4:expandtab
