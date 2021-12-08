#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <type.h>
#include <args.h>
#include <tokens.h>
#include <statements.h>
#include <errors/error.h>
#include <struct.h>
#include <symbol_table.h>
#include <exceptions.h>
#include <scope.h>

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

    stmt->if_block = get_scope(&tok, NULL);

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

        scope_enter();
        stmt->else_block = get_scope(&tok, NULL);
        symbol_pos();
        scope_exit();

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