#include <stdio.h>
#include <stdlib.h>

#include <codegen/start.h>
#include <codegen/conds.h>
#include <codegen/vars.h>
#include <codegen/regs.h>

#include <errors/error.h>

void
emit_if_else(Statement_t *statement)
{
    Expression_t *expr = statement->expr;

    emit_compare(expr); // compare the results of left and right expression

    free_reg(expr->left);
    free_reg(expr->right);

    int lbl_true = new_label();
    int lbl_done = new_label();

    if (expr->left->type.t == _FLOAT) // emit comparison for float
    {
        switch(expr->cond_type)
        {
            case EXPR_CMP:
                emit("je .LC%.5d\n", lbl_true);
                break;
            case EXPR_DIFF:
                emit("jne .LC%.5d\n", lbl_true);
                break;
            case EXPR_LOWER:
                emit("jbe .LC%.5d\n", lbl_true);
                break;
            case EXPR_GREATER:
                emit("jae .LC%.5d\n", lbl_true);
                break;
            case EXPR_LOWER_EQ:
                emit("jbe .LC%.5d\n", lbl_true);
                break;
            case EXPR_GREATER_EQ:
                emit("jae .LC%.5d\n", lbl_true);
                break;
            default:
                break;
        }
    }
    else
    {
        switch (expr->cond_type)
        {
            case EXPR_CMP:
                emit("je .LC%.5d\n", lbl_true);
                break;
            case EXPR_DIFF:
                emit("jne .LC%.5d\n", lbl_true);
                break;
            case EXPR_LOWER:
                if (expr->left->type.t == INTEGER) // signed
                    emit("jl .LC%.5d\n", lbl_true);
                else if (expr->left->type.t == _BYTE)
                    emit("jb .LC%.5d\n", lbl_true); // unsigned
                break;
            case EXPR_GREATER:
                if (expr->left->type.t == INTEGER)
                    emit("jg .LC%.5d\n", lbl_true); // signed
                else if (expr->left->type.t == _BYTE)
                    emit("ja .LC%.5d\n", lbl_true); // unsigned
                break;
            case EXPR_LOWER_EQ:
                if (expr->left->type.t == INTEGER)
                    emit("jle .LC%.5d\n", lbl_true);
                else if (expr->left->type.t == _BYTE)
                    emit("jbe .LC%.5d\n", lbl_true);
                break;
            case EXPR_GREATER_EQ:
                if (expr->left->type.t == INTEGER)
                    emit("jge .LC%.5d\n", lbl_true);
                else if (expr->left->type.t == _BYTE)
                    emit("jae .LC%.5d\n", lbl_true);
                break;
            default:
                break; // never here

        }
    }
    
    // else block...;
    Statement_t *stmt = statement->else_block;
    if (statement->else_block != NULL)
        emit_statements(&stmt);

    emit("jmp .LC%.5d\n", lbl_done);

    emit(".LC%.5d:\n", lbl_true);
    // if block
    stmt = statement->if_block;
    emit_statements(&stmt);
    emit(".LC%.5d:\n", lbl_done);
}

void
emit_while(Statement_t *statement)
{
    int lbl_cond = new_label();
    int lbl_out = new_label();

    // Adding a new loop into ll
    LOOP *prev = current_loop;
    current_loop = xmalloc(sizeof(LOOP));
    current_loop->prev = prev;
    current_loop->lbl = lbl_out;
    //

    emit(".LC%.5d:\n", lbl_cond);

    Expression_t *expr = statement->expr;
    emit_compare(expr);

    free_reg(expr->left);
    free_reg(expr->right);
    /* Here we generate the reverse condition
       because while loop layout is as:
        expr
        cmp
        conditional jump
        .LC000x: #inside loop block
        ...
        .LC000y: #outside loop block
        ...
    */
    if (expr->left->type.t == _FLOAT)
    {
        switch(expr->cond_type)
        {
            case EXPR_CMP:
                emit("jne .LC%.5d\n", lbl_out);
                break;
            case EXPR_DIFF:
                emit("je .LC%.5d\n", lbl_out);
                break;
            case EXPR_LOWER:
                emit("jae .LC%.5d\n", lbl_out);
                break;
            case EXPR_GREATER:
                emit("jbe .LC%.5d\n", lbl_out);
                break;
            case EXPR_LOWER_EQ:
                emit("ja .LC%.5d\n", lbl_out);
                break;
            case EXPR_GREATER_EQ:
                emit("jb .LC%.5d\n", lbl_out);
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
                emit("jne .LC%.5d\n", lbl_out);
                break;
            case EXPR_DIFF:
                emit("je .LC%.5d\n", lbl_out);
                break;
            case EXPR_LOWER:
                if (expr->left->type.t == INTEGER)
                    emit("jge .LC%.5d\n", lbl_out); // signed
                else if (expr->left->type.t == _BYTE)
                    emit("jae .LC%.5d\n", lbl_out); // unsigned
                break;
            case EXPR_GREATER:
                if (expr->left->type.t == INTEGER)
                    emit("jle .LC%.5d\n", lbl_out); // signed
                else if (expr->left->type.t == _BYTE)
                    emit("jbe .LC%.5d\n", lbl_out); // unsigned
                break;
            case EXPR_LOWER_EQ:
                if (expr->left->type.t == INTEGER)
                    emit("jg .LC%.5d\n", lbl_out); // signed
                else if (expr->left->type.t == _BYTE)
                    emit("ja .LC%.5d\n", lbl_out); // unsigned
                break;
            case EXPR_GREATER_EQ:
                if (expr->left->type.t == INTEGER)
                    emit("jl .LC%.5d\n", lbl_out); // signed
                else if (expr->left->type.t == _BYTE)
                    emit("jb .LC%.5d\n", lbl_out); // unsigned
            default:
                break; // never here
        }
    }
    
    Statement_t *stmt = statement->if_block;
    emit_statements(&stmt);

    emit("jmp .LC%.5d\n", lbl_cond);

    emit(".LC%.5d:\n", lbl_out);


    // Deleting last loop
    prev = current_loop->prev;
    free(current_loop);
    current_loop = prev;

}

void
emit_for(Statement_t *statement)
{
    int lbl_cond = new_label();
    int lbl_out = new_label();

    // Adding a new loop into ll
    LOOP *prev = current_loop;
    current_loop = xmalloc(sizeof(LOOP));
    current_loop->prev = prev;
    current_loop->lbl = lbl_out;
    //

    Statement_t *for_loop = statement->for_loop;
    emit_statements(&for_loop);

    emit(".LC%.5d:\n", lbl_cond);

    Expression_t *expr = statement->expr;
    
    emit_compare(expr);

    free_reg(expr->left);
    free_reg(expr->right);
    /* Here we generate the reverse condition
       because while loop layout is as:
        expr
        cmp
        conditional jump
        .LC000x: #inside loop block
        ...
        .LC000y: #outside loop block
        ...
    */

    if (expr->left->type.t == _FLOAT)
    {
        switch(expr->cond_type)
        {
            case EXPR_CMP:
                emit("jne .LC%.5d\n", lbl_out);
                break;
            case EXPR_DIFF:
                emit("je .LC%.5d\n", lbl_out);
                break;
            case EXPR_LOWER:
                emit("jae .LC%.5d\n", lbl_out);
                break;
            case EXPR_GREATER:
                emit("jbe .LC%.5d\n", lbl_out);
                break;
            case EXPR_LOWER_EQ:
                emit("ja .LC%.5d\n", lbl_out);
                break;
            case EXPR_GREATER_EQ:
                emit("jb .LC%.5d\n", lbl_out);
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
                emit("jne .LC%.5d\n", lbl_out);
                break;
            case EXPR_DIFF:
                emit("je .LC%.5d\n", lbl_out);
                break;
            case EXPR_LOWER:
                if (expr->left->type.t == INTEGER)
                    emit("jge .LC%.5d\n", lbl_out); // signed
                else if (expr->left->type.t == _BYTE)
                    emit("jae .LC%.5d\n", lbl_out); // unsigned
                break;
            case EXPR_GREATER:
                if (expr->left->type.t == INTEGER)
                    emit("jle .LC%.5d\n", lbl_out); // signed
                else if (expr->left->type.t == _BYTE)
                    emit("jbe .LC%.5d\n", lbl_out); // unsigned
                break;
            case EXPR_LOWER_EQ:
                if (expr->left->type.t == INTEGER)
                    emit("jg .LC%.5d\n", lbl_out); // signed
                else if (expr->left->type.t == _BYTE)
                    emit("ja .LC%.5d\n", lbl_out); // unsigned
                break;
            case EXPR_GREATER_EQ:
                if (expr->left->type.t == INTEGER)
                    emit("jl .LC%.5d\n", lbl_out); // signed
                else if (expr->left->type.t == _BYTE)
                    emit("jb .LC%.5d\n", lbl_out); // unsigned
            default:
                break; // never here
        }
    }

    Statement_t *stmt = statement->if_block;
    emit_statements(&stmt);

    Statement_t *as = statement->else_block;
    emit_statements(&as);

    emit("jmp .LC%.5d\n", lbl_cond);

    emit(".LC%.5d:\n", lbl_out);

    // Deleting last loop
    prev = current_loop->prev;
    free(current_loop);
    current_loop = prev;
}

void
emit_compare(Expression_t *expr)
{
    emit_expression(expr->left, expr->left->type.t);

    if (expr->right != NULL)
        emit_expression(expr->right, expr->right->type.t);

    if (expr->left->type.t == INTEGER)
        emit("cmp %s, %s\n",
            reg_name(expr->left->reg),
            reg_name(expr->right->reg));

    else if (expr->left->type.t == _FLOAT)
        emit("comisd %s, %s\n",
            xmm_reg_name(expr->left->reg),
            xmm_reg_name(expr->right->reg));

    else if (expr->left->type.t == _BYTE)
        emit("cmp %s, %s\n",
            reg_name_l(expr->left->reg),
            reg_name_l(expr->right->reg));

    else if (expr->left->type.t == _CHAR)
        emit("cmp %s, %s\n",
            reg_name_l(expr->left->reg),
            reg_name_l(expr->right->reg));
}

void
emit_break(Statement_t *stmt)
{
    if (!current_loop)
        return;

    emit("jmp .LC%.5d\n", current_loop->lbl);
}