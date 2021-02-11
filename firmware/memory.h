#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

uint16_t memory_read_addr();
uint8_t  memory_read_data();

void     memory_set_data(uint8_t data);

void     memory_write(uint16_t addr, uint8_t data, bool wait_for_completion);
bool     memory_read_page(uint16_t addr, uint8_t* data, int count);
bool     memory_write_page(uint16_t addr, uint8_t* data, size_t count, uint16_t* checksum);

#endif

// vim:ts=4:sts=4:sw=4:expandtab
