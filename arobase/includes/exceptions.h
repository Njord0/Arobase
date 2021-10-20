#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

typedef struct exception {
    int no;
    const char *name; // Exception name
} Exception_t;

#include <tokens.h>
#include <utils/vectors.h>
#include <statements.h>

Statement_t *stmt_parse_try_block(Token_t **token);
Statement_t *stmt_parse_raise(Token_t **token);

bool is_valid_exception(const char *name);

void add_exception(const char *name);
void free_exception(Exception_t *ex);

extern Vector *exception_vector;

#endif // _EXCEPTIONS_H