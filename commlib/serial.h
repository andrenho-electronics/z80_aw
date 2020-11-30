#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdint.h>

int serial_open(const char* comfile, int speed);
int     serial_send(int fd, uint8_t c);
int     serial_recv(int fd);

#endif

// vim:ts=4:sts=4:sw=4:expandtab
