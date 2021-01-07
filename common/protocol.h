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
#define Z_STEP              0xe7   //  -> [printed char]
#define Z_KEYPRESS          0xe8   // [Key] -> Z_OK
#define Z_ADD_BKP           0xe9   // [Aa] [Ab] -> Z_OK / Z_TOO_MANY_BKPS
#define Z_REMOVE_BKP        0xea   // [Aa] [Ab] -> Z_OK
#define Z_REMOVE_ALL_BKPS   0xeb   // Z_OK
#define Z_QUERY_BKPS        0xec   //  -> [count] [bkp0a] [bkp0b] ...
#define Z_CONTINUE          0xed   // Z_OK
#define Z_LAST_EVENT        0xee   // Z_OK, Z_PRINT_CHAR or Z_BKP_REACHED
#define Z_STOP              0xef   // Z_OK
#define Z_PIN_STATUS        0xf0   // [Ca] [Cb] [Cc] [Cd] [Aa] [Ab] [DATA] [Pa] [Pb] (C: cycle, A: address, P: pins)
                                   //        /- Z80 outputs --\  /----- Z80 inputs -----\  / memory \.
                                   // Pins:  M1 IORQ HALT BUSAK  WAIT INT NMI RSET BUSREQ  MREQ RD WR
#define Z_CYCLE             0xf1   // Z_OK
#define Z_PC                0xf2   //  -> [PC]

// responses

#define Z_OK                0xd0
#define Z_INVALID_CMD       0xd1
#define Z_ACK_RESPONSE      0xd2
#define Z_TOO_MANY_BKPS     0xd3
#define Z_PRINT_CHAR        0xd4   // [char]
#define Z_BKP_REACHED       0xd5

#endif //LIBZ80AW_PROTOCOL_H
