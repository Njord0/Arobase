#ifndef _ARGS_H
#define _ARGS_H

#include <type.h>
#include <tokens.h>
#include <expressions.h>

/*
    A structure representing an arguments list for functions calls or declarations,
        print statements, arrays initializations ...
*/
typedef struct args {
    struct Expression *expr; // argument expression 
    struct args *next; // next argument
    Type_s type; // the type of the argument
    const char *name; // for functions declarations or structs members
    struct _symbol *sym; // The symbol representing the argument
} Args_t;

/*
    Parse an arguments lists until finding a semicolon ';'
    Arguments are expected to be of the type t

    After function returns, 'token' reference the token next to
        the semicolon
*/
Args_t *get_args(Token_t **token, enum Type t);

/*
    Parse an arguments list while a symbol is found after the last argument,
        functions parameters are limited to 3 by default.

    After functions returns, 'token' references the token next to
        the last parameter, supposedly a right parenthesis      
*/
Args_t *get_args_decl(Token_t **token);

/*
    Return the number of arguments in the list.
*/
unsigned int get_args_count(Args_t *args);

void free_args(Args_t *args);


#endif