#include <stdio.h>
#include <limits.h>

#include <codegen/regs.h>

#include <expressions.h>
#include <errors/error.h>

static unsigned int reg_alloc(unsigned int r);
static void reg_free(Expression_t *expr);
static unsigned int xmm_reg_alloc(unsigned int reg);
static void xmm_reg_free(Expression_t *expr);

void alloc_reg(Expression_t *expr)
{
    if (expr->type.t == _FLOAT)
        expr->reg = xmm_reg_alloc(expr->reg);
    else
        expr->reg = reg_alloc(expr->reg);
}

void
free_reg(Expression_t *expr)
{
    if (expr->type.t == _FLOAT)
        xmm_reg_free(expr);
    else
        reg_free(expr);
}

static unsigned int
reg_alloc(unsigned int reg)
{
    if (reg != UINT_MAX)
        return reg;

    for (unsigned int i = 0; i < 7; i++)
    {
        if (scratch_in_use[i] == 0)
        {
            scratch_in_use[i] = 1;
            return i;
        }
    }

    fprintf(stderr,
        "Internal error: No more free reg\n");
    cc_exit();
}

const char*
reg_name(unsigned int r)
{
    return scratch_regs[r];
}

const char*
reg_name_l(unsigned int r)
{
    return scratch_regs_l[r];
}

static void
reg_free(Expression_t *expr)
{
    if (expr->reg != UINT_MAX)
    {
        scratch_in_use[expr->reg] = 0;
        expr->reg = UINT_MAX;
    }
}

static unsigned int
xmm_reg_alloc(unsigned int reg)
{
    if (reg != UINT_MAX)
        return reg; // Already allocated

    for (unsigned int i = 0; i < 8; i++)
    {
        if (xmm_in_use[i] == 0)
        {
            xmm_in_use[i] = 1;
            return i;
        }
    }

    fprintf(stderr,
        "Internal error: No more free reg\n");
    cc_exit();
}

const char*
xmm_reg_name(unsigned int r)
{
    return xmm_regs[r];
}

static void
xmm_reg_free(Expression_t *expr)
{
    if (expr->reg != UINT_MAX)
    {
        xmm_in_use[expr->reg] = 0;
        expr->reg = UINT_MAX;
    }
}
