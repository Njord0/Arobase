#ifndef _ARGS_H
#define _ARGS_H

#include <type.h>
#include <tokens.h>
#include <expressions.h>

typedef struct args {
    struct Expression *expr;
    struct args *next;
    Type_s type;
    const char *name;
    struct _symbol *sym;
} Args_t;

Args_t *get_args(Token_t **token);
Args_t *get_args_decl(Token_t **token);
void free_args(Args_t *args);


#endif