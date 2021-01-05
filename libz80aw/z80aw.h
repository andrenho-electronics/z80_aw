#ifndef Z80AW_H
#define Z80AW_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "compiler.h"

typedef struct {
    const char* serial_port;
    bool        log_to_stdout;
} Z80AW_Config;

typedef struct {
    uint16_t free_memory;
} Z80AW_ControllerInfo;

#define MAX_BLOCK_SIZE 512
#define UPLOAD_CHECKSUM_LOCATION 0x7ffe

void z80aw_init(Z80AW_Config* cfg);
void z80aw_close();

Z80AW_ControllerInfo z80aw_controller_info();

int z80aw_write_byte(uint16_t addr, uint8_t data);
int z80aw_read_byte(uint16_t addr);

int z80aw_write_block(uint16_t addr, uint16_t sz, uint8_t const* data);
int z80aw_read_block(uint16_t addr, uint16_t sz, uint8_t* data);

uint16_t z80aw_checksum(size_t sz, uint8_t const* data);

int  z80aw_upload_compiled(DebugInformation const* di);
bool z80aw_is_uploaded(DebugInformation const* di);

const char* z80aw_last_error();

#endif