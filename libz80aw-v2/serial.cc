#include "serial.hh"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <stdexcept>
#include <string>

Serial::Serial(const char* port)
{
    using namespace std::string_literals;

    fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        throw std::runtime_error("Could not open serial port: "s + strerror(errno));
    }
    
    // set interface attributes
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        throw std::runtime_error("Could not get terminal attributes: "s + strerror(errno));
    }
    cfsetospeed(&tty, B38400);
    cfsetispeed(&tty, B38400);
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
        throw std::runtime_error("Could not get terminal attributes: "s + strerror(errno));
    }
}

Serial::~Serial()
{
    close(fd);
}

Reply
Serial::request(Request const& request) const
{
    std::string data;
    request.SerializeToString(&data);

    Reply reply;
    reply.ParseFromString(talk(data));
    return reply;
}

std::string
Serial::talk(std::string const& data) const
{
    if (log_bytes_) {
        printf("\e[0;32m");
        for (uint8_t c: data)
            printf("%02X ", c);
        printf("\n");
    }
    if (log_bytes_) {
        printf("\e[0m");
    }
    return "";
}
