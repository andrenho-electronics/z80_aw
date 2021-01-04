#ifndef LIBZ80AW_TESTUTIL_H
#define LIBZ80AW_TESTUTIL_H

#include <stdbool.h>

typedef enum { EMULATOR, REALHARDWARE } HardwareType;
typedef struct {
    HardwareType hardware_type;
    const char*  serial_port;
    bool         log_to_stdout;
} Config;

Config initialize(int argc, char* argv[]);

#endif //LIBZ80AW_TESTUTIL_H
