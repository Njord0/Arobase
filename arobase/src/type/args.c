#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <args.h>
#include <expressions.h>
#include <tokens.h>
#include <error_handler.h>
#include <symbol_table.h>

Args_t *get_args(Token_t **token, enum Type t)
{
    Token_t *tok = *token;

    Args_t *args = xmalloc(sizeof(Args_t));

    args->expr = expr_create(&tok, t);
    args->type = type_evaluate(args->expr, t);
    args->sym = NULL;
    args->next = NULL;
    args->name = NULL;

    if (tok->type == COMMA)
    {
        tok = tok->next;
        args->next = get_args(&tok, t);

        *token  = tok;
        return args;
    }

    *token = tok;

    return args;
}

Args_t *get_args_decl(Token_t **token)
{
    Token_t *tok = *token;

    Args_t *args = xmalloc(sizeof(Args_t));
    args->next = NULL;
    args->expr = NULL;
    args->sym = NULL;
    
    static int c = 0;

    if (!token_expect(tok, SYMBOL))
    {
        free_args(args);
        cc_exit();
    }

    args->name = tok->value.p;

    tok = tok->next;

    args->type = get_type_decl(&tok);

    c += 1;
    
    if ((tok != NULL) && (tok->type == COMMA))
    {

        if (c == 3)
        {
            show_error_source(tok);
            fprintf(stderr, 
                "Function parameters are limited to 3 !\n");
            cc_exit();
        }

        tok  = tok->next;
        args->next = get_args_decl(&tok);
    }

    c = 0;

    *token = tok;

    return args;
}

unsigned int get_args_count(Args_t *args)
{

    unsigned int count = 0;

    while (args != NULL)
    {
        args = args->next;
        count++;
    }

    return count;

}

void free_args(Args_t *args)
{

    Args_t *next;

    while (args != NULL)
    {
        next = args->next;
        if (args->expr != NULL)
        {
            free_expression(args->expr);
            args->expr = NULL;
        }

        if ((args->sym != NULL) && args->sym->_type.is_array)
            free(args->sym->_type.ptr);

        free(args->sym);

        args->next = NULL;
        free(args);
        args = next;
    }

}