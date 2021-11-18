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
#include <exceptions.h>

Vector *exception_vector = NULL;

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

        if (!token_check(tok, COLON))
        {
            show_error_source(tok);
            fprintf(stderr,
                "Invalid syntax\n");
            cc_exit();
        }

        tok = tok->next;

        if (!token_check(tok, SYMBOL) || !is_valid_exception(tok->value.p))
        {
            show_error_source(tok);
            fprintf(stderr,
                "unknow exception %s\n",
                tok->value.p);
            cc_exit();
        }

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

Statement_t*
stmt_parse_raise(Token_t **token)
{
    Token_t *tok = *token;
    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);
    stmt->stmt_type = STMT_RAISE;

    tok = tok->next;

    if (!token_check(tok, SYMBOL) || !is_valid_exception(tok->value.p))
    {
            show_error_source(tok);
            printf("unknow exception %s\n", tok->value.p);
            cc_exit();
    }

    stmt->exception = vec_find_exception(exception_vector, tok->value.p);

    tok = tok->next;

    if (!token_check(tok, EOS))
    {
        show_error_source(tok);
        fprintf(stderr, 
            "Invalid end of statement\n");
        free_statement(stmt);
        cc_exit();
    }

    *token = tok;
    return stmt;
}

void add_exception(const char *name)
{
    Exception_t *exception = xmalloc(sizeof(Exception_t));
    exception->no = exception_vector->_nb;
    exception->name = name;
    vec_add_exception(exception, exception_vector);
}

void free_exception(Exception_t *ex)
{
    free(ex);
}

bool is_valid_exception(const char *name)
{
    Exception_t *ex = vec_find_exception(exception_vector, name);
    if (!ex)
        return false;

    return true;
}