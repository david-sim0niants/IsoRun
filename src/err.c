#include "err.h"

struct err_config err_config = {"Error: ", ". Caused by: ", ".\n"};

struct err {
    const char *message;
    int code;
};

struct err_stack {
#define ERR_STACK_SIZE 1024
    struct err buffer[ERR_STACK_SIZE];
    unsigned int top_off_abs;
};

struct err_stack err_stack;

struct err *err_stack_get_err(unsigned int i)
{
    return &err_stack.buffer[i % ERR_STACK_SIZE];
}

struct err *err_stack_get_top_err()
{
    return err_stack_get_err(err_stack.top_off_abs);
}

void err_stack_push(const char *message, int code)
{
    struct err *top_err = err_stack_get_top_err();
    top_err->message = message;
    top_err->code = code;
    ++err_stack.top_off_abs;
}

void err_stack_pop(void)
{
    --err_stack.top_off_abs;
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
    return err_stack.top_off_abs == 0;
}

bool err_stack_overflown(void)
{
    return err_stack.top_off_abs >= ERR_STACK_SIZE;
}

void get_start_and_size(unsigned int *start, unsigned int *size)
{
    *start = err_stack_overflown() ? err_stack.top_off_abs - ERR_STACK_SIZE : 0;
    *size = err_stack_overflown() ? ERR_STACK_SIZE : err_stack.top_off_abs;
}

const char *err_stack_get_message(unsigned int i)
{
    const char *message = err_stack_get_err(i)->message;
    if (NULL == message)
        message = "empty";
    return message;
}

const char *err_stack_get_suffix(unsigned int i, unsigned int size)
{
    if (i + 1 == size)
        return err_config.suffix;
    else
        return err_config.infix;
}

void err_stack_dump_file(FILE *file)
{
    unsigned int start, size;
    get_start_and_size(&start, &size);

    for (unsigned int i = 0; i < size; ++i) {
        fprintf(file, "%s%s%s",
                err_config.prefix,
                err_stack_get_message(i + start),
                err_stack_get_suffix(i, size));
    }
}

void err_stack_dump_strbuf(char *strbuf)
{
    unsigned int start, size;
    get_start_and_size(&start, &size);

    for (unsigned int i = 0; i < size; ++i) {
        sprintf(strbuf, "%s%s%s",
                err_config.prefix,
                err_stack_get_message(i + start),
                err_stack_get_suffix(i, size));
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
