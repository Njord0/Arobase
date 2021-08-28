#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <ast.h>
#include <declarations.h>
#include <expressions.h>
#include <statements.h>
#include <code_gen.h>
#include <struct.h>
#include <error_handler.h>

static FILE *file;

bool in_function_call = false;

#define emit(...) \
        fprintf(file, __VA_ARGS__)

typedef struct {        
    void *prev; 
    int lbl;    
} LOOP;

LOOP *current_loop = NULL;

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
char pos[100] = {0};

void begin_codegen(AST_t *ast, const char *out)
{
    file = fopen(out, "w");

    if (!NO_START)
        _start_def();

    Statement_t *stmt = ast->first_stmt;

    emit_statements(&stmt);
    fclose(file);
}

void emit_statements(Statement_t **statement)
{
    Statement_t *stmt = *statement;
    while (stmt != NULL)
    {
        if ((stmt->stmt_type == STMT_DECLARATION) && (stmt->decl != NULL) && (stmt->decl->decl_type == FUNCTION))        
            emit_function(&stmt);
        else if ((stmt->stmt_type == STMT_DECLARATION) && (stmt->decl != NULL) && (stmt->decl->decl_type == VARIABLE))
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

        if (stmt != NULL)
            stmt = stmt->next;
    }

    *statement = stmt;
}

void emit_expression(Expression_t *expr, enum Type t)
{   
    if (expr == NULL)
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

void emit_var_declaration(Statement_t *statement)
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

void emit_var_assign(Statement_t *statement)
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

void emit_function(Statement_t **statement)
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

void emit_if_else(Statement_t *statement)
{
    Expression_t *expr = statement->expr;

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

    int lbl_true = new_label();
    int lbl_done = new_label();

    if (expr->cond_type == EXPR_CMP)
        emit("je .LC%.5d\n", lbl_true);

    else if (expr->cond_type == EXPR_DIFF)
        emit("jne .LC%.5d\n", lbl_true);

    else if (expr->cond_type == EXPR_LOWER)
    {
        if (expr->left->type.t == INTEGER) // signed
            emit("jl .LC%.5d\n", lbl_true);
        else if (expr->left->type.t == _BYTE)
            emit("jb .LC%.5d\n", lbl_true); // unsigned
    }

    else if (expr->cond_type == EXPR_GREATER)
    {
        if (expr->left->type.t == INTEGER)
            emit("jg .LC%.5d\n", lbl_true); // signed

        else if (expr->left->type.t == _BYTE)
            emit("ja .LC%.5d\n", lbl_true); // unsigned
    }

    else if (expr->cond_type == EXPR_LOWER_EQ)
    {
        if (expr->left->type.t == INTEGER)
            emit("jle .LC%.5d\n", lbl_true);

        else if (expr->left->type.t == _BYTE)
            emit("jbe .LC%.5d\n", lbl_true);
    }

    else if (expr->cond_type == EXPR_GREATER_EQ)
    {
        if (expr->left->type.t == INTEGER)
            emit("jge .LC%.5d\n", lbl_true);

        else if (expr->left->type.t == _BYTE)
            emit("jae .LC%.5d\n", lbl_true);
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

void emit_while(Statement_t *statement)
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
    emit_expression(expr->left, expr->left->type.t);

    if (expr->right != NULL)
        emit_expression(expr->right, expr->right->type.t);

    if (expr->left->type.t == INTEGER)
        emit("cmp %s, %s\n",
            reg_name(expr->left->reg),
            reg_name(expr->right->reg));
    

    else if (expr->left->type.t == _BYTE)
        emit("cmp %s, %s\n",
            reg_name_l(expr->left->reg),
            reg_name_l(expr->right->reg));

    else if (expr->type.t == _CHAR)
        emit("cmp %s, %s\n",
            reg_name_l(expr->left->reg),
            reg_name_l(expr->right->reg));

    reg_free(expr->left);
    reg_free(expr->right);
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

    if (expr->cond_type == EXPR_CMP)
        emit("jne .LC%.5d\n", lbl_out);

    else if (expr->cond_type == EXPR_DIFF)
        emit("je .LC%.5d\n", lbl_out);

    else if (expr->cond_type == EXPR_LOWER)
    {
        if (expr->left->type.t == INTEGER)
            emit("jge .LC%.5d\n", lbl_out); // signed
        else if (expr->left->type.t == _BYTE)
            emit("jae .LC%.5d\n", lbl_out); // unsigned
    }

    else if (expr->cond_type == EXPR_GREATER)
    {
        if (expr->left->type.t == INTEGER)
            emit("jle .LC%.5d\n", lbl_out); // signed
        else if (expr->left->type.t == _BYTE)
            emit("jbe .LC%.5d\n", lbl_out); // unsigned
    }

    else if (expr->cond_type == EXPR_LOWER_EQ)
    {
        if (expr->left->type.t == INTEGER)
            emit("jg .LC%.5d\n", lbl_out); // signed
        else if (expr->left->type.t == _BYTE)
            emit("ja .LC%.5d\n", lbl_out); // unsigned
    }

    else if (expr->cond_type == EXPR_GREATER_EQ)
    {
        if (expr->left->type.t == INTEGER)
            emit("jl .LC%.5d\n", lbl_out); // signed
        else if (expr->left->type.t == _BYTE)
            emit("jb .LC%.5d\n", lbl_out); // unsigned
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

void emit_for(Statement_t *statement)
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
    emit_expression(expr->left, expr->left->type.t);

    if (expr->right != NULL)
        emit_expression(expr->right, expr->right->type.t);

    if (expr->left->type.t == INTEGER)
        emit("cmp %s, %s\n",
            reg_name(expr->left->reg),
            reg_name(expr->right->reg));
    

    else if (expr->left->type.t == _BYTE)
        emit("cmp %s, %s\n",
            reg_name_l(expr->left->reg),
            reg_name_l(expr->right->reg));

    else if (expr->type.t == _CHAR)
        emit("cmp %s, %s\n",
            reg_name_l(expr->left->reg),
            reg_name_l(expr->right->reg));

    reg_free(expr->left);
    reg_free(expr->right);
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

    if (expr->cond_type == EXPR_CMP)
        emit("jne .LC%.5d\n", lbl_out);

    else if (expr->cond_type == EXPR_DIFF)
        emit("je .LC%.5d\n", lbl_out);

    else if (expr->cond_type == EXPR_LOWER)
    {
        if (expr->left->type.t == INTEGER)
            emit("jge .LC%.5d\n", lbl_out); // signed
        else if (expr->left->type.t == _BYTE)
            emit("jae .LC%.5d\n", lbl_out); // unsigned
    }

    else if (expr->cond_type == EXPR_GREATER)
    {
        if (expr->left->type.t == INTEGER)
            emit("jle .LC%.5d\n", lbl_out); // signed
        else if (expr->left->type.t == _BYTE)
            emit("jbe .LC%.5d\n", lbl_out); // unsigned
    }

    else if (expr->cond_type == EXPR_LOWER_EQ)
    {
        if (expr->left->type.t == INTEGER)
            emit("jg .LC%.5d\n", lbl_out); // signed
        else if (expr->left->type.t == _BYTE)
            emit("ja .LC%.5d\n", lbl_out); // unsigned
    }

    else if (expr->cond_type == EXPR_GREATER_EQ)
    {
        if (expr->left->type.t == INTEGER)
            emit("jl .LC%.5d\n", lbl_out); // signed
        else if (expr->left->type.t == _BYTE)
            emit("jb .LC%.5d\n", lbl_out); // unsigned
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

void emit_return(Statement_t *stmt)
{
    static int c = 0;

    c++;

    if (stmt->expr != NULL)
    {
        emit_expression(stmt->expr, stmt->expr->type.t);
        emit("movq rax, %s\n",
            reg_name(stmt->expr->reg));

        if (c == 1)
            reg_free(stmt->expr);
    }


    emit("leave\nret\n");

    c = 0;
}

void emit_func_call(Expression_t *expr)
{
    Args_t *args = expr->args;
    unsigned int pos = 0;
    static unsigned int nested_c = 0; // nested function call count

    nested_c++;

    if (in_function_call)
        emit("push rdi\npush rsi\npush rcx\n");

    in_function_call = true;

    char tmp[7];
    memcpy(tmp, scratch_in_use, sizeof(scratch_in_use));

    for (unsigned int i = 0; i < 7; i++)
    {
        if (scratch_in_use[i] == 1)
        {
            emit("push %s\n",
                scratch_regs[i]);

            scratch_in_use[i] = 0;
        }
    }

    while (args != NULL)
    {   
        emit_expression(args->expr, args->expr->type.t);

        if ((args->type.is_array || args->type.is_structure) && (args->expr->sym_value->type == ARG))
        {
            emit("movq %s, [%s]\n",
                args_regs[pos],
                reg_name(args->expr->reg)); 
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

    for (int i = 6; i >= 0; i--)
    {
        if (tmp[i] == 1)
        {
            emit("pop %s\n", 
                scratch_regs[i]);
        }
    }

    memcpy(scratch_in_use, tmp, sizeof(scratch_in_use));

    if ((in_function_call) && (nested_c != 1))
        emit("pop rcx\npop rsi\npop rdi\n");

    in_function_call = false;
    nested_c = 0;
}

void emit_print(Statement_t *stmt)
{
    Args_t *args = stmt->args;

    while (args != NULL)
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

void emit_input(Statement_t *stmt)
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

void emit_assert(Statement_t *stmt)
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

void emit_break(Statement_t *stmt)
{
    if (!current_loop)
        return;

    emit("jmp .LC%.5d\n", current_loop->lbl);
}

void emit_move_args_to_stack(Args_t *args)
{
    unsigned int pos = 0;

    while ((args != NULL) && (pos < 3))
    {

        emit("movq [%s], %s\n", 
            symbol_s(args->sym), 
            args_regs[pos]);

        pos += 1;
        args = args->next;
    }
}

void emit_array_initialization(Args_t *args, Symbol_t *sym)
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

void emit_structure_initialization(Args_t *args, Symbol_t *sym)
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

int new_label()
{
    static int i = 0;
    return i++;
}

void emit_prologue(Statement_t *stmt)
{
    unsigned int size = get_stack_size(stmt);
    emit("push rbp\n");
    emit("mov rbp, rsp\n");
    emit("sub rsp, %u\n", size);
}

void emit_epilogue()
{
    emit("leave\nret\n");
}

unsigned int get_stack_size(Statement_t *stmt)
{
    unsigned int size = 0;

    Args_t *args = NULL;
    if (stmt->decl != NULL)
        args = stmt->decl->args;

    while (args != NULL)
    {
        size += 8;
        args = args->next;
    }

    if (stmt->decl != NULL)
        stmt = stmt->decl->code;


    while (stmt != NULL)
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
            if (stmt->if_block != NULL)
                size += get_stack_size(stmt->if_block);

            if ((stmt->for_loop != NULL) && stmt->for_loop->stmt_type == STMT_DECLARATION)
                size += 8;
        }

        stmt = stmt->next;
    }
    return size;
}

unsigned int reg_alloc(unsigned int reg)
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
}

const char *reg_name(unsigned int r)
{
    return scratch_regs[r];
}

const char *reg_name_l(unsigned int r)
{
    return scratch_regs_l[r];
}

void reg_free(Expression_t *expr)
{
    if (expr->reg != UINT_MAX)
    {
        scratch_in_use[expr->reg] = 0;
        expr->reg = UINT_MAX;
    }
}

void load_to_reg(Expression_t *expr)
{
    if ((expr != NULL) && (expr->expr_type == EXPR_FUNCCALL))
    {
        emit("movq %s, rax\n",
            reg_name(expr->reg));
    }

    else if ((expr != NULL) && (expr->expr_type == EXPR_NUMBER))
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

    else if ((expr != NULL) && (expr->expr_type == EXPR_SYMBOL))
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

    else if ((expr != NULL) && (expr->expr_type == EXPR_ARRAYA))
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

void store_to_stack(Expression_t *expr, Symbol_t *sym)
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

char *symbol_s(Symbol_t *sym)
{

    if ((sym->_type.is_array) && (sym->type != ARG))
    {

        if (sym->_type.t == _BYTE || sym->_type.t == _CHAR)
            snprintf(pos, sizeof(pos), "rbp-%u", 8 + ((((Array_s*)(sym->_type.ptr))->size + 7) & (-8)));
        else
            snprintf(pos, sizeof(pos), "rbp-%u", 8 * (sym->pos+1+((Array_s*)(sym->_type.ptr))->size));
    }
    else
    {
        snprintf(pos, sizeof(pos), "rbp-%u", 8*(sym->pos+1));
    }

    return pos;
}

void _start_def()
{
    emit(".text\n.global _start\n_start:\n");
    emit("push rbp\nmov rbp, rsp\nsub rsp, 16\n");
    emit("call main\n");
    emit("xor rax, rax\nmov al, 60\nxor rdi, rdi\nsyscall\n");
}
