#include "commands.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    const char*    command;
    void           (*function)(const char*, CommLib*);
    const char*    help;
} Command;

static void command_help(const char* line, CommLib* cl);
static void command_enquiry(const char* line, CommLib* cl);
static void command_read(const char* line, CommLib* cl);
static void command_write(const char* line, CommLib* cl);

static Command commands[] = {
    { "help",    command_help,    "help [FUNCTION]: show help for function" },
    { "enquiry", command_enquiry, "enquiry: check if the connection with the controller is active" },
    { "read",    command_read,    "read ADDR: read memory location" },
    { "write",   command_write,   "write ADDR DATA: write data to memory location" },
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
}

static void command_enquiry(const char* line, CommLib* cl)
{
    if (cl_enquiry(cl) == 0)
        printf("Ack.\n");
    else
        cl_perror(cl);
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
