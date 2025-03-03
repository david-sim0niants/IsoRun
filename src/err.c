#include "err.h"

#include <string.h>
#include <errno.h>

struct err_config err_config = {"Error: ", ". Caused by -> ", ".\n"};

struct err {
    const char *message;
    int code;
};

struct err_stack {
#define ERR_STACK_SIZE 1024
    struct err buffer[ERR_STACK_SIZE];
    unsigned int top_off_abs;
    unsigned int size;
};

static _Thread_local struct err_stack err_stack;

struct err *err_stack_get_err(unsigned int i)
{
    return &err_stack.buffer[i % ERR_STACK_SIZE];
}

struct err *err_stack_get_top_err(void)
{
    return err_stack_get_err(err_stack.top_off_abs);
}

void err_stack_push(const char *message, int code)
{
    struct err *top_err = err_stack_get_top_err();
    top_err->message = message;
    top_err->code = code;
    ++err_stack.top_off_abs;
    ++err_stack.size;
    if (err_stack.size > ERR_STACK_SIZE)
        err_stack.size = ERR_STACK_SIZE;
}

void err_stack_pop(void)
{
    if (err_stack_empty())
        return;
    --err_stack.top_off_abs;
    --err_stack.size;
}

void err_stack_peek_top(const char **message, int *code)
{
    struct err *top_err = err_stack_get_top_err();
    *message = top_err->message;
    *code = top_err->code;
}

void err_stack_clear(void)
{
    err_stack.top_off_abs = 0;
}

bool err_stack_empty(void)
{
    return err_stack.size == 0;
}

bool err_stack_overflown(void)
{
    return err_stack.top_off_abs >= ERR_STACK_SIZE;
}

const char *err_stack_get_message(unsigned int i)
{
    const char *message = err_stack_get_err(i)->message;
    if (NULL == message)
        message = "empty";
    return message;
}

const char *err_stack_get_suffix(unsigned int i)
{
    if (i == err_stack.top_off_abs - err_stack.size)
        return err_config.suffix;
    else
        return err_config.infix;
}

void err_stack_dump_file(FILE *file)
{
    const unsigned int back = err_stack.top_off_abs;
    const unsigned int front = err_stack.top_off_abs - err_stack.size;

    for (unsigned int i = back; i > front; --i) {
        fprintf(file, "%s%s%s",
                err_config.prefix,
                err_stack_get_message(i - 1),
                err_stack_get_suffix(i - 1));
    }
}

void err_stack_dump_strbuf(char *strbuf)
{
    const unsigned int back = err_stack.top_off_abs;
    const unsigned int front = err_stack.top_off_abs - err_stack.size;

    for (unsigned int i = back; i > front; --i) {
        sprintf(strbuf, "%s%s%s",
                err_config.prefix,
                err_stack_get_message(i - 1),
                err_stack_get_suffix(i - 1));
    }
}

void err(const char *message, int code)
{
    err_stack_push(message, code);
}

void err_msg(const char *message)
{
    err(message, 0);
}

void err_code(int code)
{
    err(NULL, code);
}

void err_sys(void)
{
    err_sys_custom(errno);
}

void err_sys_custom(int code)
{
    err(strerror(code), code);
}

void err_msg_sys(const char *message)
{
    err_sys();
    err_msg(message);
}

void err_msg_sys_custom(const char *message, int code)
{
    err_sys_custom(code);
    err_msg(message);
}
