#include <stdio.h>
#include <stdlib.h>

#include "isorun.h"
#include "err.h"

static void usage(const char *name);

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    int command_start = 1;
    char **command = &argv[command_start];

    int stat = isorun(command);
    err_stack_dump_file(stderr);
    return stat ? EXIT_FAILURE : EXIT_SUCCESS;
}

static void usage(const char *name)
{
    fprintf(stderr, "Usage: %s [options] <command> [args]\n", name);
}
