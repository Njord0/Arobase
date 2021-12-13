#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <errors/error.h>

#include <options/options.h>

void print_usage(const char *arg) 
{
    printf("Usage: ./%s -s source_file [options]\n"
        "Options:\n"
        "  -o\t\tOutput file name, default is 'out'\n"
        "  --no-start\tTell the compiler to not add a '_start' function\n"
        "  --assembly\tOutput assembly instead of executable\n",
        arg);
}
    
Option_t*
options_parse(char **argv, int argc)
{
    Option_t *option = xmalloc(sizeof(Option_t));
    option->output_file = NULL;
    option->source_file = NULL;
    option->has_entry = true;
    option->assembly = false;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-o") == 0)
        {
            if (i+1 < argc) 
                option->output_file = argv[i+1];
        }

        else if (strcmp(argv[i], "-s") == 0)
        {
            if (i+1 < argc) 
                option->source_file = argv[i+1];
        }

        else if (strcmp(argv[i], "--help") == 0)
        {
            print_usage(argv[0]);
            exit(0);
        }
        else if (strcmp(argv[i], "--assembly") == 0)
            option->assembly = true;

        else if (strcmp(argv[i], "--no-start") == 0)
            option->has_entry = false;

        
    }
    return option;
}

char*
make_command(char *args[], int argc)
{
    size_t size = 0;
    for (int i = 0; i < argc; i++)
        size += strlen(args[i]);

    char *ptr  = xmalloc(10000);
    
    ptr[0] = '\x00';
    strcat(ptr, "as ");

    for (int i = 0; i < argc; i++)
    {
        strcat(ptr, args[i]);
        strcat(ptr, " ");
    }
    
    ptr[size+argc+2] = '\x00';
    return ptr;
}