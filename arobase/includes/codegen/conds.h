#ifndef _CODEGEN_CONDS_H
#define _CODEGEN_CONDS_H

#include <statements.h>

void emit_if_else(Statement_t *statement);
void emit_while(Statement_t *statement);
void emit_for(Statement_t *statement);
void emit_break(Statement_t *stmt);


#endif