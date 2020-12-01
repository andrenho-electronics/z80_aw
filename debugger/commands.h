#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "commands.h"
#include "commlib.h"

void command_do(const char* line, CommLib* cl);
void command_reset(const char* line, CommLib* cl);
void command_help(const char* line, CommLib* cl);

extern bool last_was_status;

#endif

// vim:ts=4:sts=4:sw=4:expandtab
