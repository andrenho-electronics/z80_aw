#ifndef LIBZ80AW_COMM_H
#define LIBZ80AW_COMM_H

#include <stdint.h>

void open_serial_port(const char* port);
void close_serial_port();

int zsend_noreply(uint8_t byte);
int zsend_expect(uint8_t byte, uint8_t expect);
int zrecv();

#endif //LIBZ80AW_COMM_H
