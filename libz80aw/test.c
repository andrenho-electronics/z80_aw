#include "z80aw.h"

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>

static volatile bool wait_for_emulator = true;
static char* serial_port = "";

void initialize_emulator()
{
    char pid_s[16];
    snprintf(pid_s, sizeof pid_s, "%d", getpid());
    
    pid_t pid = fork();
    if (pid == 0) {
        execl("../emulator/emulator", "../emulator/emulator", "-p", pid_s, NULL);
    } else {
        // wait(pid);
    }
}

void continue_tests(int sig)
{
    wait_for_emulator = false;
}

int main()
{
    signal(SIGUSR1, continue_tests);
    initialize_emulator();
    while (wait_for_emulator);
    FILE* f = fopen("../emulator/.port", "r");
    getline(&serial_port, NULL, f);
    fclose(f);
    printf("Serial port: %s\n", serial_port);
    
    Z80AW_Config cfg = {
            .serial_port = serial_port,
    };
    z80aw_init(&cfg);
    
    z80aw_close();
}