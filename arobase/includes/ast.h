#ifndef _AST_H
#define _AST_H

#include <statements.h>
#include <lexer.h>

typedef struct {

    Statement_t *first_stmt;
    Statement_t *last_stmt;

} AST_t;

AST_t *create_ast();
void ast_parse(AST_t *ast, Lexer_t *lexer);
void free_ast(AST_t *ast);

extern AST_t *ast_g; // global ast

#endif