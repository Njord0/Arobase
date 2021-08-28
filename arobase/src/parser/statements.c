#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <type.h>
#include <args.h>
#include <tokens.h>
#include <statements.h>
#include <struct.h>
#include <error_handler.h>
#include <symbol_table.h>

#define IS_KEYWORD(x, y) \
    x->type == KEYWORD && strcmp(x->value.p, Arobase_ReservedKeywords[y]) == 0 

const char *Arobase_ReservedKeywords[] = {
    "let",
    "if",
    "else",
    "fn",
    "return",
    "while",
    "string",
    "integer",
    "byte",
    "void",
    "bool",
    "char",
    "print",
    "input",
    "import",
    "assert",
    "for",
    "break",
    "struct"
};

unsigned int loop_count = 0;

Statement_t *get_next_statement(Token_t **token)
{
    Token_t *tok = *token;

    Statement_t *stmt = NULL;
    if (IS_KEYWORD(tok, KW_LET))
        stmt = stmt_create_var_declaration(&tok);

    else if (IS_KEYWORD(tok, KW_FN))
        stmt = stmt_create_func_declaration(&tok);

    else if (IS_KEYWORD(tok, KW_IF))
        stmt = stmt_create_if_else(&tok);

    else if (IS_KEYWORD(tok, KW_WHILE))
        stmt = stmt_create_while_loop(&tok);

    else if (IS_KEYWORD(tok, KW_FOR))
        stmt = stmt_create_for_loop(&tok);

    else if (IS_KEYWORD(tok, KW_RETURN))
        stmt = stmt_create_return(&tok);

    else if (IS_KEYWORD(tok, KW_PRINT))
        stmt = stmt_create_print(&tok);
    
    else if (IS_KEYWORD(tok, KW_INPUT))
        stmt = stmt_create_input(&tok);

    else if (IS_KEYWORD(tok, KW_IMPORT))
        stmt = stmt_create_import(&tok);

    else if (IS_KEYWORD(tok, KW_ASSERT))
        stmt = stmt_create_assert(&tok);

    else if (IS_KEYWORD(tok, KW_BREAK))
        stmt = stmt_create_break(&tok);

    else if (IS_KEYWORD(tok, KW_STRUCT))
        stmt = stmt_create_struct(&tok);

    else if (tok->type == SYMBOL)
    {
        if ((tok->next != NULL) && ((tok->next->type == ASSIGN) || (tok->next->type == LBRACKET) || (tok->next->type == DOT)))
            stmt = stmt_create_var_assign(&tok);

        else if ((tok->next != NULL) && (tok->next->type == LPAR))
            stmt = stmt_create_func_call(&tok);

        else
            invalid_syntax_error(tok);
    }
    else if (IS_KEYWORD(tok, KW_ELSE))
    {

        show_error_source(tok);
        fprintf(stderr,
            "'else' statement without 'if' statement!\n");
        cc_exit();
    }

    else
    {
        show_error_source(tok);
        fprintf(stderr,
            "Unknown syntax.\n");
        cc_exit();
    }

    *token = tok;
    return stmt;
}

Statement_t *stmt_create_var_declaration(Token_t **token)
{
    Token_t *tok = *token;
    Token_t *next_token = tok->next;

    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    if (!token_expect(next_token, SYMBOL))
    {
        free(stmt);
        cc_exit();
    }

    char *stmt_name = next_token->value.p;

    if (is_reserved(stmt_name))
    {
        show_error_source(next_token);
        fprintf(stderr, 
            "'%s' is a reserved keyword\n",
            stmt_name);
        free(stmt);
        cc_exit();
    }

    Symbol_t *sym = NULL;
    if (is_declared_var(symtab_g, stmt_name, &sym))
    {
        show_error_source(next_token);
        fprintf(stderr, 
            "Multiple definitions of symbol '%s'\n",
            stmt_name);
        free(stmt);
        cc_exit();
    }

    next_token = next_token->next;

    Type_s type = get_type(&next_token);

    if (!token_checks(next_token, 2, ASSIGN, EOS))
    {
        free(stmt);
        invalid_syntax_error(next_token);
    }    

    stmt->stmt_type = STMT_DECLARATION;

    stmt->decl = declaration_create_var(&next_token, stmt_name, type);

    if (type.is_array)
    {
        if (get_args_count(stmt->decl->args) > ((Array_s*)(type.ptr))->size)
        {
            show_error_source(next_token);
            fprintf(stderr,
                "Array initialization is larger than declaration...\n");
            cc_exit();
        }

        // Check array elements
        Args_t *args = stmt->decl->args;
        while (args != NULL)
        {
            if (args->type.t != type.t)
            {
                show_error_source(next_token);
                fprintf(stderr, 
                    "Invalid type in array initialization\n");
                cc_exit();
            }
            args = args->next;
        }
    }

    else if (type.is_structure && stmt->decl->is_initialised)
    {
        Statement_t *str = get_struct_by_name(type.ptr);
        //if (!str)

        if (get_args_count(str->args) != get_args_count(stmt->decl->args))
        {
            show_error_source(next_token);
            fprintf(stderr, 
                "Invalid structure initialization\n");
            cc_exit();
        }

        Args_t *args = str->args;
        Args_t *args_decl = stmt->decl->args;

        while (args)
        {
            if (args->type.t != args_decl->type.t)
            {
                show_error_source(next_token);
                fprintf(stderr,
                    "Invalid type for member '%s', '%s' was expected not '%s'\n",
                    args_decl->name,
                    type_name(args_decl->type.t),
                    type_name(args->type.t));
                cc_exit();
            }
            
            args = args->next;
            args_decl = args_decl->next;
        }
    }

    if (!token_expect(next_token, EOS))
    {
        free_statement(stmt);
        cc_exit();
    }

    *token = next_token;

    return stmt;
}

Statement_t *stmt_create_var_assign(Token_t **token)
{

    Token_t *tok = *token;
    Token_t *next_token = tok->next;
    
    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    char *name = tok->value.p;

    Symbol_t *sym = symbol_resolve(symtab_g, name);

    if (sym == NULL) 
    {
        free_statement(stmt);
        show_error_source(next_token);
        fprintf(stderr,
            "Undeclared variable '%s'\n",
            name);
        cc_exit();
    }

    if (sym->decl)
        sym->decl->is_initialised = true;

    if (token_check(next_token, LBRACKET))
    {
        next_token = next_token->next;

        stmt->access = expr_create(&next_token, INTEGER);

        if (!token_check(next_token, RBRACKET))
        {
            free(stmt);
            show_error_source(next_token);
            fprintf(stderr,
                "Invalid access to array\n");
            cc_exit();
        }

        next_token = next_token->next;
    }

    if (token_check(next_token, DOT))
    {
        stmt = stmt_create_struct_assign(&next_token, stmt, name);
        *token = next_token;
        return stmt;
    }

    next_token = next_token->next;

    stmt->stmt_type = STMT_ASSIGN;

    stmt->expr = expr_create(&next_token, sym->_type.t);

    if (stmt->expr == NULL)
    {
        show_error_source(next_token);
        fprintf(stderr, 
            "Invalid expression");
        free_statement(stmt);
        cc_exit();
    }

    stmt->expr->sym = sym;
    stmt->expr->type = sym->_type;

    type_set(stmt->expr, sym->_type);

    Type_s type = type_evaluate(stmt->expr, sym->_type.t);

    if (sym->_type.t != type.t)
    {
        show_error_source(next_token);
        fprintf(stderr, 
            "Can't assign '%s' value to a '%s' variable\n",
            type_name(type.t),
            type_name(sym->_type.t));
        free_statement(stmt);
        cc_exit();
    }

    if (!token_check(next_token, EOS))
    {
        show_error_source(next_token);
        fprintf(stderr,
            "Invalid end of statement\n");

        free_statement(stmt);
        cc_exit();
    }

    *token = next_token;

    return stmt;
}

Statement_t *stmt_create_struct_assign(Token_t **token, Statement_t *stmt, const char *name)
{
    Token_t *tok = *token;
    tok = tok->next;

    Symbol_t *sym = symbol_resolve(symtab_g, name);

    Statement_t *str = get_struct_by_name(sym->_type.ptr);

    Args_t *member = struct_get_member(str, tok->value.p);

    if (!member)
    {
        show_error_source(tok);
        fprintf(stderr,
            "Structure '%s' has no member '%s'\n",
            (char*)sym->_type.ptr,
            tok->value.p);
        free_statement(stmt);
        cc_exit();
    }

    tok = tok->next;

    if (!token_expect(tok, ASSIGN))
    {
        free(stmt);
        cc_exit();
    }

    tok = tok->next;

    stmt->stmt_type = STMT_ASSIGN;
    stmt->expr = expr_create(&tok, member->type.t); // type of member to assign the value.

    if (!stmt->expr)
    {
        show_error_source(tok);
        fprintf(stderr, 
            "Invalid expression\n");
        free_statement(stmt);
        cc_exit();
    }
    stmt->expr->sym = sym;
    stmt->expr->type = member->type;
    //stmt->expr->string_value = member->name;
    stmt->expr->args = member;

    type_set(stmt->expr, member->type);

    Type_s type = type_evaluate(stmt->expr, sym->_type.t);

    if (member->type.t != type.t)
    {
        show_error_source(tok);
        fprintf(stderr, 
            "Can't assign '%s' value to member '%s' (%s) of structure '%s'.\n",
            type_name(type.t),
            member->name,
            type_name(member->type.t),
            str->import_name);
        free_statement(stmt);
        cc_exit();
    }

    if (!token_check(tok, EOS))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Invalid end of statement\n");
        free_statement(stmt);
        cc_exit();
    }

    *token = tok;
    return stmt;
}

Statement_t *stmt_create_func_declaration(Token_t **token)
{
    Token_t *tok = *token;
    Token_t *next_token = tok->next;

    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    if (!token_expect(next_token, SYMBOL))
    {
        free(stmt);
        cc_exit();
    }

    char *name = next_token->value.p;
    current_function = stmt;

    stmt->stmt_type = STMT_DECLARATION;
    stmt->decl = malloc(sizeof(Declaration_t));
    stmt->decl = declaration_create_func(&next_token, name, stmt->decl);

    if ((stmt->decl != NULL) && (stmt->decl->code == NULL))
        fprintf(stderr, 
            "WARNING: empty function body for function '%s'\n", 
            name);

    *token = next_token;

    current_function = NULL;

    return stmt;
}

Statement_t *stmt_create_func_call(Token_t **token)
{
    Token_t *tok = *token;
    Token_t *next_token = tok->next;

    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    assert (tok->type == SYMBOL);

    stmt->stmt_type = STMT_EXPR;
    stmt->expr = expr_create_funccall(&next_token, tok->value.p);

    if (!token_check(next_token, EOS))
    {
        show_error_source(next_token);
        fprintf(stderr,
            "Invalid end of statement\n");
        free_statement(stmt);
        cc_exit();
    }

    *token = next_token;
    return stmt;
}

Statement_t *stmt_create_if_else(Token_t **token)
{
    Token_t *tok = *token;

    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);
    stmt->stmt_type = STMT_IF_ELSE;

    tok = tok->next;
    if (!token_check(tok, LPAR))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Missing left parenthesis before condition\n");
        free(stmt);
        cc_exit();
    }

    tok = tok->next;

    stmt->expr = expr_create_cond(&tok, _VOID);

    if (!token_check(tok, RPAR))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Missing right parenthesis after condition\n");
        free_statement(stmt);
        cc_exit();
    }

    tok = tok->next;

    if (!token_expect(tok, LBRACE))
    {
        free_statement(stmt);
        invalid_syntax_error(tok);
    }

    Statement_t *stmtt = NULL;
    Statement_t *last_stmt = NULL;

    tok = tok->next;

    while ((tok != NULL) && (tok->type != RBRACE))
    {
        stmtt = get_next_statement(&tok);

        if ((stmtt != NULL) && (stmtt->stmt_type == STMT_DECLARATION) && (stmtt->decl->expr == NULL))
        {
            free_statement(stmtt);
            invalid_syntax_error(tok);
        }

        if (stmt->if_block == NULL)
        {
            stmt->if_block = stmtt;
            last_stmt = stmtt;
        }

        else
        {
            last_stmt->next = stmtt;
            last_stmt = stmtt;
        }

        tok = tok->next;
    }

    if (tok == NULL)
    {
        fprintf(stderr, 
            "Missing '}'\n");
        free_statement(stmt);
        cc_exit();
    }

    if (!token_expect(tok, RBRACE))
    {
        free_statement(stmt);
        cc_exit();
    }

    Token_t *tmp = tok;
    tok = tok->next;

    if ((tok != NULL) && (tok->type == KEYWORD) && (strcmp(tok->value.p, Arobase_ReservedKeywords[KW_ELSE]) == 0))
    {
        tok = tok->next;

        if (!token_expect(tok, LBRACE))
        {
            free_statement(stmt);
            cc_exit();
        }

        tok = tok->next;

        stmtt = NULL;
        last_stmt = NULL;


        while ((tok != NULL) && (tok->type != RBRACE))
        {
            stmtt = get_next_statement(&tok);

            if ((stmtt != NULL) && (stmtt->stmt_type == STMT_DECLARATION) && (stmtt->decl->expr == NULL))
            {
                free_statement(stmtt);
                invalid_syntax_error(tok);
            }

            if (stmt->else_block == NULL)
            {
                stmt->else_block = stmtt;
                last_stmt = stmtt;
            }

            else
            {
                last_stmt->next = stmtt;
                last_stmt = stmtt;
            }

            tok = tok->next;
        }

        if (!token_expect(tok, RBRACE))
        {
            free_statement(stmt);
            cc_exit();
        }
    }
    else
        tok = tmp;

    *token = tok;
    return stmt;
}

Statement_t *stmt_create_while_loop(Token_t **token)
{
    Token_t *tok = *token;

    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);
    tok = tok->next;

    if (!token_check(tok, LPAR))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Missing left parenthesis before condition\n");
        free(stmt);
        cc_exit();
    }

    tok = tok->next;

    stmt->stmt_type = STMT_WHILE;
    stmt->expr = expr_create_cond(&tok, _VOID);


    if (!token_check(tok, RPAR))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Missing right parenthesis after condition\n");
        free_statement(stmt);
        cc_exit();
    }

    tok = tok->next;

    if (!token_expect(tok, LBRACE))
    {
        free_statement(stmt);
        cc_exit();
    }

    Statement_t *stmtt = NULL;
    Statement_t *last_stmt = NULL;

    tok = tok->next;

    loop_count++;

    while (!token_check(tok, RBRACE))
    {
        stmtt = get_next_statement(&tok);

        if ((stmtt != NULL) && (stmtt->stmt_type == STMT_DECLARATION) && (stmtt->decl->expr == NULL))
        {
            free_statement(stmtt);
            invalid_syntax_error(tok);
        }

        if (stmt->if_block == NULL)
        {
            stmt->if_block = stmtt;
            last_stmt = stmtt;
        }

        else
        {
            last_stmt->next = stmtt;
            last_stmt = stmtt;
        }

        tok = tok->next;
    }

    loop_count--;

    if (!token_expect(tok, RBRACE))
    {
        free_statement(stmt);
        cc_exit();
    }

    *token = tok;
    return stmt;
}

Statement_t *stmt_create_for_loop(Token_t **token)
{
    Token_t *tok = *token;
    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    tok = tok->next;

    if (!token_check(tok, LPAR))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Missing left parenthesis before 'for' expression\n");

        free(stmt);
        cc_exit();
    }

    tok = tok->next;

    stmt->stmt_type = STMT_FOR;

    Statement_t *stmtt = get_next_statement(&tok);

    if ((stmtt->stmt_type != STMT_DECLARATION) && (stmtt->stmt_type != STMT_ASSIGN))
    {
        free_statement(stmtt);
        free(stmt);
        show_error_source(tok);
        fprintf(stderr, 
            "Variable declaration or assignement was expected here\n");
        cc_exit();
    }

    if ((stmtt->stmt_type == STMT_DECLARATION) && (stmtt->decl->type.t != INTEGER))
    {
        free_statement(stmtt);
        free(stmt);
        show_error_source(tok);
        fprintf(stderr,
            "Only integer can be declared inside for loop initialization\n");
        cc_exit();
    }

    stmt->for_loop = stmtt;
    tok = tok->next;
    stmt->expr = expr_create_cond(&tok, _VOID);

    if (!token_expect(tok, EOS))
    {
        free_statement(stmt);
        cc_exit();
    }


    tok = tok->next;

    stmt->else_block = get_next_statement(&tok);

    if (stmt->else_block->stmt_type != STMT_ASSIGN)
    {
        show_error_source(tok);
        fprintf(stderr,
            "Variable assignement was expected here\n");
        free_statement(stmt);
        cc_exit();
    }

    tok = tok->next;

    if (!token_check(tok, RPAR))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Missing right parenthesis after 'for' expression\n");
        free_statement(stmt);
        cc_exit();
    }

    tok = tok->next;

    if (!token_expect(tok, LBRACE))
    {
        free_statement(stmt);
        cc_exit();
    }

    stmtt = NULL;
    Statement_t *last_stmt = NULL;

    tok = tok->next;

    loop_count++;

    while (!token_check(tok, RBRACE))
    {
        stmtt = get_next_statement(&tok);

        if ((stmtt != NULL) && (stmtt->stmt_type == STMT_DECLARATION) && (stmtt->decl->expr == NULL))
        {
            free_statement(stmtt);
            invalid_syntax_error(tok);
        }

        if (stmt->if_block == NULL)
        {
            stmt->if_block = stmtt;
            last_stmt = stmtt;
        }

        else
        {
            last_stmt->next = stmtt;
            last_stmt = stmtt;
        }

        tok = tok->next;
    }

    loop_count--;

    if (!token_expect(tok, RBRACE))
    {
        free_statement(stmt);
        cc_exit();
    }

    *token = tok;
    return stmt;
}

Statement_t *stmt_create_return(Token_t **token)
{
    Token_t *tok = *token;
    Statement_t *stmt = xmalloc(sizeof(Statement_t));

    tok = tok->next;

    stmt_init(stmt);
    stmt->stmt_type = STMT_RETURN;
    stmt->expr = expr_create(&tok, _VOID);

    if (!token_check(tok, EOS))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Invalid end of statement\n");
        free_statement(stmt);
        cc_exit();
    }

    *token = tok;

    return stmt;
}

Statement_t *stmt_create_print(Token_t **token)
{
    Token_t *tok = *token;
    Statement_t *stmt = xmalloc(sizeof(Statement_t));

    tok = tok->next;

    stmt_init(stmt);
    stmt->stmt_type = STMT_PRINT;
    stmt->args = get_args(&tok, _VOID);

    if (!token_check(tok, EOS))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Invalid end of statement\n");
        free_statement(stmt);
        cc_exit();
    }

    // Check args type...
    Args_t *args = stmt->args;
    while (args)
    {
        if ((args->type.is_array && args->expr->expr_type != EXPR_ARRAYA) || args->type.is_structure)
        {
            show_error_source(tok);
            fprintf(stderr,
                "Can't print '%s'(%s) type\n",
                type_name(args->type.t),
                (args->type.is_array ? "array" : "structure"));

            free_statement(stmt);
            cc_exit();
        }
        args = args->next;
    }

    *token = tok;
    return stmt;
}

Statement_t *stmt_create_input(Token_t **token)
{
    Token_t *tok = *token;
    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    tok = tok->next;

    if (!(token_expect(tok, SYMBOL)))
        cc_exit();

    Symbol_t *sym = NULL;

    if (!is_declared_var(symtab_g, tok->value.p, &sym))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Undeclared variable '%s'",
            tok->value.p);
        free(stmt);
        cc_exit();
    }

    if ((sym->_type.is_array) || (sym->_type.t == STRING) || sym->_type.is_structure)
    {
        show_error_source(tok);
        fprintf(stderr,
            "Invalid type with input statement\n");
        free(stmt);
        cc_exit();
    }

    if (sym->decl)
        sym->decl->is_initialised = true;

    stmt->stmt_type = STMT_INPUT;
    stmt->decl = sym->decl;

    tok = tok->next;

    if (!token_expect(tok, EOS))
        cc_exit();        

    *token = tok;
    return stmt;
}

Statement_t *stmt_create_import(Token_t **token)
{
    Token_t *tok = *token;
    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    stmt->stmt_type = STMT_IMPORT;

    tok = tok->next;

    if (!token_expect(tok, SYMBOL))
        cc_exit();

    stmt->import_name = xmalloc(strlen(tok->value.p)+1);
    strcpy(stmt->import_name, tok->value.p);

    tok = tok->next;

    while (token_check(tok, DOT))
    {
        tok = tok->next;
        if (!token_expect(tok, SYMBOL))
            cc_exit();

        stmt->import_name = xrealloc(stmt->import_name, strlen(stmt->import_name)+1+strlen(tok->value.p)+1);

        strcat(stmt->import_name, "/");
        strcat(stmt->import_name, tok->value.p);

        tok = tok->next;
    }

    stmt->import_name = xrealloc(stmt->import_name, strlen(stmt->import_name)+6);

    strcat(stmt->import_name, ".aroh"); // arobase header file

    if (!token_expect(tok, EOS))
        cc_exit();

    import_from(stmt->import_name);

    *token = tok;
    return stmt;
}

Statement_t *stmt_create_assert(Token_t **token)
{
    Token_t *tok = *token;
    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    stmt->stmt_type = STMT_ASSERT;

    tok = tok->next;

    stmt->expr = expr_create_cond(&tok, _VOID);

    if (token_check(tok, COMMA))
    {
        tok = tok->next;
        if (!token_check(tok, TOK_STRING))
        {
            show_error_source(tok);
            fprintf(stderr,
                "A string was expected here\n");
            free(stmt);
            cc_exit();
        }

        stmt->import_name = tok->value.p;
        tok = tok->next;
    }

    if (!token_check(tok, EOS))
    {
        show_error_source(tok);
        fprintf(stderr, 
            "Invalid end of statement\n");
        free_statement(stmt);
        cc_exit();
    }

    *token = tok;
    return stmt;
}

Statement_t *stmt_create_break(Token_t **token)
{
    Token_t *tok = *token;
    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);

    stmt->stmt_type = STMT_BREAK;

    if (loop_count == 0)
    {
        show_error_source(tok);
        fprintf(stderr,
            "'break' statement outside loop\n");
        free(stmt);
        cc_exit();
    }

    tok = tok->next;

    if (!token_check(tok, EOS))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Invalid end of statement\n");
        free(stmt);
        cc_exit();
    }

    *token = tok;
    return stmt;
}

Statement_t *stmt_create_struct(Token_t **token)
{
    Token_t *tok = *token;
    Statement_t *stmt = xmalloc(sizeof(Statement_t));
    stmt_init(stmt);
    stmt->stmt_type = STMT_STRUCT;

    tok = tok->next;

    if (!token_check(tok, SYMBOL))
    {

        show_error_source(tok);
        fprintf(stderr,
            "Missing structure name\n");
        free(stmt);
        cc_exit();
    }

    stmt->import_name = tok->value.p;

    tok = tok->next;

    if (!token_expect(tok, LBRACE))
    {
        free(stmt);
        cc_exit();
    }

    tok = tok->next;

    stmt->args = struct_get_args(&tok);

    if (!stmt->args)
    {
        show_error_source(tok);
        fprintf(stderr,
            "Empty structure\n");
            free_statement(stmt);
        cc_exit();
    }

    if (!token_expect(tok, RBRACE))
    {
        free_args(stmt->args);
        free(stmt);
        cc_exit();
    }

    tok = tok->next;

    if (!token_check(tok, EOS))
    {
        show_error_source(tok);
        fprintf(stderr,
            "Invalid end of statement\n");
        free_args(stmt->args);
        free(stmt);
        cc_exit();
    }

    *token = tok;
    return stmt;
}

void stmt_init(Statement_t *stmt)
{
    stmt->decl = NULL;
    stmt->expr = NULL;
    stmt->access = NULL;
    stmt->args = NULL;
    stmt->if_block = NULL;
    stmt->else_block = NULL;
    stmt->next = NULL;
    stmt->import_name = NULL;
    stmt->for_loop = NULL;
}

void free_statement(Statement_t *stmt)
{
    if (stmt != NULL)
    {
        if ((stmt->decl != NULL) && (stmt->expr == NULL) && (stmt->stmt_type != STMT_INPUT))
        {
            free_declaration(stmt->decl);
        }

        if (stmt->expr != NULL)
            free_expression(stmt->expr);
    }

    if (stmt->stmt_type == STMT_IF_ELSE)
        free_if_else_statement(stmt);

    else if (stmt->stmt_type == STMT_WHILE)
        free_while_loop(stmt);

    else if (stmt->stmt_type == STMT_IMPORT)
        free(stmt->import_name);

    if (stmt->args != NULL)
        free_args(stmt->args);

    if (stmt->access != NULL)
        free_expression(stmt->access);

    if (stmt->stmt_type == STMT_ASSERT)
        free(stmt->import_name);

    if (stmt->stmt_type == STMT_FOR)
        free_for_loop(stmt);


    free(stmt);
}

void free_while_loop(Statement_t *stmt)
{
    Statement_t *prev = stmt->if_block;
    Statement_t *last = NULL;

    while (prev != NULL)
    {
        last = prev->next;
        free_statement(prev);
        prev = last;
    }

}

void free_for_loop(Statement_t *stmt)
{
    Statement_t *prev = stmt->if_block;
    Statement_t *last = NULL;

    free_statement(stmt->else_block);
    free_statement(stmt->for_loop);

    while (prev != NULL)
    {
        last = prev->next;
        free_statement(prev);
        prev = last;
    }
}

void free_if_else_statement(Statement_t *stmt)
{
    Statement_t *prev = stmt->if_block;
    Statement_t *last = NULL;

    while (prev != NULL)
    {
        last = prev->next;
        free_statement(prev);
        prev = last;
    }

    prev = stmt->else_block;
    last = NULL;

    while (prev != NULL)
    {
        last = prev->next;
        free_statement(prev);
        prev = last;
    }
}


unsigned int find_keyword(const char *ptr)
{
    for (unsigned int i = 0; i < KW_NO; i++)
    {
        if (strcmp(ptr, Arobase_ReservedKeywords[i]) == 0)
            return i;
    }

    return 5;
}

bool is_reserved(const char *str)
{
    for (unsigned int i = 0; i < KW_NO; i++)
    {
        if (strcmp(str, Arobase_ReservedKeywords[i]) == 0)
            return true;
    }

    return false;
}