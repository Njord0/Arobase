#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <struct.h>
#include <errors/error.h>
#include <symbol_table.h>

int fcount = 0;

Symtable_t*
symtab_create()
{
    Symtable_t *st = xmalloc(sizeof(Symtable_t));

    st->scope = xmalloc(sizeof(ScopeTable_t));

    st->scope->next = NULL;
    st->scope->symbol = NULL; 

    return st;
}

void
symtab_free(Symtable_t *st)
{
    if (!st)
        return;

    if (st->scope)
    {

        Symbol_t *sym = st->scope->symbol;
        Symbol_t *next;
        while (sym)
        {
            next = sym->next;
            if (sym->decl && (sym->decl->is_imported))
                free_declaration(sym->decl);

            sym = next;   
        }
        scopetable_free(st->scope);
    }
    free(st);
}


void
scopetable_free(ScopeTable_t *st)
{
    if (!st)
        return;

    if (st->next)
        scopetable_free(st->next);

    free(st);
}

void
scope_enter()
{
    ScopeTable_t *scope = xmalloc(sizeof(ScopeTable_t));

    scope->next = symtab_g->scope;
    symtab_g->scope = scope; // latest scope

    scope->symbol = NULL;
}

void
scope_exit()
{
    ScopeTable_t *scope = symtab_g->scope;
    if (!scope->next) // can't exit global scope
        return;
    
    symtab_g->scope = scope->next;
    scope->next = NULL; // to avoid next one being free'd
    scopetable_free(scope);
}

void
symbol_pos()
{
    ScopeTable_t *scope = symtab_g->scope;
    Symbol_t *sym = scope->symbol;

    static unsigned int pos = 0;

    while (sym)
    {
        sym->pos = pos++;

        if ((sym->_type.is_array) && (sym->type != ARG))
        {

            if (sym->_type.t == _CHAR || sym->_type.t == _BYTE)
                pos += ((((Array_s*)(sym->_type.ptr))->size + 7) & (-8)) / 8 + 1;
            else
                pos += ((Array_s*)(sym->_type.ptr))->size;
        }

        if (sym->_type.is_structure && (sym->type != ARG))
        {
            Statement_t *str = get_struct_by_name(sym->_type.ptr);
            if (!str)
                continue;

            Args_t *arg = str->args;

            pos -= 1;

            while (arg)
            {
                pos += 1;
                arg = arg->next;
            }
        }

        sym = sym->next;

        if (sym == NULL && scope->next && scope->next->next) // three scopes or more, not re-initializing pos
            return;
    }

    pos = 0;
}


void
add_symbol(Symtable_t *symtab, Declaration_t *decl)
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
        while (args)
        {
            switch (args->type.t)
            {
                case INTEGER:
                    st->rname = realloc(st->rname, strlen(st->rname)+9);
                    strcat(st->rname, "Zinteger");
                    break;
                case _FLOAT:
                    st->rname  = realloc(st->rname, strlen(st->rname)+7);
                    strcat(st->rname, "Zfloat");
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
                case _BOOL:
                    st->rname = realloc(st->rname, strlen(st->rname)+6);
                    strcat(st->rname, "Zbool");
                    break;
                case STRUCTURE:
                    st->rname = realloc(st->rname, strlen(st->rname)+strlen(args->type.ptr)+5);
                    strcat(st->rname, "Zstr");
                    strcat(st->rname, args->type.ptr);
                    break;
                default:
                    fprintf(stderr,
                        "Error:\n\tInvalid type in function parameters\n");
                    cc_exit();
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

    if (!sym)
    {
        scope->symbol = st;
    }
    else 
    {
        while (sym)
        {
            prev = sym;
            sym = sym->next;
        }

        prev->next = st;
    }

}

void
add_symbol_from_args(Symtable_t *symtab, Args_t *args)
{
    Symbol_t *sym = NULL;
    Symbol_t *prev = NULL;

    ScopeTable_t *scope = symtab->scope;

    if (!args)
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

    if (!sym)
    {
        scope->symbol = st;
    }

    else
    {
        while (sym)
        {
            prev = sym;
            sym = sym->next;
        }

        prev->next = st;
    }

    if (args->next)
        add_symbol_from_args(symtab, args->next);

}

Symbol_t*
symbol_resolve(Symtable_t *symtab, const char *name)
{

    ScopeTable_t *scope = symtab->scope;
    Symbol_t *symbol = NULL;

    while (scope)
    {
        symbol = scope->symbol;
        while (symbol)
        {
            if (strcmp(symbol->name, name) == 0)
                return symbol;

            symbol = symbol->next;
        }
        scope = scope->next;
    }

    return NULL;

}

Symbol_t*
symbol_resolve_func(Symtable_t *symtab, const char *name)
{

    ScopeTable_t *scope = symtab->scope;
    Symbol_t *symbol = NULL;

    while (scope)
    {
        symbol = scope->symbol;
        while (symbol)
        {
            if (strcmp(symbol->name, name) == 0 && symbol->decl->decl_type == FUNCTION)
                return symbol;

            symbol = symbol->next;
        }
        scope = scope->next;
    }

    return NULL;

}

bool
is_declared_func(Symtable_t *symtab, const char *name, Symbol_t **symbol)
{

    Symbol_t *sym = symbol_resolve_func(symtab, name);
    *symbol = sym;

    return (sym && (sym->decl->decl_type == FUNCTION));

}

bool
is_declared_var(Symtable_t *symtab, const char *name, Symbol_t **symbol)
{
    Symbol_t *sym = symbol_resolve(symtab, name);

    *symbol = sym;

    if (sym && !sym->decl)
        return true;

    return (sym && (sym->decl->decl_type == VARIABLE));
 }

Symbol_t*
find_matching_function(const char *name, Args_t *c_args)
{
    ScopeTable_t *scope = symtab_g->scope;
    Symbol_t *symbol = NULL;

    Args_t *tmp;

    while (scope)
    {
        symbol = scope->symbol;
        while (symbol)
        {
            if ((strcmp(symbol->name, name) == 0) && (symbol->decl->decl_type == FUNCTION))
            {    
                /* Checking if function call parameters and function decl parameters are the same */
                Args_t *args = symbol->decl->args;
                tmp = c_args;
                while (args && tmp)
                {
                    if (((args->type.is_array != tmp->type.is_array) && (tmp->expr != NULL && tmp->expr->expr_type != EXPR_ARRAYA)) || (args->type.t != tmp->type.t))
                        break;
                    args = args->next;
                    tmp = tmp->next;
                }

                if (!args && !tmp)
                    return symbol; // symbol matching the prototype...

            }
            symbol = symbol->next;
        }
        scope = scope->next;
    }
    return NULL;

}

void
import_from(const char *str)
{

    char *ptr = xmalloc(strlen("/usr/local/include/arobase/")+strlen(str)+1);

    ptr[0] = '\x00';

    strcat(ptr, "/usr/local/include/arobase/");
    strcat(ptr, str);

    Lexer_t *lexer = lexer_create(ptr);
    if (!lexer)
    {

        lexer = lexer_create(str); // trying to import from current folder...
        if (!lexer)
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

    while (tok)
    { 
        if ((tok->type != KEYWORD) || (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_FN]) != 0))
        {
            show_error_source(tok);
            fprintf(stderr,
                "Invalid function prototype in header\n");
            lexer_free(lexer);
            cc_exit();
        }

        tok = tok->next;

        Declaration_t *decl = xmalloc(sizeof(Declaration_t));
        decl_init(decl);

        if (!token_check(tok, SYMBOL))
        {
            show_error_source(tok);
            fprintf(stderr,
                "Invalid function name\n");
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
            show_error_source(tok);
            fprintf(stderr,
                "Invalid function prototype\n");
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
            show_error_source(tok);
            fprintf(stderr,
                "Invalid function prototype\n");
            free_declaration(decl);
            lexer_free(lexer);
            cc_exit();
        }

        tok = tok->next;

        if (!token_check(tok, COLON))
        {
            show_error_source(tok);
            fprintf(stderr,
                "Invalid function prototype\n");
            free_declaration(decl);
            lexer_free(lexer);
            cc_exit();
        }

        tok = tok->next;

        check_function_return_value(tok, decl);
        add_symbol(symtab_g, decl);

        tok = tok->next;

        if (!token_check(tok, EOS))
        {
            show_error_source(tok);
            fprintf(stderr,
                "Invalid function prototype\n");
            free_declaration(decl);
            lexer_free(lexer);
            cc_exit();
        }

        tok = tok->next;
    }

    lexer_free(lexer);

}