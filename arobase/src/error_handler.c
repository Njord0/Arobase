#include <stdio.h>
#include <stdlib.h>

#include <error_handler.h>
#include <lexer.h>
#include <ast.h>

#include <symbol_table.h>

Statement_t *current_function = NULL;

void invalid_syntax_error(Token_t *token)
{
    long unsigned int line = 0;
    if (token != NULL)
        line = token->lineno;

    fprintf(stderr, 
        "Error on line : %lu\n\tInvalid syntax\n", 
        line);
    cc_exit();
}

void undeclared_variable_error(const char *name, long unsigned int line)
{
    fprintf(stderr, 
        "Error on line : %lu \n\tUndeclared symbol \"%s\"\n",
        line,
        name);

    cc_exit();
}

void missing_type(long unsigned int line)
{
    fprintf(stderr, 
        "Error on line : %lu\n\tMissing type\n",
        line);

    cc_exit();
}

void invalid_variable_name(const char *ptr)
{
    fprintf(stderr, 
        "Error on line : 0\n\tCan't name variable '%s' because it's a reserved keyword\n",
        ptr);

    cc_exit();
}

void cc_exit()
{
    if (current_function != NULL)
        scope_exit();

    symtab_free(symtab_g);
    free_ast(ast_g);
    lexer_free(lexer_g);

    if (current_function != NULL)
        free_statement(current_function);

    exit(1);
}


void *xmalloc(size_t size)
{
    void *ptr = malloc(size);

    if (ptr == NULL)
    {
        fprintf(stderr,
            "ERROR: unable to allocate memory (%lu bytes) !\n",
            size);

        cc_exit();
    }
    return ptr;
}

void *xrealloc(void *ptr, size_t size)
{
    void *new = realloc(ptr, size);
    if (new == NULL)
    {
        free(ptr);
        fprintf(stderr, 
            "Error: unable to allocate memory (%lu bytes) ! \n",
            size);

        cc_exit();
    }

    return new;
}