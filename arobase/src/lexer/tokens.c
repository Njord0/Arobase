#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include <statements.h>
#include <tokens.h>
#include <errors/error.h>

const char *tokens[29] = {
    "number",
    "plus",
    "minus",
    "*",
    "/",
    "(",
    ")",
    "{",
    "}",
    "\"",
    ":",
    "=",
    "symbol",
    ",",
    "==",
    "<",
    ">",
    ";",
    "keyword",
    "@",
    "[",
    "]",
    "!=",
    "'",
    ".",
    "<=",
    ">=",
    "%%"
};

Token_t*
create_token_integer(int64_t value)
{
    Token_t *tok = xmalloc(sizeof(Token_t));

    tok->type = TOK_INTEGER;
    tok->value.i = value;

    return tok;
}
Token_t*
create_token_float(double value)
{
    Token_t *tok = xmalloc(sizeof(Token_t));
    tok->type = TOK_FLOAT;
    tok->value.d = value;

    return tok;
}


Token_t*
create_token_char(int type, char c)
{
    Token_t *tok = xmalloc(sizeof(Token_t));

    tok->type = type;
    tok->value.c = c;

    return tok;
}

Token_t*
create_token_s(char *ptr)
{
    Token_t *tok = xmalloc(sizeof(Token_t));

    tok->type = SYMBOL;
    tok->value.p = ptr;

    return tok;
}

bool
token_symbol_is_reserved(const char *str)
{
    for (int i = 0; i < KW_NO; i++)
    {
        if (strcmp(str, Arobase_ReservedKeywords[i]) == 0)
            return true;
    }
    return false;
}

bool
token_check(Token_t *tok, enum TokensTypes t)
{
    if ((tok == NULL) || (tok->type != t))
        return false;

    return true;
}

bool
token_checks(Token_t *tok, unsigned int count, ...)
{
    if (!tok)
        return false;

    va_list params;
    va_start(params, count);

    unsigned int i = 0;

    while (i < count)
    {
        unsigned int var = (unsigned int)va_arg(params, unsigned int);
        if (tok->type == var)
            return true;
        i++;
    }
    va_end(params);
    return false;
}

void
free_token(Token_t *token)
{
    if (!token)
        return;

    if (token->type == SYMBOL || token->type == TOK_STRING)
        free(token->value.p);
        
    free(token);
}

