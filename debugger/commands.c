#include "commands.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

static void command_help(const char* line, CommLib* cl);
static void command_enquiry(const char* line, CommLib* cl);
static void command_read(const char* line, CommLib* cl);
static void command_write(const char* line, CommLib* cl);
static void command_status(const char* line, CommLib* cl);
static void command_cycle(const char* line, CommLib* cl);

static Command commands[] = {
    { "help",    command_help,    "help [FUNCTION]: show help for function" },
    { "enquiry", command_enquiry, "enquiry: check if the connection with the controller is active" },
    { "read",    command_read,    "read ADDR: read memory location" },
    { "write",   command_write,   "write ADDR DATA: write data to memory location" },
    { "status",  command_status,  "status: print current Z80 status" },
    { "cycle",   command_cycle,   "cycle: run one Z80 cycle" },
};

static void command_read(const char* line, CommLib* cl)
{
    uint16_t addr;
    int n = sscanf(line, "%hx", &addr);
    if (n != 1) {
        printf("Command with invalid syntax (expected 'read ADDR').\n");
        return;
    }
    
    uint8_t data;
    if (cl_read_memory(cl, &data, 1) == 1)
        printf("0x%02X\n", data);
    else
        cl_perror(cl);
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
    if (cl_write_memory(cl, addr, &data, 1) == 1)
        printf("Ok.\n");
    else
        cl_perror(cl);
    last_was_status = false;
}

static void command_help(const char* line, CommLib* _)
{
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
    CL_Status s;
    if (cl_status(cl, &s) == 0) {
        if (!last_was_status)
            printf("CYCLE    DATA ADDR MREQ WR RD M1 IORQ HALT BUSACK\n");
        else
            printf("\033[1A");
        printf("%04X     %02X   %04X   %s   %s  %s  %s   %s    %s     %s\n",
                s.cycle, s.data, s.addr, bit(s.inputs.mreq), bit(s.inputs.wr),
                bit(s.inputs.rd), bit(s.inputs.m1), bit(s.inputs.iorq),
                bit(s.inputs.halt), bit(s.inputs.busack));
    } else {
        cl_perror(cl);
    }
    last_was_status = true;
}

static void command_cycle(const char* line, CommLib* cl)
{
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

// vim:ts=4:sts=4:sw=4:expandtab
