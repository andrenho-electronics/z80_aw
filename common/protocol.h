#ifndef LIBZ80AW_PROTOCOL_H
#define LIBZ80AW_PROTOCOL_H

// requests                            parameters -> response

#define Z_EXIT_EMULATOR     0xe0   //  -> Z_OK
#define Z_ACK_REQUEST       0xe1   //  -> Z_ACK_RESPONSE
#define Z_CTRL_INFO         0xe2   //  -> [Fa] [Fb]   (F = free memory)
#define Z_WRITE_BLOCK       0xe3   // [Aa] [Ab] [Sa] [Sb] [bytes...] -> [Ca] [Cb]  (A = address, S = size, C = checksum)
#define Z_READ_BLOCK        0xe4   // [Aa] [Ab] [Sa] [Sb] -> [bytes...]
#define Z_RESET             0xe5   //  -> Z_OK
#define Z_REGISTERS         0xe6   //  -> [A] [F] [B] [C] [D] [E] [H] [L] [A'] [F'] [B'] [C'] [D'] [E'] [H'] [L'] [SPa] [SPb] [IXa] [IXb] [IYa] [IYb] [I] [R] [HALT]
#define Z_STEP              0xe7   //  -> Z_OK

// responses

#define Z_OK                0xd0
#define Z_INVALID_CMD       0xd1
#define Z_ACK_RESPONSE      0xd2

#endif //LIBZ80AW_PROTOCOL_H
