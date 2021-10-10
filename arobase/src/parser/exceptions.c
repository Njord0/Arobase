#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <type.h>
#include <args.h>
#include <tokens.h>
#include <statements.h>
#include <error_handler.h>
#include <struct.h>
#include <symbol_table.h>

Statement_t*
stmt_parse_try_block(Token_t **token)
{
    Token_t *tok = *token;
    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);
    stmt->stmt_type = STMT_TRY_EXCEPT;

    tok = tok->next;

    if (!token_check(tok, LBRACE))
    {
        show_error_source(tok);
        free(stmt);
        cc_exit();
    }

    tok = tok->next;

    scope_enter();

    Statement_t *stmtt = NULL, *last_stmt = NULL;

    while (!token_check(tok, RBRACE))
    {
        stmtt = get_next_statement(&tok);

        if (stmtt && (stmtt->stmt_type == STMT_DECLARATION) && !stmtt->decl->expr)
        {
            free_statement(stmtt);
            invalid_syntax_error(tok);
        }

        if (!stmt->if_block) // used as try block
        {
            stmt->if_block = stmtt;
            last_stmt = stmtt;
        }

        else
        {
            last_stmt->next = stmtt;
            last_stmt = stmtt;
        }

        tok = tok->next;
    }

    if (!tok)
    {
        show_error_source(tok);
        fprintf(stderr, 
            "Missing '}'\n");
        cc_exit();
    }
    
    symbol_pos();
    scope_exit();

    tok = tok->next;

    if (IS_KEYWORD(tok, KW_EXCEPT))
    {
        tok = tok->next;

        if (!token_check(tok, LBRACE))
        {
            show_error_source(tok);
            free(stmt);
            cc_exit();
        }

        tok = tok->next;

        stmtt = NULL;
        last_stmt = NULL;

        while (!token_check(tok, RBRACE))
        {
            stmtt = get_next_statement(&tok);

            if (stmtt && (stmtt->stmt_type == STMT_DECLARATION) && !stmtt->decl->expr)
            {
                free_statement(stmtt);
                invalid_syntax_error(tok);
            }

            if (!stmt->else_block) // used as except block
            {
                stmt->else_block = stmtt;
                last_stmt = stmtt;
            }

            else
            {
                last_stmt->next = stmtt;
                last_stmt = stmtt;
            }

            tok = tok->next;
        }

        

    }

    else
    {
        show_error_source(tok);
        fprintf(stderr, "Missing except statement after try\n");
        cc_exit();
    }
    
    *token = tok;

    return stmt;
}