#include "isorun.h"
#include "err.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/wait.h>

#define STACK_SIZE 1024
static _Thread_local char run_stack[STACK_SIZE];
static int run(void *arg);

struct run_context {
    char *const *command;
    int pipe_fds[2];
};

static stat init_run_context(char *const command[], struct run_context *ctx);
static stat isolate_and_run(struct run_context *ctx, pid_t *pid);
static stat wait_process(pid_t pid, int *exit_status);
static stat get_exit_code(int exit_status, int *exit_code);
static stat validate_run_status(struct run_context *ctx);
static stat read_run_errc(struct run_context *ctx, int *errc);
static stat write_run_errc(struct run_context *ctx, int errc);
static void deinit_run_context(struct run_context *ctx);

stat isorun(char *const command[], int *exit_code)
{
    assert(NULL != command[0] && "Empty command passed");

    struct run_context run_ctx;
    pid_t pid;
    int exit_status;

    if (init_run_context(command, &run_ctx) == STAT_FAIL)
        return STAT_FAIL;

    stat s =
        isolate_and_run(&run_ctx, &pid) &&
        wait_process(pid, &exit_status) &&
        get_exit_code(exit_status, exit_code) &&
        validate_run_status(&run_ctx);

    deinit_run_context(&run_ctx);
    return s;
}

static int run(void *arg)
{
    struct run_context *ctx = (struct run_context *)arg;

    char *const *command = ctx->command;
    int ret = execvp(command[0], command);
    if (ret)
        write_run_errc(ctx, errno);

    deinit_run_context(ctx);
    err_stack_dump_file(stderr);

    return 0;
}

static stat init_run_context(char *const command[], struct run_context *ctx)
{
    ctx->command = command;
    if (pipe(ctx->pipe_fds) == -1) {
        err_msg_sys("pipe() failed");
        return STAT_FAIL;
    }

    int read_pipe = ctx->pipe_fds[0];
    int flags = fcntl(read_pipe, F_GETFL, 0);
    flags |= O_NONBLOCK;
    if (fcntl(read_pipe, F_SETFL, flags)) {
        err_msg_sys("fcntl() failed");
        return STAT_FAIL;
    }

    return STAT_OK;
}

static stat isolate_and_run(struct run_context *ctx, pid_t *pid_out)
{
    pid_t pid = clone(run, run_stack + STACK_SIZE, CLONE_NEWPID | SIGCHLD, (void *)ctx);
    if (pid == -1) {
        err_msg_sys("clone() failed");
        return STAT_FAIL;
    }
    *pid_out = pid;
    return STAT_OK;
}

static stat wait_process(pid_t pid, int *exit_status)
{
    int ret = waitpid(pid, exit_status, 0);
    if (ret == -1) {
        err_msg_sys("waitpid() failed");
        return STAT_FAIL;
    }

    if (!WIFEXITED(*exit_status)) {
        err_msg("child process did not exit well");
        return STAT_FAIL;
    }

    return STAT_OK;
}

static stat get_exit_code(int exit_status, int *exit_code)
{
    *exit_code = WEXITSTATUS(exit_status);
    return STAT_OK;
}

static stat validate_run_status(struct run_context *ctx)
{
    int run_errc = 0;
    stat s = read_run_errc(ctx, &run_errc);
    if (s == STAT_FAIL)
        return s;

    if (run_errc) {
        err_msg_sys_custom("execvp() failed", run_errc);
        err_msg("failed running the command");
        return STAT_FAIL;
    }
    return STAT_OK;
}

static stat read_run_errc(struct run_context *ctx, int *errc)
{
    stat s = STAT_FAIL;
    *errc = 0;

    const int pipe_fd = ctx->pipe_fds[0];
    ssize_t bytes_read = read(pipe_fd, errc, sizeof(*errc));
    if (bytes_read == -1 && errno != EAGAIN)
        err_msg_sys("failed reading from pipe");
    else if (bytes_read == 0)
        err_msg("unexpected EOF when reading from pipe");
    else
        s = STAT_OK;

    return s;
}

static stat write_run_errc(struct run_context *ctx, int errc)
{
    stat s = STAT_FAIL;

    const int pipe_fd = ctx->pipe_fds[1];
    ssize_t bytes_written = write(pipe_fd, &errc, sizeof(errc));
    if (bytes_written == -1)
        err_msg_sys("failed writing to pipe");
    else
        s = STAT_OK;

    return s;
}

static void deinit_run_context(struct run_context *ctx)
{
    close(ctx->pipe_fds[0]);
    close(ctx->pipe_fds[1]);
}
