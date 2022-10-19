
#include "lisp.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <inttypes.h>

#define FAIL(...) fail(__FILE__, __LINE__, __VA_ARGS__)

static void fail(char const * file, int line, char const * fmt, ...)
{
    FILE * out = stderr;
    fprintf(out, "%s:%d: [FAIL] ", file, line);
    va_list args;
    va_start(args, fmt);
    vfprintf(out, fmt, args);
    va_end(args);
    exit(1);
}

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

static void next()
{
    (void) getc(stdin);
}

static bool at_whitespace()
{
    return is_whitespace(peek());
}

static void skip_whitespace()
{
    while (at_whitespace())
    {
        next();
    }
}

static bool at_eof()
{
    return peek() == -1;
}

static Expr read_expr();

static Expr read_list()
{
    next();
    return nil;
}

static Expr read_symbol()
{
    next();
    return nil;
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
        /* fall-through */
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
