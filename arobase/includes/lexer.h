#ifndef _LEXER_H
#define _LEXER_H

#include <stdlib.h>

#include <tokens.h>

typedef struct Lexer {
    const char *filename; // The source file being compiled
    FILE *file;
    unsigned long int pos;
    unsigned long int current_lineno; // For error handling
    Token_t *first_token;
    Token_t *last_token;
} Lexer_t;

Lexer_t *lexer_create(const char *filename);

void lexer_free(Lexer_t *lexer);
void lexer_tokenize(Lexer_t *lexer);

char lexer_skip_whitespaces(Lexer_t *lexer);
Token_t *lexer_get_next_token(Lexer_t *lexer);

char *lexer_get_symbolname(Lexer_t *lexer);
char *lexer_get_string(Lexer_t *lexer);

int lexer_get_integer(Lexer_t *lexer, int64_t *value);

extern Lexer_t *lexer_g; // global lexer

#endif // _LEXER_H