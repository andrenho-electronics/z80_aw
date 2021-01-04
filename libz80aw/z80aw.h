#ifndef Z80AW_H
#define Z80AW_H

typedef struct {
    const char* serial_port;
} Z80AW_Config;

void z80aw_init(Z80AW_Config* cfg);
void z80aw_close();

#endif