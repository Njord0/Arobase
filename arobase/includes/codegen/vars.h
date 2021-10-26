#ifndef _CODEGEN_VARS_H
#define _CODEGEN_VARS_H

#include <statements.h>
#include <expressions.h>
#include <symbol_table.h>
#include <args.h>


void emit_expression(Expression_t *expr, enum Type t);
void emit_var_declaration(Statement_t *statement);
void emit_var_assign(Statement_t *stamement);

void emit_array_initialization(Args_t *args, Symbol_t *sym);
void emit_structure_initialization(Args_t *args, Symbol_t *sym);

#endif