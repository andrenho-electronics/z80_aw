#ifndef IO_H_
#define IO_H_

#include <stdint.h>

#include "messages.h"

void io_init();
void io_z80_clock();
void io_z80_reset();
void io_z80_init();

void io_read_inputs(uint16_t* addr, uint8_t* data, Inputs* inputs);

uint8_t io_read_memory(uint16_t addr);
void    io_write_memory(uint16_t addr, uint8_t data);

#endif

// vim:ts=4:sts=4:sw=4:expandtab
