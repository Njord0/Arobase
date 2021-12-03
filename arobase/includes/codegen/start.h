#ifndef _CODEGEN_START_H
#define _CODEGEN_START_H

#include <ast.h>
#include <symbol_table.h>
#include <statements.h>

void begin_codegen(AST_t *ast, const char *out);
void emit_statements(Statement_t **statement);

unsigned int reg_alloc(unsigned int r);
const char *reg_name(unsigned int r);
const char *reg_name_l(unsigned int r);
void reg_free(Expression_t *expr);

/* For floating point instruction */
unsigned int xmm_reg_alloc(unsigned int reg);
const char* xmm_reg_name(unsigned int r);
void xmm_reg_free(Expression_t *expr);

void load_to_reg(Expression_t *expr);
void store_to_stack(Expression_t *expr, Symbol_t *sym);
int new_label();

char* symbol_s(Symbol_t *sym);
void _start_def();

extern bool NO_START;
extern FILE *file;

extern const char *scratch_regs[];
extern const char *scratch_regs_l[];
extern const char *args_regs[];
extern char scratch_in_use[7];

extern const char *xmm_args_regs[];
extern const char *xmm_regs[];
extern char xmm_in_use[8];

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