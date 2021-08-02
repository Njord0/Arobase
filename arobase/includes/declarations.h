#ifndef _DECLARATIONS_H
#define _DECLARATIONS_H

#include <stdbool.h>

#include <args.h>
#include <tokens.h>
#include <expressions.h>
#include <statements.h>

enum DeclType {
    VARIABLE,
    FUNCTION
};

typedef struct decl_ {
    enum DeclType decl_type;
    char *name;
    Expression_t *expr;         // for variable declaration only
    Type_s type;
    struct statement_ *code; // for function only
    Args_t *args;            // for function only   
    struct _symbol *sym;
    bool is_imported;
    bool is_initialised;
} Declaration_t;


Declaration_t *declaration_create_var(Token_t **token, char *name, Type_s type);
Declaration_t *declaration_create_func(Token_t **token, char *name, Declaration_t *decl);

void decl_init(Declaration_t *decl);

void free_declaration(Declaration_t *decl);
#endif