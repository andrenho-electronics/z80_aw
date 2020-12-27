#include <pty.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

static int master;

char last_comm = '\0';

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
        send(c);
    }
    
    close(slave);
    close(master);
}
