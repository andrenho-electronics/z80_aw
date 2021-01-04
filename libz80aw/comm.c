#include "comm.h"
#include "z80aw_priv.h"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int fd = -1;
static bool log_to_stdout = false;

void open_serial_port(char const* port, bool log_to_stdout_)
{
    fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    
    log_to_stdout = log_to_stdout_;
    
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
    tty.c_cc[VMIN] = 0;   // should block
    tty.c_cc[VTIME] = 0;
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
    if (log_to_stdout) {
        printf("\e[0;34m%02X \e[0m", byte);
        fflush(stdout);
    }
    if (write(fd, &byte, 1) != 1)
        ERROR("Cannot write byte 0x%02X to controller", byte);
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
        ERROR("Response does not match: expected 0x%02X, found 0x%02X", expect, c);
    return 0;
}

int zrecv()
{
    fd_set set;
    struct timeval timeout;
    FD_ZERO(&set);
    FD_SET(fd, &set);
    timeout.tv_sec = 5;    // 5 seconds
    timeout.tv_usec = 0;
    int r = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
    if (r == 0)
        ERROR("Did not receive a response from controller in 5 seconds.");
    
    uint8_t c;
    r = read(fd, &c, 1);
    if (log_to_stdout) {
        printf("\e[0;33m%02X \e[0m", c);
        fflush(stdout);
    }
    if (r == -1)
        ERROR("Cannot read byte from controller");
    return c;
}

int zrecv16() {
    int a = zrecv();
    int b = zrecv();
    if (a < 0) return a;
    if (b < 0) return b;
    return a | (b << 8);
}

bool z_empty_buffer()
{
    // wait for file descriptor
    fd_set set;
    struct timeval timeout;
    FD_ZERO(&set);
    FD_SET(fd, &set);
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;   // 0.01 sec
    int r = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
    return r == 0;
}

void z_assert_empty_buffer()
{
    if (!z_empty_buffer()) {
        fprintf(stderr, "Serial buffer is not empty.\n");
        exit(EXIT_FAILURE);
    }
}