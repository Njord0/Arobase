#ifndef _CODEGEN_EXCEPTIONS_H
#define _CODEGEN_EXCEPTIONS_H

#include <statements.h>

void emit_try_block(Statement_t *stmt);
void emit_raise(Statement_t *stmt);

#endif