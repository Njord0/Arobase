#include <stdio.h>
#include <stdlib.h>

#include <scope.h>
#include <statements.h>
#include <tokens.h>
#include <errors/error.h>

Statement_t*
get_scope(Token_t **token, Declaration_t *decl)
{

    Token_t *tok = *token;

    Statement_t *stmt = NULL;
    Statement_t *last_stmt = NULL;
    Statement_t *scope = NULL;

    while (tok && (tok->type != RBRACE))
    {
        stmt = get_next_statement(&tok);
        if (stmt && stmt->decl &&(stmt->decl->decl_type == FUNCTION))
        {
            show_error_source(tok);
            fprintf(stderr, 
                "Nested function declaration are not allowed\n");
            free_statement(stmt);
            cc_exit();
        }

        if (stmt && (stmt->stmt_type == STMT_STRUCT))
        {
            show_error_source(tok);
            fprintf(stderr,
                "Structure definition must be outside scope!\n");
            free_statement(stmt);
            cc_exit();
        }

        if (!scope)
        {
            scope = stmt;
            last_stmt = stmt;
        }
        else
        {
            last_stmt->next = stmt;
            last_stmt = stmt;
        }

        tok = tok->next;
    }

    *token = tok;
    return scope;
}

void
scope_check_return_value_type(Statement_t *stmt, Declaration_t *decl, Token_t *tok)
{
    while (stmt)
    {
        switch(stmt->stmt_type)
        {
            case STMT_IF_ELSE:
            case STMT_TRY_EXCEPT:
                scope_check_return_value_type(stmt->if_block, decl, tok);
                scope_check_return_value_type(stmt->else_block, decl, tok);
                break;
            case STMT_WHILE:
            case STMT_FOR:
                scope_check_return_value_type(stmt->if_block, decl, tok);
                break;

            case STMT_RETURN:
                type_set(stmt->expr, decl->type);
                type_check(stmt->expr);
                
                if (decl->type.t != type_evaluate(stmt->expr, decl->type.t).t)
                {
                    show_error_source(tok);
                    fprintf(stderr,
                        "Invalid return value type for function '%s'\n",
                        decl->name);
                    cc_exit();
                }

            default:
                break;
        }

        stmt = stmt->next;
    }
}