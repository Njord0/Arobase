#ifndef _TYPE_H
#define _TYPE_H

#include <stdbool.h>

enum Type {
    INTEGER,
    _BYTE,
    _VOID,
    _BOOL,
    _CHAR,
    STRING,
    STRUCTURE
};

typedef struct type_s {
    enum Type t;
    bool is_array;
    bool is_structure;
    void *ptr; // future use
} Type_s;

typedef struct array_s {
    enum Type type;
    unsigned int size;
} Array_s;


#include <expressions.h>
#include <tokens.h>

Type_s get_type(Token_t **token);
Type_s get_type_decl(Token_t **token);

Type_s type_evaluate(Expression_t *expr, enum Type t);
void type_check(Expression_t *expr);

Type_s type_of_first_symbol(Expression_t *expr);
void type_set(Expression_t *expr, Type_s type);

Array_s *type_create_array(enum Type t, unsigned int size);

char *type_name(enum Type t);

#endif