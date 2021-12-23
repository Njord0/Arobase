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
#include <var.h>

Statement_t*
stmt_parse_var_declaration(Token_t **token)
{
    Token_t *tok = *token;
    Token_t *next_token = tok->next;

    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    if (!token_check(next_token, SYMBOL))
    {
        show_error_source(next_token);
        fprintf(stderr,
            "Expected variable name\n");
        free(stmt);
        cc_exit();
    }

    char *stmt_name = next_token->value.p;

    if (is_reserved(stmt_name))
    {
        show_error_source(next_token);
        fprintf(stderr, 
            "'%s' is a reserved keyword\n",
            stmt_name);
        free(stmt);
        cc_exit();
    }

    Symbol_t *sym = NULL;
    if (is_declared_var(symtab_g, stmt_name, &sym))
    {
        show_error_source(next_token);
        fprintf(stderr, 
            "Multiple definitions of symbol '%s'\n",
            stmt_name);
        free(stmt);
        cc_exit();
    }

    next_token = next_token->next;

    Type_s type = get_type(&next_token);

    if (!token_checks(next_token, 2, ASSIGN, EOS))
    {
        show_error_source(next_token);
        fprintf(stderr,
            "Invalid syntax\n");
        free(stmt);
        cc_exit();
    }    

    stmt->stmt_type = STMT_DECLARATION;

    stmt->decl = declaration_create_var(&next_token, stmt_name, type);

    if (type.is_array)
    {
        if (get_args_count(stmt->decl->args) > ((Array_s*)(type.ptr))->size)
        {
            show_error_source(next_token);
            fprintf(stderr,
                "Array initialization is larger than declaration...\n");
            cc_exit();
        }

        // Check array elements
        Args_t *args = stmt->decl->args;
        while (args)
        {
            if (args->type.t != type.t)
            {
                show_error_source(next_token);
                fprintf(stderr, 
                    "Invalid type in array initialization: expected '%s', found '%s'\n",
                    type_name(type.t),
                    type_name(args->type.t));
                cc_exit();
            }
            args = args->next;
        }
    }

    else if (type.is_structure && stmt->decl->is_initialised)
    {
        Statement_t *str = get_struct_by_name(type.ptr);
        //if (!str)

        if (get_args_count(str->args) != get_args_count(stmt->decl->args))
        {
            show_error_source(next_token);
            fprintf(stderr, 
                "Invalid structure initialization\n");
            cc_exit();
        }

        // Checking type for every member of structure...

        Args_t *args = str->args;
        Args_t *args_decl = stmt->decl->args;

        while (args)
        {
            type_set(args_decl->expr, args->type);
            args_decl->type = args_decl->expr->type;
            if (args->type.t != args_decl->type.t)
            {
                show_error_source(next_token);
                fprintf(stderr,
                    "Invalid type for member '%s', '%s' was expected, not '%s'\n",
                    args->name,
                    type_name(args->type.t),
                    type_name(args_decl->type.t));
                cc_exit();
            }
            
            args = args->next;
            args_decl = args_decl->next;
        }
    }

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
stmt_parse_var_assign(Token_t **token)
{
    Token_t *tok = *token;
    Token_t *next_token = tok->next;
    
    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    char *name = tok->value.p;

    Symbol_t *sym = symbol_resolve(symtab_g, name);

    if (!sym) 
    {
        show_error_source(next_token);
        fprintf(stderr,
            "Undeclared variable '%s'\n",
            name);
        free_statement(stmt);
        cc_exit();
    }

    if (sym->decl)
        sym->decl->is_initialised = true;

    if (token_check(next_token, LBRACKET))
    {
        next_token = next_token->next;

        stmt->access = expr_create(&next_token, INTEGER);

        if (!token_check(next_token, RBRACKET))
        {
            free(stmt);
            show_error_source(next_token);
            fprintf(stderr,
                "Invalid access to array\n");
            cc_exit();
        }

        next_token = next_token->next;
    }

    if (token_check(next_token, DOT))
    {
        stmt = stmt_parse_struct_assign(&next_token, stmt, name);
        *token = next_token;
        return stmt;
    }

    next_token = next_token->next;

    stmt->stmt_type = STMT_ASSIGN;

    stmt->expr = expr_create(&next_token, sym->_type.t);

    if (!stmt->expr)
    {
        show_error_source(next_token);
        fprintf(stderr, 
            "Invalid expression");
        free_statement(stmt);
        cc_exit();
    }

    stmt->expr->sym = sym;
    stmt->expr->type = sym->_type;

    type_set(stmt->expr, sym->_type);

    Type_s type = type_evaluate(stmt->expr, sym->_type.t);

    if (sym->_type.t != type.t)
    {
        show_error_source(next_token);
        fprintf(stderr, 
            "Can't assign '%s' value to a '%s' variable\n",
            type_name(type.t),
            type_name(sym->_type.t));
        free_statement(stmt);
        cc_exit();
    }

    else if (sym->_type.t == STRUCTURE && type.t == STRUCTURE)
    {
        show_error_source(next_token);
        fprintf(stderr,
            "Can't assign structure to another structure\n");
        free_statement(stmt);
        cc_exit();
    }

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
stmt_parse_struct_assign(Token_t **token, Statement_t *stmt, const char *name)
{
    Token_t *tok = *token;
    tok = tok->next;

    Symbol_t *sym = symbol_resolve(symtab_g, name);

    Statement_t *str = get_struct_by_name(sym->_type.ptr);

    Args_t *member = struct_get_member(str, tok->value.p);

    if (!member)
    {
        show_error_source(tok);
        fprintf(stderr,
            "Structure '%s' has no member '%s'\n",
            (char*)sym->_type.ptr,
            tok->value.p);
        free_statement(stmt);
        cc_exit();
    }

    tok = tok->next;

    if (!token_check(tok, ASSIGN))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Expected '='\n");
        free(stmt);
        cc_exit();
    }

    tok = tok->next;

    stmt->stmt_type = STMT_ASSIGN;
    stmt->expr = expr_create(&tok, member->type.t); // type of member to assign the value.

    if (!stmt->expr)
    {
        show_error_source(tok);
        fprintf(stderr, 
            "Invalid expression\n");
        free_statement(stmt);
        cc_exit();
    }
    stmt->expr->sym = sym;
    stmt->expr->type = member->type;
    //stmt->expr->string_value = member->name;
    stmt->expr->args = member;

    Type_s type = type_evaluate(stmt->expr, sym->_type.t);

    if (member->type.t != type.t)
    {
        show_error_source(tok);
        fprintf(stderr, 
            "Can't assign '%s' value to member '%s' (%s) of structure '%s'.\n",
            type_name(type.t),
            member->name,
            type_name(member->type.t),
            str->import_name);
        free_statement(stmt);
        cc_exit();
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