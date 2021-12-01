#ifndef _SCOPE_H
#define _SCOPE_H

#include <tokens.h>
#include <statements.h>
#include <declarations.h>

Statement_t *get_scope(Token_t **token, Declaration_t *decl);

#endif