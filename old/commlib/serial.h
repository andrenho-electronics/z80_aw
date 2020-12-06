#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdbool.h>
#include <stdint.h>

int serial_open(const char* comfile, int speed);
int serial_send(int fd, uint8_t c);
int serial_send16(int fd, uint16_t data);
int serial_recv(int fd);
int serial_recv16(int fd);

extern bool serial_debug;

#endif

// vim:ts=4:sts=4:sw=4:expandtab
