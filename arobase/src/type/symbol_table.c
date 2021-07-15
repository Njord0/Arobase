#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <error_handler.h>
#include <symbol_table.h>

int fcount = 0;

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
    {

        Symbol_t *sym = st->scope->symbol;
        Symbol_t *next;
        while (sym != NULL)
        {
            next = sym->next;
            if ((sym->decl != NULL) && (sym->decl->is_imported))
                free_declaration(sym->decl);

            sym = next;
            
        }

        scopetable_free(st->scope);
    }

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
        return;
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

        if ((sym->_type.is_array) && (sym->type != ARG))
        {

            if (sym->_type.t == _CHAR || sym->_type.t == _BYTE)
                pos += ((((Array_s*)(sym->_type.ptr))->size + 7) & (-8)) / 8 + 1;
            else
                pos += ((Array_s*)(sym->_type.ptr))->size;
        }
    
        sym = sym->next;

    }

}


void add_symbol(Symtable_t *symtab, Declaration_t *decl)
{
    Symbol_t *sym = NULL;
    Symbol_t *prev = NULL;
    ScopeTable_t *scope = symtab->scope;


    if (strcmp(decl->name, "main") == 0 && (is_declared_func(symtab_g, "main", &sym)))
    {
        fprintf(stderr,
            "Error\n\tFunction 'main' can't be overloaded!\n");
        cc_exit();
    }

    Symbol_t *st = xmalloc(sizeof(Symbol_t));

    st->decl = decl;
    st->next = NULL;
    st->type = LOCAL ? (scope->next != NULL): GLOBAL; 
    st->name = st->decl->name;
    st->_type = decl->type;
    st->rname = xmalloc(strlen(st->name)+1);

    memset(st->rname, '\x00', strlen(st->name)+1);
    
    strcat(st->rname, st->name);

    if ((strcmp(decl->name, "main") != 0))
    {
        Args_t *args = decl->args;
        while (args != NULL)
        {
            switch (args->type.t)
            {
                case INTEGER:
                    st->rname = realloc(st->rname, strlen(st->rname)+9);
                    strcat(st->rname, "Zinteger");
                    break;
                case STRING:
                    st->rname = realloc(st->rname, strlen(st->rname)+8);
                    strcat(st->rname, "Zstring");
                    break;
                case _CHAR:
                    st->rname = realloc(st->rname, strlen(st->rname)+6);
                    strcat(st->rname, "Zchar");
                    break;
                case _BYTE:
                    st->rname = realloc(st->rname, strlen(st->rname)+6);
                    strcat(st->rname, "Zbyte");
                    break;
            }

            if (args->type.is_array)
            {
                st->rname = realloc(st->rname, strlen(st->rname)+4);
                strcat(st->rname, "Arr");
            }

            args = args->next;
        }
    }

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
    st->type = ARG;
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

Symbol_t *find_corresponding_function(const char *name, Args_t *c_args)
{
    ScopeTable_t *scope = symtab_g->scope;
    Symbol_t *symbol = NULL;

    Args_t *tmp;

    while (scope != NULL)
    {
        symbol = scope->symbol;
        while (symbol != NULL)
        {
            if ((strcmp(symbol->name, name) == 0) && (symbol->decl->decl_type == FUNCTION))
            {    
                /* Checking if function call parameters and function decl parameters are the same */
                Args_t *args = symbol->decl->args;
                tmp = c_args;
                while ((args != NULL) && (tmp != NULL))
                {
                    if ((args->type.t != tmp->type.t) || (args->type.is_array != tmp->type.is_array))
                        break;
                    args = args->next;
                    tmp = tmp->next;
                }

                if ((args == NULL) && (tmp == NULL))
                    return symbol; // symbol matching the prototype...

            }
            symbol = symbol->next;
        }
        scope = scope->next;
    }
    return NULL;

}

void import_from(const char *str)
{

    char *ptr = xmalloc(strlen("/usr/local/include/arobase/")+strlen(str)+1);

    ptr[0] = '\x00';

    strcat(ptr, "/usr/local/include/arobase/");
    strcat(ptr, str);

    Lexer_t *lexer = lexer_create(ptr);
    if (lexer == NULL)
    {

        lexer = lexer_create(str); // trying to import from current folder...
        if (lexer == NULL)
        {
            fprintf(stderr,
                "Error while trying to import '%s'\n\tNo file named '%s'\n",
                str, str);
            free(ptr);
            cc_exit();
        }
        
    }

    free(ptr);

    lexer_tokenize(lexer);

    Token_t *tok = lexer->first_token;

    while (tok != NULL)
    { 
        if ((tok->type != KEYWORD) || (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_FN]) != 0))
        {
            fprintf(stderr,
                "Error on line : %lu in file '%s'\n\tInvalid function prototype in header\n",
                tok->lineno ? tok != NULL : (long unsigned int)0,
                str);
            lexer_free(lexer);
            cc_exit();
        }

        tok = tok->next;

        Declaration_t *decl = xmalloc(sizeof(Declaration_t));
        decl_init(decl);

        if (!token_check(tok, SYMBOL))
        {

            fprintf(stderr,
                "Error in line : %lu in file '%s'\n\tInvalid function name\n",
                tok->lineno ? tok != NULL : (long unsigned int)0,
                str);

            free_declaration(decl);
            lexer_free(lexer);
            cc_exit();
        }

        decl->name = xmalloc(strlen(tok->value.p)+1);
        strcpy(decl->name, tok->value.p);

        decl->decl_type = FUNCTION;
        decl->is_imported = true;

        tok = tok->next;

        if (!token_check(tok, LPAR))
        {
            fprintf(stderr,
                "Error in line : %lu in file '%s'\n\tInvalid function prototype\n",
                tok->lineno ? tok != NULL : (long unsigned int)0,
                str);
            free_declaration(decl);
            lexer_free(lexer);
            cc_exit();
        }
        tok = tok->next;

        if (!token_check(tok, RPAR))
            decl->args = get_args_decl(&tok);

        scope_enter();
        add_symbol_from_args(symtab_g, decl->args);
        scope_exit();

        if (!token_check(tok, RPAR))
        {
            fprintf(stderr,
                "Error in line : %lu in file '%s'\n\tInvalid function prototype\n",
                tok->lineno ? tok != NULL : (long unsigned int)0,
                str);
            free_declaration(decl);
            lexer_free(lexer);
            cc_exit();
        }

        tok = tok->next;

        if (!token_expect(tok, COLON))
        {
            fprintf(stderr,
                "Error in line : %lu in file '%s'\n\tInvalid function prototype\n",
                tok->lineno ? tok != NULL : (long unsigned int)0,
                str);
            free_declaration(decl);
            lexer_free(lexer);
            cc_exit();
        }

        tok = tok->next;

        if (!token_expect(tok, KEYWORD))
        {
            fprintf(stderr,
                "Error in line : %lu in file '%s'\n\tInvalid function prototype\n",
                tok->lineno ? tok != NULL : (long unsigned int)0,
                str);
            free_declaration(decl);
            lexer_free(lexer);
            cc_exit();
        }

        if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_INT]) == 0)
            decl->type.t = INTEGER;

        else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_BYTE]) == 0)
            decl->type.t = _BYTE;

        else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_VOID]) == 0)
            decl->type.t = _VOID;

        else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_CHAR]) == 0)
            decl->type.t = _CHAR;
        
        else if (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_STR]) == 0)
        {
            fprintf(stderr,
                "Error in line : %lu in file '%s'\n\tFunction can't return string\n",
                tok->lineno ? tok != NULL : (long unsigned int)0,
                str);
            free_declaration(decl);
            lexer_free(lexer);
            cc_exit();
        }

        add_symbol(symtab_g, decl);

        tok = tok->next;

        if (!token_check(tok, EOS))
        {
            fprintf(stderr,
                "Error in line : %lu in file '%s'\n\tInvalid function prototype\n",
                tok->lineno ? tok != NULL : (long unsigned int)0,
                str);
            free_declaration(decl);
            lexer_free(lexer);
            cc_exit();
        }

        tok = tok->next;

        if (tok == NULL)
            break;

    }

    lexer_free(lexer);

}