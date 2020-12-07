#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdint.h>

uint16_t memory_read_addr();
uint8_t  memory_read_data();

uint16_t memory_read(uint16_t addr);
void     memory_read_page(uint8_t page, uint8_t data[0x100]);

#endif

// vim:ts=4:sts=4:sw=4:expandtab
