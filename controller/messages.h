#ifndef MESSAGES_H_
#define MESSAGES_H_

// debugger -> controller
#define READ    0x1
#define WRITE   0x2
#define ENQUIRY 0x3

// controller -> debugger
#define ACK              0
#define NAK             -1
#define BUS_BUSY        -2
#define INVALID_COMMAND -3

#endif

// vim:ts=4:sts=4:sw=4:expandtab
