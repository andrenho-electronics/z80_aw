#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "commlib.h"

#include "args.h"
#include "commands.h"
#include "messages.h"

int main(int argc, char* argv[])
{
    Args args = parse_args(argc, argv);

    printf("Welcome to the Z80-AW debugger! Type 'help' for a list of commands.\n");
    printf("Connecting to the embedded controller... ");
    
    CommLib* cl = cl_init(args.port, args.speed);
    if (args.debug)
        cl_set_debug(cl, true);
    if (!cl) {
        printf("unable to connect to controller.\n");
        return EXIT_FAILURE;
    }

    int r = cl_enquiry(cl);
    if (r != ACK) {
        printf("Controller did not respond to enquiry with an acknowledgment.\n");
        return EXIT_FAILURE;
    }

    printf("ok.\n");

    char* buf;
    char* last = NULL;
    while ((buf = readline("Z80> ")) != NULL) {
        if (strlen(buf) > 0) {
            add_history(buf);
            command_do(buf, cl);
            free(last);
            last = strdup(buf);
        } else {
            command_do(last, cl);
        }
        free(buf);
    }

    cl_free(cl);
    return EXIT_SUCCESS;
}

// vim:ts=4:sts=4:sw=4:expandtab
