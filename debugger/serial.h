#ifndef SERIAL_H_
#define SERIAL_H_

void ser_init();
void set_printchar(char c);
void ser_printstr(const char* text);
char ser_input(unsigned* data1, unsigned* data2);

#endif

// vim:ts=4:sts=4:sw=4:expandtab
