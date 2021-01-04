#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#include "../z80aw.h"
#include "protocol.h"
#include "../comm.h"
#include "testutil.h"

#define ASSERT(msg, expr)  \
    printf("%s... ", msg); \
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
    
    ASSERT("Controller info - free memory", z80aw_controller_info().free_memory > 10);
    
    ASSERT("Invalid command", zsend_expect(Z_ACK_REQUEST, 0) == -1);
    ASSERT("Error message", strcmp(z80aw_last_error(), "No error.") != 0);
    
    zsend_noreply(Z_ACK_REQUEST);
    if (config.log_to_stdout) printf("\n");
    ASSERT("Empty buffer (not empty)", !z_empty_buffer());
    zrecv();
    ASSERT("Empty buffer (empty)", z_empty_buffer());
    
    ASSERT("Finalizing emulator", zsend_expect(Z_EXIT_EMULATOR, Z_OK) == 0);
    
    z80aw_close();
}