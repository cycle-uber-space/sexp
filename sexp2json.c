
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
    ASSERT(peek() == '(');
    (void) next();

    Expr head = nil, tail = nil;
    while (true)
    {
        skip_whitespace();
        int ch = peek();
        if (ch == -1)
        {
            FAIL("unexpected end of stream\n");
            return nil;
        }
        else if (ch == ')')
        {
            break;
        }
        else
        {
            Expr next = cons(read_expr(), nil);
            if (head)
            {
                rplacd(tail, next);
                tail = next;
            }
            else
            {
                head = tail = next;
            }
        }
    }
    return head;
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
    //fprintf(stderr, "%s\n", buffer);

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

static void render_expr(Expr exp);

static void render_nil(Expr exp)
{
    if (is_nil(exp))
    {
        printf("null");
    }
    else
    {
        FAIL("cannot render expression of type %" PRIx64 "\n", expr_type(exp));
    }
}

static void render_symbol(Expr exp)
{
    ASSERT_DEBUG(is_symbol(exp));
    printf("%s", symbol_name(exp));
}

static void render_pair(Expr exp)
{
    ASSERT_DEBUG(is_pair(exp));
    Expr head = car(exp);
    if (head == intern("object"))
    {
        // TODO
        printf("{...}");
    }
    else if (head == intern("array"))
    {
        printf("[");
        render_expr(head);
        for (Expr iter = cdr(exp); iter; iter = cdr(iter))
        {
            printf(", ");
            if (is_pair(iter))
            {
                render_expr(car(iter));
            }
            else
            {
                FAIL("cannot map dotted list to json\n");
                break;
            }
        }
        printf("]");
    }
}

static void render_expr(Expr exp)
{
    switch (expr_type(exp))
    {
    case TYPE_NIL:
        render_nil(exp);
        break;
    case TYPE_SYMBOL:
        render_symbol(exp);
        break;
    case TYPE_PAIR:
        render_pair(exp);
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
