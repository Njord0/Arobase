#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <type.h>
#include <args.h>
#include <tokens.h>
#include <statements.h>
#include <struct.h>
#include <error_handler.h>
#include <symbol_table.h>
#include <scope.h>

#include <conds.h>


unsigned int loop_count = 0;

Statement_t*
stmt_parse_if_else(Token_t **token)
{
    Token_t *tok = *token;

    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);
    stmt->stmt_type = STMT_IF_ELSE;

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
        show_error_source(tok);
        fprintf(stderr,
            "Missing right parenthesis after condition\n");
        free_statement(stmt);
        cc_exit();
    }
    else if (token_check(tok, RPAR) && !has_left_par)
    {
        show_error_source(tok);
        fprintf(stderr,
            "Unmatched right-parenthesis\n");
        free_statement(stmt);
        cc_exit();
    }

    if (has_left_par)
        tok = tok->next;

    if (!token_expect(tok, LBRACE))
    {
        free_statement(stmt);
        invalid_syntax_error(tok);
    }

    tok = tok->next;

    stmt->if_block = get_scope(&tok, NULL);

    if (!tok)
    {
        fprintf(stderr, 
            "Missing '}'\n");
        free_statement(stmt);
        cc_exit();
    }

    if (!token_expect(tok, RBRACE))
    {
        free_statement(stmt);
        cc_exit();
    }

    Token_t *tmp = tok;
    tok = tok->next;

    if (tok && (tok->type == KEYWORD) && (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_ELSE]) == 0))
    {
        tok = tok->next;

        if (!token_expect(tok, LBRACE))
        {
            free_statement(stmt);
            cc_exit();
        }

        tok = tok->next;

        stmt->else_block = get_scope(&tok, NULL);

        if (!token_expect(tok, RBRACE))
        {
            free_statement(stmt);
            cc_exit();
        }
    }
    else
        tok = tmp;

    *token = tok;
    return stmt;
}

Statement_t*
stmt_parse_while_loop(Token_t **token)
{
    Token_t *tok = *token;

    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);
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
        show_error_source(tok);
        fprintf(stderr,
            "Missing right parenthesis after condition\n");
        free_statement(stmt);
        cc_exit();
    }
    else if (token_check(tok, RPAR) && !has_left_par)
    {
        show_error_source(tok);
        fprintf(stderr,
            "Unmatched right-parenthesis\n");
        free_statement(stmt);
        cc_exit();
    }

    if (has_left_par)
        tok = tok->next;

    if (!token_expect(tok, LBRACE))
    {
        free_statement(stmt);
        cc_exit();
    }

    tok = tok->next;

    loop_count++;
    scope_enter();

    stmt->if_block = get_scope(&tok, NULL);

    loop_count--;
    symbol_pos();
    scope_exit();

    if (!token_expect(tok, RBRACE))
    {
        free_statement(stmt);
        cc_exit();
    }

    *token = tok;
    return stmt;
}

Statement_t*
stmt_parse_for_loop(Token_t **token)
{
    Token_t *tok = *token;
    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    tok = tok->next;

    if (!token_check(tok, LPAR))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Missing left parenthesis before 'for' expression\n");

        free(stmt);
        cc_exit();
    }

    tok = tok->next;

    stmt->stmt_type = STMT_FOR;

    Statement_t *stmtt = get_next_statement(&tok);

    if ((stmtt->stmt_type != STMT_DECLARATION) && (stmtt->stmt_type != STMT_ASSIGN))
    {
        free_statement(stmtt);
        free(stmt);
        show_error_source(tok);
        fprintf(stderr, 
            "Variable declaration or assignement was expected here\n");
        cc_exit();
    }

    if ((stmtt->stmt_type == STMT_DECLARATION) && (stmtt->decl->type.t != INTEGER))
    {
        free_statement(stmtt);
        free(stmt);
        show_error_source(tok);
        fprintf(stderr,
            "Only integer can be declared inside for loop initialization\n");
        cc_exit();
    }

    stmt->for_loop = stmtt;
    tok = tok->next;
    stmt->expr = expr_create_cond(&tok, _VOID);

    if (!token_expect(tok, EOS))
    {
        free_statement(stmt);
        cc_exit();
    }

    tok = tok->next;

    stmt->else_block = get_next_statement(&tok);

    if (stmt->else_block->stmt_type != STMT_ASSIGN)
    {
        show_error_source(tok);
        fprintf(stderr,
            "Variable assignement was expected here\n");
        free_statement(stmt);
        cc_exit();
    }

    tok = tok->next;

    if (!token_check(tok, RPAR))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Missing right parenthesis after 'for' expression\n");
        free_statement(stmt);
        cc_exit();
    }

    tok = tok->next;

    if (!token_expect(tok, LBRACE))
    {
        free_statement(stmt);
        cc_exit();
    }

    tok = tok->next;

    loop_count++;
    scope_enter();

    stmt->if_block = get_scope(&tok, NULL);

    loop_count--;
    symbol_pos();
    scope_exit();

    if (!token_expect(tok, RBRACE))
    {
        free_statement(stmt);
        cc_exit();
    }

    *token = tok;
    return stmt;
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