#include <pty.h>

#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>

static int master;
static char serial_port_name[256];
static int test_pid = 0;

void get_options(int argc, char* argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
            case 'p':
                test_pid = strtol(optarg, NULL, 10);
                printf("Being run from test process on pid %d.\n", test_pid);
                break;
            default:
                fprintf(stderr, "Invalid option.\n");
                exit(EXIT_FAILURE);
        }
    }
}

void open_serial()
{
    int slave;
    
    int e = openpty(&master, &slave, serial_port_name, NULL, NULL);
    if (e < 0) {
        perror("openpty");
        exit(EXIT_FAILURE);
    }
    if (fcntl(master, F_SETFL, FNDELAY) < 0) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
    
    printf("Listening in port: %s\n", serial_port_name);
}

uint8_t recv() {
    uint8_t c;
    int r;
    do {
        r = read(master, &c, 1);
    } while (r == -1);
    return c;
}

static void send_port_to_test()
{
    FILE* fp = fopen("./.port", "w");
    fprintf(fp, "%s", serial_port_name);
    fclose(fp);
    printf("Sending signal to test...\n");
    kill(test_pid, SIGUSR1);
}

int main(int argc, char* argv[])
{
    get_options(argc, argv);
    open_serial();
    if (test_pid)
        send_port_to_test();
    while (1)
        recv();
}