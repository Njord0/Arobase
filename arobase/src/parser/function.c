#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <type.h>
#include <args.h>
#include <tokens.h>
#include <statements.h>
#include <struct.h>
#include <error_handler.h>
#include <symbol_table.h>

Statement_t*
stmt_parse_func_declaration(Token_t **token)
{
    Token_t *tok = *token;
    Token_t *next_token = tok->next;

    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    if (!token_expect(next_token, SYMBOL))
    {
        free(stmt);
        cc_exit();
    }

    char *name = next_token->value.p;
    current_function = stmt;

    stmt->stmt_type = STMT_DECLARATION;
    stmt->decl = malloc(sizeof(Declaration_t));
    stmt->decl = declaration_create_func(&next_token, name, stmt->decl);

    if (stmt->decl && !stmt->decl->code)
        fprintf(stderr, 
            "WARNING: empty function body for function '%s'\n", 
            name);

    *token = next_token;

    current_function = NULL;

    return stmt;
}

Statement_t*
stmt_parse_func_call(Token_t **token)
{
    Token_t *tok = *token;
    Token_t *next_token = tok->next;

    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    assert (tok->type == SYMBOL);

    stmt->stmt_type = STMT_EXPR;
    stmt->expr = expr_create_funccall(&next_token, tok->value.p);

    if (!token_check(next_token, EOS))
    {
        show_error_source(next_token);
        fprintf(stderr,
            "Invalid end of statement\n");
        free_statement(stmt);
        cc_exit();
    }

    *token = next_token;
    return stmt;
}


Statement_t*
stmt_parse_return(Token_t **token)
{
    Token_t *tok = *token;
    Statement_t *stmt = xmalloc(sizeof(Statement_t));

    tok = tok->next;

    stmt_init(stmt);
    stmt->stmt_type = STMT_RETURN;
    stmt->expr = expr_create(&tok, _VOID);

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