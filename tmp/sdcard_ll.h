#ifndef SDCARD_LL_H_
#define SDCARD_LL_H_

#include <stdbool.h>
#include <stdint.h>

void    sd_setup();
void    sd_cs(bool enabled);
uint8_t sd_send_spi_byte(uint8_t byte);
uint8_t sd_recv_spi_byte();

void    sd_command(uint8_t cmd, uint32_t args, uint8_t crc);

#endif
