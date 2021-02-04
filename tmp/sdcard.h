#ifndef SDCARD_H_
#define SDCARD_H_

#include <stdint.h>

#include "sdcard_ll.h"

void    sdcard_setup();
void    sdcard_initialize();
R1      sdcard_set_spi_mode();
R7      sdcard_if_cond();
R1      sdcard_init_process();
R3      sdcard_get_info();
R1      sdcard_read_block(uint32_t block, uint8_t* data);
R1      sdcard_write_block(uint32_t block, uint8_t* data);

#endif
