#include "args.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

Args parse_args(int argc, char* argv[])
{
    Args args = {
        .port = "/dev/ttyUSB0",
        .speed = 38400,
        .debug = false,
    };

    int opt;

    while ((opt = getopt(argc, argv, "p:s:d")) != -1) {
        switch (opt) {
            case 'p':
                args.port = optarg;
                break;
            case 's':
                args.speed = strtoul(optarg, NULL, 10);
                break;
            case 'd':
                args.debug = true;
                break;
            default:
                fprintf(stderr, "Usage: %s [-p PORT] [-s SPEED] [-d]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    return args;
}

// vim:ts=4:sts=4:sw=4:expandtab
