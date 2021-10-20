#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <error_handler.h>
#include <utils/vectors.h>
#include <exceptions.h>

Vector*
vec_create(enum VecType type)
{
    Vector *vec = xmalloc(sizeof(Vector));
    vec->_type = type;
    vec->_nb = 0;
    vec->node = NULL;

    return vec;
}

void
vec_free(Vector *vec)
{
    if (!vec)
        return;

    if (vec->_type == EXCEPTIONS)
    {
        Node *node = vec->node;
        while (vec->_nb-- && node)
        {
            Exception_t *ex = (Exception_t*)node->value.p;
            free_exception(ex);
            node = node->next;
        }
    }
    
}

Exception_t *vec_find_exception(Vector *vec, const char *name)
{
    if (!vec)
        return NULL;

    int nb = vec->_nb;

    Node *node = vec->node;
    while (nb-- && node)
    {
        Exception_t *exception = (Exception_t*)node->value.p;
    
        if (strcmp(exception->name, name) == 0)
            return exception;
    }

    return NULL;
}

void vec_add_exception(Exception_t *ex, Vector *vec)
{
    if (!vec || !ex)
        return;

    Node *node = xmalloc(sizeof(node));

    node->next = vec->node;
    node->value.p = ex;
    vec->_nb += 1;

    vec->node = node;
}