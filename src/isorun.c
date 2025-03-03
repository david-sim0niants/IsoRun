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

#define STACK_SIZE 1024
static _Thread_local char run_stack[STACK_SIZE];
static int run(void *arg);

struct run_args {
    char *const *command;
    int err_code;
};

static stat isolate_and_run(struct run_args *args, pid_t *pid);
static stat wait_process(pid_t pid, int *exit_status);
static stat validate_run_status(struct run_args *args);
static stat get_exit_code(int exit_status, int *exit_code);

stat isorun(char *const command[], int *exit_code)
{
    assert(NULL != command[0] && "Empty command passed");

    struct run_args run_args = {command, 0};
    pid_t pid;
    int exit_status;

    return
        isolate_and_run(&run_args, &pid) &&
        wait_process(pid, &exit_status) &&
        validate_run_status(&run_args) &&
        get_exit_code(exit_status, exit_code);
}

static stat isolate_and_run(struct run_args *args, pid_t *pid_out)
{
    pid_t pid = clone(run, run_stack + STACK_SIZE, CLONE_NEWPID | CLONE_VM | SIGCHLD, (void *)args);
    if (pid == -1) {
        err(strerror(errno), errno);
        err_msg("clone() failed");
        return STAT_FAIL;
    }
    *pid_out = pid;
    return STAT_OK;
}

static stat wait_process(pid_t pid, int *exit_status)
{
    int ret = waitpid(pid, exit_status, 0);
    if (ret == -1) {
        err_msg("waitpid() failed");
        return STAT_FAIL;
    }

    if (!WIFEXITED(*exit_status)) {
        err_msg("child process did not exit well");
        return STAT_FAIL;
    }

    return STAT_OK;
}

static stat validate_run_status(struct run_args *args)
{
    if (args->err_code) {
        err(strerror(args->err_code), args->err_code);
        err_msg("execvp() failed");
        err_msg("failed running the command");
        return STAT_FAIL;
    }
    return STAT_OK;
}

static stat get_exit_code(int exit_status, int *exit_code)
{
    *exit_code = WEXITSTATUS(exit_status);
    return STAT_OK;
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
