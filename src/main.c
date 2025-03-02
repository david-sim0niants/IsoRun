#include <stdio.h>

static void usage(const char *name);

int main(int argc, char **argv)
{
    if (argc < 2)
        usage(argv[0]);
}

static void usage(const char *name)
{
    fprintf(stderr, "Usage: %s [options] <command> [args]\n", name);
}
