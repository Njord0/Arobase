#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ast.h>
#include <lexer.h>
#include <tokens.h>
#include <codegen/start.h>
#include <struct.h>
#include <statements.h>
#include <symbol_table.h>
#include <exceptions.h>

Lexer_t *lexer_g = NULL;
AST_t *ast_g = NULL;
Symtable_t *symtab_g = NULL;

bool NO_START = false;

void print_usage(const char *arg) 
{
    printf("Usage: ./%s -s source_file [options]\n"
        "\toptions:\n"
        "\t-o\t\tOutput file name, default is 'out.s'\n"
        "\t--no-start\tTell the compiler to not add a '_start' function\n",
        arg);
}

void parse_args(int argc, char **argv, char **out, char **src)
{
    *src = NULL;
    *out = NULL;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-o") == 0)
        {
            if (i+1 < argc) 
                *out = argv[i+1];
        }

        else if (strcmp(argv[i], "-s") == 0)
        {
            if (i+1 < argc) 
                *src = argv[i+1];
        }

        else if (strcmp(argv[i], "--help") == 0)
        {
            print_usage(argv[0]);
            exit(1);
        }

        else if (strcmp(argv[i], "--no-start") == 0)
            NO_START = true;
    }
}

int main(int argc, char **argv)
{
    
    char *src;
    char *out;

    parse_args(argc, argv, &out, &src);

    if (!src)
    {
        fprintf(stderr,
            "Error: \n\tNo source file specified!\n");
        return 1;
    }

    if (!out)
    {
        fprintf(stderr,
            "Warning: \n\tNo output file specified, default is 'out.s'.\n");
        out = "out.s";
    }

    lexer_g = lexer_create(src);
    if (!lexer_g)
    {
        fprintf(stderr, 
            "Error: \n\tNo file named '%s'\n",
            src);
        return 1;
    }

    lexer_tokenize(lexer_g);

    ast_g = create_ast();
    if (!ast_g)
    {
        lexer_free(lexer_g);
        return 1;
    }

    symtab_g = symtab_create();
    if (!symtab_g)
    {
        lexer_free(lexer_g);
        free_ast(ast_g);
        return 1;
    }

    exception_vector = vec_create(EXCEPTIONS);
    add_exception("Exception");

    ast_parse(ast_g, lexer_g);

    begin_codegen(ast_g, out);

    symtab_free(symtab_g);
    free_ast(ast_g);
    lexer_free(lexer_g);
    struct_free();
    vec_free(exception_vector);

    return 0;
}