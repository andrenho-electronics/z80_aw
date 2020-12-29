#ifndef PROTOCOL_HH
#define PROTOCOL_HH

#define C_OK           0x0f
#define C_ACK          0xf0
#define C_RAM_BYTE     0xf2
#define C_RAM_BLOCK    0xf3
#define C_KEYPRESS     0xf4
#define C_RESET        0xf5
#define C_STEP         0xf6
#define C_REGISTERS    0xf7

// upload
#define C_UPLOAD       0xfe
#define C_UPLOAD_ACK   0x01
#define C_UPLOAD_ERROR 0x02

#endif
