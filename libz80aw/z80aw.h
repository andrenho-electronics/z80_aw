#ifndef Z80AW_H
#define Z80AW_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    const char* serial_port;
    bool        log_to_stdout;
} Z80AW_Config;

typedef struct {
    uint16_t free_memory;
} Z80AW_ControllerInfo;

void z80aw_init(Z80AW_Config* cfg);
void z80aw_close();

Z80AW_ControllerInfo z80aw_controller_info();

const char* z80aw_last_error();

#endif