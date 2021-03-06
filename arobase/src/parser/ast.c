#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ast.h>
#include <lexer.h>
#include <struct.h>
#include <statements.h>
#include <errors/error.h>

AST_t*
create_ast()
{
    AST_t *ast = xmalloc(sizeof(AST_t));

    ast->first_stmt = NULL;
    return ast;
}

void
free_ast(AST_t *ast)
{
    
    Statement_t *stmt;
    Statement_t *prev;

    if (!ast)
        return;

    if (ast->first_stmt)
    {
        prev = ast->first_stmt;

        if (!prev->next)
        {
            free_statement(prev);
            free(ast);
            return;
        }

        while (prev && prev->next)
        {
            stmt = prev;
            prev = prev->next;

            free_statement(stmt);
        }

        free_statement(prev);
    }

    free(ast);
}

void
ast_parse(AST_t *ast, Lexer_t *lexer)
{
    Token_t *tok;
    tok = lexer->first_token;

    Statement_t *stmt = NULL;

    while (tok)
    {
        stmt = get_next_statement(&tok);

        if (stmt->stmt_type == STMT_STRUCT)
        {
            struct_add(stmt);
            tok = tok->next;
            continue;
        }

        if (((stmt->stmt_type != STMT_DECLARATION) || (stmt->decl->decl_type != FUNCTION)) && (stmt->stmt_type != STMT_IMPORT))
        {
            show_error_source(tok);
            fprintf(stderr,
                "Code outside function...\n");
            free_statement(stmt);
            cc_exit();
        }
        

        if (ast->first_stmt == NULL)
        {
            ast->first_stmt = stmt;
            ast->last_stmt = stmt;
        }

        else
        {
            ast->last_stmt->next = stmt;
            ast->last_stmt = stmt;
        }

        tok = tok->next;        
    }
}