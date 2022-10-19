
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

static bool maybe_read_expr(Expr * pexp)
{
    *pexp = nil;
    return false;
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
