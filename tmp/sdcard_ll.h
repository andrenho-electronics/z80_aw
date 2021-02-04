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
} R1_Reg;

typedef struct __attribute__((packed)) {
    R1_Reg   r1;
    uint32_t ocr;
} R3_Reg;

typedef struct __attribute__((packed)) {
    R1_Reg   r1;
    uint8_t  cmd_version : 4;
    uint16_t reserved    : 16;
    uint8_t  voltage     : 4;
    uint8_t  check       : 8;
} R7_Reg;

typedef union {
    R1_Reg  r1;
    uint8_t value;
} R1;

typedef union {
    R3_Reg   r3;
    uint8_t  bytes[5];
    uint64_t value;
} R3;

typedef union {
    R7_Reg   r7;
    uint8_t  bytes[5];
    uint64_t value;
} R7;

void    sd_setup();

void    sd_cs(bool enabled);
uint8_t sd_send_spi_byte(uint8_t byte);
uint8_t sd_recv_spi_byte();

R1 sd_command_r1(uint8_t cmd, uint32_t args, uint8_t crc);
R3 sd_command_r3(uint8_t cmd, uint32_t args, uint8_t crc);
R7 sd_command_r7(uint8_t cmd, uint32_t args, uint8_t crc);
R1 sd_command_read_block(uint8_t cmd, uint32_t block, uint8_t* data);

#endif
