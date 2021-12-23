#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <type.h>
#include <args.h>
#include <tokens.h>
#include <statements.h>
#include <struct.h>
#include <errors/error.h>
#include <symbol_table.h>
#include <conds.h>
#include <function.h>
#include <var.h>
#include <exceptions.h>

const char *Arobase_ReservedKeywords[] = {
    "let",
    "if",
    "else",
    "fn",
    "return",
    "while",
    "string",
    "integer",
    "byte",
    "void",
    "bool",
    "char",
    "print",
    "input",
    "import",
    "assert",
    "for",
    "break",
    "struct",
    "try",
    "except",
    "raise",
    "float"
};


Statement_t*
get_next_statement(Token_t **token)
{
    Token_t *tok = *token;

    Statement_t *stmt = NULL;
    if (IS_KEYWORD(tok, KW_LET))
        stmt = stmt_parse_var_declaration(&tok);

    else if (IS_KEYWORD(tok, KW_FN))
        stmt = stmt_parse_func_declaration(&tok);

    else if (IS_KEYWORD(tok, KW_IF))
        stmt = stmt_parse_if_else(&tok);

    else if (IS_KEYWORD(tok, KW_WHILE))
        stmt = stmt_parse_while_loop(&tok);

    else if (IS_KEYWORD(tok, KW_FOR))
        stmt = stmt_parse_for_loop(&tok);

    else if (IS_KEYWORD(tok, KW_RETURN))
        stmt = stmt_parse_return(&tok);

    else if (IS_KEYWORD(tok, KW_PRINT))
        stmt = stmt_parse_print(&tok);
    
    else if (IS_KEYWORD(tok, KW_INPUT))
        stmt = stmt_parse_input(&tok);

    else if (IS_KEYWORD(tok, KW_IMPORT))
        stmt = stmt_parse_import(&tok);

    else if (IS_KEYWORD(tok, KW_ASSERT))
        stmt = stmt_parse_assert(&tok);

    else if (IS_KEYWORD(tok, KW_BREAK))
        stmt = stmt_parse_break(&tok);

    else if (IS_KEYWORD(tok, KW_STRUCT))
        stmt = stmt_parse_struct(&tok);

    /* else if (IS_KEYWORD(tok, KW_TRY))
        stmt = stmt_parse_try_block(&tok);
    
    else if (IS_KEYWORD(tok, KW_RAISE))
        stmt = stmt_parse_raise(&tok); */

    else if (tok->type == SYMBOL)
    {
        if (tok->next && ((tok->next->type == ASSIGN) || (tok->next->type == LBRACKET) || (tok->next->type == DOT)))
            stmt = stmt_parse_var_assign(&tok);

        else if (tok->next && (tok->next->type == LPAR))
            stmt = stmt_parse_func_call(&tok);

        else
        {
            show_error_source(tok);
            fprintf(stderr,
                "invalid expression\n");
            cc_exit();
        }
    }

    else if (IS_KEYWORD(tok, KW_ELSE))
    {

        show_error_source(tok);
        fprintf(stderr,
            "'else' statement without 'if' statement!\n");
        cc_exit();
    }

    else if (IS_KEYWORD(tok, KW_EXCEPT))
    {
        show_error_source(tok);
        fprintf(stderr,
            "'except' statement without 'try' statement\n");
    }

    else
    {
        show_error_source(tok);
        fprintf(stderr,
            "Unknown syntax.\n");
        cc_exit();
    }

    *token = tok;
    return stmt;
}


Statement_t*
stmt_parse_print(Token_t **token)
{
    Token_t *tok = *token;
    Statement_t *stmt = xmalloc(sizeof(Statement_t));

    tok = tok->next;

    stmt_init(stmt);
    stmt->stmt_type = STMT_PRINT;
    stmt->args = get_args(&tok, _VOID);

    if (!token_check(tok, EOS))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Invalid end of statement\n");
        free_statement(stmt);
        cc_exit();
    }

    // Check args type...
    Args_t *args = stmt->args;
    while (args)
    {
        if ((args->type.is_array && args->expr->expr_type != EXPR_ARRAYA) || args->type.is_structure)
        {
            show_error_source(tok);
            fprintf(stderr,
                "Can't print '%s'(%s) type\n",
                type_name(args->type.t),
                (args->type.is_array ? "array" : "structure"));

            free_statement(stmt);
            cc_exit();
        }
        args = args->next;
    }

    *token = tok;
    return stmt;
}

Statement_t*
stmt_parse_input(Token_t **token)
{
    Token_t *tok = *token;
    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    tok = tok->next;

    if (!(token_check(tok, SYMBOL)))
    {
        show_error_source(tok);
        fprintf(stderr,
            "A symbol was expected here\n");
        free(stmt);
        cc_exit();
    }

    Symbol_t *sym = NULL;

    if (!is_declared_var(symtab_g, tok->value.p, &sym))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Undeclared variable '%s'",
            tok->value.p);
        free(stmt);
        cc_exit();
    }

    if ((sym->_type.is_array) || (sym->_type.t == STRING) || sym->_type.is_structure || sym->_type.t == _BOOL)
    {
        show_error_source(tok);
        fprintf(stderr,
            "Invalid type with input statement\n");
        free(stmt);
        cc_exit();
    }

    if (sym->decl)
        sym->decl->is_initialised = true;

    stmt->stmt_type = STMT_INPUT;
    stmt->decl = sym->decl;

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

Statement_t*
stmt_parse_import(Token_t **token)
{
    Token_t *tok = *token;
    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    stmt->stmt_type = STMT_IMPORT;

    tok = tok->next;

    if (!token_check(tok, SYMBOL))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Invalid import name\n");
        free(stmt);
        cc_exit();
    }

    stmt->import_name = xmalloc(strlen(tok->value.p)+1);
    strcpy(stmt->import_name, tok->value.p);

    tok = tok->next;

    while (token_check(tok, DOT))
    {
        tok = tok->next;
        if (!token_check(tok, SYMBOL))
        {
            show_error_source(tok);
            fprintf(stderr,
                "Invalid import name\n");
            free(stmt);
            cc_exit();
        }

        stmt->import_name = xrealloc(stmt->import_name, strlen(stmt->import_name)+1+strlen(tok->value.p)+1);

        strcat(stmt->import_name, "/");
        strcat(stmt->import_name, tok->value.p);

        tok = tok->next;
    }

    stmt->import_name = xrealloc(stmt->import_name, strlen(stmt->import_name)+6);

    strcat(stmt->import_name, ".aroh"); // arobase header file

    if (!token_check(tok, EOS))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Invalid end of statement\n");
        free_statement(stmt);
        cc_exit();
    }

    import_from(stmt->import_name);

    *token = tok;
    return stmt;
}

Statement_t*
stmt_parse_assert(Token_t **token)
{
    Token_t *tok = *token;
    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    stmt->stmt_type = STMT_ASSERT;

    tok = tok->next;

    stmt->expr = expr_create_cond(&tok, _VOID);

    if (token_check(tok, COMMA))
    {
        tok = tok->next;
        if (!token_check(tok, TOK_STRING))
        {
            show_error_source(tok);
            fprintf(stderr,
                "A string was expected here\n");
            free(stmt);
            cc_exit();
        }

        stmt->import_name = tok->value.p;
        tok = tok->next;
    }

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

Statement_t*
stmt_parse_struct(Token_t **token)
{
    Token_t *tok = *token;
    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);
    stmt->stmt_type = STMT_STRUCT;

    tok = tok->next;

    if (!token_check(tok, SYMBOL))
    {

        show_error_source(tok);
        fprintf(stderr,
            "Missing structure name\n");
        free(stmt);
        cc_exit();
    }

    stmt->import_name = tok->value.p;

    tok = tok->next;

    if (!token_check(tok, LBRACE))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Expected '{' after structure name\n");
        free(stmt);
        cc_exit();
    }

    tok = tok->next;

    stmt->args = struct_get_args(&tok);

    if (!stmt->args)
    {
        show_error_source(tok);
        fprintf(stderr,
            "Empty structure\n");
            free_statement(stmt);
        cc_exit();
    }

    if (!token_check(tok, RBRACE))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Missing '}'\n");
        free_args(stmt->args);
        free(stmt);
        cc_exit();
    }

    tok = tok->next;

    if (!token_check(tok, EOS))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Invalid end of statement\n");
        free_args(stmt->args);
        free(stmt);
        cc_exit();
    }

    *token = tok;
    return stmt;
}

void
stmt_init(Statement_t *stmt)
{
    stmt->decl = NULL;
    stmt->expr = NULL;
    stmt->access = NULL;
    stmt->args = NULL;
    stmt->if_block = NULL;
    stmt->else_block = NULL;
    stmt->next = NULL;
    stmt->import_name = NULL;
    stmt->for_loop = NULL;
    stmt->exception = NULL;
}

void
free_statement(Statement_t *stmt)
{
    if (stmt)
    {
        if (stmt->decl && !stmt->expr && (stmt->stmt_type != STMT_INPUT))
        {
            free_declaration(stmt->decl);
        }

        if (stmt->expr)
            free_expression(stmt->expr);
    }

    if (stmt->stmt_type == STMT_IF_ELSE)
        free_if_else_statement(stmt);

    else if (stmt->stmt_type == STMT_WHILE)
        free_while_loop(stmt);

    else if (stmt->stmt_type == STMT_IMPORT)
        free(stmt->import_name);

    if (stmt->args)
        free_args(stmt->args);

    if (stmt->access)
        free_expression(stmt->access);

    if (stmt->stmt_type == STMT_ASSERT)
        free(stmt->import_name);

    else if (stmt->stmt_type == STMT_FOR)
        free_for_loop(stmt);

    else if (stmt->stmt_type == STMT_TRY_EXCEPT)
        free_try_except(stmt);

    free(stmt);
}

void
free_while_loop(Statement_t *stmt)
{
    Statement_t *prev = stmt->if_block;
    Statement_t *last = NULL;

    while (prev)
    {
        last = prev->next;
        free_statement(prev);
        prev = last;
    }

}

void
free_for_loop(Statement_t *stmt)
{
    Statement_t *prev = stmt->if_block;
    Statement_t *last = NULL;

    free_statement(stmt->else_block);
    free_statement(stmt->for_loop);

    while (prev)
    {
        last = prev->next;
        free_statement(prev);
        prev = last;
    }
}

void
free_if_else_statement(Statement_t *stmt)
{
    Statement_t *prev = stmt->if_block;
    Statement_t *last = NULL;

    while (prev)
    {
        last = prev->next;
        free_statement(prev);
        prev = last;
    }

    prev = stmt->else_block;
    last = NULL;

    while (prev)
    {
        last = prev->next;
        free_statement(prev);
        prev = last;
    }
}

void
free_try_except(Statement_t *stmt)
{
    Statement_t *prev = stmt->if_block;
    Statement_t *last = NULL;

    while (prev)
    {
        last = prev->next;
        free_statement(prev);
        prev = last;
    }

    prev = stmt->else_block;
    last = NULL;

    while (prev)
    {
        last = prev->next;
        free_statement(prev);
        prev = last;
    }
}

unsigned int
find_keyword(const char *ptr)
{
    for (unsigned int i = 0; i < KW_NO; i++)
    {
        if (strcmp(ptr, Arobase_ReservedKeywords[i]) == 0)
            return i;
    }

    return 5;
}

bool
is_reserved(const char *str)
{
    for (unsigned int i = 0; i < KW_NO; i++)
    {
        if (strcmp(str, Arobase_ReservedKeywords[i]) == 0)
            return true;
    }

    return false;
}