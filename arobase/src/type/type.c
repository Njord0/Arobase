#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errors/error.h>
#include <expressions.h>
#include <tokens.h>
#include <type.h>
#include <struct.h>
#include <statements.h>
#include <symbol_table.h>

#define MISSING_TYPE(X) \
    show_error_source(X); \
        fprintf(stderr, \
            "Missing type\n"); \
        cc_exit(); \

Type_s
get_type(Token_t **token)
{
    Token_t *tok = *token;

    if (!token_expect(tok, COLON))
    {
        MISSING_TYPE(tok);
    }
    
    tok = tok->next;

    if (!token_checks(tok, 2 , KEYWORD, SYMBOL))
    {
        MISSING_TYPE(tok);
    }
    

    Type_s type;
    type.is_array = false;
    type.is_structure = false;
    type.ptr = NULL;

    if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_INT]) == 0)
    {
        type.t = INTEGER;
    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_BYTE]) == 0)
    {
        type.t = _BYTE;
    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_VOID]) == 0)
    {
        show_error_source(tok);
        fprintf(stderr, 
            "Can't declare variable of type 'void'\n");
        cc_exit();

    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_CHAR]) == 0)
    {
        type.t = _CHAR;
    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_STR]) == 0)
    {
        type.t = STRING;
    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_FLOAT]) == 0)
    {
        type.t = _FLOAT;
    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_BOOL]) == 0)
    {
        type.t = _BOOL;
    }
    
    else
    {
        if (is_defined_struct(tok->value.p))
        {
            type.t = STRUCTURE;
            type.is_structure = true;
            type.ptr = tok->value.p;
        }

        else
        {
            show_error_source(tok);
            fprintf(stderr, 
                "Unknow type '%s'\n", 
                tok->value.p);
            cc_exit();
        }
    }

    tok = tok->next;

    if (token_check(tok, LBRACKET))
    {

        if (type.t == STRING)
        {
            show_error_source(tok);
            fprintf(stderr,
                "Can't create an array of string\n");
            cc_exit();
        }

        tok = tok->next;

        if (!token_expect(tok, TOK_INTEGER))
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


Type_s
get_type_decl(Token_t **token)
{
    Token_t *tok = *token;

    if (!token_expect(tok, COLON))
    {
        MISSING_TYPE(tok);
    }
    
    tok = tok->next;

    if (!token_checks(tok, 2 , KEYWORD, SYMBOL))
    {
        MISSING_TYPE(tok);
    }

    Type_s type;
    type.is_array = false;
    type.is_structure = false;
    type.ptr = NULL;

    if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_INT]) == 0)
    {
        type.t = INTEGER;
    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_FLOAT]) == 0)
    {
        type.t = _FLOAT;
    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_BYTE]) == 0)
    {
        type.t = _BYTE;
    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_VOID]) == 0)
    {
        show_error_source(tok);
        fprintf(stderr, 
            "Can't declare variable of type 'void'\n");
        cc_exit();
    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_CHAR]) == 0)
    {
        type.t = _CHAR;
    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_STR]) == 0)
    {
        type.t = STRING;
    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_FLOAT]) == 0)
    {
        type.t = _FLOAT;
    }

    else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_BOOL]) == 0)
    {
        type.t = _BOOL;
    }
    
    else
    {
        if (is_defined_struct(tok->value.p))
        {
            type.t = STRUCTURE;
            type.is_structure = true;
            type.ptr = tok->value.p;
        }
        else
        {
            show_error_source(tok);
            fprintf(stderr, 
                "Unknow type '%s'\n", 
                tok->value.p);
            cc_exit();
        }

    }

    tok = tok->next;

    if (token_check(tok, LBRACKET))
    {

        if (type.t == STRING)
        {
            show_error_source(tok);
            fprintf(stderr,
                "Can't create an array of string\n");
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


Type_s
type_evaluate(Expression_t *expr, enum Type t)
{
    Type_s type;
    Symbol_t *sym;
    Type_s left = {_VOID, false, NULL};
    Type_s right = {_VOID, false, NULL};

    type.is_array = false;
    type.is_structure = false;
    type.ptr = NULL;

    if (!expr)
    {
        type.t = _VOID;
        return type;
    }

    char c;

    switch (expr->expr_type)
    {
        case EXPR_PLUS:
        case EXPR_MINUS:
        case EXPR_DIV:
        case EXPR_MUL:
        case EXPR_MOD:
            if (expr->left)
                left = type_evaluate(expr->left, t);

            if (expr->right)
                right = type_evaluate(expr->right, t);

            c = expr->expr_type == EXPR_PLUS ? '+' : expr->expr_type == EXPR_MINUS ? '-' : expr->expr_type == EXPR_DIV ? '/' : expr->expr_type == EXPR_MUL ? '*' : '%'; 

            if (left.t != right.t)
            {
                show_error_source(expr->token);
                fprintf(stderr, 
                    "can't use operator '%c' between '%s' and '%s'\n",
                    c,
                    type_name(left.t),
                    type_name(right.t));
                cc_exit();
            }

            if (expr->expr_type == EXPR_MOD && (left.t == _FLOAT || right.t == _FLOAT)) // Mod operator can't be used with float type
            {
                show_error_source(expr->token);
                fprintf(stderr,
                    "can't use operator '%%' with 'float' type\n");
                cc_exit();
            }

            if (left.t == STRING || left.t == _CHAR)
            {
                show_error_source(expr->token);
                fprintf(stderr,
                    "can't use operator '%c' with '%s' type\n",
                    c,
                    type_name(left.t));
                cc_exit();
            }

            type.t = left.t;
            break;

        case EXPR_INTEGER:
            if (t == INTEGER)
                type.t = INTEGER;
            else if (t == _BYTE)
                type.t = _BYTE;
            else
                type.t = INTEGER; //default choice
            break;
        case EXPR_FLOAT:
            type.t = _FLOAT;
            break;

        case EXPR_ARRAYA:
        case EXPR_SYMBOL:
            if (!is_declared_var(symtab_g, expr->string_value, &sym))
            {
                show_error_source(expr->token);
                fprintf(stderr, 
                    "undeclared variable '%s'\n",
                    expr->string_value);
                cc_exit();
            }

            type = sym->_type;
            break;

        case EXPR_FUNCCALL:
            if (!is_declared_func(symtab_g, expr->string_value, &sym))
                cc_exit();
            
            sym = find_matching_function(expr->string_value, expr->args);
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
            if (type.t == STRING || type.t == _CHAR || type.t == STRUCTURE)
            {
                show_error_source(expr->token);
                fprintf(stderr,
                    "can't negate '%s' type\n",
                    type_name(type.t));
                cc_exit();
            }
            break;

        case EXPR_STRUCTA:
            type.t = expr->type.t;
            break;
        case EXPR_COND:
            type.t = _BOOL;
            break;            

        default:
            fprintf(stderr, "Type checking error...\n");
            cc_exit();
            break;
    }

    return type;
}

void
type_check(Expression_t *expr)
{
    if (!expr)
        return;

    if (expr->left && expr->right && (expr->expr_type == EXPR_INTEGER))
    {
        if (expr->type.t == _BYTE)
        {
            /*
                Checks if byte value is in [0;255]
            */
            if ((expr->int_value < 0) || (expr->int_value > 255))
            {
                fprintf(stderr, 
                    "Warning : \n\tByte value must be in range 0-255.\n\tConverting '%ld 'to '%ld'\n",
                    expr->int_value,
                    expr->int_value & 255);

                expr->int_value &= 255;
            }
        }
    }
}

Type_s
type_of_first_symbol(Expression_t *expr)
{
    Expression_t *l = expr->left;
    Expression_t *r = expr->right;

    if (expr->expr_type == EXPR_SYMBOL)
        return expr->sym_value->_type;

    if (l && ((l->expr_type == EXPR_SYMBOL) || (l->expr_type == EXPR_FUNCCALL)))
        return l->sym_value->_type;
        
    else if (r && ((r->expr_type == EXPR_SYMBOL) || (r->expr_type == EXPR_FUNCCALL)))
        return r->sym_value->_type;

    if (l)
        return type_of_first_symbol(l);

    else if (r)
        return type_of_first_symbol(r);

    if (expr->expr_type == EXPR_UNARY_MINUS)
        printf("here\n");

    Type_s type;
    type.t = expr->type.t;
    type.is_array = false;
    type.ptr = NULL;

    return type;
}

void
type_set(Expression_t *expr, Type_s type)
{
    if (!expr)
        return;
        
    if (type.t == _BOOL)
    {
        expr->type = type;
        return;
    }

    if (expr->left)
        type_set(expr->left, type);

    if (expr->right)
        type_set(expr->right, type);


    if (expr->expr_type == EXPR_UNARY_MINUS)
        expr->type = type;

    if (expr && !((expr->type.t == INTEGER && type.t == _FLOAT) || (expr->type.t == _FLOAT && type.t == INTEGER) || (expr->type.t == _BYTE && type.t == _FLOAT) || (expr->type.t == _FLOAT && type.t == _BYTE)))
        expr->type = type;

    
}

Array_s*
type_create_array(enum Type t, unsigned int size)
{
    Array_s *array = xmalloc(sizeof(Array_s));

    array->type = t;
    array->size = size;

    return array;
}

char*
type_name(enum Type t)
{
    switch (t)
    {
        case INTEGER:
            return "integer";
        case _FLOAT:
            return "float";
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
        case STRUCTURE:
            return "struct";
        default:
            return "unknown";
    }
}