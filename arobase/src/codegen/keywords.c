#include <stdlib.h>
#include <stdio.h>

#include <codegen/start.h>
#include <codegen/keywords.h>
#include <codegen/vars.h>
#include <codegen/regs.h>
#include <codegen/conds.h>


#include <statements.h>

void
emit_print(Statement_t *stmt)
{
    Args_t *args = stmt->args;

    while (args)
    {
        emit_expression(args->expr, args->expr->type.t);

        if (args->expr->type.t == INTEGER)
        {
            emit("movq rdi, %s\n",
                reg_name(args->expr->reg));
            emit("call print_integer\n");
        }

        else if (args->expr->type.t == _BOOL)
        {
            emit("movq rdi, %s\n",
                reg_name(args->expr->reg));
            emit("call print_bool\n");
        }

        else if (args->expr->type.t == _FLOAT)
        {
            emit("movsd xmm0, %s\n", 
                xmm_reg_name(args->expr->reg));
            emit("call print_float\n");
        }

        else if (args->expr->type.t == _BYTE)
        {
            emit("xor rdi, rdi\n");
            emit("mov dil, %s\n",
                reg_name_l(args->expr->reg));
            emit("call print_integer\n");
        }

        else if (args->expr->type.t == _CHAR)
        {
            emit("xor rdi, rdi\n");
            emit("mov dil, %s\n",
                reg_name_l(args->expr->reg));
            emit("call print_char\n");
        }

        else if (args->expr->type.t == STRING)
        {
            emit("movq rdi, %s\n",
                reg_name(args->expr->reg));
            emit("call print_string\n");
        }
        
        free_reg(args->expr);

        args = args->next;
    }
}

void
emit_input(Statement_t *stmt)
{
    if (stmt->decl->sym->_type.t == INTEGER)
    {
        emit("call input_integer\n");
        emit("movq [%s], rax\n",
            symbol_s(stmt->decl->sym));
    }

    else if (stmt->decl->sym->_type.t == _FLOAT)
    {
        emit("call input_float\n");
        emit("movsd [%s], xmm0\n",
            symbol_s(stmt->decl->sym));
    }

    else if (stmt->decl->sym->_type.t == _BYTE)
    {
        emit ("call input_integer\n");
        emit("mov byte ptr [%s], al\n",
            symbol_s(stmt->decl->sym));
    }

    else if (stmt->decl->sym->_type.t == _CHAR)
    {
        emit("call input_char\n");
        emit("mov byte ptr [%s], al\n",
            symbol_s(stmt->decl->sym));
    }
}

void
emit_assert(Statement_t *stmt)
{
    Expression_t *expr = stmt->expr;

    emit_compare(expr);

    int lbl_false = new_label();
    int lbl_done = new_label();

    if (expr->type.t == _BOOL)
    {
        emit("jne .LC%.5d\n", lbl_false);
    }
    else if (expr->left->type.t == _FLOAT)
    {
        switch(expr->cond_type)
        {
            case EXPR_CMP:
                emit("jne .LC%.5d\n", lbl_false);
                break;
            case EXPR_DIFF:
                emit("je .LC%.5d\n", lbl_false);
                break;
            case EXPR_LOWER:
                emit("jae .LC%.5d\n", lbl_false);
                break;
            case EXPR_GREATER:
                emit("jbe .LC%.5d\n", lbl_false);
                break;
            case EXPR_LOWER_EQ:
                emit("ja .LC%.5d\n", lbl_false);
                break;
            case EXPR_GREATER_EQ:
                emit("jb .LC%.5d\n", lbl_false);
                break;
            default:
                break;
        }
    }
    else
    {
        switch(expr->cond_type)
        {
            case EXPR_CMP:
                emit("jne .LC%.5d\n", lbl_false);
                break;
            case EXPR_DIFF:
                emit("je .LC%.5d\n", lbl_false);
                break;
            case EXPR_LOWER:
                if (expr->left->type.t == INTEGER)
                    emit("jge .LC%.5d\n", lbl_false); // signed
                else if (expr->left->type.t == _BYTE)
                    emit("jae .LC%.5d\n", lbl_false); // unsigned
                break;
            case EXPR_GREATER:
                if (expr->left->type.t == INTEGER)
                    emit("jle .LC%.5d\n", lbl_false); // signed
                else if (expr->left->type.t == _BYTE)
                    emit("jbe .LC%.5d\n", lbl_false); // unsigned
                break;
            case EXPR_LOWER_EQ:
                if (expr->left->type.t == INTEGER)
                    emit("jg .LC%.5d\n", lbl_false); // signed
                else if (expr->left->type.t == _BYTE)
                    emit("ja .LC%.5d\n", lbl_false); // unsigned
                break;
            case EXPR_GREATER_EQ:
                if (expr->left->type.t == INTEGER)
                    emit("jl .LC%.5d\n", lbl_false); // signed
                else if (expr->left->type.t == _BYTE)
                    emit("jb .LC%.5d\n", lbl_false); // unsigned
            default:
                break; // never here
        }
    }
    
    
    emit("jmp .LC%.5d\n", lbl_done);

    emit(".LC%.5d:\n", lbl_false);
    // if assert failed
    emit("mov rdi, 0\n");
    if (stmt->import_name != NULL)
    {
        int lbl = new_label();
        emit(".data\ns%d: .asciz \"%s\"\n",
            lbl, 
            stmt->import_name);

        emit(".text\n");
        emit("lea rdi, [rip+s%d]\n",
            lbl);   
    }
 
    emit("call _internal_assert\n");
    emit(".LC%.5d:\n", lbl_done);
}