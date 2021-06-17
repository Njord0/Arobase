#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <statements.h>
#include <tokens.h>
#include <error_handler.h>

const char *tokens[28] = {
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
    ">="
};

Token_t *create_token_number(int64_t value)
{
    Token_t *tok = xmalloc(sizeof(Token_t));

    tok->type = NUMBER;
    tok->value.i = value;

    return tok;
}

Token_t *create_token_char(int type, char c)
{
    Token_t *tok = xmalloc(sizeof(Token_t));

    tok->type = type;
    tok->value.c = c;

    return tok;
}

Token_t *create_token_s(char *ptr)
{
    Token_t *tok = xmalloc(sizeof(Token_t));

    tok->type = SYMBOL;
    tok->value.p = ptr;

    return tok;
}

bool token_symbol_is_reserved(const char *str)
{
    for (int i = 0; i < KW_NO; i++)
    {
        if (strcmp(str, Arobase_ReservedKeywords[i]) == 0)
            return true;
    }
    return false;
}

bool token_expect(Token_t *tok, enum TokensTypes t)
{
    const char *name;

    if (tok != NULL)
        name = tokens[tok->type];
    else
        name = "none";

    long unsigned int line = 0;

    if (tok != NULL)
        line = tok->lineno;
    

    if ((tok == NULL) || (tok->type != t))
    {
        fprintf(stderr, 
            "Syntax error on line: %lu\n\tExpected '%s', found '%s'\n",
            line,
            tokens[t],
            name);

        return false;
    }

    return true;
}

bool token_check(Token_t *tok, enum TokensTypes t)
{
    if ((tok == NULL) || (tok->type != t))
        return false;

    return true;
}

void free_token(Token_t *token)
{
    if (token == NULL)
        return;

    if ((token->type == SYMBOL))
        free(token->value.p);
        
    free(token);
}

