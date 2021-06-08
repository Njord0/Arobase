#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <error_handler.h>
#include <symbol_table.h>


Symtable_t *symtab_create()
{
    Symtable_t *st = xmalloc(sizeof(Symtable_t));

    st->scope = xmalloc(sizeof(ScopeTable_t));

    st->scope->next = NULL;
    st->scope->symbol = NULL; 

    return st;
}

void symtab_free(Symtable_t *st)
{
    if (st == NULL)
        return;

    if (st->scope != NULL)
        scopetable_free(st->scope);

    free(st);
}


void scopetable_free(ScopeTable_t *st)
{
    if (st == NULL)
        return;

    if (st->next != NULL)
        scopetable_free(st->next);

    free(st);
}

void scope_enter()
{
    ScopeTable_t *scope = xmalloc(sizeof(ScopeTable_t));

    scope->next = symtab_g->scope;
    symtab_g->scope = scope; // latest scope

    scope->symbol = NULL;
}

void scope_exit()
{
    ScopeTable_t *scope = symtab_g->scope;
    if (scope->next == NULL)
    {
        fprintf(stderr, 
            "Can't exit GLOBAL scope\n");
        cc_exit();
    }


    symtab_g->scope = scope->next;
    scope->next = NULL; // to avoid next one being free'd
    scopetable_free(scope);
}

void symbol_pos()
{
    ScopeTable_t *scope = symtab_g->scope;
    Symbol_t *sym = scope->symbol;

    unsigned int pos = 0;

    while (sym != NULL)
    {
        sym->pos = pos++;
        sym = sym->next;
    }

}


void add_symbol(Symtable_t *symtab, Declaration_t *decl)
{
    Symbol_t *sym = NULL;
    Symbol_t *prev = NULL;
    ScopeTable_t *scope = symtab->scope;

    Symbol_t *st = xmalloc(sizeof(Symbol_t));

    st->decl = decl;
    st->next = NULL;
    st->type = LOCAL ? (scope->next != NULL): GLOBAL; 
    st->name = st->decl->name;
    st->_type = decl->type;

    decl->sym = st;

    sym  = scope->symbol;

    if (sym == NULL)
    {
        scope->symbol = st;
    }
    else 
    {
        while (sym != NULL)
        {
            prev = sym;
            sym = sym->next;
        }

        prev->next = st;
    }

}

void add_symbol_from_args(Symtable_t *symtab, Args_t *args)
{
    Symbol_t *sym = NULL;
    Symbol_t *prev = NULL;

    ScopeTable_t *scope = symtab->scope;

    if (args == NULL)
        return;
    Symbol_t *st = xmalloc(sizeof(Symbol_t));

    st->decl = NULL;
    st->next = NULL;
    st->type = LOCAL;
    st->name = args->name;
    st->_type = args->type;

    sym = scope->symbol;
    prev = sym;

    args->sym = st;

    if (sym == NULL)
    {
        scope->symbol = st;
    }

    else
    {
        while (sym != NULL)
        {
            prev = sym;
            sym = sym->next;
        }

        prev->next = st;
    }

    if ((args->next != NULL))
            add_symbol_from_args(symtab, args->next);

}

Symbol_t *symbol_resolve(Symtable_t *symtab, const char *name)
{

    ScopeTable_t *scope = symtab->scope;
    Symbol_t *symbol = NULL;

    while (scope != NULL)
    {
        symbol = scope->symbol;
        while (symbol != NULL)
        {
            if (strcmp(symbol->name, name) == 0)
                return symbol;

            symbol = symbol->next;
        }
        scope = scope->next;
    }

    return NULL;

}

bool is_declared_func(Symtable_t *symtab, const char *name, Symbol_t **symbol)
{

    Symbol_t *sym = symbol_resolve(symtab, name);
    *symbol = sym;

    return ((sym != NULL) && (sym->decl->decl_type == FUNCTION));

}

bool is_declared_var(Symtable_t *symtab, const char *name, Symbol_t **symbol)
{
    Symbol_t *sym = symbol_resolve(symtab, name);

    *symbol = sym;

    if ((sym != NULL) && (sym->decl == NULL))
        return true;

    return ((sym != NULL) && (sym->decl->decl_type == VARIABLE));
 }