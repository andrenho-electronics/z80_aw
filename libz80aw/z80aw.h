#ifndef Z80AW_H
#define Z80AW_H

#include <stdbool.h>

typedef struct {
    const char* serial_port;
    bool        log_to_stdout;
} Z80AW_Config;

void z80aw_init(Z80AW_Config* cfg);
void z80aw_close();

const char* z80aw_last_error();

#endif