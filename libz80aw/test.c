#include "z80aw.h"

int main()
{
    Z80AW_Config cfg = {};
    Z80AW* z80aw = z80aw_init(&cfg);
    
    z80aw_free(z80aw);
}