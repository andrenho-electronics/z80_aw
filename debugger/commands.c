#include "commands.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "test.h"

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define MAGENTA "\033[35m"
#define RESET   "\033[0m"

typedef struct {
    const char*    command;
    void           (*function)(const char*, CommLib*);
    const char*    help;
} Command;

bool last_was_status = false;

static void command_quit(const char* line, CommLib* cl);
static void command_init(const char* line, CommLib* cl);
static void command_enquiry(const char* line, CommLib* cl);
static void command_read(const char* line, CommLib* cl);
static void command_write(const char* line, CommLib* cl);
static void command_dump(const char* line, CommLib* cl);
static void command_status(const char* line, CommLib* cl);
static void command_cycle(const char* line, CommLib* cl);
static void command_upload(const char* line, CommLib* cl);

static Command commands[] = {
    { "cycle",   command_cycle,   "cycle: run one Z80 cycle" },
    { "dump",    command_dump,    "dump PAGE: dumps a 0x100 page of memory" },
    { "enquiry", command_enquiry, "enquiry: check if the connection with the controller is active" },
    { "help",    command_help,    "help [FUNCTION]: show help for function" },
    { "init",    command_init,    "init: initialize/reset the Z80" },
    { "quit",    command_quit,    "quit: exit debugger" },
    { "read",    command_read,    "read ADDR: read memory location" },
    { "reset",   command_reset,   "reset: hold the Z80 reset line low (active) -- type 'init' to finish the initialization" },
    { "upload",  command_upload,  "upload: upload a binary file to ROM" },
    { "status",  command_status,  "status: print current Z80 status" },
    { "write",   command_write,   "write ADDR DATA: write data to memory location" },
    { "test",    command_test,    "test TESTNAME: run tests on the controller (options: rom, ram)" },
};

static void command_quit(const char* line, CommLib* cl)
{
    (void) line;
    (void) cl;

    exit(EXIT_SUCCESS);
}

static void command_init(const char* line, CommLib* cl)
{
    (void) line;

    if (cl_init_z80(cl) == 0) {
        printf("Z80 initialized.\n");
        last_was_status = false;
        command_status(line, cl);
    } else {
        cl_perror(cl);
    }
    last_was_status = true;
}

static void command_read(const char* line, CommLib* cl)
{
    uint16_t addr;
    int n = sscanf(line, "%hx", &addr);
    if (n != 1) {
        printf("Command with invalid syntax (expected 'read ADDR').\n");
        return;
    }
    
    uint8_t data;
    if (cl_read_memory(cl, addr, &data, 1) == 0)
        printf("0x%02X\n", data);
    else
        cl_perror(cl);
    last_was_status = false;
}

static void command_dump(const char* line, CommLib* cl)
{
    uint16_t addr;
    int n = sscanf(line, "%hx", &addr);
    if (n != 1) {
        printf("Command with invalid syntax (expected 'dump PAGE').\n");
        return;
    }
    if (addr > 0xff) {
        printf("Page must be <= 0xFF.\n");
        return;
    }

    uint8_t data[0x100];
    if (cl_read_memory(cl, addr * 0x100, data, 0x100) != 0) {
        cl_perror(cl);
        return;
    }
    
    printf("        _0 _1 _2 _3 _4 _5 _6 _7  _8 _9 _A _B _C _D _E _F\n\n");
    for (uint16_t i = 0x0; i < 0x100; i += 0x10) {
        printf("%03X_    ", ((addr * 0x100) + i) / 0x10);
        for (uint16_t j = 0; j < 0x10; ++j) {
            printf("%02X ", data[i + j]);
            if (j == 7)
                printf(" ");
        }
        printf("   ");
        for (uint16_t j = 0; j < 0x10; ++j) {
            uint8_t c = data[i + j];
            printf("%c", (c >= 32 && c < 127) ? c : '.');
            if (j == 7)
                printf(" ");
        }
        printf("\n");
    }
    last_was_status = false;
}

static void command_write(const char* line, CommLib* cl)
{
    uint16_t addr;
    uint8_t data;
    int n = sscanf(line, "%hx %hhx", &addr, &data);
    if (n != 2) {
        printf("Command with invalid syntax (expected 'write ADDR DATA').\n");
        return;
    }
    if (cl_write_memory(cl, addr, &data, 1) == 0)
        printf("Ok.\n");
    else
        cl_perror(cl);
    last_was_status = false;
}

void command_help(const char* line, CommLib* cl)
{
    (void) cl;

    char cmd[21];
    int n = sscanf(line, "%20s", cmd);
    if (n == 0 || n == EOF) {
        for (size_t i = 0; i < sizeof commands / sizeof(Command); ++i) {
            printf("%-15s", commands[i].command);
            if (i % 4 == 3)
                printf("\n");
        }
        printf("\n");
    } else if (n == 1) {
        for (size_t i = 0; i < sizeof commands / sizeof(Command); ++i) {
            if (strncmp(cmd, commands[i].command, strlen(cmd)) == 0) {
                printf("%s\n", commands[i].help);
                return;
            }
        } 
        printf("Command not found.\n");
    }
    last_was_status = false;
}

static void command_enquiry(const char* line, CommLib* cl)
{
    (void) line;

    if (cl_enquiry(cl) == 0)
        printf("Ack.\n");
    else
        cl_perror(cl);
    last_was_status = false;
}

static const char* bit(bool v)
{
    return v ? GREEN "1" RESET : RED "0" RESET;
}

static void command_status(const char* line, CommLib* cl)
{
    (void) line;

    CL_Status s;
    if (cl_status(cl, &s) == 0) {
        if (!last_was_status)
            printf("CYCLE    DATA ADDR MREQ WR RD M1 IORQ HALT BUSACK\n");
        else
            printf("\033[1A");
        printf("%04X     ", s.cycle);
        if (s.inputs.mreq || (s.inputs.wr && s.inputs.rd))   // memory bus is free
            printf("--   ----   ");
        else                 // memory is being accessed
            printf("%02X   %04X   ", s.data, s.addr);

        printf("%s   %s  %s  %s   %s    %s     %s\n",
                bit(s.inputs.mreq), bit(s.inputs.wr), bit(s.inputs.rd), bit(s.inputs.m1), bit(s.inputs.iorq),
                bit(s.inputs.halt), bit(s.inputs.busack));
    } else {
        cl_perror(cl);
    }
    last_was_status = true;
}

static void command_cycle(const char* line, CommLib* cl)
{
    (void) line;

    if (cl_cycle(cl) == 0) {
        command_status(line, cl);
    } else {
        cl_perror(cl);
    }
    last_was_status = true;
}

void command_do(const char* line, CommLib* cl)
{
    char cmd[21];
    int nxt;
    int n = sscanf(line, "%20s%n", cmd, &nxt);
    while (line[nxt] == ' ')
        ++nxt;
    if (n == 1) {
        for (size_t i = 0; i < sizeof commands / sizeof(Command); ++i) {
            if (strncmp(cmd, commands[i].command, strlen(cmd)) == 0) {
                commands[i].function(&line[nxt], cl);
                return;
            }
        }
        printf("Syntax error.\n");
    }
}

void command_reset(const char* line, CommLib* cl)
{
    (void) line;

    if (cl_reset_z80(cl) == 0) {
        printf("The Z80 reset line is being held low (active). The bus lines (addr & data) are now in high impedance.\n");
        printf("Type 'init' to initialize the processor.\n");
    } else {
        cl_perror(cl);
    }
    last_was_status = false;
}

void command_upload(const char* line, CommLib* cl)
{
    printf("Not implemented yet.\n");
}

// vim:ts=4:sts=4:sw=4:expandtab
