#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pty.h>

#include "ansi.h"
#include "programatic.h"

uint8_t memory[32 * 1024];
int master, slave;

void _delay_ms(int ms)
{
    usleep(ms * 1000);
}

void z80_powerdown()
{
}

void z80_clock_cycle()
{
}

bool z80_controls_bus()
{
    return false;
}

void
serial_send(uint8_t byte)
{
    printf(ANSI_GREEN "> %02X " ANSI_RESET, byte);
    fflush(stdout);
    write(master, &byte, 1);
}

uint8_t serial_recv()
{
    uint8_t c;
    read(master, &c, 1);
    printf(ANSI_RED "> %02X " ANSI_RESET, c);
    fflush(stdout);
    return c;
}

uint16_t serial_recv16()
{
    uint16_t r = serial_recv();
    r |= ((uint16_t) serial_recv()) << 8;
    return r;
}

uint16_t memory_read(uint16_t addr)
{
    usleep(100);
    return memory[addr];
}

void
memory_write(uint16_t addr, uint8_t data, bool wait_for_completion)
{
    memory[addr] = data;
    if (wait_for_completion)
        _delay_ms(10);
}

bool
memory_write_page(uint16_t addr, uint8_t data[64], int count)
{
    for (uint16_t i = 0; i < count; ++i) {
        memory_write(addr + i, data[i], true);
    }
    return true;
}

void
memory_read_page(uint16_t addr, uint8_t data[64], int count)
{
    for (uint16_t i = 0; i < count; ++i)
        data[i] = memory_read(addr + i);
}

int main()
{
    char name[255];

    struct termios t;
    cfsetospeed(&t, 115200);
    cfsetispeed(&t, 115200);
    t.c_cflag = (t.c_cflag & ~CSIZE) | CS8;
    t.c_iflag &= ~IGNBRK;
    t.c_lflag = 0;
    t.c_oflag = 0;
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 30;
    t.c_iflag &= ~(IXON | IXOFF | IXANY);
    t.c_cflag |= (CLOCAL | CREAD);
    t.c_cflag &= ~(PARENB | PARODD);
    t.c_cflag |= 0;
    t.c_cflag &= ~CSTOPB;

    int pty = openpty(&master, &slave, name, &t, NULL);
    if (pty == 0) {
        printf("Listening on '%s'.\n", name);
    } else {
        perror("openpty");
        exit(1);
    }

    while (1) {
        uint8_t byte = serial_recv();
        if (byte != 0xfe) {
            fprintf(stderr, "Invalid byte read from serial port: expected 0xFE, found 0x%02X.\n", byte);
            exit(1);
        }
        programatic_upload();
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
