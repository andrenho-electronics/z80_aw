#include "logging.h"
#include <stdio.h>
#include "protocol.h"

bool log_to_stdout = false;

void log_command(uint8_t byte, Direction dir)
{
#define CASE(X) case X: printf("[%s] ", #X); break;
    if (dir == SEND)
        printf("\e[0;34m");
    else
        printf("\e[0;33m");
    switch (byte) {
        CASE(Z_EXIT_EMULATOR)
        CASE(Z_ACK_REQUEST)
        CASE(Z_CTRL_INFO)
        CASE(Z_WRITE_BLOCK)
        CASE(Z_READ_BLOCK)
        CASE(Z_RESET)
        CASE(Z_STEP)
        CASE(Z_KEYPRESS)
        CASE(Z_ADD_BKP)
        CASE(Z_REMOVE_BKP)
        CASE(Z_REMOVE_ALL_BKPS)
        CASE(Z_QUERY_BKPS)
        CASE(Z_CONTINUE)
        CASE(Z_LAST_EVENT)
        CASE(Z_STOP)
        CASE(Z_PIN_STATUS)
        CASE(Z_CYCLE)
        CASE(Z_PC)
        CASE(Z_POWERDOWN)
        CASE(Z_NEXT)
        CASE(Z_REGFETCH_MODE)
        CASE(Z_REGISTERS)
        CASE(Z_NMI)
        CASE(Z_RUN)
        CASE(Z_OK)
        CASE(Z_INVALID_CMD)
        CASE(Z_ACK_RESPONSE)
        CASE(Z_TOO_MANY_BKPS)
        CASE(Z_PRINT_CHAR)
        CASE(Z_BKP_REACHED)
        CASE(Z_INCORRECT_BUS)
        CASE(Z_EMULATOR_ONLY)
    }
    printf("\e[0m");
#undef CASE
}

void logd(uint16_t byte, Direction dir)
{
    if (!log_to_stdout)
        return;

    if (byte & Z_COMMAND && dir == SEND)
        printf("\n");

    if (dir == SEND)
        printf("\e[0;34m");
    else
        printf("\e[0;33m");
    printf("%02X ", byte & 0xff);
    printf("\e[0m");
    
    if (byte & Z_COMMAND && dir == SEND)
        log_command(byte & 0xff, dir);

    fflush(stdout);
}