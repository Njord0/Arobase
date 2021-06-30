#ifndef _TOKENS_H
#define _TOKENS_H

#include <stdint.h>
#include <stdbool.h>

enum TokensTypes {
    NUMBER,
    PLUS,
    MINUS,
    MUL,
    DIV,
    LPAR,
    RPAR,
    LBRACE, // {
    RBRACE, // }
    DQUOTE,
    COLON,
    ASSIGN,
    SYMBOL,
    COMMA,
    CMP,
    OP_LOWER,
    OP_GREATER,
    EOS, // End Of Statement
    KEYWORD,
    AROBASE,
    LBRACKET,
    RBRACKET,
    DIFF,
    QUOTE,
    TOK_STRING,
    DOT,
    OP_LOWER_EQ,
    OP_GREATER_EQ,
    MODULO,
};

typedef struct Token {
    enum TokensTypes type;
    union
    {
        int64_t i;
        char c;
        char *p;
    } value;

    unsigned long int lineno;
    struct Token *next;

} Token_t;


Token_t *create_token_number(int64_t value);
Token_t *create_token_char(int type, char c);
Token_t *create_token_s(char *ptr);

bool token_symbol_is_reserved(const char *str);

bool token_expect(Token_t *tok, enum TokensTypes t);
bool token_check(Token_t *tok, enum TokensTypes t);

bool token_checks(Token_t *tok, unsigned int count, ...);

void free_token(Token_t *token);

extern const char *tokens[29];

#endif