#ifndef LIBZ80AW_LOGGING_H
#define LIBZ80AW_LOGGING_H

#include <stdbool.h>
#include <stdint.h>

#define Z_COMMAND 0x100

typedef enum { SEND, RECV } Direction;

extern bool log_to_stdout;

void logd(uint16_t byte, Direction dir);
void log_command(uint8_t byte, Direction dir);

#endif //LIBZ80AW_LOGGING_H
