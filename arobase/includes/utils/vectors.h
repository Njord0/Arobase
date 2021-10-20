#ifndef _AROBASE_VECTORS_H
#define _AROBASE_VECTORS_H

enum VecType {
    EXCEPTIONS, // Vector stores ptr tow  
};

typedef struct node_s{
    union {
        int i;
        void *p;
    } value;
    
    struct node_s *next; 
} Node;

typedef struct {
    int _nb; // Number of elements
    Node *node;
    enum VecType _type;
} Vector;

/* The type of elements stored in the vector */

#include <exceptions.h>

Vector *vec_create(enum VecType type);

void vec_free(Vector *vec);

Exception_t *vec_find_exception(Vector *vec, const char *name);
void vec_add_exception(Exception_t *ex, Vector *vec);


#endif
