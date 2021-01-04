#ifndef LIBZ80AW_PROTOCOL_H
#define LIBZ80AW_PROTOCOL_H

// requests

#define Z_EXIT_EMULATOR     0xe0
#define Z_ACK_REQUEST       0xe1

// responses

#define Z_OK                0xd0
#define Z_INVALID_CMD       0xd1
#define Z_ACK_RESPONSE      0xd2

#endif //LIBZ80AW_PROTOCOL_H
