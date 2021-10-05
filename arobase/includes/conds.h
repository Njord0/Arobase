#ifndef _CONDS_H
#define _CONDS_H

#include <tokens.h>
#include <declarations.h>
#include <expressions.h>
#include <args.h>

Statement_t *stmt_parse_if_else(Token_t **token);
Statement_t *stmt_parse_while_loop(Token_t **token);
Statement_t *stmt_parse_for_loop(Token_t **token);
Statement_t *stmt_parse_break(Token_t **token);

#endif // _CONDS_H