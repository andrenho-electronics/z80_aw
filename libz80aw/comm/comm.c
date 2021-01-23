#include "comm.h"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "logging.h"
#include "z80aw_priv.h"

static int fd = -1;
bool assert_empty_buffer = false;

int open_serial_port(char const* port)
{
    fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        ERROR("Could not open serial port: %s", strerror(errno));
    }
    
    // set interface attributes
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        ERROR("Could not get terminal attributes: %s", strerror(errno));
    }
    cfsetospeed(&tty, 114583);
    cfsetispeed(&tty, 114583);
    cfmakeraw(&tty);
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 1;   // should block
    tty.c_cc[VTIME] = 0;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        ERROR("Could not get terminal attributes: %s", strerror(errno));
    }
    return 0;
}

void close_serial_port()
{
    close(fd);
}

int zsend_noreply(uint16_t byte)
{
    logd(byte, SEND);
    byte &= 0xff;
    if (write(fd, &byte, 1) != 1) {
        ERROR("Cannot write byte 0x%02X to controller", byte);
    }
    return 0;
}

int zsend_expect(uint16_t byte, uint8_t expect)
{
    // send byte
    int r = zsend_noreply(byte);
    if (r != 0)
        return r;
    
    // receive response
    int c = zrecv_response();
    if (c < 0)
        return r;

    // check response
    if (c != expect) {
        ERROR("Response does not match: expected 0x%02X, found 0x%02X", expect, c);
    }
    return 0;
}

int zrecv()
{
    uint8_t c;
    int r;
    while ((r = read(fd, &c, 1)) == 0);
    logd(c, RECV);
    if (r == -1) {
        ERROR("Cannot read byte from controller");
    }
    return c;
}

int zrecv16() {
    int a = zrecv();
    int b = zrecv();
    if (a < 0) return a;
    if (b < 0) return b;
    return a | (b << 8);
}

int zrecv_response()
{
    uint8_t c = zrecv();
    if (log_to_stdout)
        log_command(c, RECV);
    return c;
}

int zrecv16_response()
{
    uint16_t cc = zrecv16();
    if (log_to_stdout)
        log_command(cc, RECV);
    return cc;
}

bool z_empty_buffer()
{
    // wait for file descriptor
    fd_set set;
    struct timeval timeout_;
    FD_ZERO(&set);
    FD_SET(fd, &set);
    timeout_.tv_sec = 0;
    timeout_.tv_usec = 20000;   // 0.1 sec
    int r = select(FD_SETSIZE, &set, NULL, NULL, &timeout_);
    return r == 0;
}

void z_assert_empty_buffer()
{
    if (assert_empty_buffer) {
        if (!z_empty_buffer()) {
            fprintf(stderr, "Serial buffer is not empty.\n");
            exit(EXIT_FAILURE);
        }
    }
}
