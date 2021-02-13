#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdint.h>

// this 512 byte buffer uses half of the MCU RAM, and should be used for memory-expensive operations
extern uint8_t buffer[];

#endif
