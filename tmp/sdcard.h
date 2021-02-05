#ifndef SDCARD_H_
#define SDCARD_H_

#include <stdint.h>

#include "sdcard_ll.h"

typedef struct __attribute__((packed)) {
    bool idle        : 1;
    bool erase_reset : 1;
    bool illegal_cmd : 1;
    bool crc_error   : 1;
    bool erase_seq   : 1;
    bool address_err : 1;
    bool param_err   : 1;
} R1_Reg;

typedef union {
    R1_Reg  r1;
    uint8_t value;
} R1;

typedef enum {
    SD_MCU_SETUP   = 0x0, SD_RESET, SD_GO_IDLE, SD_IF_COND, SD_INIT,
    SD_GET_OCR     = 0x10,
    SD_READ_OK     = 0x20, SD_READ_REJECTED, SD_READ_TIMEOUT, SD_READ_CRC_FAILED,
    SD_WRITE_OK    = 0x30, SD_WRITE_REJECTED, SD_WRITE_TIMEOUT, SD_WRITE_DATA_REJECTED, SD_WRITE_DATA_TIMEOUT,
    SD_NOT_INITIALIZED = 0xff,
} SDCardStage;

void    sdcard_set_last_status(SDCardStage stage, R1 r1);

SDCardStage sdcard_last_stage();
R1          sdcard_last_response();

bool sdcard_init();
bool sdcard_read_block(uint32_t block, void(*rd)(uint16_t idx, uint8_t byte, void* data), void* data);
bool sdcard_write_block(uint32_t block, uint8_t(*wd)(uint16_t idx, void* data), void* data);

/*
void    sdcard_setup();
void    sdcard_initialize();
R1      sdcard_set_spi_mode();
R7      sdcard_if_cond();
R1      sdcard_init_process();
R3      sdcard_get_info();
R1      sdcard_read_block(uint32_t block, uint8_t* data);
R1      sdcard_write_block(uint32_t block, uint8_t* data);
*/

#endif
