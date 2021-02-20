#ifndef LIBZ80AW_PROTOCOL_H
#define LIBZ80AW_PROTOCOL_H

#define Z_FOLLOWS_PROTOBUF_REQ    0xf0  // [this] [szB] [szA] [contents...] [chksumA] [chksumB]
#define Z_FOLLOWS_PROTOBUF_RESP   0xf1  // [this] [szB] [szA] [contents...] [chksumA] [chksumB]
#define Z_CHECKSUM_NO_MATCH       0xf2
#define Z_MESSAGE_TOO_LARGE       0xf3

/*

// requests                            parameters -> response

#define Z_EXIT_EMULATOR     0xe0   //  -> Z_OK
#define Z_ACK_REQUEST       0xe1   //  -> Z_ACK_RESPONSE
#define Z_CTRL_INFO         0xe2   //  -> [Fa] [Fb]   (F = free memory)
#define Z_WRITE_BLOCK       0xe3   // [Aa] [Ab] [Sa] [Sb] [bytes...] -> [Status] [Ca] [Cb]  (A = address, S = size, C = checksum)
#define Z_READ_BLOCK        0xe4   // [Aa] [Ab] [Sa] [Sb] -> [Status] [bytes...]
#define Z_RESET             0xe5   //  -> Z_OK
#define Z_STEP              0xe6   //  if NMI or EMULATOR -> [A] [F] [B] [C] [D] [E] [H] [L] [A'] [F'] [B'] [C'] [D'] [E'] [H'] [L'] [SPa] [SPb] [IXa] [IXb] [IYa] [IYb] [I] [R] [HALT] [printed char]
                                   //  if DISABLED -> [printed char]
#define Z_KEYPRESS          0xe8   // [Key] -> Z_OK
#define Z_ADD_BKP           0xe9   // [Aa] [Ab] -> Z_OK / Z_TOO_MANY_BKPS
#define Z_REMOVE_BKP        0xea   // [Aa] [Ab] -> Z_OK
#define Z_REMOVE_ALL_BKPS   0xeb   // Z_OK
#define Z_QUERY_BKPS        0xec   //  -> [count] [bkp0a] [bkp0b] ...
#define Z_CONTINUE          0xed   // Z_OK
#define Z_LAST_EVENT        0xee   // Z_OK [char_pressed] [bkp_reached]
#define Z_STOP              0xef   // Z_OK
#define Z_PIN_STATUS        0xf0   // [Ca] [Cb] [Cc] [Cd] [Aa] [Ab] [DATA] [Pa] [Pb] (C: cycle, A: address, P: pins)
                                   //        /- Z80 outputs --\  /----- Z80 inputs -----\  / memory \.
                                   // Pins:  M1 IORQ HALT BUSAK  WAIT INT NMI RSET BUSREQ  MREQ RD WR
#define Z_CYCLE             0xf1   // Z_OK
#define Z_PC                0xf2   //  -> [PC]
#define Z_POWERDOWN         0xf3   // Z_OK
#define Z_NEXT              0xf4   // Z_OK
#define Z_REGFETCH_MODE     0xf5   // [0: no fetch, 1: nmi, 2: emulator] -> [Z_OK or Z_EMULATOR_ONLY]
#define Z_REGISTERS         0xf6   // if emulator -> Z_OK [A] [F] [B] [C] [D] [E] [H] [L] [A'] [F'] [B'] [C'] [D'] [E'] [H'] [L'] [SPa] [SPb] [IXa] [IXb] [IYa] [IYb] [I] [R] [HALT]
                                   // if real hardware -> Z_EMULATOR_ONLY
#define Z_NMI               0xf7   // Z_OK
#define Z_RUN               'R'    // no response

#define Z_READ_DISK         0xf8   // [b0] [b1] [b2] -> [Status] [bytes...]  (b = block number)
#define Z_WRITE_DISK        0xf9   // [b0] [b1] [b2] [bytes..] -> [Status]   (b = block number)
#define Z_HAS_DISK          0xfa   // -> [Z_OK / Z_NO_DISK]
#define Z_UPDATE_DISK       0xfb   // [path (zero terminated)] -> [Z_OK / Z_NO_DISK / Z_EMULATOR_ONLY]
#define Z_DISK_LAST_STATUS  0xfc   // -> Z_OK STAGE LAST_RESPONSE
#define Z_LOAD_BOOT         0xfd   // -> Z_OK / Z_NO_DISK / Z_INCORRECT_BUS

// responses

#define Z_OK                0xd0
#define Z_INVALID_CMD       0xd1
#define Z_ACK_RESPONSE      0xd2
#define Z_TOO_MANY_BKPS     0xd3
#define Z_PRINT_CHAR        0xd4   // [char]
#define Z_BKP_REACHED       0xd5
#define Z_INCORRECT_BUS     0xd6
#define Z_EMULATOR_ONLY     0xd7
#define Z_NO_DISK           0xd8
#define Z_OUT_OF_BOUNDS     0xd9

*/

#endif //LIBZ80AW_PROTOCOL_H
