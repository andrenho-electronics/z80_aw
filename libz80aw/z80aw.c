#include "z80aw.h"

#include <stdarg.h>
#include <stdio.h>

#include "comm.h"

static char last_error[256] = "No error.";
static bool log_to_stdout = false;

void z80aw_init(Z80AW_Config* cfg)
{
    open_serial_port(cfg->serial_port, cfg->log_to_stdout);
}

void z80aw_close()
{
    close_serial_port();
}

void z80aw_set_error(char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(last_error, sizeof last_error, fmt, ap);
    if (log_to_stdout)
        printf("\e[0;31mERROR: %s\e[0m\n", last_error);
    va_end(ap);
}

const char* z80aw_last_error()
{
    return last_error;
}