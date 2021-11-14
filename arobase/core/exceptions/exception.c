#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <stdio.h>

#include "exception.h"

jmp_buf jmp_struct;

static struct exception *exceptions = NULL;

void
add_exception(jmp_buf env)
{
    if (!exceptions)
    {
        exceptions = malloc(sizeof(struct exception));
        memcpy(exceptions->env, env, sizeof(jmp_buf));
        exceptions->next = NULL;
    }
    else
    {
        struct exception *next = exceptions;

        while(next->next)
            next = next->next;

        next->next = malloc(sizeof(struct exception));
        memcpy(next->next->env, env, sizeof(jmp_buf));

        //next->next->env = env;
        next->next->next = NULL;
    }
}

void
leave_exception()
{
    if (!exceptions)
        return;

    struct exception *next = exceptions;
    struct exception *prev = NULL;

    while (next->next)
    {
        prev = next;
        next = next->next;
    }

    free(next);
    if (prev)
        prev->next = NULL;
    else
        exceptions = NULL;
}

struct __jmp_buf_tag*
get_exception_env()
{
    if (!exceptions)
        return NULL; // Not in any try/except block

    struct exception *next = exceptions;

    while (next->next)
        next = next->next;

    return next->env;
}

void
exception()
{
    printf("Un-handled exception!\n");
    exit(1);
}