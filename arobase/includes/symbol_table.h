#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H

#include <declarations.h>

enum SymbolType {
    GLOBAL,
    LOCAL,
    ARG,
};

struct _scope;
struct _symbol;

typedef struct _symbol {
    enum SymbolType type;
    struct decl_ *decl;
    struct _symbol *next; // next symbol in scope, order is meaningless
    const char *name; // Symbol name as used in the program
    char *rname; // Symbol real name, for function only after name mangling
    Type_s _type;
    unsigned int pos;
} Symbol_t;


typedef struct _scope {
    struct _scope *next;
    Symbol_t *symbol; 

} ScopeTable_t;

typedef struct symtable {
    ScopeTable_t *scope;

} Symtable_t; // Symtable_t

Symtable_t *symtab_create();
void symtab_free(Symtable_t *st);
void scopetable_free(ScopeTable_t *st);

/* Add a scope on top of stack */
void scope_enter();

/* Pop a scope from stack */
void scope_exit();

void symbol_pos();

void add_symbol(Symtable_t *symtab, Declaration_t *decl);
void add_symbol_from_args(Symtable_t *symtab, Args_t *args);

Symbol_t *symbol_resolve(Symtable_t *symtab, const char *name);
Symbol_t *symbol_resolve_func(Symtable_t *symtab, const char *name);


bool is_declared_func(Symtable_t *symtab, const char *name, Symbol_t **symbol);
bool is_declared_var(Symtable_t *symtab, const char *name, Symbol_t **symbol);

Symbol_t *find_matching_function(const char *name, Args_t *c_args);

void import_from(const char *str);


extern Symtable_t *symtab_g;
extern int fcount;

#endif