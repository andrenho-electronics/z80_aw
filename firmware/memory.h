#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdint.h>

uint16_t memory_read_addr();
uint8_t  memory_read_data();

uint16_t memory_read(uint16_t addr);

/*static*/ void set_addr(uint16_t addr);   // TODO

#endif

// vim:ts=4:sts=4:sw=4:expandtab
