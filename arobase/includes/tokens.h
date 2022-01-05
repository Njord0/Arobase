#ifndef _TOKENS_H
#define _TOKENS_H

#include <stdint.h>
#include <stdbool.h>
#include <datatype99.h>

enum TokensTypes {
    TOK_INTEGER,
    TOK_FLOAT,
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

datatype(
    TokenKind,
    (Tok_Int, int64_t),
    (Tok_Float, double),
    (Tok_Char, char),
    (Tok_String, char*),
    (Tok_Plus),
    (Tok_Minus),
    (Tok_Mul),
    (Tok_Div),
    (Tok_Lpar),
    (Tok_Rpar),
    (Tok_Lbrace),
    (Tok_Rbrace),
    (Tok_Dquote),
    (Tok_Colon),
    (Tok_Assign),
    (Tok_Symbol),
    (Tok_Comma),
    (Tok_Cmp),
    (Tok_OpLower),
    (Tok_OpGreater),
    (Tok_Eos),
    (Tok_Keyword),
    (Tok_Arobase),
    (Tok_Lbracket),
    (Tok_Rbracket),
    (Tok_Diff),
    (Tok_Quote),
    (Tok_Dot),
    (Tok_OpLowerEq),
    (Tok_OpGreaterEq),
    (Tok_Modulo)
);

typedef struct Token {
    TokenKind kind;
    unsigned long int lineno; // The line on which the token was lexed
    struct Token *next; // Pointer to the next token

} Token_t;

Token_t *create_token_integer(int64_t value);
Token_t *create_token_float(double value);
Token_t *create_token_char(int type, char c);
Token_t *create_token_s(char *ptr);

bool token_symbol_is_reserved(const char *str);

bool token_check(Token_t *tok, enum TokensTypes t);
bool token_checks(Token_t *tok, unsigned int count, ...);

void free_token(Token_t *token);

extern const char *tokens[29];

#endif