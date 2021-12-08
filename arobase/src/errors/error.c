#include <stdio.h>
#include <stdlib.h>

#include <utils/vectors.h>
#include <exceptions.h>
#include <errors/error.h>
#include <symbol_table.h>
#include <struct.h>
#include <lexer.h>
#include <ast.h>

Statement_t *current_function = NULL;

void
invalid_syntax_error(Token_t *token)
{
    long unsigned int line = 0;
    if (token)
        line = token->lineno;

    fprintf(stderr, 
        "Error on line : %lu\n\tInvalid syntax\n", 
        line);
    cc_exit();
}

void
cc_exit()
{
    if (current_function)
        scope_exit();

    symtab_free(symtab_g);
    free_ast(ast_g);
    lexer_free(lexer_g);
    struct_free();
    vec_free(exception_vector);

    if (current_function)
        free_statement(current_function);

    exit(1);
}

void
show_error_source(Token_t *token)
{
    char *ptr = dump_line(token);
    if (!ptr)
        return;

    unsigned int line = token ? token->lineno : 0;

    fprintf(stderr,
        "Error : File '%s', line %u\n%s\n",
        lexer_g->filename,
        line,
        ptr);

    free(ptr);
    
}

char*
dump_line(Token_t *token)
{
    if (!token)
        return NULL;

    char c;
    unsigned int l = 1;

    fseek(lexer_g->file, 0, SEEK_SET);

    while ((c=(char)getc(lexer_g->file)) && l < token->lineno)
    {
        if (c == '\n')
            l++;
    }

    ungetc(c, lexer_g->file);
    l = 1;
    char *ptr = xmalloc(sizeof(char));

    while ((c=(char)getc(lexer_g->file)) && c != '\n' && c != EOF)
    {
        ptr = xrealloc(ptr, l);

        ptr[l-1] = c;
        l++;
    }

    ptr[l] = '\x00';

    return ptr;

}

void*
xmalloc(size_t size)
{
    void *ptr = malloc(size);

    if (!ptr)
    {
        fprintf(stderr,
            "ERROR: unable to allocate memory (%lu bytes) !\n",
            size);

        cc_exit();
    }
    return ptr;
}

void*
xrealloc(void *ptr, size_t size)
{
    void *new = realloc(ptr, size);
    if (!new)
    {
        free(ptr);
        fprintf(stderr, 
            "ERROR: unable to allocate memory (%lu bytes) ! \n",
            size);

        cc_exit();
    }

    return new;
}