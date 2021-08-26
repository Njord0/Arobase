#ifndef _STRUCT_H
#define _STRUCT_H

#include <statements.h>

void struct_add(Statement_t *stmt);
void struct_free();
bool is_defined_struct(const char *name);
Statement_t* get_struct_by_name(const char *name);
Args_t *struct_get_member(Statement_t *str, const char *name);
unsigned int struct_member_pos(Statement_t *str, const char *name);




extern Statement_t *struct_l;

#endif