#ifndef ARGS_H_
#define ARGS_H_

typedef struct {
    const char* port;
    int         speed;
} Args;

Args parse_args(int argc, char* argv[]);

#endif

// vim:ts=4:sts=4:sw=4:expandtab
