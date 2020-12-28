#include "realhardware.hh"
#include "../protocol.hh"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

RealHardware::RealHardware(std::string const& serial_port)
{
    open_serial_port(serial_port);
    if (!send_expect(C_ACK, C_OK)) {
        fprintf(stderr, "Controller did not respond to acknowledgment.\n");
        exit(EXIT_FAILURE);
    }
}

void RealHardware::open_serial_port(std::string const& serial_port)
{
    fd = open(serial_port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    
    // set interface attributes
    struct termios tty {};
    if (tcgetattr(fd, &tty) != 0) {
        perror("tgetattr");
        exit(EXIT_FAILURE);
    }
    cfsetospeed(&tty, 114583);
    cfsetispeed(&tty, 114583);
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

void RealHardware::set_memory(uint16_t addr, uint8_t data)
{
    (void) addr; (void) data;
    throw std::runtime_error("This function cannot be called on the real hardware.");
}

uint8_t RealHardware::get_memory(uint16_t addr) const
{
    return send({ C_RAM_BYTE, (uint8_t)(addr & 0xff), (uint8_t)(addr >> 8) }, 1).at(0);
}

std::vector<uint8_t> RealHardware::get_memory(uint16_t addr, uint16_t sz) const
{
    return send({ C_RAM_BLOCK, (uint8_t)(addr & 0xff), (uint8_t)(addr >> 8), (uint8_t)(sz & 0xff), (uint8_t)(sz >> 8) }, sz);
}

void RealHardware::reset()
{
    send_expect(C_RESET, C_OK);
}

void RealHardware::step()
{
    std::vector<uint8_t> s = send({ C_STEP }, 3);
    pc_ = s[0] | (s[1] << 8);
    // printed_char = s[2];
}

bool RealHardware::send_expect(uint8_t data, uint8_t expected) const
{
    if (write(fd, &data, 1) != 1)
        return false;
    uint8_t c;
    if (read(fd, &c, 1) != 1)
        return false;
    if (c != expected)
        return false;
    return true;
}

std::vector<uint8_t> RealHardware::send(std::vector<uint8_t> const& data, size_t expect) const
{
    for (uint8_t byte: data) {
        if (write(fd, &byte, 1) != 1)
            throw std::runtime_error("Unable to write byte to controller.");
    }
    
    std::vector<uint8_t> r;
    r.reserve(expect);
    for (size_t i = 0; i < expect; ++i) {
        uint8_t c;
        if (read(fd, &c, 1) != 1)
            throw std::runtime_error("Unable to read byte from controller.");
        r.push_back(c);
    }
    return r;
}

void RealHardware::upload()
{
    // TODO
}

