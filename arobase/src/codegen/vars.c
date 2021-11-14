#include <stdlib.h>
#include <stdio.h>

#include <codegen/start.h>
#include <codegen/vars.h>
#include <codegen/functions.h>

#include <struct.h>
#include <error_handler.h>

void
emit_expression(Expression_t *expr, enum Type t)
{   
    if (!expr)
        return;

    switch (expr->expr_type)
    {
        case EXPR_CHAR:
        case EXPR_NUMBER:
        case EXPR_SYMBOL:
        case EXPR_STRING_LITTERAL:
        case EXPR_ARRAYA:
        case EXPR_STRUCTA:
            expr->reg = reg_alloc(expr->reg);
            load_to_reg(expr);
            break;

        case EXPR_PLUS:
            emit_expression(expr->right, t);
            emit_expression(expr->left, t);
            
            if (t == INTEGER)
                emit("addq %s, %s\n", 
                    reg_name(expr->right->reg),
                    reg_name(expr->left->reg));

            else if (t == _BYTE)
                emit("add %s, %s\n",
                    reg_name_l(expr->right->reg),
                    reg_name_l(expr->left->reg));

            expr->reg = expr->right->reg;
            reg_free(expr->left);
            break;

        case EXPR_MINUS:
            emit_expression(expr->right, t);
            emit_expression(expr->left, t);
            if (expr->type.t == INTEGER)
                emit("subq %s, %s\n",
                    reg_name(expr->right->reg),
                    reg_name(expr->left->reg));

            else if (expr->type.t == _BYTE)
                emit("sub %s, %s\n",
                    reg_name_l(expr->right->reg),
                    reg_name_l(expr->left->reg));


            expr->reg = expr->right->reg;
            reg_free(expr->left);
            break;

        case EXPR_MUL:
            emit_expression(expr->right, t);
            emit_expression(expr->left, t);

            if (expr->type.t == INTEGER)
                emit("imul %s, %s\n",
                    reg_name(expr->right->reg),
                    reg_name(expr->left->reg));

            else if (expr->type.t == _BYTE)
            {
                emit("mov al, %s\n",
                    reg_name_l(expr->right->reg));
                emit("imul %s\n",
                    reg_name_l(expr->left->reg));
                emit("mov %s, al\n",
                    reg_name_l(expr->right->reg));
            }

            expr->reg = expr->right->reg;
            reg_free(expr->left);
            break;

        case EXPR_DIV:
            emit_expression(expr->right, t);
            emit_expression(expr->left, t);

            emit("xor rdx, rdx\n");
            emit("mov rax, %s\n",
                reg_name(expr->right->reg));
            emit("idiv %s\n",
                reg_name(expr->left->reg));
            emit("mov %s, rax\n",
                reg_name(expr->right->reg));

            expr->reg = expr->right->reg;
            reg_free(expr->left);
            break;

        case EXPR_MOD:
            emit_expression(expr->right, t);
            emit_expression(expr->left, t);

            emit("xor rdx, rdx\n");
            emit("mov rax, %s\n",
                reg_name(expr->right->reg));
            emit("idiv %s\n",
                reg_name(expr->left->reg));
            emit("mov %s, rdx\n",
                reg_name(expr->right->reg));

            expr->reg = expr->right->reg;
            reg_free(expr->left);
            break;

        case EXPR_FUNCCALL:
            emit_func_call(expr);
            expr->reg = reg_alloc(expr->reg);
            load_to_reg(expr);
            break;

        case EXPR_UNARY_MINUS:
            emit_expression(expr->left, t);

            if (expr->left->type.t == INTEGER)
                emit("neg %s\n",
                    reg_name(expr->left->reg));
            else
                emit("neg %s\n",
                    reg_name_l(expr->left->reg));

            expr->reg = expr->left->reg;
            break;

        default:
            printf("Un-handled error !!\n");
            cc_exit();
            return;
    }
}

void
emit_var_declaration(Statement_t *statement)
{
    if (statement->decl->type.is_array)
    {
        emit_array_initialization(statement->decl->args, statement->decl->sym);
        return;
    }

    else if (statement->decl->type.is_structure)
    {
        emit_structure_initialization(statement->decl->args, statement->decl->sym);
        return;
    }

    else if (statement->decl->expr == NULL)
    {
        return;
    }

    else if ((statement->decl->expr->left == NULL) && (statement->decl->expr->right == NULL))
    {   

        if ((statement->decl->type.t == INTEGER) && (statement->decl->expr->expr_type == EXPR_NUMBER))
            emit("movq [%s], %ld\n",
                symbol_s(statement->decl->sym),
                statement->decl->expr->int_value);

        else if ((statement->decl->type.t == _BYTE) && (statement->decl->expr->expr_type == EXPR_NUMBER))
        {
            emit("mov byte ptr [%s], %ld\n",
                symbol_s(statement->decl->sym),
                statement->decl->expr->int_value);
        }

        else if ((statement->decl->type.t == _CHAR) && (statement->decl->expr->expr_type == EXPR_CHAR))
        {
            emit("mov byte ptr [%s], '%s'\n",
                symbol_s(statement->decl->sym),
                statement->decl->expr->string_value);
        }


        else if ((statement->decl->type.t == STRING) && (statement->decl->expr->expr_type == EXPR_STRING_LITTERAL))
        {
            int lbl = new_label();
            emit(".data\ns%d: .asciz \"%s\"\n",
                lbl, statement->decl->expr->string_value);

            emit(".text\n");
            statement->decl->expr->reg = reg_alloc(statement->decl->expr->reg);
            emit("lea %s, [rip+s%d]\n",
                reg_name(statement->decl->expr->reg),
                lbl);

            emit("mov [%s], %s\n",
                symbol_s(statement->decl->sym),
                reg_name(statement->decl->expr->reg));

            reg_free(statement->decl->expr);

        }

        else if (statement->decl->expr->expr_type == EXPR_SYMBOL)
        {
            if (statement->decl->sym->_type.t == INTEGER)
            {
                statement->decl->expr->reg = reg_alloc(statement->decl->expr->reg);
                emit("movq %s, [%s]\n", 
                    reg_name(statement->decl->expr->reg),
                    symbol_s(statement->decl->expr->sym_value));

                emit("movq [%s], %s\n",
                    symbol_s(statement->decl->sym),
                    reg_name(statement->decl->expr->reg));

                reg_free(statement->decl->expr);
            }
            else if ((statement->decl->sym->_type.t == _BYTE) || (statement->decl->sym->_type.t == _CHAR))
            {
                statement->decl->expr->reg = reg_alloc(statement->decl->expr->reg);
                emit("mov %s, [%s]\n",
                    reg_name_l(statement->decl->expr->reg),
                    symbol_s(statement->decl->expr->sym_value));

                emit("mov byte ptr [%s], %s\n",
                    symbol_s(statement->decl->sym),
                    reg_name_l(statement->decl->expr->reg));

                reg_free(statement->decl->expr);
            }

            else if (statement->decl->sym->_type.t == STRING)
            {
                statement->decl->expr->reg = reg_alloc(statement->decl->expr->reg);
                emit("movq %s, [%s]\n", 
                    reg_name(statement->decl->expr->reg),
                    symbol_s(statement->decl->expr->sym_value));

                emit("movq [%s], %s\n",
                    symbol_s(statement->decl->sym),
                    reg_name(statement->decl->expr->reg));

                reg_free(statement->decl->expr);
            }
        }

        else if (statement->decl->expr->expr_type == EXPR_FUNCCALL)
        {
            emit_func_call(statement->decl->expr);

            emit("movq [%s], rax\n",
                symbol_s(statement->decl->sym));
        }

        else
        {
            emit_expression(statement->decl->expr, statement->decl->sym->_type.t);
            store_to_stack(statement->decl->expr, statement->decl->sym); 
        }
    }

    else
    {
        emit_expression(statement->decl->expr, statement->decl->sym->_type.t);
        store_to_stack(statement->decl->expr, statement->decl->sym); 
    }

    reg_free(statement->decl->expr);
}

void
emit_var_assign(Statement_t *statement)
{
    if (statement->expr->sym->_type.is_array)
    {
        emit_expression(statement->expr, statement->expr->sym->_type.t);
        emit_expression(statement->access, INTEGER);

        emit("lea rdi, [%s]\n",
            symbol_s(statement->expr->sym));

        if (statement->expr->sym->type == ARG) // argument
            emit("mov rdi, [rdi]\n");

        emit("movq rsi, %s\n",
            reg_name(statement->access->reg));

        emit("movq rdx, %s\n",
            reg_name(statement->expr->reg));

        if (statement->expr->sym->_type.t == _BYTE || statement->expr->sym->_type.t == _CHAR)
            emit("call array_set_element_c\n");
        else
            emit("call array_set_element\n");

        reg_free(statement->expr);
        reg_free(statement->access);
    }
    else if (statement->expr->sym->_type.is_structure)
    {
        emit_expression(statement->expr, statement->expr->type.t);
        store_to_stack(statement->expr, statement->expr->sym);
        reg_free(statement->expr);       
    }

    else
    {
        emit_expression(statement->expr, statement->expr->sym->_type.t);
        store_to_stack(statement->expr, statement->expr->sym);
        reg_free(statement->expr);
    }

}









void
emit_array_initialization(Args_t *args, Symbol_t *sym)
{
    unsigned int count = 0;

    emit("movq [%s], %u\n",
        symbol_s(sym),
        ((Array_s*)(sym->_type.ptr))->size);

    while (args)
    {
        emit_expression(args->expr, args->type.t);

        if (args->type.t == INTEGER)
            emit("movq [%s+8*%u], %s\n",
                symbol_s(sym),
                count+1,
                reg_name(args->expr->reg));

        else if (args->type.t == _CHAR || args->type.t == _BYTE)
            emit("mov byte ptr [%s+8+%u], %s\n",
                symbol_s(sym),
                count,
                reg_name_l(args->expr->reg));

        count++;

        reg_free(args->expr);
        args = args->next;

    }
}

void
emit_structure_initialization(Args_t *args, Symbol_t *sym)
{

    Statement_t *str = get_struct_by_name(sym->_type.ptr);
    if (!str)
        return;

    Args_t *args_decl = str->args;

    while (args)
    {
        emit_expression(args->expr, args->type.t);
        unsigned int pos = struct_member_pos(str, args_decl->name);

        emit("lea rcx, [%s]\n",
            symbol_s(sym));

        if (args->expr->type.t == INTEGER)
            emit("movq [rcx-%d*8], %s\n",
                pos,
                reg_name(args->expr->reg));

        else if (args->expr->type.t == _BYTE)
            emit("mov [rcx-%d*8], %s\n",
                pos,
                reg_name_l(args->expr->reg));

        else if (args->expr->type.t == _CHAR)
            emit("mov [rcx-%d*8], %s\n",
                pos,
                reg_name_l(args->expr->reg));

        else if (args->expr->type.t == STRING)
            emit("mov [rcx-%d*8], %s\n",
            pos,
            reg_name(args->expr->reg));

        reg_free(args->expr);
        args = args->next;
        args_decl = args_decl->next;

    }

}