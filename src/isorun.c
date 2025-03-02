#include "isorun.h"
#include "err.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>

struct run_args {
    char *const *command;
    int err_code;
};

static int run(void *arg);

#define STACK_SIZE 1024
__thread char stack[STACK_SIZE];

int isorun(char *const command[], int *exit_code)
{
    assert(NULL != command[0] && "Empty command passed");

    struct run_args run_args = {command, 0};

    pid_t pid = clone(run, stack + STACK_SIZE, CLONE_NEWPID | CLONE_VM | SIGCHLD, (void *)&run_args);
    if (pid == -1) {
        err(strerror(errno), errno);
        err_msg("clone() failed");
        return errno;
    }

    int exit_status;
    int ret = waitpid(pid, &exit_status, 0);
    if (ret == -1) {
        err_msg("waitpid() failed");
        return -1;
    }

    if (!WIFEXITED(exit_status)) {
        err_msg("child process did not exit well");
        return -1;
    }

    if (run_args.err_code) {
        err(strerror(run_args.err_code), run_args.err_code);
        err_msg("execvp() failed");
        err_msg("failed running the command");
        return run_args.err_code;
    }

    *exit_code = WEXITSTATUS(exit_status);
    return 0;
}

static int run(void *arg)
{
    struct run_args *args = (struct run_args *)arg;

    char *const *command = args->command;

    int ret = execvp(command[0], command);
    if (ret)
        args->err_code = errno;

    return 0;
}
