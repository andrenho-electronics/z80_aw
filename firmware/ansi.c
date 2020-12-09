#include "ansi.h"

#include <avr/pgmspace.h>

PGM_P ANSI_CLRSCR  =ii PSTR("\033[2J\033[H");
PGM_P ANSI_RED     = PSTR("\033[31m");
PGM_P ANSI_GREEN   = PSTR("\033[32m");
PGM_P ANSI_MAGENTA = PSTR("\033[35m");
PGM_P ANSI_RESET   = PSTR("\033[0m");
PGM_P ANSI_UP      = PSTR("\033[1A");

// vim:ts=4:sts=4:sw=4:expandtab
