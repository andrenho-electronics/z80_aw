#include <pty.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "protocol.h"

static int master;

char last_comm = '\0';

uint8_t memory[64 * 1024];

uint8_t recv(bool* eof) {
    uint8_t c;
    int r = read(master, &c, 1);
    if (r > 0) {
        if (last_comm != 'R')
            printf("\n\x1b[32m< ");
        printf("[%02X] ", c);
        fflush(stdout);
        last_comm = 'R';
        return c;
    } else {
        if (eof)
            *eof = true;
        return 0;
    }
}

void send(uint8_t c) {
    if (last_comm != 'W')
        printf("\n\x1b[31m> ");
    printf("[%02X] ", c);
    fflush(stdout);
    last_comm = 'W';
    write(master, &c, 1);
}

int main()
{
    int slave;
    char name[256];
    
    int e = openpty(&master, &slave, name, NULL, NULL);
    if (e < 0) {
        perror("openpty");
        return EXIT_FAILURE;
    }
    
    printf("Open connection to: %s\n", name);
    
    bool eof = false;
    while (!eof) {
        uint8_t c = recv(&eof);
        switch (c) {
            case C_ACK:
                send(C_ACK_RESPONSE);
                break;
            case C_RAM_BYTE: {
                    uint8_t a = recv(NULL);
                    uint8_t b = recv(NULL);
                    send(memory[a | (b << 8)]);
                }
                break;
            case C_RAM_BLOCK: {
                    uint8_t a = recv(NULL);
                    uint8_t b = recv(NULL);
                    uint8_t c1 = recv(NULL);
                    uint8_t c2 = recv(NULL);
                    uint16_t addr = a | (b << 8);
                    uint16_t sz = c1 | (c2 << 8);
                    for (uint16_t i = 0; i < sz; ++i)
                        send(memory[addr + i]);
                }
                break;
            default:
                fprintf(stderr, "Unexpected byte.");
                goto exit;
        }
    }
    
exit:
    close(slave);
    close(master);
}
