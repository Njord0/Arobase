#ifndef _ERROR_HANDLER
#define _ERROR_HANDLER

#include <statements.h>
#include <tokens.h>
#include <lexer.h>
#include <ast.h>

void invalid_syntax_error(Token_t *token);

void show_error_source(Token_t *token);
char *dump_line(Token_t *token);

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);

void cc_exit();

extern Statement_t *current_function;

#endif