#include "commands.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    const char*    command;
    void         (*function)();
    const char*    help;
} Command;

static void command_help()
{

}

static Command commands[] = {
    { "help", command_help, "help [FUNCTION]: show help for function" },
};

void command_do(const char* line)
{
    char cmd[21];
    int n = sscanf(line, "%20s", cmd);
    if (n == 1) {
        for (size_t i = 0; i < sizeof commands / sizeof(Command); ++i) {
            if (strncmp(cmd, commands[i].command, strlen(cmd)) == 0)
                commands[i].function();
        }
    }
}

// vim:ts=4:sts=4:sw=4:expandtab
