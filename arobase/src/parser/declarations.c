#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <args.h>
#include <tokens.h>
#include <struct.h>
#include <declarations.h>
#include <errors/error.h>
#include <symbol_table.h>
#include <scope.h>

Declaration_t*
declaration_create_var(Token_t **token, char *name, Type_s type)
{
    Token_t *tok = *token;

    Declaration_t *decl = xmalloc(sizeof(Declaration_t));

    decl_init(decl);
    decl->name = name;
    decl->type = type;
    decl->decl_type = VARIABLE;

    add_symbol(symtab_g, decl);

    if (!token_check(tok, ASSIGN))
    {
        *token = tok;
        return decl;
    }

    tok = tok->next;

    decl->is_initialised = true; 

    if (type.is_array)
    {
        if (!token_check(tok, LBRACKET))
        {
            show_error_source(tok);
            fprintf(stderr, 
                "Missing '[' before array initialization\n");
            free_declaration(decl);
            cc_exit();
        }

        tok = tok->next;

        decl->args = get_args(&tok, type.t);

        if (!token_check(tok, RBRACKET))
        {
            show_error_source(tok);
            fprintf(stderr, 
                "Missing ']'\n");
            free_declaration(decl);
            cc_exit();
        }

        tok = tok->next;
    }

    else if (type.is_structure)
    {
        if (!token_check(tok, LBRACE))
        {
            show_error_source(tok);
            fprintf(stderr, 
                "Missing '{' before structure initialization\n");
            free_declaration(decl);
            cc_exit();
        }

        tok = tok->next;

        decl->args = get_args(&tok, _VOID);

        if (!token_check(tok, RBRACE))
        {
            show_error_source(tok);
            fprintf(stderr, 
                "Missing '}'\n");
            free_declaration(decl);
            cc_exit();
        }

        tok = tok->next;
    }

    else if (tok)
    {

        decl->expr = expr_create(&tok, decl->type.t);
        if (!decl->expr)
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
            show_error_source(tok);
            fprintf(stderr, "Can't assign value of type '%s' to variable of type '%s' \n",
                type_name(type_evaluate(decl->expr, decl->type.t).t),
                type_name(decl->type.t));
            cc_exit();
        }

    }

    if (!token_check(tok, EOS))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Invalid end of statement\n");
        free(decl);
        cc_exit();
    }

    *token = tok;
    return decl;

}

Declaration_t*
declaration_create_func(Token_t **token, char *name, Declaration_t *decl)
{
    Token_t *tok = *token;

    decl_init(decl);
    decl->decl_type = FUNCTION;
    decl->name = name;

    tok = tok->next;
    if (!token_check(tok, LPAR))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Expected '(' after function name\n");
        cc_exit();
    }

    tok = tok->next;
    if (!token_check(tok, RPAR) && token_check(tok, SYMBOL))
        decl->args = get_args_decl(&tok);

    if (!token_check(tok, RPAR))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Missing ')'\n");
        cc_exit();
    }

    tok = tok->next;

    if (!token_expect(tok, COLON))
        cc_exit();
        
    tok = tok->next;

    check_function_return_value(tok, decl);
    add_symbol(symtab_g, decl);
    scope_enter();
    add_symbol_from_args(symtab_g, decl->args); // visible only inside function scope

    Symbol_t *sym = find_matching_function(decl->name, decl->args);

    sym->_type = decl->type;

    tok = tok->next;

    if (!token_check(tok, LBRACE))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Expected '{' after function header\n");
        cc_exit();
    }
    
    tok = tok->next;

    decl->code = get_scope(&tok, decl);
    scope_check_return_value_type(decl->code, decl, tok);
    
    if (!token_expect(tok, RBRACE))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Missing '}'\n");
        cc_exit();
    }

    symbol_pos();
    scope_exit();

    *token = tok;
    return decl;

}

void
decl_init(Declaration_t *decl)
{
    decl->name = NULL;
    decl->expr = NULL;
    decl->code = NULL;
    decl->args = NULL;
    decl->sym = NULL;
    decl->is_imported = false;
    decl->type.is_array = false;
    decl->is_initialised = false;
}

void
check_function_return_value(Token_t *token, Declaration_t *decl)
{
    if (strcmp(token->value.p, Arobase_ReservedKeywords[KW_INT]) == 0)
        decl->type.t = INTEGER;

    else if (strcmp(token->value.p, Arobase_ReservedKeywords[KW_FLOAT]) == 0)
        decl->type.t = _FLOAT;

    else if (strcmp(token->value.p, Arobase_ReservedKeywords[KW_BYTE]) == 0)
        decl->type.t = _BYTE;

    else if (strcmp(token->value.p, Arobase_ReservedKeywords[KW_VOID]) == 0)
        decl->type.t = _VOID;

    else if (strcmp(token->value.p, Arobase_ReservedKeywords[KW_CHAR]) == 0)
        decl->type.t = _CHAR;
    
    else if (strcmp(token->value.p, Arobase_ReservedKeywords[KW_BOOL]) == 0)
        decl->type.t = _BOOL;
    
    else if (strcmp(token->value.p, Arobase_ReservedKeywords[KW_STR]) == 0)
    {
        show_error_source(token);
        fprintf(stderr,
            "Functions can't return strings\n");
        cc_exit();
    }

    else
    {
        if (is_defined_struct(token->value.p))
        {
            show_error_source(token);
            fprintf(stderr,
                "Functions can't return structures\n");
            cc_exit();
        }
        else
        {
            show_error_source(token);
            fprintf(stderr, 
                "Unknow type '%s'\n", 
                token->value.p);
            cc_exit();
        }

    }

    if (token->next && token->next->type == LBRACKET)
    {
        show_error_source(token);
        fprintf(stderr,
            "Functions can't return arrays\n");
        cc_exit();
    }
}


void
free_declaration(Declaration_t *decl)
{
    if (decl->expr)
    {
        free_expression(decl->expr);
        decl->expr = NULL;
    }

    if (decl->sym)
    {
        free(decl->sym->rname);
        free(decl->sym);
    }

    if (decl->code)
    {

        Statement_t *stmt = decl->code;
        Statement_t *next = NULL;
        while (stmt)
        {
            next = stmt->next;
            free_statement(stmt);
            stmt = next;
        }

        if (next)
            free_statement(next);
    }

    if (decl->args)
    {
        free_args(decl->args);
        decl->args = NULL;
    }

    if (decl->is_imported && decl->name)
        free(decl->name);

    if (decl->type.is_array)
        free(decl->type.ptr);

    free(decl);
}