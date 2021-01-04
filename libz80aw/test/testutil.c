#include "testutil.h"

#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static volatile bool wait_for_emulator = true;
static char serial_port[256];

static const char* initialize_emulator()
{
    static char serial_port[256];
    
    char pid_s[16];
    snprintf(pid_s, sizeof pid_s, "%d", getpid());
    
    pid_t pid = fork();
    if (pid == 0) {
        execl("../emulator/emulator", "../emulator/emulator", "-p", pid_s, NULL);
    } else {
        // wait(pid);
    }
    return serial_port;
}

static void continue_execution(int sig)
{
    (void) sig;
    wait_for_emulator = false;
}

Config initialize(int argc, char* argv[])
{
    Config config = { .hardware_type = EMULATOR };
    
    int opt;
    while ((opt = getopt(argc, argv, "hr:l")) != -1) {
        switch (opt) {
            case 'l':
                config.log_to_stdout = true;
                break;
            case 'r':
                config.hardware_type = REALHARDWARE;
                config.serial_port = optarg;
                break;
            default:
                printf("Usage: %s [-r PORT]\n", argv[0]);
                printf("     -r      Run on real hardware, where PORT is the serial port (ex. /dev/ttyUSB0)\n");
                printf("     -l      Log bytes to stdout\n");
                exit(EXIT_FAILURE);
        }
    }
    
    if (config.hardware_type == EMULATOR) {
        signal(SIGUSR1, continue_execution);
        initialize_emulator();
        while (wait_for_emulator);
        FILE* f = fopen("./.port", "r");
        fread(serial_port, sizeof serial_port, sizeof serial_port - 1, f);
        fclose(f);
        unlink("./.port");
        config.serial_port = serial_port;
    }
    
    return config;
}
