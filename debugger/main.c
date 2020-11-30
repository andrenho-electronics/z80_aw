#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "commlib.h"

int main()
{
    printf("Welcome to the Z80-AW debugger! Type 'help' for a list of commands.\n");
    printf("Connecting to the embedded controller... ");
    
    CommLib* cl = cl_init("/dev/ttyUSB0", 38400);  // TODO
    if (!cl) {
        printf("unable to connect to controller.\n");
        return EXIT_FAILURE;
    }

    printf("ok.\n");

    char* buf;
    while ((buf = readline("Z80> ")) != NULL) {
        if (strlen(buf) > 0)
            add_history(buf);

        printf("[%s]\n", buf);
        free(buf);
    }

    cl_free(cl);
    return EXIT_SUCCESS;
}

// vim:ts=4:sts=4:sw=4:expandtab
