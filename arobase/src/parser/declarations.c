#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <args.h>
#include <tokens.h>
#include <declarations.h>
#include <error_handler.h>
#include <symbol_table.h>

Declaration_t *declaration_create_var(Token_t **token, char *name, Type_s type)
{
    Token_t *tok = *token;

    Declaration_t *decl = xmalloc(sizeof(Declaration_t));

    decl_init(decl);
    decl->name = name;
    decl->type = type;
    decl->decl_type = VARIABLE;

    add_symbol(symtab_g, decl);

    if (token_check(tok, ASSIGN))
        tok = tok->next;
    else
    {
        *token = tok;
        return decl;
    }

    if (tok != NULL)
    {

        decl->expr = expr_create(&tok, decl->type.t);
        if (decl->expr == NULL)
        {
            fprintf(stderr, 
                "Error on line : %lu\n\t Invalid expression",
                tok->lineno);
            free_declaration(decl);
            cc_exit();
        }

        decl->expr->sym = decl->sym;

        type_set(decl->expr, decl->type);

        if (decl->type.t != type_evaluate(decl->expr, decl->type.t).t)
        {
            fprintf(stderr, "Error on line : %lu\n\t Can't assign value to variable '%s', wrong type\n",
                tok->lineno,
                decl->name);
            cc_exit();
        }

    }

    *token = tok;
    return decl;

}

Declaration_t *declaration_create_func(Token_t **token, char *name, Declaration_t *decl)
{
    Token_t *tok = *token;

    decl_init(decl);
    decl->decl_type = FUNCTION;
    decl->name = name;

    add_symbol(symtab_g, decl);
    scope_enter();

    tok = tok->next;
    if (!token_expect(tok, LPAR))
        cc_exit();

    tok = tok->next;
    if (!token_check(tok, RPAR))
        decl->args = get_args_decl(&tok);

    add_symbol_from_args(symtab_g, decl->args);

    if (!token_expect(tok, RPAR))
        cc_exit();

    tok = tok->next;

    if (!token_expect(tok, COLON))
        cc_exit();
    tok = tok->next;

    if (!token_expect(tok, KEYWORD))
        cc_exit();

    if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_INT]) == 0)
        decl->type.t = INTEGER;

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_BYTE]) == 0)
        decl->type.t = _BYTE;

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_VOID]) == 0)
        decl->type.t = _VOID;

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_CHAR]) == 0)
        decl->type.t = _CHAR;
    
    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_STR]) == 0)
    {
        fprintf(stderr,
            "Error on line : %lu\n\t Function can't return strings\n",
            tok->lineno);
        cc_exit();
    }


    Symbol_t *sym = symbol_resolve(symtab_g, name);
    assert (sym != NULL);

    sym->_type = decl->type;


    tok = tok->next;

    if (!token_expect(tok, LBRACE))
    {
        free(decl);
        cc_exit();
    }

    tok = tok->next;
    Statement_t *stmt = NULL;
    Statement_t *last_stmt = NULL;

    while ((tok != NULL) && (tok->type != RBRACE))
    {
        stmt = get_next_statement(&tok);
        if ((stmt != NULL) && (stmt->decl != NULL) &&(stmt->decl->decl_type == FUNCTION))
        {
            fprintf(stderr, 
                "Error on line : %lu\n\tNested function declaration are not allowed\n",
                tok->lineno);
            free_statement(stmt);
            free_declaration(decl);
            cc_exit();
        }

        if ((stmt != NULL) && (stmt->stmt_type == STMT_RETURN))
        {
            type_set(stmt->expr, decl->type);
            type_check(stmt->expr);
            if (decl->type.t != type_evaluate(stmt->expr, decl->type.t).t)
            {
                fprintf(stderr,
                    "Error on line :%lu\n\tInvalid return value type\n",
                    tok->lineno);
                free_statement(stmt);
                cc_exit();
            }
        }


        if (decl->code == NULL)
        {
            decl->code = stmt;
            last_stmt = stmt;
        }

        else
        {
            last_stmt->next = stmt;
            last_stmt = stmt;
        }

        tok = tok->next;        

    }

    if (!token_expect(tok, RBRACE))
    {
        free_declaration(decl);
        cc_exit();
    }

    symbol_pos();
    scope_exit();

    *token = tok;
    return decl;

}

void decl_init(Declaration_t *decl)
{
    decl->name = NULL;
    decl->expr = NULL;
    decl->code = NULL;
    decl->args = NULL;
    decl->sym = NULL;
}


void free_declaration(Declaration_t *decl)
{
    if (decl->expr != NULL)
        free_expression(decl->expr);

    if (decl->sym != NULL)
        free(decl->sym);

    if (decl->code != NULL)
    {

        Statement_t *stmt = decl->code;
        Statement_t *next = NULL;
        while (stmt != NULL)
        {
            next = stmt->next;
            free_statement(stmt);
            stmt = next;
        }

        if (next != NULL)
            free_statement(next);
    }

    if (decl->args != NULL)
        free_args(decl->args);

    free(decl);
}