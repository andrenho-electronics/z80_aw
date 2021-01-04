#include "z80aw.h"

#include <stdlib.h>

#include "comm.h"

void z80aw_init(Z80AW_Config* cfg)
{
    open_serial_port(cfg->serial_port);
}

void z80aw_close()
{
    close_serial_port();
}