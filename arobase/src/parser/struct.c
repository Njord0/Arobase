#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <error_handler.h>
#include <statements.h>
#include <struct.h>

Statement_t *struct_l = NULL; 

void struct_add(Statement_t *stmt)
{
    if (!stmt)
        return;

    if (!struct_l)
    {
        struct_l = stmt;
    }
    
    else
    {
        stmt->next = struct_l;
        struct_l = stmt;
    }
}

void struct_free()
{
    Statement_t *next;
    Statement_t *stmt = struct_l;
    while (stmt)
    {
        next = stmt->next;
        free_args(stmt->args);
        //free(stmt->import_name);
        free(stmt);
        stmt = next;
    }
}

bool is_defined_struct(const char *name)
{
    Statement_t *stmt = struct_l;
    
    while (stmt)
    {
        if (strcmp(stmt->import_name, name) == 0)    
            return true;
        
        stmt = stmt->next;
    }

    return false;
}

Statement_t *get_struct_by_name(const char *name)
{
    Statement_t *stmt = struct_l;
    
    while (stmt)
    {
        if (strcmp(stmt->import_name, name) == 0)    
            return stmt;
        
        stmt = stmt->next;
    }

    return NULL;
}

Args_t *struct_get_member(Statement_t *str, const char *name)
{
    Args_t *member = str->args;

    while (member)
    {
        if (strcmp(member->name, name) == 0)
            return member;

        member = member->next;
    }

    return NULL;
}

unsigned int struct_member_pos(Statement_t *str, const char *name)
{
    Args_t *member = struct_get_member(str, name);
    if (!member)
        return 0;

    Args_t *members = str->args;

    unsigned int pos = 0;

    while (members)
    {
        if (strcmp(members->name, name) == 0)
            return pos;

        pos += 1;

        members = members->next;
    }

    return 0;
}

Args_t *struct_get_args(Token_t **token)
{
    Token_t *tok = *token;

    Args_t *args = xmalloc(sizeof(Args_t));
    args->next = NULL;
    args->expr = NULL;
    args->sym = NULL;
    
    if (!token_expect(tok, SYMBOL))
    {
        free_args(args);
        cc_exit();
    }

    args->name = tok->value.p;

    tok = tok->next;

    args->type = get_type_decl(&tok);
    
    if ((tok != NULL) && (tok->type == COMMA))
    {

        if (args->type.is_array)
        {
            fprintf(stderr, 
                "Error on line : %lu\n\tArrays can't be structs members!\n",
                tok->lineno);
            free_args(args);
            cc_exit();
        }
        tok  = tok->next;
        args->next = get_args_decl(&tok);
    }

    *token = tok;

    return args;
}
