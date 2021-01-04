#include "comm.h"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static int fd = -1;

void open_serial_port(const char* port)
{
    fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    
    // set interface attributes
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        perror("tgetattr");
        exit(EXIT_FAILURE);
    }
    cfsetospeed(&tty, 114583);
    cfsetispeed(&tty, 114583);
    cfmakeraw(&tty);
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 1;   // should block
    tty.c_cc[VTIME] = 5;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

void close_serial_port()
{
    close(fd);
}

int zsend_noreply(uint8_t byte)
{
    // TODO - logging
    if (write(fd, &byte, 1) != 1)
        return -1;    // TODO - deal with errors
    return 0;
}

int zsend_expect(uint8_t byte, uint8_t expect)
{
    // send byte
    int r = zsend_noreply(byte);
    if (r != 0)
        return r;
    
    // receive response
    int c = zrecv();
    if (c < 0)
        return r;
        
    // check response
    if (c != expect)
        return -1;   // TODO
    return 0;
}

int zrecv()
{
    int r;
    uint8_t c;
    do {
        r = read(fd, &c, 1);
    } while (r == -1 && errno == EAGAIN);
    if (r == -1)
        return -1;  // TODO
    return c;
}