#ifndef SDCARD_LL_H_
#define SDCARD_LL_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct __attribute__((packed)) {
    bool idle        : 1;
    bool erase_reset : 1;
    bool illegal_cmd : 1;
    bool crc_error   : 1;
    bool erase_seq   : 1;
    bool address_err : 1;
    bool param_err   : 1;
} R1;

typedef struct __attribute__((packed)) {
    R1       r1;
    uint32_t ocr;
} R7;

void    sd_setup();

void    sd_cs(bool enabled);
uint8_t sd_send_spi_byte(uint8_t byte);
uint8_t sd_recv_spi_byte();

R1 sd_command_r1(uint8_t cmd, uint32_t args, uint8_t crc);
R7 sd_command_r7(uint8_t cmd, uint32_t args, uint8_t crc);

#endif
