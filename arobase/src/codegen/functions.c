#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <codegen/start.h>
#include <codegen/functions.h>
#include <codegen/vars.h>


#include <statements.h>
#include <args.h>
#include <struct.h>

void
emit_prologue(Statement_t *stmt)
{
    unsigned int size = get_stack_size(stmt);
    emit("push rbp\n");
    emit("mov rbp, rsp\n");
    emit("sub rsp, %u\n", size);
}

void
emit_epilogue()
{
    emit("leave\nret\n");
}

unsigned int
get_stack_size(Statement_t *stmt)
{
    unsigned int size = 0;

    Args_t *args = NULL;
    if (stmt->decl)
        args = stmt->decl->args;

    while (args)
    {
        size += 8;
        args = args->next;
    }

    if (stmt->decl)
        stmt = stmt->decl->code;


    while (stmt)
    {
        if (stmt->stmt_type == STMT_DECLARATION)
        {
            Symbol_t *sym = stmt->decl->sym;

            if (sym->_type.is_array)
            {

                if (sym->_type.t == _BYTE || sym->_type.t == _CHAR)
                    size += 16 + ((((Array_s*)(sym->_type.ptr))->size + 7) & (-8));
                else
                    size += 8 * ((Array_s*)(sym->_type.ptr))->size + 8;
            }
            else if (sym->_type.is_structure)
            {
                Statement_t *str = get_struct_by_name(sym->_type.ptr);

                Args_t *args = str->args;
                while (args)
                {
                    size += 8;
                    args = args->next;
                }
            }

            else
                size += 8;
        }
    
        else if ((stmt->stmt_type == STMT_FOR) || (stmt->stmt_type == STMT_WHILE))
        {
            if (stmt->if_block)
                size += get_stack_size(stmt->if_block);

            if (stmt->for_loop && stmt->for_loop->stmt_type == STMT_DECLARATION)
                size += 8;
        }

        stmt = stmt->next;
    }
/*     if (size % 16 == 8)
        size += 8; */
    
    return size;
}

void
emit_function(Statement_t **statement)
{
    Statement_t *stmt = *statement;

    emit(".globl %s\n", stmt->decl->sym->rname);
    emit(".type %s, @function\n", stmt->decl->sym->rname);
    emit("%s:\n", stmt->decl->sym->rname);

    emit_prologue(stmt);

    emit_move_args_to_stack(stmt->decl->args);
    
    stmt = stmt->decl->code;
    emit_statements(&stmt);

    emit_epilogue();
}

void
emit_func_call(Expression_t *expr)
{
    Args_t *args = expr->args;
    unsigned int pos = 0;
    unsigned int xmm_pos = 0;
    static unsigned int nested_c = 0; // nested function call count

    nested_c++;

    if (in_function_call)
        emit("push rdi\npush rsi\npush rcx\n");

    in_function_call = true;

    char tmp[7];
    char tmp_xmm[8];
    memcpy(tmp, scratch_in_use, sizeof(scratch_in_use));
    memcpy(tmp_xmm, xmm_in_use, sizeof(xmm_in_use));

    for (unsigned int i = 0; i < 7; i++)
    {
        if (scratch_in_use[i] == 1)
        {
            emit("push %s\n",
                scratch_regs[i]);

            scratch_in_use[i] = 0;
        }
    }

    for (unsigned int i = 0; i < 8; i++)
    {
        if (xmm_in_use[i] == 1)
        {
            emit("sub rsp, 8\n");
            emit("movsd [rsp], %s\n",
                xmm_regs[i]);

            xmm_in_use[i] = 0;
        }
    }

    while (args)
    {   
        emit_expression(args->expr, args->expr->type.t);

        if ((args->type.is_array || args->type.is_structure) && (args->expr->sym_value->type == ARG))
        {
            emit("movq %s, [%s]\n",
                args_regs[pos],
                reg_name(args->expr->reg)); 
        }
        
        else if (args->type.t == _FLOAT)
        {
            emit("movq %s, %s\n",
                xmm_args_regs[xmm_pos],
                xmm_reg_name(args->expr->reg));
        }

        else
        {
            emit("movq %s, %s\n",
                args_regs[pos],
                reg_name(args->expr->reg));
        }

        reg_free(args->expr);

        args = args->next;
        pos += 1;
    }

    emit("call %s\n", expr->sym_value->rname);

    for (int i = 7; i >= 0; i--)
    {
        if (tmp_xmm[i] == 1)
        {
            emit("movsd %s, [rsp]\n", 
                xmm_regs[i]);
            emit("add rsp, 8\n");
        }
    }

    for (int i = 6; i >= 0; i--)
    {
        if (tmp[i] == 1)
        {
            emit("pop %s\n", 
                scratch_regs[i]);
        }
    }

    memcpy(scratch_in_use, tmp, sizeof(scratch_in_use)); // restore in_use registers
    memcpy(xmm_in_use, tmp_xmm, sizeof(xmm_in_use));

    if ((in_function_call) && (nested_c != 1))
        emit("pop rcx\npop rsi\npop rdi\n");

    in_function_call = false;
    nested_c = 0;
}

void
emit_return(Statement_t *stmt)
{
    static int c = 0;

    c++;

    if (stmt->expr != NULL)
    {
        emit_expression(stmt->expr, stmt->expr->type.t);

        if (stmt->expr->type.t == _FLOAT)
            emit("movsd xmm0, %s\n",
                xmm_reg_name(stmt->expr->reg));
        else
            emit("movq rax, %s\n",
                reg_name(stmt->expr->reg));

        if (c == 1)
        {
            if (stmt->expr->type.t == _FLOAT)
                xmm_reg_free(stmt->expr);
            else
                reg_free(stmt->expr);
        }
    }


    emit("leave\nret\n");

    c = 0;
}

void
emit_move_args_to_stack(Args_t *args)
{
    unsigned int pos = 0;
    unsigned int xmm_pos = 0;

    while (args && (pos < 3))
    {
        if (args->sym->_type.t == _FLOAT)
            emit("movq [%s], %s\n",
                symbol_s(args->sym),
                xmm_args_regs[xmm_pos++]);
        else
            emit("movq [%s], %s\n", 
                symbol_s(args->sym), 
                args_regs[pos++]);

        args = args->next;
    }
}
