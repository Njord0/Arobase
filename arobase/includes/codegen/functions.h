#ifndef _CODEGEN_FUNCTION_H
#define _CODEGEN_FUNCTION_H

#include <statements.h>
#include <expressions.h>

void emit_epilogue();
void emit_prologue(Statement_t *stmt);

unsigned int get_stack_size(Statement_t *stmt);

void emit_function(Statement_t **statement);
void emit_func_call(Expression_t *expr);
void emit_return(Statement_t *stmt);

/* Moving arguments from registers rdi, rsi, rcx to stack */
void emit_move_args_to_stack(Args_t *args);


#endif