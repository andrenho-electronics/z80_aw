#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#include "../z80aw.h"
#include "protocol.h"
#include "../comm.h"
#include "testutil.h"

#define ASSERT(msg, expr)                       \
    printf("%s... ", msg);                       \
    if (expr) { printf("\e[0;32m✔\e[0m\n"); } else { printf("\e[0;31m❌\e[0m\n"); exit(1); }

int main(int argc, char* argv[])
{
    Config config = initialize(argc, argv);
    printf("Serial port: %s\n", config.serial_port);
    
    Z80AW_Config cfg = {
            .serial_port = config.serial_port,
            .log_to_stdout = config.log_to_stdout,
    };
    z80aw_init(&cfg);
    
    ASSERT("Invalid command", zsend_expect(0, 0) == -1);
    ASSERT("Error message", strcmp(z80aw_last_error(), "No error.") != 0);
    
    ASSERT("Finalizing emulator", zsend_expect(Z_EXIT_EMULATOR, Z_OK) == 0);
    
    z80aw_close();
}