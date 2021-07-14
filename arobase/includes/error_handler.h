#ifndef _ERROR_HANDLER
#define _ERROR_HANDLER

#include <statements.h>
#include <tokens.h>
#include <lexer.h>
#include <ast.h>

void invalid_syntax_error(Token_t *token);
void undeclared_variable_error(const char *name, long unsigned int line);
void missing_type(long unsigned int line);
void invalid_variable_name(const char *ptr);

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);


void cc_exit();

extern Statement_t *current_function;

#endif