#ifndef _FUNCTION_H
#define _FUNCTION_H

#include <statements.h>
#include <tokens.h>


Statement_t *stmt_parse_func_declaration(Token_t **token);
Statement_t *stmt_parse_func_call(Token_t **token);
Statement_t *stmt_parse_return(Token_t **token);


#endif // _FUNCTION_H