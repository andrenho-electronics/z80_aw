#ifndef Z80AW_H
#define Z80AW_H

typedef struct Z80AW Z80AW;

typedef struct {
} Z80AW_Config;

Z80AW* z80aw_init(Z80AW_Config* cfg);
void   z80aw_free(Z80AW* z80aw);

#endif