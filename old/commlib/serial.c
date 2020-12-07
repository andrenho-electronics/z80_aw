#include "serial.h"

#include <errno.h>
#include <fcntl.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define MAGENTA "\033[35m"
#define RESET   "\033[0m"

bool serial_debug = false;

int serial_open(const char* comfile, int speed)
{
    int fd = open(comfile, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        printf("error %d opening %s: %s", errno, comfile, strerror (errno));
        exit(EXIT_FAILURE);
    }
    
    struct termios tty;
    if (tcgetattr (fd, &tty) != 0)
    {
        fprintf(stderr, "error %d from tcgetattr", errno);
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
                                    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 1;            // read blocks
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= 0;               // no parity
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        fprintf(stderr, "error %d from tcsetattr", errno);
        exit(EXIT_FAILURE);
    }
    return fd;
}

int
serial_send(int fd, uint8_t c)
{
    if (serial_debug) {
        printf(MAGENTA " >%02X " RESET, c);
        fflush(stdout);
    }
    return write(fd, &c, 1);
}

int
serial_send16(int fd, uint16_t data)
{
    uint8_t v[2] = { data & 0xff, data >> 8 };
    if (serial_debug) {
        printf(MAGENTA " >%02X >%02X " RESET, v[0], v[1]);
        fflush(stdout);
    }
    return write(fd, v, 2);
}

int
serial_recv(int fd)
{
    uint8_t c;
    int n = read(fd, &c, 1);
    if (serial_debug) {
        printf(MAGENTA " <%02X " RESET, c);
        fflush(stdout);
    }
    if (n < 0)
        return n;
    else
        return c;
}

int
serial_recv16(int fd)
{
    uint16_t v = 0;
    v |= serial_recv(fd);
    v |= ((uint16_t) serial_recv(fd)) << 8;
    return v;
}

// vim:ts=4:sts=4:sw=4:expandtab