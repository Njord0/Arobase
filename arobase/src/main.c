#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ast.h>
#include <lexer.h>
#include <tokens.h>
#include <codegen/start.h>
#include <options/options.h>
#include <struct.h>
#include <statements.h>
#include <symbol_table.h>
#include <exceptions.h>

Lexer_t *lexer_g = NULL;
AST_t *ast_g = NULL;
Symtable_t *symtab_g = NULL;
Option_t *option_g = NULL;

int main(int argc, char **argv)
{
    option_g = options_parse(argv, argc); 

    if (!option_g->source_file)
    {
        fprintf(stderr,
            "Error: \n\tNo source file specified!\n");
        return 1;
    }

    if (!option_g->output_file)
    {
        fprintf(stderr,
            "Warning: \n\tNo output file specified, default is 'out'.\n");
        option_g->output_file = "out";
    }

    lexer_g = lexer_create(option_g->source_file);
    if (!lexer_g)
    {
        fprintf(stderr, 
            "Error: \n\tNo file named '%s'\n",
            option_g->source_file);
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


    if (option_g->assembly)
    {
        begin_codegen(ast_g, option_g->output_file);    
    }
    else
    {
        char asm_name[] = "asmXXXXXX";
        char obj_name[] = "objXXXXXX";

        mkstemp(asm_name);
        mkstemp(obj_name);

        begin_codegen(ast_g, asm_name);

        char *args_as[5] = {"-o", obj_name, asm_name, "-mnaked-reg", "-msyntax=intel"};
        char *args_ld[5] = {"-o", option_g->output_file, obj_name, "-larobase", "-lc"};

        char *cmd = make_command(args_as, 5);
        if (system(cmd) != 0)
        {
            free(cmd);
            fprintf(stderr,
                "An error occured while assembling.\n");
        }
        else
        {
            free(cmd);
            cmd = make_command(args_ld, 5);
            cmd[0] = 'l'; cmd[1] = 'd';
        
            if (system(cmd) != 0)
            {
                fprintf(stderr,
                    "An error occured while linking.\n");
            }
            free(cmd);
        }

        
        remove(asm_name);
        remove(obj_name);
    }


    symtab_free(symtab_g);
    free_ast(ast_g);
    lexer_free(lexer_g);
    struct_free();
    vec_free(exception_vector);

   

    return 0;
}