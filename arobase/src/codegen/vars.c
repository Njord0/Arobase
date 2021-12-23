#include <stdlib.h>
#include <stdio.h>

#include <codegen/conds.h>
#include <codegen/start.h>
#include <codegen/vars.h>
#include <codegen/functions.h>
#include <codegen/regs.h>

#include <struct.h>
#include <errors/error.h>

void
emit_expression(Expression_t *expr, enum Type t)
{   
    if (!expr)
        return;

    switch (expr->expr_type)
    {
        case EXPR_FLOAT:
        case EXPR_CHAR:
        case EXPR_INTEGER:
        case EXPR_SYMBOL:
        case EXPR_STRING_LITTERAL:
        case EXPR_ARRAYA:
        case EXPR_STRUCTA:
            alloc_reg(expr);
            load_to_reg(expr);
            break;

        case EXPR_PLUS:
            emit_expression(expr->right, t);
            emit_expression(expr->left, t);
            
            if (t == INTEGER)
                emit("addq %s, %s\n", 
                    reg_name(expr->right->reg),
                    reg_name(expr->left->reg));

            else if (t == _FLOAT)
                emit("addsd %s, %s\n",
                    xmm_reg_name(expr->right->reg),
                    xmm_reg_name(expr->left->reg));

            else if (t == _BYTE)
                emit("add %s, %s\n",
                    reg_name_l(expr->right->reg),
                    reg_name_l(expr->left->reg));

            expr->reg = expr->right->reg;
            free_reg(expr->left);

            break;

        case EXPR_MINUS:
            emit_expression(expr->right, t);
            emit_expression(expr->left, t);
            if (expr->type.t == INTEGER)
                emit("subq %s, %s\n",
                    reg_name(expr->right->reg),
                    reg_name(expr->left->reg));

            else if (expr->type.t == _FLOAT)
                emit("subsd %s, %s\n", 
                    xmm_reg_name(expr->right->reg),
                    xmm_reg_name(expr->left->reg));

            else if (expr->type.t == _BYTE)
                emit("sub %s, %s\n",
                    reg_name_l(expr->right->reg),
                    reg_name_l(expr->left->reg));


            expr->reg = expr->right->reg;
            free_reg(expr->left);

            break;

        case EXPR_MUL:
            emit_expression(expr->right, t);
            emit_expression(expr->left, t);

            if (expr->type.t == INTEGER)
                emit("imul %s, %s\n",
                    reg_name(expr->right->reg),
                    reg_name(expr->left->reg));
            
            else if (expr->type.t == _FLOAT)
                emit("mulsd %s, %s\n",
                    xmm_reg_name(expr->right->reg),
                    xmm_reg_name(expr->left->reg));

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
            free_reg(expr->left);
            break;

        case EXPR_DIV:
            emit_expression(expr->right, t);
            emit_expression(expr->left, t);

            if (expr->type.t == INTEGER)
            {
                emit("xor rdx, rdx\n");
                emit("mov rax, %s\n",
                    reg_name(expr->right->reg));
                emit("idiv %s\n",
                    reg_name(expr->left->reg));
                emit("mov %s, rax\n",
                    reg_name(expr->right->reg));
            }
            else if (expr->type.t == _FLOAT)
                emit("divsd %s, %s\n",
                    xmm_reg_name(expr->right->reg),
                    xmm_reg_name(expr->left->reg));

            expr->reg = expr->right->reg;
            free_reg(expr->left);
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
            free_reg(expr->left);
            break;

        case EXPR_FUNCCALL:
            emit_func_call(expr);

            alloc_reg(expr);
            load_to_reg(expr);
            break;

        case EXPR_UNARY_MINUS:
            emit_expression(expr->left, t);

            if (expr->left->type.t == INTEGER)
                emit("neg %s\n",
                    reg_name(expr->left->reg));

            else if (expr->left->type.t == _FLOAT)
                emit(
                    "push rcx\n"
                    "mov rcx, 0x8000000000000000\n"
                    "vmovq xmm15, rcx\n"
                    "pxor %s, xmm15\n"
                    "pop rcx\n",
                    xmm_reg_name(expr->left->reg)); // TO-DO
            else
                emit("neg %s\n",
                    reg_name_l(expr->left->reg));

            expr->reg = expr->left->reg;
            break;
        
        case EXPR_COND: // boolean
            if (!expr->right)
            {
                alloc_reg(expr);
                load_to_reg(expr);
                emit("and %s, 1\n",
                    reg_name(expr->left->reg));
            }
            else
            {
                int lbl_in = new_label();
                int lbl_out = new_label();

                emit_compare(expr);
                
                switch (expr->cond_type)
                {
                    case EXPR_CMP:
                        emit("je .LC%.5d\n", lbl_in);
                        break;
                    case EXPR_DIFF:
                        emit("jne .LC%.5d\n", lbl_in);
                        break;
                    case EXPR_LOWER:
                        emit("jb .LC%.5d\n", lbl_in); // unsigned
                        break;
                    case EXPR_GREATER:
                        emit("ja .LC%.5d\n", lbl_in); // unsigned
                        break;
                    case EXPR_LOWER_EQ:
                        emit("jbe .LC%.5d\n", lbl_in);
                        break;
                    case EXPR_GREATER_EQ:
                        emit("jae .LC%.5d\n", lbl_in);
                        break;
                    default:
                        break; // never here
                }

                alloc_reg(expr);
                emit("mov %s, 0\n", reg_name(expr->reg));
                emit("jmp .LC%.5d\n", lbl_out);
                emit(".LC%.5d:\n", lbl_in);
                emit("mov %s, 1\n", reg_name(expr->reg));
                emit(".LC%.5d:\n", lbl_out);

            }
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

        if ((statement->decl->type.t == INTEGER) && (statement->decl->expr->expr_type == EXPR_INTEGER))
            emit("movq [%s], %ld\n",
                symbol_s(statement->decl->sym),
                statement->decl->expr->int_value);

        else if ((statement->decl->type.t == _BYTE) && (statement->decl->expr->expr_type == EXPR_INTEGER))
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
            alloc_reg(statement->decl->expr);
            emit("lea %s, [rip+s%d]\n",
                reg_name(statement->decl->expr->reg),
                lbl);

            emit("mov [%s], %s\n",
                symbol_s(statement->decl->sym),
                reg_name(statement->decl->expr->reg));

            free_reg(statement->decl->expr);

        }

        else if (statement->decl->expr->expr_type == EXPR_SYMBOL)
        {
            if (statement->decl->sym->_type.t == INTEGER || statement->decl->sym->_type.t == _BOOL)
            {
                alloc_reg(statement->decl->expr);
                emit("movq %s, [%s]\n", 
                    reg_name(statement->decl->expr->reg),
                    symbol_s(statement->decl->expr->sym_value));

                emit("movq [%s], %s\n",
                    symbol_s(statement->decl->sym),
                    reg_name(statement->decl->expr->reg));

                free_reg(statement->decl->expr);
            }
            else if ((statement->decl->sym->_type.t == _BYTE) || (statement->decl->sym->_type.t == _CHAR))
            {
                alloc_reg(statement->decl->expr);
                emit("mov %s, [%s]\n",
                    reg_name_l(statement->decl->expr->reg),
                    symbol_s(statement->decl->expr->sym_value));

                emit("mov byte ptr [%s], %s\n",
                    symbol_s(statement->decl->sym),
                    reg_name_l(statement->decl->expr->reg));

                free_reg(statement->decl->expr);
            }

            else if (statement->decl->sym->_type.t == STRING)
            {
                alloc_reg(statement->decl->expr);
                emit("movq %s, [%s]\n", 
                    reg_name(statement->decl->expr->reg),
                    symbol_s(statement->decl->expr->sym_value));

                emit("movq [%s], %s\n",
                    symbol_s(statement->decl->sym),
                    reg_name(statement->decl->expr->reg));

                free_reg(statement->decl->expr);
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
    
    free_reg(statement->decl->expr);
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



        if (statement->expr->sym->_type.t == _BYTE || statement->expr->sym->_type.t == _CHAR)
        {
            emit("movq rdx, %s\n",
                reg_name(statement->expr->reg));
            emit("call array_set_element_c\n");
        }
        else if (statement->expr->sym->_type.t == _FLOAT)
        {
            emit("movsd xmm0, %s\n",
                xmm_reg_name(statement->expr->reg));
            emit("call array_set_element_f\n");  
        }
        else
        {
            emit("movq rdx, %s\n",
                reg_name(statement->expr->reg));
            emit("call array_set_element\n");
        }

        free_reg(statement->expr);
        free_reg(statement->access);
    }
    else if (statement->expr->sym->_type.is_structure)
    {
        emit_expression(statement->expr, statement->expr->type.t);
        store_to_stack(statement->expr, statement->expr->sym);
        free_reg(statement->expr);       
    }

    else
    {
        emit_expression(statement->expr, statement->expr->sym->_type.t);
        store_to_stack(statement->expr, statement->expr->sym);
        free_reg(statement->expr);
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

        if (args->type.t == INTEGER || args->type.t == _BOOL)
            emit("movq [%s+8*%u], %s\n",
                symbol_s(sym),
                count+1,
                reg_name(args->expr->reg));

        else if (args->type.t == _FLOAT)
            emit("movsd [%s+8*%u], %s\n",
                symbol_s(sym),
                count+1,
                xmm_reg_name(args->expr->reg));

        else if (args->type.t == _CHAR || args->type.t == _BYTE)
            emit("mov byte ptr [%s+8+%u], %s\n",
                symbol_s(sym),
                count,
                reg_name_l(args->expr->reg));

        count++;

        free_reg(args->expr);
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

        switch(args->expr->type.t)
        {
            case INTEGER:
            case _BOOL:
                emit("movq [rcx-%u*8], %s\n",
                    pos,
                    reg_name(args->expr->reg));
                break;
            
            case _FLOAT:
                emit("movq [rcx-%u*8], %s\n",
                    pos,
                    xmm_reg_name(args->expr->reg));
                break;
            
            case _BYTE:
            case _CHAR:
                emit("mov [rcx-%u*8], %s\n",
                    pos,
                    reg_name_l(args->expr->reg));
                break;
            
            case STRING:
                emit("mov [rcx-%u*8], %s\n",
                    pos,
                    reg_name(args->expr->reg));
                break;
            default:
                break; // never here
        }
            
    
        free_reg(args->expr);
        args = args->next;
        args_decl = args_decl->next;

    }

}
