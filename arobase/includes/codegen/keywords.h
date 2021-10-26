#ifndef _CODEGEN_KEYWORDS_H
#define _CODEGEN_KEYWORDS_H

#include <statements.h>

void emit_print(Statement_t *stmt);
void emit_input(Statement_t *stmt);
void emit_assert(Statement_t *stmt);

#endif