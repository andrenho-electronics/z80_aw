#ifndef LIBZ80AW_COMM_H
#define LIBZ80AW_COMM_H

#include <stdbool.h>
#include <stdint.h>

int  open_serial_port(char const* port);
void close_serial_port();

int zsend_noreply(uint16_t byte);
int zsend_expect(uint16_t byte, uint8_t expect);
int zrecv();
int zrecv16();
int zrecv_response();
int zrecv16_response();

bool z_empty_buffer();
void z_assert_empty_buffer();

extern bool assert_empty_buffer;

#endif //LIBZ80AW_COMM_H
