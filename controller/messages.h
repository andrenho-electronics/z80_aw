#ifndef MESSAGES_H_
#define MESSAGES_H_

#include <stdbool.h>

typedef struct {
    bool     mreq   : 1;
    bool     wr     : 1;
    bool     rd     : 1;
    bool     m1     : 1;
    bool     iorq   : 1;
    bool     halt   : 1;
    bool     busack : 1;
} Inputs;

// debugger -> controller
#define CMD_READ    0x1
#define CMD_WRITE   0x2
#define CMD_ENQUIRY 0x3
#define CMD_STATUS  0x4
#define CMD_CYCLE   0x5
#define CMD_INIT    0x6
#define CMD_RESET   0x7

// controller -> debugger
#define ACK                  0
#define NAK                 -1
#define BUS_BUSY            -2
#define INVALID_COMMAND     -3
#define Z80_NOT_INITIALIZED -4

#endif

// vim:ts=4:sts=4:sw=4:expandtab
