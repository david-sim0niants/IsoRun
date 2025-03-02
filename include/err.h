#include <stdio.h>
#include <stdbool.h>

struct err_config {
    const char *prefix;
    const char *infix;
    const char *suffix;
};
extern struct err_config err_config;

void err_stack_push(const char *message, int code);
void err_stack_pop(void);
void err_stack_peek_top(const char **message, int *code);
void err_stack_clear(void);
bool err_stack_empty(void);
void err_stack_dump_file(FILE *file);
void err_stack_dump_strbuf(char *strbuf);

void err(const char *message, int code);
void err_msg(const char *message);
void err_code(int code);
