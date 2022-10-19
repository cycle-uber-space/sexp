
#include "lisp.h"


#include <inttypes.h>

static bool is_whitespace(int ch)
{
    return ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r';
}

static int peek()
{
    FILE * in = stdin;
    int ch = getc(in);
    ungetc(ch, in);
    return ch;
}

static int next()
{
    return getc(stdin);
}

static bool at_whitespace()
{
    return is_whitespace(peek());
}

static void skip_whitespace()
{
    while (at_whitespace())
    {
        (void) next();
    }
}

static bool at_eof()
{
    return peek() == -1;
}

static Expr read_expr();

static Expr read_list()
{
    (void) next();
    return nil;
}

static Expr read_symbol()
{
    char buffer[4096];
    char * pout = buffer;
    //fprintf(stderr, "%c", peek());
    while (true)
    {
        int ch = peek();
        if (ch == -1)
        {
            break;
        }
        else if (is_whitespace(ch))
        {
            break;
        }
        else if (ch == '(' || ch == ')')
        {
            break;
        }
        else
        {
            //fprintf(stderr, "%c", peek());
            ASSERT(pout - buffer < 4096);
            *pout++ = next();
        }
    }

    ASSERT(pout - buffer < 4096);
    *pout++ = 0;
    fprintf(stderr, "%s\n", buffer);

    return intern(buffer);
}

static Expr read_expr()
{
    skip_whitespace();
    switch (peek())
    {
    case '(':
        return read_list();
    default:
        return read_symbol();
    }
    return nil;
}

static bool maybe_read_expr(Expr * pexp)
{
    skip_whitespace();
    if (at_eof())
    {
        return false;
    }
    *pexp = read_expr();
    return true;
}

static void render_expr(Expr exp)
{
    switch (expr_type(exp))
    {
    case TYPE_NIL:
        if (is_nil(exp))
        {
            printf("null");
            break;
        }
        FAIL("cannot render expression of type %" PRIx64 "\n", expr_type(exp));
        break;
    case TYPE_SYMBOL:
        printf("%s", symbol_name(exp));
        break;
    default:
        FAIL("cannot render expression of type %" PRIx64 "\n", expr_type(exp));
        break;
    }
}

static void sexp2json()
{
    Expr exp;
    if (!maybe_read_expr(&exp))
    {
        /* map empty stream => empty stream */
        return;
    }

    render_expr(exp);
    printf("\n");
}

int main(int argc, char ** argv)
{
    sexp2json();
    return 0;
}

#define LISP_IMPLEMENTATION
#include "lisp.h"
