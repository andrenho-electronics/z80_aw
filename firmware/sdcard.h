#ifndef SDCARD_H_
#define SDCARD_H_

#include <stdint.h>

#include "sdcard_ll.h"
#include "../common/sdcardstage.h"

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

void    sdcard_set_last_status(SDCardStage stage, R1 r1);

SDCardStage sdcard_last_stage();
R1          sdcard_last_response();

bool sdcard_init();
bool sdcard_load_boot();
bool sdcard_read_block(uint32_t block, void(*rd)(uint16_t idx, uint8_t byte, void* data), void* data);
bool sdcard_write_block(uint32_t block, uint8_t(*wd)(uint16_t idx, void* data), void* data);

#endif
