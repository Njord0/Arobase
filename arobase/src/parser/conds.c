#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <type.h>
#include <args.h>
#include <tokens.h>
#include <statements.h>
#include <struct.h>
#include <symbol_table.h>
#include <scope.h>
#include <errors/error.h>
#include <conds.h>


unsigned int loop_count = 0;

Statement_t*
stmt_parse_if_else(Token_t **token)
{
    Token_t *tok = *token;

    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);
    stmt->stmt_type = STMT_IF_ELSE;

    char *t_error = NULL;

    tok = tok->next;

    bool has_left_par = false;
    if (token_check(tok, LPAR))
    {
        has_left_par = true;
        tok = tok->next;
    }

    stmt->expr = expr_create_cond(&tok, _VOID);

    if (!token_check(tok, RPAR) && has_left_par)
    {
        t_error = "Missing right parenthesis after condition";
        goto error;
    }
    else if (token_check(tok, RPAR) && !has_left_par)
    {
        t_error = "Unmatched right-parenthesis";
        goto error;
    }

    if (has_left_par)
        tok = tok->next;

    if (!token_check(tok, LBRACE))
    {
        t_error = "Expected '{' after condition";
        goto error;
    }

    tok = tok->next;

    stmt->if_block = get_scope(&tok, NULL);

    if (!tok || !token_check(tok, RBRACE))
    {
        t_error = "Missing '}'";
        goto error;
    }

    Token_t *tmp = tok;
    tok = tok->next;

    if (tok && (tok->type == KEYWORD) && (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_ELSE]) == 0))
    {
        tok = tok->next;

        if (!token_check(tok, LBRACE))
        {
            t_error = "Expected '{' after 'else' keyword";
            goto error;
        }

        tok = tok->next;

        stmt->else_block = get_scope(&tok, NULL);

        if (!token_check(tok, RBRACE))
        {
            t_error = "Missing '{'";
            goto error;
        }
    }
    else
        tok = tmp;

    *token = tok;
    return stmt;

    error:
        show_error_source(tok);
        fprintf(stderr,
            "%s\n", t_error);
        free_statement(stmt);
        cc_exit();
        return NULL;
}

Statement_t*
stmt_parse_while_loop(Token_t **token)
{
    Token_t *tok = *token;

    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    char *t_error = NULL;

    tok = tok->next;

    bool has_left_par = false;
    if (token_check(tok, LPAR))
    {
        has_left_par = true;
        tok = tok->next;
    }

    stmt->stmt_type = STMT_WHILE;
    stmt->expr = expr_create_cond(&tok, _VOID);

    if (!token_check(tok, RPAR) && has_left_par)
    {
        t_error = "Missing right parenthesis after condition";
        goto error;
    }
    else if (token_check(tok, RPAR) && !has_left_par)
    {
        show_error_source(tok);
        fprintf(stderr,
            "Missing ')'\n");
        free_statement(stmt);
        cc_exit();
    }

    if (has_left_par)
        tok = tok->next;

    if (!token_check(tok, LBRACE))
    {
        t_error = "Expected '{' after condition";
        goto error;
    }

    tok = tok->next;

    loop_count++;
    scope_enter();

    stmt->if_block = get_scope(&tok, NULL);

    loop_count--;
    symbol_pos();
    scope_exit();

    if (!token_check(tok, RBRACE))
    {
        t_error = "Missing '}'";
        goto error;
    }

    *token = tok;
    return stmt;

    error:
        show_error_source(tok);
        fprintf(stderr,
            "%s\n", t_error);
        free_statement(stmt);
        cc_exit();
        return NULL;
}

Statement_t*
stmt_parse_for_loop(Token_t **token)
{
    Token_t *tok = *token;
    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    char *t_error = NULL;

    tok = tok->next;

    if (!token_check(tok, LPAR))
    {
        t_error = "Missing left parenthesis before 'for' expression";
        goto error;
    }

    tok = tok->next;

    stmt->stmt_type = STMT_FOR;
    
    scope_enter();
    Statement_t *stmtt = get_next_statement(&tok);

    if ((stmtt->stmt_type != STMT_DECLARATION) && (stmtt->stmt_type != STMT_ASSIGN))
    {
        t_error = "Variable declaration or assignement was expected here";
        goto error;
    }

    stmt->for_loop = stmtt;
    tok = tok->next;
    stmt->expr = expr_create_cond(&tok, _VOID);

    if (!token_check(tok, EOS))
    {
        t_error = "Invalid end of statement";
        goto error;
    }

    tok = tok->next;

    stmt->else_block = get_next_statement(&tok);

    if (stmt->else_block->stmt_type != STMT_ASSIGN)
    {
        t_error = "Variable assignement was expected here";
        goto error;
    }

    tok = tok->next;

    if (!token_check(tok, RPAR))
    {
        t_error = "Missing right parenthesis after 'for' expression";
        goto error;
    }

    tok = tok->next;

    if (!token_check(tok, LBRACE))
    {
        t_error = "Expected '{' after for loop initialisation";
        goto error;
    }

    tok = tok->next;

    loop_count++;

    stmt->if_block = get_scope(&tok, NULL);

    loop_count--;
    symbol_pos();
    scope_exit();

    if (!token_check(tok, RBRACE))
    {
        t_error = "Missing '}'";
        goto error;
    }

    *token = tok;
    return stmt;

    error:
        show_error_source(tok);
        fprintf(stderr,
            "%s\n", t_error);
        free(stmt);
        cc_exit();
        return NULL;
}

Statement_t*
stmt_parse_break(Token_t **token)
{
    Token_t *tok = *token;
    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    stmt->stmt_type = STMT_BREAK;

    if (loop_count == 0)
    {
        show_error_source(tok);
        fprintf(stderr,
            "'break' statement outside loop\n");
        free(stmt);
        cc_exit();
    }

    tok = tok->next;

    if (!token_check(tok, EOS))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Invalid end of statement\n");
        free(stmt);
        cc_exit();
    }

    *token = tok;
    return stmt;
}