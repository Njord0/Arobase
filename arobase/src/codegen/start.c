#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>

#include <codegen/start.h>
#include <codegen/keywords.h>
#include <codegen/conds.h>
#include <codegen/functions.h>
#include <codegen/vars.h>
#include <codegen/exceptions.h>

#include <statements.h>
#include <struct.h>

bool in_function_call = false;
LOOP *current_loop = NULL;

FILE *file;

const char *scratch_regs[] = {
    "rbx",
    "r10",
    "r11",
    "r12",
    "r13",
    "r14",
    "r15"
};

const char *scratch_regs_l[] = {
    "bl",
    "r10b",
    "r11b",
    "r12b",
    "r13b",
    "r14b",
    "r15b"
};

const char *args_regs[] = {
    "rdi",
    "rsi",
    "rdx",
};

char scratch_in_use[7] = {0};
char symbol_stack_pos[100] = {0};

void
begin_codegen(AST_t *ast, const char *out)
{
    file = fopen(out, "w");

    if (!NO_START)
        _start_def();

    Statement_t *stmt = ast->first_stmt;

    emit_statements(&stmt);
    fclose(file);
}

void
emit_statements(Statement_t **statement)
{
    Statement_t *stmt = *statement;
    while (stmt)
    {
        if ((stmt->stmt_type == STMT_DECLARATION) && stmt->decl && (stmt->decl->decl_type == FUNCTION))        
            emit_function(&stmt);
        else if ((stmt->stmt_type == STMT_DECLARATION) && stmt->decl && (stmt->decl->decl_type == VARIABLE))
            emit_var_declaration(stmt);

        else if ((stmt->stmt_type == STMT_EXPR))
            emit_expression(stmt->expr, stmt->expr->type.t);

        else if (stmt->stmt_type == STMT_ASSIGN)
            emit_var_assign(stmt);

        else if (stmt->stmt_type == STMT_IF_ELSE)
            emit_if_else(stmt);

        else if (stmt->stmt_type == STMT_WHILE)
            emit_while(stmt);
        
        else if (stmt->stmt_type == STMT_FOR)
            emit_for(stmt);

        else if (stmt->stmt_type == STMT_RETURN)
            emit_return(stmt);

        else if (stmt->stmt_type == STMT_PRINT)
            emit_print(stmt);

        else if (stmt->stmt_type == STMT_INPUT)
            emit_input(stmt);

        else if (stmt->stmt_type == STMT_ASSERT)
            emit_assert(stmt);

        else if (stmt->stmt_type == STMT_BREAK)
            emit_break(stmt);

        else if (stmt->stmt_type == STMT_TRY_EXCEPT)
            emit_try_block(stmt);
        
        else if (stmt->stmt_type == STMT_RAISE)
            emit_raise(stmt);

        if (stmt)
            stmt = stmt->next;
    }

    *statement = stmt;
}

unsigned int
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

    return UINT_MAX;
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

void
reg_free(Expression_t *expr)
{
    if (expr->reg != UINT_MAX)
    {
        scratch_in_use[expr->reg] = 0;
        expr->reg = UINT_MAX;
    }
}

void
load_to_reg(Expression_t *expr)
{
    if ((expr != NULL) && (expr->expr_type == EXPR_FUNCCALL))
    {
        emit("movq %s, rax\n",
            reg_name(expr->reg));
    }

    else if (expr && (expr->expr_type == EXPR_NUMBER))
    {
        if (expr->type.t == INTEGER)
            emit("movq %s, %ld\n", 
                reg_name(expr->reg), 
                expr->int_value);

        else if (expr->type.t == _BYTE)
            emit("mov %s, %ld\n",
                reg_name_l(expr->reg),
                expr->int_value);            
    }

    else if (expr->expr_type == EXPR_STRUCTA)
    {
        Statement_t *str = get_struct_by_name(expr->sym_value->_type.ptr);

        unsigned int pos;
        if (expr->args)
            pos = struct_member_pos(str, expr->args->name);
        else
            pos = struct_member_pos(str, expr->string_value);


        emit("lea rcx, [%s]\n",
            symbol_s(expr->sym_value));

        if (expr->sym_value->type == ARG) // If structure is an argument, load it. 
            emit("mov rcx, [rcx]\n");
        
        if (expr->type.t == INTEGER)
            emit("movq %s, [rcx-%d*8]\n", 
                reg_name(expr->reg), 
                pos);

        else if (expr->type.t == _BYTE)
            emit("mov %s, [rcx-%d*8]\n",
                reg_name_l(expr->reg),
                pos);

        else if (expr->type.t == _CHAR)
            emit("mov %s, [rcx-%d*8]\n",
                reg_name_l(expr->reg),
                pos);

        else if (expr->type.t == STRING)
            emit("mov %s, [rcx-%d*8]\n",
                reg_name(expr->reg),
                pos);
    }

    else if (expr && (expr->expr_type == EXPR_SYMBOL))
    {

        if (expr->sym_value->_type.is_array || expr->sym_value->_type.is_structure)
        {
            emit("lea %s, [%s]\n",
                reg_name(expr->reg),
                symbol_s(expr->sym_value));
        }

        else
        {
            if (expr->sym_value->_type.t == INTEGER)
                emit("movq %s, [%s]\n", 
                    reg_name(expr->reg), 
                    symbol_s(expr->sym_value));

            else if (expr->sym_value->_type.t == _BYTE)
                emit("mov %s, [%s]\n",
                    reg_name_l(expr->reg),
                    symbol_s(expr->sym_value));

            else if (expr->sym_value->_type.t == _CHAR)
                emit("mov %s, [%s]\n",
                    reg_name_l(expr->reg),
                    symbol_s(expr->sym_value));

            else if (expr->sym_value->_type.t == STRING)
                emit("mov %s, [%s]\n",
                    reg_name(expr->reg),
                    symbol_s(expr->sym_value));

            else if (expr->sym_value->_type.t == STRUCTURE)
                emit("lea %s, [%s]\n",
                    reg_name(expr->reg),
                    symbol_s(expr->sym_value));
        }

        
    }

    else if (expr && (expr->expr_type == EXPR_ARRAYA))
    {
        emit_expression(expr->access, INTEGER);
        if (in_function_call)
            emit("push rdi\npush rsi\npush rcx\n");

        emit("lea rdi, [%s]\n",
                symbol_s(expr->sym_value));

        if (expr->sym_value->decl == NULL) // Arg
            emit("mov rdi, [rdi]\n");

        emit("movq rsi, %s\n",
            reg_name(expr->access->reg));

        if (expr->sym_value->_type.t == _BYTE || expr->sym_value->_type.t == _CHAR)
            emit("call array_get_element_c\n");
        else
            emit("call array_get_element\n");

        if (in_function_call)
            emit("pop rcx\npop rsi\npop rdi\n");

        switch (expr->sym_value->_type.t)
        {
            case INTEGER:
                emit("movq %s, rax\n",
                    reg_name(expr->reg));
                break;

            case _CHAR:
            case _BYTE:
                emit("mov %s, al\n",
                reg_name_l(expr->reg));        

            default:
                break;
        }


        reg_free(expr->access);
    }

    else if ((expr != NULL) && (expr->type.t == _CHAR))
    {
        emit("mov %s, '%s'\n",
            reg_name_l(expr->reg),
            expr->string_value);    
    }

    else if ((expr != NULL) && (expr->type.t == STRING))
    {
        int lbl = new_label();
        emit(".data\ns%d: .asciz \"%s\"\n",
            lbl, expr->string_value);

        emit(".text\n");
        expr->reg = reg_alloc(expr->reg);
        emit("lea %s, [rip+s%d]\n",
            reg_name(expr->reg),
            lbl);
    }
}

void
store_to_stack(Expression_t *expr, Symbol_t *sym)
{
    if (sym->_type.t == INTEGER)
        emit("movq [%s], %s\n",
            symbol_s(sym),
            reg_name(expr->reg));

    else if (sym->_type.t == _BYTE)
        emit("mov byte ptr [%s], %s\n", 
            symbol_s(sym), 
            reg_name_l(expr->reg));

    else if (sym->_type.t == _CHAR)
        emit("mov byte ptr [%s], %s\n",
            symbol_s(sym),
            reg_name_l(expr->reg));

    else if (sym->_type.t == STRING)
        emit("movq [%s], %s\n",
            symbol_s(sym),
            reg_name(expr->reg));

    else if (sym->_type.t == STRUCTURE)
    {
        Statement_t *str = get_struct_by_name(sym->_type.ptr);

        unsigned int pos;
        if (expr->args)
            pos = struct_member_pos(str, expr->args->name);
        else
            pos = struct_member_pos(str, expr->string_value);

        emit("lea rcx, [%s]\n",
            symbol_s(sym));

        if (sym->type == ARG) // If structure is an argument, load it. 
            emit("mov rcx, [rcx]\n");    

        if (expr->type.t == INTEGER)
            emit("movq [rcx-%d*8], %s\n", 
                pos,
                reg_name(expr->reg));

        else if (expr->type.t == _BYTE)
            emit("mov [rcx-%d*8], %s\n",
                pos,
                reg_name_l(expr->reg));

        else if (expr->type.t == _CHAR)
            emit("mov [rcx-%d*8], %s\n",
                pos,
                reg_name_l(expr->reg));

        else if (expr->type.t == STRING)
            emit("mov [rcx-%d*8], %s\n",
                pos,
                reg_name(expr->reg));
    }
}

int
new_label()
{
    static int i = 0;
    return i++;
}

char*
symbol_s(Symbol_t *sym)
{

    if ((sym->_type.is_array) && (sym->type != ARG))
    {

        if (sym->_type.t == _BYTE || sym->_type.t == _CHAR)
            snprintf(symbol_stack_pos, sizeof(symbol_stack_pos), "rbp-%u", 8 + ((((Array_s*)(sym->_type.ptr))->size + 7) & (-8)));
        else
            snprintf(symbol_stack_pos, sizeof(symbol_stack_pos), "rbp-%u", 8 * (sym->pos+1+((Array_s*)(sym->_type.ptr))->size));
    }
    else
    {
        snprintf(symbol_stack_pos, sizeof(symbol_stack_pos), "rbp-%u", 8*(sym->pos+1));
    }

    return symbol_stack_pos;
}

void
_start_def()
{
    emit(".text\n.global _start\n_start:\n");
    emit("push rbp\nmov rbp, rsp\nsub rsp, 16\n");
    emit("call main\n");
    emit("xor rax, rax\nmov al, 60\nxor rdi, rdi\nsyscall\n");
}