#include <stdlib.h>
#include <stdio.h>

#include <codegen/start.h>
#include <codegen/keywords.h>
#include <codegen/vars.h>


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
        
        reg_free(args->expr);

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

    emit_expression(expr->left, expr->type.t);

    if (expr->right != NULL)
        emit_expression(expr->right, expr->right->type.t);

    if (expr->type.t == INTEGER)
        emit("cmp %s, %s\n",
            reg_name(expr->left->reg),
            reg_name(expr->right->reg));
    
    else if (expr->type.t == _BYTE)
        emit("cmp %s, %s\n",
            reg_name_l(expr->left->reg),
            reg_name_l(expr->right->reg));

    else if (expr->type.t == _CHAR)
        emit("cmp %s, %s\n",
            reg_name_l(expr->left->reg),
            reg_name_l(expr->right->reg));


    reg_free(expr->left);
    reg_free(expr->right);

    int lbl_false = new_label();
    int lbl_done = new_label();

    if (expr->cond_type == EXPR_CMP)
        emit("jne .LC%.5d\n", lbl_false);

    else if (expr->cond_type == EXPR_DIFF)
        emit("je .LC%.5d\n", lbl_false);

    else if (expr->cond_type == EXPR_LOWER)
    {
        if (expr->left->type.t == INTEGER) // signed
            emit("jge .LC%.5d\n", lbl_false);
        else if (expr->left->type.t == _BYTE)
            emit("jae .LC%.5d\n", lbl_false); // unsigned
    }

    else if (expr->cond_type == EXPR_GREATER)
    {
        if (expr->left->type.t == INTEGER)
            emit("jle .LC%.5d\n", lbl_false); // signed

        else if (expr->left->type.t == _BYTE)
            emit("jbe .LC%.5d\n", lbl_false); // unsigned
    }

    else if (expr->cond_type == EXPR_LOWER_EQ)
    {
        if (expr->left->type.t == INTEGER)
            emit("jg .LC%.5d\n", lbl_false);

        else if (expr->left->type.t == _BYTE)
            emit("ja .LC%.5d\n", lbl_false);
    }

    else if (expr->cond_type == EXPR_GREATER_EQ)
    {
        if (expr->left->type.t == INTEGER)
            emit("jl .LC%.5d\n", lbl_false);

        else if (expr->left->type.t == _BYTE)
            emit("jb .LC%.5d\n", lbl_false);
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