#ifndef _CODE_GEN
#define _CODE_GEN

#include <stdbool.h>

#include <ast.h>
#include <symbol_table.h>
#include <statements.h>
#include <expressions.h>
#include <type.h>



void begin_codegen(AST_t *ast, const char *out);
void emit_statements(Statement_t **statement);
void emit_function(Statement_t **statement);

void emit_expression(Expression_t *expr, enum Type t);
void emit_var_declaration(Statement_t *statement);
void emit_var_assign(Statement_t *stamement);
void emit_prologue(Statement_t *stmt);
void emit_if_else(Statement_t *statement);
void emit_while(Statement_t *statement);
void emit_func_call(Expression_t *expr);
void emit_return(Statement_t *stmt);
void emit_print(Statement_t *stmt);
void emit_input(Statement_t *stmt);

void emit_array_initialization(Args_t *args, Symbol_t *sym);


void emit_move_args_to_stack(Args_t *args);

void emit_epilogue();
unsigned int get_stack_size(Statement_t *stmt);

unsigned int reg_alloc(unsigned int r);
const char *reg_name(unsigned int r);
const char *reg_name_l(unsigned int r);

void reg_free(Expression_t *expr);

void load_to_reg(Expression_t *expr);
void store_to_stack(Expression_t *expr, Symbol_t *sym);
int new_label();

char* symbol_s(Symbol_t *sym);
void _start_def();

extern bool NO_START;


#endif // _CODE_GEN