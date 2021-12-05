#ifndef _CODEGEN_START_H
#define _CODEGEN_START_H

#include <ast.h>
#include <symbol_table.h>
#include <statements.h>

void begin_codegen(AST_t *ast, const char *out);
void emit_statements(Statement_t **statement);

void load_to_reg(Expression_t *expr);
void store_to_stack(Expression_t *expr, Symbol_t *sym);
int new_label();

char* symbol_s(Symbol_t *sym);
void _start_def();

extern bool NO_START;
extern FILE *file;


extern char symbol_stack_pos[100];
extern bool in_function_call;


#define emit(...) \
        fprintf(file, __VA_ARGS__)


typedef struct {        
    void *prev; 
    int lbl;    
} LOOP;

extern LOOP *current_loop;

#endif