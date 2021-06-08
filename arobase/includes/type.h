#ifndef _TYPE_H
#define _TYPE_H

#include <stdbool.h>

enum Type {
    INTEGER,
    _BYTE,
    _VOID,
    _BOOL,
    _CHAR,
    STRING
};

typedef struct type_s {
    enum Type t;
    bool is_complex;
    void *ptr; // future use
} Type_s;


#include <expressions.h>
#include <tokens.h>

Type_s get_type(Token_t **token);
Type_s type_evaluate(Expression_t *expr, enum Type t);
void type_check(Expression_t *expr);

Type_s type_of_first_symbol(Expression_t *expr);
void type_set(Expression_t *expr, Type_s type);

#endif