#ifndef _STATEMENTS_H
#define _STATEMENTS_H

#include <tokens.h>
#include <declarations.h>
#include <expressions.h>
#include <args.h>

enum statement {
    STMT_DECLARATION,
    STMT_ASSIGN,
    STMT_EXPR,
    STMT_BLOCK,
    STMT_IF_ELSE,
    STMT_WHILE,
    STMT_RETURN,
    STMT_PRINT,
    STMT_INPUT,
    STMT_IMPORT
};

typedef struct statement_ {
    enum statement stmt_type;

    struct decl_ *decl;
    Expression_t *expr;
    Expression_t *access;
    Args_t *args;
    struct statement_ *if_block;
    struct statement_ *else_block;
    struct statement_ *next;

    char *import_name;
} Statement_t;

Statement_t *get_next_statement(Token_t **token);

Statement_t *stmt_create_var_declaration(Token_t **token);
Statement_t *stmt_create_var_assign(Token_t **token);

Statement_t *stmt_create_func_declaration(Token_t **token);
Statement_t *stmt_create_func_call(Token_t **token);

Statement_t *stmt_create_if_else(Token_t **token);
Statement_t *stmt_create_while_loop(Token_t **token);

Statement_t *stmt_create_return(Token_t **token);
Statement_t *stmt_create_print(Token_t **token);
Statement_t *stmt_create_input(Token_t **token);

Statement_t *stmt_create_import(Token_t **token);

void stmt_init(Statement_t *stmt);

void free_statement(Statement_t *stmt);
void free_while_loop(Statement_t *stmt);
void free_if_else_statement(Statement_t *stmt);


#define KW_LET      0
#define KW_IF       1
#define KW_ELSE     2
#define KW_FN       3
#define KW_RETURN   4
#define KW_WHILE    5
#define KW_STR      6
#define KW_INT      7
#define KW_BYTE     8
#define KW_VOID     9
#define KW_BOOL     10
#define KW_CHAR     11
#define KW_PRINT    12
#define KW_INPUT    13
#define KW_IMPORT   14

#define KW_NO       15

extern char *Arobase_ReservedKeywords[15];

unsigned int find_keyword(const char *ptr);
bool is_reserved(const char *str);


#endif