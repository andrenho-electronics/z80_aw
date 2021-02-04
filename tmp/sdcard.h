#ifndef SDCARD_H_
#define SDCARD_H_

#include <stdint.h>

#include "sdcard_ll.h"

void    sdcard_setup();
void    sdcard_initialize();
R1      sdcard_set_spi_mode();
/*
uint8_t sdcard_init_process();
*/

#endif
