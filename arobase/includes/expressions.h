#ifndef _EXPRESSIONS_H
#define _EXPRESSIONS_H

#include <tokens.h>
#include <type.h>

enum expression {

    EXPR_PLUS,
    EXPR_MINUS,
    EXPR_DIV,
    EXPR_MUL,

    EXPR_NUMBER,
    EXPR_STRING_LITTERAL,
    EXPR_CHAR,
    EXPR_FUNCCALL,
    EXPR_SYMBOL, // variable
    EXPR_COND,
    EXPR_ARRAYA, // ARRAY_ACCESS
};

enum condtype {
    EXPR_CMP,           // == 
    EXPR_LOWER,         // <
    EXPR_GREATER,       // >
    EXPR_DIFF,          // !=
    EXPR_LOWER_EQ,      // <=
    EXPR_GREATER_EQ,    // >=
};

typedef struct Expression {
    enum expression expr_type;
    enum condtype cond_type;
    Type_s type;
    struct Expression *left;
    struct Expression *right;
    struct Expression *access;

    struct args *args;
    int64_t int_value;
    char *string_value;
    struct _symbol *sym_value;

    struct _symbol *sym;
    unsigned int reg;

} Expression_t;

Expression_t *expr_create(Token_t **token, enum Type t);
Expression_t *expr_create_funccall(Token_t **token, char *name);

Expression_t *expr_factor(Token_t **token, enum Type t);
Expression_t *expr_term(Token_t **token, enum Type t);
Expression_t *expr_(Token_t **token, enum Type t);

Expression_t *expr_create_cond(Token_t **token, enum Type t);

Expression_t *expr_fold(Expression_t *expr);
void expr_init(Expression_t *expr);


void free_expression(Expression_t *expr);



#endif