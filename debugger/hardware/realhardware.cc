#include "realhardware.hh"
#include "../protocol.hh"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <iostream>
#include <iomanip>

RealHardware::RealHardware(std::string const& serial_port, std::optional<std::string> const& log_file)
{
    open_serial_port(serial_port);
    if (!send_expect(C_ACK, C_OK)) {
        fprintf(stderr, "Controller did not respond to acknowledgment.\n");
        exit(EXIT_FAILURE);
    }
    
    if (log_file) {
        logfile_ = std::ofstream();
        logfile_->open(*log_file);
        if (!logfile_->is_open())
            throw std::runtime_error("Could not open log file.");
        *logfile_ << std::setfill('0') << std::setw(2) << std::right << std::uppercase << std::hex;
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
    uint8_t c = send({ C_RAM_BYTE, (uint8_t)(addr & 0xff), (uint8_t)(addr >> 8) }, 1).at(0);
    return c;
}

std::vector<uint8_t> RealHardware::get_memory(uint16_t addr, uint16_t sz) const
{
    std::vector<uint8_t> r = send({ C_RAM_BLOCK, (uint8_t)(addr & 0xff), (uint8_t)(addr >> 8), (uint8_t)(sz & 0xff), (uint8_t)(sz >> 8) }, sz);
    return r;
}

void RealHardware::reset()
{
    if (!send_expect(C_RESET, C_OK)) {
        throw std::runtime_error("Could not reset hardware.");
    }
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
    if (logfile_) {
        *logfile_ << "WRITE: [" << (int) data << "]\n";
        *logfile_ << "READ:  [" << (int) c << "]\n";
    }
    if (c != expected)
        return false;
    return true;
}

std::vector<uint8_t> RealHardware::send(std::vector<uint8_t> const& data, size_t expect) const
{
    if (logfile_)
        *logfile_ << "WRITE: ";
    for (uint8_t byte: data) {
        if (write(fd, &byte, 1) != 1)
            throw std::runtime_error("Unable to write byte to controller.");
        if (logfile_)
            *logfile_ << "[" << (int) byte << "] ";
    }
    
    if (logfile_)
        *logfile_ << "\nREAD:  ";
    
    std::vector<uint8_t> r;
    r.reserve(expect);
    for (size_t i = 0; i < expect; ++i) {
        uint8_t c;
        if (read(fd, &c, 1) != 1)
            throw std::runtime_error("Unable to read byte from controller.");
        if (logfile_)
            *logfile_ << "[" << (int) c << "] ";
        r.push_back(c);
    }
    
    if (logfile_)
        *logfile_ << "\n";
    
    return r;
}

void RealHardware::upload()
{
    for (auto const& st: upload_staging_areas_) {
        // send upload command
        if (!send_expect(C_UPLOAD, C_UPLOAD_ACK))
            throw std::runtime_error("Could not upload code.");
    
        // send data
        for (size_t i = 0; i < st.data.size(); i += 64) {
            auto r = send({ (uint8_t) ((st.addr + i) & 0xff), (uint8_t) ((st.addr + i) >> 8) }, 1);
            if (r.at(0) != C_UPLOAD_ACK)
                throw std::runtime_error("Error uploading code.");
            size_t n = (i + 64 > st.data.size()) ? st.data.size() % 64 : 64;
            send({ (uint8_t) (n & 0xff), (uint8_t) (n >> 8) }, 0);
            std::vector chunk(st.data.begin() + i, st.data.begin() + i + n);
            auto checksum = send(chunk, 2);
            if ((checksum.at(0) | checksum.at(1) << 8) != calculate_checksum(chunk))
                throw std::runtime_error("Chunk checksum does not match.");
        }
        
        // send checksum
        auto r = send({ CHECKSUM_ADDR & 0xff, (CHECKSUM_ADDR >> 8) & 0xff }, 1);
        if (r.at(0) != C_UPLOAD_ACK)
            throw std::runtime_error("Error uploading checksum.");
        send({ 2, 0 }, 0);  // 2 bytes
        send({ (uint8_t)(upload_staging_checksum_ & 0xff), (uint8_t)(upload_staging_checksum_ >> 8) }, 2);
        // will not check checksum's checksum
    
        // finalize
        send({ 0, 0, 0, 0 }, 1);
    }
}

uint16_t RealHardware::calculate_checksum(std::vector<uint8_t> const& data)
{
    uint16_t checksum1 = 0, checksum2 = 0;
    for (uint8_t b : data) {
        checksum1 = (checksum1 + b) % 255;
        checksum2 = (checksum2 + checksum1) % 255;
    }
    return checksum1 | (checksum2 << 8);
}
