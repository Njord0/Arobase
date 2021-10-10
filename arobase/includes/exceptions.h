#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#include <statements.h>
#include <tokens.h>

Statement_t *stmt_parse_try_block(Token_t **token);

#endif // _EXCEPTIONS_H