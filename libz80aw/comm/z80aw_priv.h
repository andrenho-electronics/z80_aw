#ifndef LIBZ80AW_Z80AW_PRIV_H
#define LIBZ80AW_Z80AW_PRIV_H

#define ERROR(...) do { z80aw_set_error(__VA_ARGS__); return -1; } while (0);
#define ERROR_N(...) do { z80aw_set_error(__VA_ARGS__); return NULL; } while (0);

void z80aw_set_error(char* fmt, ...);

#endif //LIBZ80AW_Z80AW_PRIV_H
