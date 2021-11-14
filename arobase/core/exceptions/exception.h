#ifndef _CORE_EXCEPTION_H
#define _CORE_EXCEPTION_H

#ifndef __ASSEMBLER__

#include <setjmp.h>

struct exception {
    jmp_buf env;
    struct exception *next;
};

extern jmp_buf jmp_struct;

static struct exception *exceptions;

void add_exception(jmp_buf env);
void leave_exception();
struct __jmp_buf_tag* get_exception_env();
void exception();

#endif
#endif