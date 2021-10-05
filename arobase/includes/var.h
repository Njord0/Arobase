#ifndef _VAR_H
#define _VAR_H

#include <tokens.h>
#include <statements.h>

Statement_t *stmt_parse_var_declaration(Token_t **token);
Statement_t *stmt_parse_var_assign(Token_t **token);
Statement_t *stmt_parse_struct_assign(Token_t **token, Statement_t *stmt, const char *name);

#endif