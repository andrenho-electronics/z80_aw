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

static Command commands[] = {
    { "help",    command_help,    "help [FUNCTION]: show help for function" },
    { "enquiry", command_enquiry, "enquity: check if the connection is working" },
};

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
            if (strncmp(cmd, commands[i].command, strlen(cmd)) == 0)
                commands[i].function(&line[nxt], cl);
        }
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
