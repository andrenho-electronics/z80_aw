#include "z80aw.h"

#include <stdlib.h>

typedef struct Z80AW {
} Z80AW;

Z80AW* z80aw_init(Z80AW_Config* cfg)
{
    Z80AW* z80aw = calloc(1, sizeof(Z80AW));
    return z80aw;
}

void z80aw_free(Z80AW* z80aw)
{
    free(z80aw);
}
