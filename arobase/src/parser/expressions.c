#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <args.h>
#include <expressions.h>
#include <error_handler.h>
#include <symbol_table.h>

Expression_t *expr_create(Token_t **token, enum Type t)
{
    Token_t *tok = *token;

    Expression_t *expr = xmalloc(sizeof(Expression_t));

    expr_init(expr);

    if (token_checks(tok, 4, NUMBER, LPAR, SYMBOL, MINUS))
    {
        free(expr);
        expr = expr_(&tok, t);
    }

    else if (tok->type == EOS)
    {
        free(expr);
        return NULL;
    }

    else if (tok->type == TOK_STRING) // string litteral
    {
        expr->expr_type = EXPR_STRING_LITTERAL;
        expr->string_value = tok->value.p;
        expr->type.t = STRING;
        tok = tok->next;
    }

    else if (tok->type == QUOTE)
    {
        tok = tok->next;
        if (!token_check(tok, SYMBOL) || (strlen(tok->value.p) != 1))
        {
            fprintf(stderr,
                "Error on line : %lu\n\tExpected a character, found '%s'\n",
                tok->lineno,
                tok->value.p);
            cc_exit();
        }

        expr->expr_type = EXPR_CHAR;
        expr->string_value = tok->value.p;
        expr->type.t = _CHAR;

        tok = tok->next;
        if (!token_expect(tok, QUOTE))
            cc_exit();

        tok = tok->next;
    }

    else
    {
        fprintf(stderr,
            "Error on line : %lu\n\tInvalid expression\n",
            tok->lineno);
        cc_exit();
    }

    *token = tok;

    expr = expr_fold(expr);

    type_set(expr, type_of_first_symbol(expr));
    type_check(expr);

    return expr;
}

Expression_t *expr_factor(Token_t **token, enum Type t)
{
    Token_t *tok = *token;

    Expression_t *expr = xmalloc(sizeof(Expression_t));

    expr_init(expr);

    if (!token_checks(tok, 4, NUMBER, LPAR, SYMBOL, MINUS))
    {
        free_expression(expr);
        invalid_syntax_error(tok);
    }

    if (tok->type == NUMBER)
    {
        expr->expr_type = EXPR_NUMBER;
        expr->int_value = tok->value.i;
        expr->type.t = t;

        if (t == _VOID)
            expr->type.t = INTEGER;

        tok = tok->next;
    }

    else if (tok->type == LPAR)
    {
        free(expr);
        tok = tok->next;
        expr = expr_(&tok, t);

        if (!token_expect(tok, RPAR))
        {
            free_expression(expr);
            cc_exit();
        }

        tok = tok->next;
    }

    else if (tok->type == MINUS)
    {
        expr->expr_type = EXPR_UNARY_MINUS;
        tok  = tok->next;
        expr->left = expr_(&tok, t);
    }

    else if (tok->type == SYMBOL)
    {
        Symbol_t *sym;
        if (is_declared_var(symtab_g, tok->value.p, &sym))
        {
            if ((sym->_type.t != INTEGER) && (sym->_type.t != _BYTE) && (sym->_type.t != _CHAR) && (sym->_type.t != STRING))
            {
                fprintf(stderr,
                    "Error on line : %lu\n\t Invalid type in expression\n", 
                    tok->lineno);
                free_expression(expr);
                cc_exit();
            }

            expr->string_value = tok->value.p;

            if (token_check(tok->next, LBRACKET))
            {

                expr->expr_type = EXPR_ARRAYA;
                expr->type.t = sym->_type.t;
                expr->sym_value = sym;

                tok = tok->next;
                if (!sym->_type.is_array)
                {
                    fprintf(stderr,
                        "Error on line : %lu\n\tCan't index something that is not an array\n",
                        tok->lineno);
                }

                tok = tok->next;

                expr->access = expr_create(&tok, INTEGER);

                if (!token_check(tok, RBRACKET))
                {
                    fprintf(stderr, "Error\n");
                    cc_exit();
                }

            }
            else
            {
                expr->expr_type = EXPR_SYMBOL;
                expr->sym_value = sym;
                expr->type.t = sym->_type.t;
            }

        }

        else if (is_declared_func(symtab_g, tok->value.p, &sym))
        {
            if ((sym->_type.t != INTEGER) && (sym->_type.t != _BYTE) && (sym->_type.t != _CHAR) && (sym->_type.t != STRING))
            {
                fprintf(stderr,
                    "Error on line : %lu\n\t Invalid type in expression\n", 
                    tok->lineno);
                free_expression(expr);
                cc_exit();
            }

            free(expr);
            char *name = tok->value.p;
            
            tok = tok->next;
            expr = expr_create_funccall(&tok, name);
            expr->expr_type = EXPR_FUNCCALL;

            *token = tok;
            return expr;
        }

        else
        {
            free_expression(expr);
            undeclared_variable_error(tok->value.p, tok->lineno);
        }


        tok = tok->next;
    }

    *token = tok;
    return expr;
}

Expression_t *expr_term(Token_t **token, enum Type t)
{
    Token_t *tok = *token;

    Expression_t *node = expr_factor(&tok, t);
    Expression_t *tmp;

    while (token_checks(tok, 3, MUL, DIV, MODULO))
    {
        Expression_t *expr = xmalloc(sizeof(Expression_t));

        expr_init(expr);

        if (tok->type == MUL)
            expr->expr_type = EXPR_MUL;
        else if (tok->type == DIV)
            expr->expr_type = EXPR_DIV;
        else 
            expr->expr_type = EXPR_MOD;


        tok = tok->next;

        Expression_t *factor = expr_factor(&tok, t);

        if (factor == NULL)
            invalid_syntax_error(tok);

        tmp = node;

        node = expr;

        node->left = factor;
        node->right = tmp;
    }

    *token = tok;

    return node;
}

Expression_t *expr_(Token_t **token, enum Type t)
{
    Token_t *tok = *token;

    Expression_t *node = expr_term(&tok, t);
    Expression_t *tmp;

    while (token_checks(tok, 2, PLUS, MINUS))
    {
        Expression_t *expr = xmalloc(sizeof(Expression_t));

        expr_init(expr);

        if (tok->type == PLUS)
            expr->expr_type = EXPR_PLUS;
        else
            expr->expr_type = EXPR_MINUS;

        tok = tok->next;
        Expression_t *term = expr_term(&tok, t);
    
        if (term == NULL)
            invalid_syntax_error(tok);

        tmp = node;
        node = expr;

        node->left = term;
        node->right = tmp;

    }

    *token = tok;
    return node;
}

Expression_t *expr_create_funccall(Token_t **token, char *name)
{
    Token_t *tok = *token;
    Token_t *next_token = tok->next;

    Expression_t *expr = xmalloc(sizeof(Expression_t));

    expr_init(expr);

    expr->expr_type = EXPR_FUNCCALL;
    expr->string_value = name;

    Symbol_t *sym = NULL;

    if (!is_declared_func(symtab_g, name, &sym))
        undeclared_variable_error(name, tok->lineno);

    Args_t *args = sym->decl->args;

    assert (tok->type == LPAR);

    if (token_check(next_token, RPAR))
    {

        if (args != NULL)
        {
            fprintf(stderr, "Error on line : %lu\n\tmissing parameter(s) for function '%s'\n",
                tok->lineno, 
                expr->string_value);
            free(expr);
            cc_exit();
        }

        next_token = next_token->next;

        *token = next_token;
        return expr;
    }

    else if (!token_check(next_token, RPAR))
    {
        expr->args = get_args(&next_token, _VOID);

        if (!token_expect(next_token, RPAR))
        {
            free_args(args);
            cc_exit();
        }
    
        next_token = next_token->next;
    }

    else
    {
        free_expression(expr);
        cc_exit();
    }

    Args_t *c_args = expr->args;

    sym = find_corresponding_function(name, c_args);
    if (sym == NULL)
    {
        fprintf(stderr, 
            "Error on line : %lu\n\tCan't find a function with matching prototype\n",
            tok->lineno);
        free(expr);
        cc_exit();
    }

    expr->sym_value = sym;
    expr->type = sym->_type;

    *token = next_token;
    return expr;

}

Expression_t *expr_create_cond(Token_t **token, enum Type t)
{
    Token_t *tok = *token;

    Expression_t *expr = xmalloc(sizeof(Expression_t));

    expr_init(expr);

    expr->expr_type = EXPR_COND;
    expr->left = expr_create(&tok, t);

    Type_s type = type_of_first_symbol(expr->left);

    if (token_checks(tok, 6, CMP, OP_LOWER, OP_GREATER, DIFF, OP_GREATER_EQ, OP_LOWER_EQ))
    {

        if (tok->type == CMP) expr->cond_type = EXPR_CMP;
        else if (tok->type == OP_GREATER) expr->cond_type = EXPR_GREATER;
        else if (tok->type == OP_LOWER) expr->cond_type = EXPR_LOWER;
        else if (tok->type == DIFF) expr->cond_type = EXPR_DIFF;
        else if (tok->type == OP_GREATER_EQ) expr->cond_type = EXPR_GREATER_EQ;
        else if (tok->type == OP_LOWER_EQ) expr->cond_type = EXPR_LOWER_EQ;

        tok = tok->next;
        expr->right = expr_create(&tok, type.t);

        type_set(expr, type);

        if (type_evaluate(expr->left, type.t).t != type_evaluate(expr->right, type.t).t)
        {
            fprintf(stderr, 
                "Error on line : %lu\n\tComparison between two differents type...\n",
                 tok->lineno);
            free_expression(expr);
            cc_exit();
        }
    }

    else
    {
        free_expression(expr);
        invalid_syntax_error(tok);
    }

    *token = tok;
    return expr;
}

Expression_t *expr_fold(Expression_t *expr)
{
    if (expr->left != NULL)
        expr->left = expr_fold(expr->left);

    if (expr->right != NULL)
        expr->right = expr_fold(expr->right);

    if (((expr->left != NULL) && (expr->left->expr_type == EXPR_NUMBER))
        && ((expr->right != NULL) && (expr->right->expr_type == EXPR_NUMBER)))
    {
        
        Expression_t *e = xmalloc(sizeof(Expression_t));
        expr_init(e);
    
        e->expr_type = EXPR_NUMBER;
        e->type = expr->left->type;


        switch (expr->expr_type)
        {
            case EXPR_PLUS:
                e->int_value = expr->right->int_value + expr->left->int_value;
                break;
            case EXPR_MINUS:
                e->int_value = expr->right->int_value - expr->left->int_value;
                break;
            case EXPR_DIV:
                if (expr->left->int_value == 0)
                {
                    fprintf(stderr, 
                        "Error: \n\tDivision by 0!\n");
                    free_expression(expr);
                    free(e);
                    cc_exit();
                }
                e->int_value = expr->right->int_value / expr->left->int_value;
                break;
            case EXPR_MUL:
                e->int_value = expr->right->int_value * expr->left->int_value;
                break;
            case EXPR_MOD:
                e->int_value = expr->right->int_value % expr->left->int_value;
                break;
            default:
                fprintf(stderr,
                    "Error\n\tUnknow error\n");
                free_expression(expr);
                free(e);
                cc_exit();
                break;
        }

        free_expression(expr->left); expr->left = NULL;
        free_expression(expr->right); expr->right = NULL;
        free(expr);

        return e;

    }
    else
    {
        return expr;
    }
}

void expr_init(Expression_t *expr)
{
    expr->left = NULL;
    expr->right = NULL;
    expr->args = NULL;
    expr->access = NULL;
    expr->string_value = NULL;
    expr->sym_value = NULL;
    expr->sym = NULL;
    expr->reg = UINT_MAX;
    expr->int_value = 0;
}

void free_expression(Expression_t *expr)
{

    if (expr == NULL)
        return;

    if (expr->right != NULL)
        free_expression(expr->right);

    if (expr->left != NULL)
        free_expression(expr->left);

    if (expr->access != NULL)
        free_expression(expr->access);

    if (expr->args != NULL)
        free_args(expr->args);

    if (expr->expr_type == EXPR_STRING_LITTERAL)
        free(expr->string_value);
    
    free(expr);
}