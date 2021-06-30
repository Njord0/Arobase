#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <error_handler.h>
#include <expressions.h>
#include <tokens.h>
#include <type.h>
#include <statements.h>
#include <symbol_table.h>

Type_s get_type(Token_t **token)
{
    Token_t *tok = *token;

    if (!token_expect(tok, COLON))
        missing_type(tok->lineno);

    
    tok = tok->next;

    if (!token_expect(tok, KEYWORD))
        missing_type(tok->lineno);


    Type_s type;

    if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_INT]) == 0)
    {
        type.t = INTEGER;
        type.is_array = false;
        type.ptr = NULL;
    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_BYTE]) == 0)
    {
        type.t = _BYTE;
        type.is_array = false;
        type.ptr = NULL;
    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_VOID]) == 0)
    {
        fprintf(stderr, 
            "Error on line : %lu\n\tCan't declare variable of type 'void'\n",
            tok->lineno);
        cc_exit();

    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_CHAR]) == 0)
    {
        type.t = _CHAR;
        type.is_array = false;
        type.ptr = NULL;
    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_STR]) == 0)
    {
        type.t = STRING,
        type.is_array = false;
        type.ptr = NULL;
    }
    
    else
    {
        fprintf(stderr, 
            "Error on line : %lu\n\tUnknow type '%s'\n", 
            tok->lineno, tok->value.p);
        cc_exit();
    }

    tok = tok->next;

    if (token_check(tok, LBRACKET))
    {

        if (type.t == STRING)
        {
            fprintf(stderr,
                "Error on line : %lu\n\tCan't create an array of string\n",
                tok->lineno);

            cc_exit();
        }

        tok = tok->next;

        if (!token_expect(tok, NUMBER))
            cc_exit();


        type.is_array = true;
        type.ptr = (void*)type_create_array(type.t, (unsigned int)tok->value.i);

        tok = tok->next;

        if (!token_expect(tok, RBRACKET))
            cc_exit();

        tok = tok->next;

    }

    *token = tok;

    return type;
}


Type_s get_type_decl(Token_t **token)
{
    Token_t *tok = *token;

    if (!token_expect(tok, COLON))
        missing_type(tok->lineno);

    
    tok = tok->next;

    if (!token_expect(tok, KEYWORD))
        missing_type(tok->lineno);


    Type_s type;

    if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_INT]) == 0)
    {
        type.t = INTEGER;
        type.is_array = false;
        type.ptr = NULL;
    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_BYTE]) == 0)
    {
        type.t = _BYTE;
        type.is_array = false;
        type.ptr = NULL;
    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_VOID]) == 0)
    {
        fprintf(stderr, 
            "Error on line : %lu\n\tCan't declare variable of type 'void'\n",
            tok->lineno);
        cc_exit();

    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_CHAR]) == 0)
    {
        type.t = _CHAR;
        type.is_array = false;
        type.ptr = NULL;
    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_STR]) == 0)
    {
        type.t = STRING,
        type.is_array = false;
        type.ptr = NULL;
    }
    
    else
    {
        fprintf(stderr, 
            "Error on line : %lu\n\tUnknow type '%s'\n", 
            tok->lineno, tok->value.p);
        cc_exit();
    }

    tok = tok->next;

    if (token_check(tok, LBRACKET))
    {

        if (type.t == STRING)
        {
            fprintf(stderr,
                "Error on line : %lu\n\tCan't create an array of string\n",
                tok->lineno);

            cc_exit();
        }

        type.is_array = true;
        type.ptr = (void*)type_create_array(type.t, (unsigned int)tok->value.i);

        tok = tok->next;

        if (!token_expect(tok, RBRACKET))
            cc_exit();

        tok = tok->next;

    }

    *token = tok;

    return type;
}


Type_s type_evaluate(Expression_t *expr, enum Type t)
{
    Type_s type;
    Symbol_t *sym;
    Type_s left;
    Type_s right;

    type.is_array = false;
    type.ptr = NULL;

    if (expr == NULL)
    {
        type.t = _VOID;
        return type;
    }

    switch (expr->expr_type)
    {
        case EXPR_PLUS:
        case EXPR_MINUS:
        case EXPR_DIV:
        case EXPR_MUL:
        case EXPR_MOD:
            if (expr->left != NULL)
                left = type_evaluate(expr->left, t);

            if (expr->right != NULL)
                right = type_evaluate(expr->right, t);


            if (left.t != right.t)
            {
                fprintf(stderr, 
                    "Error:\n\t can't use operator '?' between '%s' and '%s'\n",
                    type_name(left.t),
                    type_name(right.t));
                cc_exit();
            }
            type.t = left.t;
            break;

        case EXPR_NUMBER:
            if (t == INTEGER)
                type.t = INTEGER;
            else if (t == _BYTE)
                type.t = _BYTE;
            else
                type.t = INTEGER; //default choice
            break;

        case EXPR_ARRAYA:
        case EXPR_SYMBOL:
            if (!is_declared_var(symtab_g, expr->string_value, &sym))
            {
                fprintf(stderr, "Error:\n\tUndeclared variable\n");
                cc_exit();
            }

            type = sym->_type;
            break;

        case EXPR_FUNCCALL:
            if (!is_declared_func(symtab_g, expr->string_value, &sym))
                undeclared_variable_error(expr->string_value, 0);
            sym = find_corresponding_function(expr->string_value, expr->args);
            if (sym == NULL)
                cc_exit();

            type = sym->_type;
            break;

        case EXPR_CHAR:
            type.t = _CHAR;
            break;

        case EXPR_STRING_LITTERAL:
            type.t = STRING;
            break;

        case EXPR_UNARY_MINUS:
            type.t = type_evaluate(expr->left, t).t;
            break;

        default:
            fprintf(stderr, "Type checking error...\n");
            cc_exit();
            break;

    }

    return type;

}

void type_check(Expression_t *expr)
{

    if (expr == NULL)
        return;

    if ((expr->left == NULL) && (expr->right == NULL) && (expr->expr_type == EXPR_NUMBER))
    {
        if (expr->type.t == _BYTE)
        {
            if ((expr->int_value < 0) || (expr->int_value > 255))
            {
                fprintf(stderr, 
                    "Warning : \n\tByte value must be in range 0-255.\n\tConverting '%ld 'to '%ld'\n",
                    expr->int_value,
                    expr->int_value & 255);

                expr->int_value &= 255;
            }
        }

        else if (expr->type.t == INTEGER)
        {

        }

    }

}

Type_s type_of_first_symbol(Expression_t *expr)
{
    Expression_t *l = expr->left;
    Expression_t *r = expr->right;

    if (expr->expr_type == EXPR_SYMBOL)
        return expr->sym_value->_type;

    if ((l != NULL) && ((l->expr_type == EXPR_SYMBOL) || (l->expr_type == EXPR_FUNCCALL)))
        return l->sym_value->_type;
        
    else if ((r != NULL) && ((r->expr_type == EXPR_SYMBOL) || (r->expr_type == EXPR_FUNCCALL)))
        return r->sym_value->_type;

    if (l != NULL)
        return type_of_first_symbol(l);

    else if (r != NULL)
        return type_of_first_symbol(r);

    
    Type_s type;
    type.t = expr->type.t;
    type.is_array = false;
    type.ptr = NULL;

    return type;

}

void type_set(Expression_t *expr, Type_s type)
{
    if (expr == NULL)
        return;

    if (expr->left != NULL)
        type_set(expr->left, type);

    if (expr->right != NULL)
        type_set(expr->right, type);

    if (expr != NULL)
        expr->type = type;
}

Array_s *type_create_array(enum Type t, unsigned int size)
{
    Array_s *array = xmalloc(sizeof(Array_s));

    array->type = t;
    array->size = size;

    return array;
}

char *type_name(enum Type t)
{
    switch (t)
    {
        case INTEGER:
            return "integer";
        case _BYTE:
            return "byte";
        case _VOID:
            return "void";
        case _BOOL:
            return "bool";
        case _CHAR:
            return "char";
        case STRING:
            return "string";
    }
}