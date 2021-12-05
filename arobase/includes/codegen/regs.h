#ifndef _REGS_H
#define _REGS_H

#include <type.h>

void alloc_reg(Expression_t *expr);
void free_reg(Expression_t *expr);


const char *reg_name(unsigned int r);
const char *reg_name_l(unsigned int r);

/* For floating point instruction */

const char* xmm_reg_name(unsigned int r);

extern const char *scratch_regs[];
extern const char *scratch_regs_l[];
extern const char *args_regs[];
extern char scratch_in_use[7];

extern const char *xmm_args_regs[];
extern const char *xmm_regs[];
extern char xmm_in_use[8];


#endif