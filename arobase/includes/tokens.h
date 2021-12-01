#ifndef _TOKENS_H
#define _TOKENS_H

#include <stdint.h>
#include <stdbool.h>

enum TokensTypes {
    TOK_INTEGER,
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
    TOK_CHAR,
};


typedef struct Token {
    enum TokensTypes type; // The kind of the token
    union
    {
        int64_t i;
        char c;
        char *p;
    } value; // An union to store the value associated with the token, depends on the token type

    unsigned long int lineno; // The line on which the token was lexed
    struct Token *next; // Pointer to the next token

} Token_t;


Token_t *create_token_integer(int64_t value);
Token_t *create_token_char(int type, char c);
Token_t *create_token_s(char *ptr);

bool token_symbol_is_reserved(const char *str);

bool token_expect(Token_t *tok, enum TokensTypes t);
bool token_check(Token_t *tok, enum TokensTypes t);
bool token_checks(Token_t *tok, unsigned int count, ...);

void free_token(Token_t *token);

extern const char *tokens[29];

#endif