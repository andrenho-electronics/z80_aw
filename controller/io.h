#ifndef IO_H_
#define IO_H_

#include <stdint.h>

#include "messages.h"

void io_set_high_impedance();
void io_read_inputs(uint16_t* addr, uint8_t* data, Inputs* inputs);
void io_z80_clock();

#endif

// vim:ts=4:sts=4:sw=4:expandtab
