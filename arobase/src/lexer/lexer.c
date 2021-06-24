#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#include <lexer.h>
#include <statements.h>
#include <error_handler.h>

Lexer_t *lexer_create(char *filename)
{
    Lexer_t *lexer = xmalloc(sizeof(Lexer_t));

    lexer->filename = filename;
    lexer->pos = 0;
    FILE *file = fopen(filename, "r");
    if (file == NULL)
        return NULL;

    lexer->file = file;
    lexer->current_lineno = 1;

    return lexer;
}

void lexer_free(Lexer_t *lexer)
{
    if (lexer != NULL)
        fclose(lexer->file);

    Token_t *tok = lexer->first_token;
    Token_t *next;

    while (tok != NULL)
    {
        next = tok->next;
        free_token(tok);
        tok = next;
    }
    
    free(lexer);

}

void lexer_tokenize(Lexer_t *lexer)
{
    Token_t *tok;

    tok = lexer_get_next_token(lexer);
    if (tok == NULL)
        return;

    lexer->first_token = tok;
    lexer->last_token = tok;

    while ((tok = lexer_get_next_token(lexer)) != NULL)
    {
        lexer->last_token->next = tok;
        lexer->last_token = tok;
    }

}

Token_t *lexer_get_next_token(Lexer_t *lexer)
{
    char c;
    int64_t value;
    Token_t *tok = NULL;

    c = lexer_skip_whitespaces(lexer);
    if (c == EOF)
        return NULL;

    if (isdigit(c))
    {
        ungetc(c, lexer->file);
        value = lexer_get_number(lexer);
        tok = create_token_number(value);
    }

    else if (c == '(')
        tok = create_token_char(LPAR, c);
    
    else if (c == ')')
        tok = create_token_char(RPAR, c);

    else if (c == '+')
        tok = create_token_char(PLUS, c);

    else if (c == '-')
        tok = create_token_char(MINUS, c);

    else if (c == '/')
        tok = create_token_char(DIV, c);

    else if (c == '*')
        tok = create_token_char(MUL, c);

    else if (c == '=')
    {
        c = (char)getc(lexer->file);
        if (c == '=')
            tok = create_token_char(CMP, c);
        else
        {
            tok = create_token_char(ASSIGN, c);
            ungetc(c, lexer->file);            
        }    
    }

    else if (c == '!')
    {
        c = (char)getc(lexer->file);
        if (c == '=')
            tok = create_token_char(DIFF, c);
        else
        {
            fprintf(stderr, 
                "Unknow token on line : %lu\n",
                lexer->current_lineno);
            cc_exit();
        }
    }

    else if (c == '<')
    {
        c = (char)getc(lexer->file);
        if (c == '=')
        {
            tok = create_token_char(OP_LOWER_EQ, c);
        }
        else
        {
            tok = create_token_char(OP_LOWER, c);
            ungetc(c, lexer->file);
        }
    }

    else if (c == '>')
    {   
        c = (char)getc(lexer->file);
        if (c == '=')
        {
            tok = create_token_char(OP_GREATER_EQ, c);            
        }
        else
        {
            tok = create_token_char(OP_GREATER, c);
            ungetc(c, lexer->file);
        }
    }

    else if (c == ';')
        tok = create_token_char(EOS, c);

    else if (c == '{')
        tok = create_token_char(LBRACE, c);
    
    else if (c == '}')
        tok = create_token_char(RBRACE, c);

    else if (c == '"')
    {
        char *ptr = lexer_get_string(lexer);

        tok = create_token_s(ptr);
        tok->type = TOK_STRING;
    }
    
    else if (c == '\'')
        tok = create_token_char(QUOTE, c);

    else if (c == ',')
        tok = create_token_char(COMMA, c);

    else if (c == ':')
        tok = create_token_char(COLON, c);

    else if (c == '@')
    {
        c = (char)getc(lexer->file);
        char d = '\x00';
        if (c == '@')
        {
            while (((c != '@') || (d != '@')) && (c != EOF))
            {
                c = d;
                d = (char)getc(lexer->file);

                if (c == '\n')
                    lexer->current_lineno += 1;
            }

            return lexer_get_next_token(lexer);

        }
        else
        {
            while ((c != '\n') && (c != EOF))
                c = (char)getc(lexer->file);

            lexer->current_lineno += 1;

            return lexer_get_next_token(lexer);
        }
    }

    else if (c == '[')
        tok = create_token_char(LBRACKET, c);

    else if (c == ']')
        tok = create_token_char(RBRACKET, c);

    else if (c == '.')
        tok = create_token_char(DOT, c);

    else if (c == '%')
        tok = create_token_char(MODULO, c);

    else if (isalpha(c))
    {
        ungetc(c, lexer->file);
        char *ptr = lexer_get_symbolname(lexer); // check if is reserved keyword
        tok = create_token_s(ptr);

        if(token_symbol_is_reserved(ptr))
        {
            tok->type = KEYWORD;
            tok->value.p = Arobase_ReservedKeywords[find_keyword(ptr)];
            free(ptr);
        }

    }

    else
    {
        fprintf(stderr, 
            "Unknow token on line : %lu\n",
            lexer->current_lineno);
        cc_exit();
    }
    
    tok->next = NULL;
    tok->lineno = lexer->current_lineno;
    return tok;

}

char lexer_skip_whitespaces(Lexer_t *lexer)
{
    int c;
    while (((((c = getc(lexer->file)) == ' ')) || (c == '\n')) 
            && (c != EOF)) 
    {
        if (c == '\n')
            lexer->current_lineno += 1;
    }

    return (char)c;

}

int64_t lexer_get_number(Lexer_t *lexer)
{
    int64_t value = 0;
    int c;

    char a[20] = {0};
    unsigned int i = 0;

    while (((c = getc(lexer->file)) != EOF) && isdigit(c) && (i <= 18))
        a[i++] = (char)c;
    
    a[i] = '\x00';

    if ((i == 19) && (isdigit(c)))
    {
        fprintf(stderr,
            "Error on line : %lu\n\tNumber too big to fit in an integer\n",
            lexer->current_lineno);
        cc_exit();
    }

    ungetc(c, lexer->file);

    for (unsigned int j = 0; j < i; j++)
    {
        value *= 10;
        value += (a[j] - 0x30);
    }
    
    return value;
}

char *lexer_get_symbolname(Lexer_t *lexer)
{
    int c;
    unsigned int i = 0;
    
    char *ptr = xmalloc(sizeof(char) * (i+1));

    while (((c = getc(lexer->file)) != EOF) && ((isalnum(c)) || c == '_'))
    {
        i++;

        ptr = realloc(ptr, sizeof(char) * (i+1));
        if (ptr == NULL)
            return NULL;

        ptr[i-1] = (char)c;
    }

    ungetc(c, lexer->file);

    ptr[i] = '\x00';

    return ptr;

}

char *lexer_get_string(Lexer_t *lexer)
{
    int c;
    unsigned int i = 0;
    
    char *ptr = xmalloc(sizeof(char) * (i+1));

    while (((c = getc(lexer->file)) != EOF) && (isascii(c) || (c == ' ')) && (c != '"') && (c != '\n'))
    {
        i++;

        ptr = realloc(ptr, sizeof(char) * (i+1));
        if (ptr == NULL)
            return NULL;

        ptr[i-1] = (char)c;
    }

    if ((c == EOF) || (c == '\n'))
    {
        fprintf(stderr,
            "Error while lexing on line: %lu\n",
            lexer->current_lineno);
        cc_exit();
    }

    else if (!isascii(c))
    {
        fprintf(stderr,
            "Error on line : %lu\n\tNon-ascii character in string\n",
            lexer->current_lineno);
        cc_exit();
    }

    ptr[i] = '\x00';

    return ptr;
}