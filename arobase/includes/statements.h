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
    STMT_IMPORT,
    STMT_ASSERT,
    STMT_FOR,
    STMT_BREAK,
    STMT_STRUCT,
    STMT_TRY_EXCEPT,
    STMT_RAISE
};

typedef struct statement_ {
    enum statement stmt_type;

    struct decl_ *decl;
    Expression_t *expr;
    Expression_t *access;
    Args_t *args;
    struct statement_ *if_block;
    struct statement_ *else_block;
    struct statement_ *for_loop;
    struct statement_ *next;
    struct exception *exception;

    char *import_name;
} Statement_t;

Statement_t *get_next_statement(Token_t **token);

Statement_t *stmt_parse_print(Token_t **token);
Statement_t *stmt_parse_input(Token_t **token);
Statement_t *stmt_parse_import(Token_t **token);
Statement_t *stmt_parse_assert(Token_t **token);

Statement_t *stmt_parse_struct(Token_t **token);

void stmt_init(Statement_t *stmt);

void free_statement(Statement_t *stmt);

void free_while_loop(Statement_t *stmt);
void free_for_loop(Statement_t *stmt);
void free_if_else_statement(Statement_t *stmt);

void free_try_except(Statement_t *stmt);


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
#define KW_ASSERT   15
#define KW_FOR      16
#define KW_BREAK    17
#define KW_STRUCT   18
#define KW_TRY      19
#define KW_EXCEPT   20
#define KW_RAISE    21

#define KW_NO       22

extern const char *Arobase_ReservedKeywords[22];

unsigned int find_keyword(const char *ptr);
bool is_reserved(const char *str);


// macro to check keyword
#define IS_KEYWORD(x, y) \
    x->type == KEYWORD && strcmp(x->value.p, Arobase_ReservedKeywords[y]) == 0 

#endif