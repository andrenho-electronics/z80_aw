#ifndef SDCARD_H_
#define SDCARD_H_

#include <stdint.h>

void    sdcard_setup();
void    sdcard_initialize();
uint8_t sdcard_set_spi_mode();

#endif
