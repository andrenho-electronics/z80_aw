#ifndef PROTOCOL_HH
#define PROTOCOL_HH

#define C_OK           0x0f
#define C_ERR          0x66

#define C_ACK          0xf0
#define C_RAM_BYTE     0xf2
#define C_RAM_BLOCK    0xf3
#define C_KEYPRESS     0xf4
#define C_RESET        0xf5
#define C_STEP         0xf6
#define C_REGISTERS    0xf7
#define C_ADD_BKP      0xf8
#define C_REMOVE_BKP   0xf9
#define C_CONTINUE     0xfa
#define C_BREAK        0xfb

// upload
#define C_UPLOAD       0xfe
#define C_UPLOAD_ACK   0x01
#define C_UPLOAD_ERROR 0x02

// execution
#define C_PRINT        0xe0
#define C_DONE         0xe1

#endif
