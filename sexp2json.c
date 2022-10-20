
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

char const * expr_type_name(Expr exp)
{
    switch (expr_type(exp))
    {
    case TYPE_NIL:
        return "nil";
    case TYPE_SYMBOL:
        return "symbol";
    case TYPE_KEYWORD:
        return "keyword";
    case TYPE_PAIR:
        return "pair";
    default:
        return "#:<unknown>";
    }
}

static void render_expr(Expr exp);

int g_indent = 0;
int g_col = 0;
int g_line = 0;

static void emit_char(char ch)
{
    if (ch == '\n')
    {
        putc('\n', stdout);
        g_col = 0;
        g_line++;
    }
    else
    {
        if (g_col == 0)
        {
            for (int i = 0; i < g_indent; i++)
            {
                putc(' ', stdout);
            }
        }
        putc(ch, stdout);
        g_col++;
    }
}

static void emit_str(char const * str)
{
    ASSERT_DEBUG(str);
    for (char const * p = str; *p; p++)
    {
        emit_char(*p);
    }
}

static void indent()
{
    g_indent += 2;
}

static void dedent()
{
    g_indent -= 2;
}

static void render_nil(Expr exp)
{
    if (is_nil(exp))
    {
        printf("null");
    }
    else
    {
        FAIL("cannot render expression %" PRIx64 "\n", exp);
    }
}

static void render_symbol(Expr exp)
{
    ASSERT_DEBUG(is_symbol(exp));
    emit_str(symbol_name(exp));
}

static void render_keyword(Expr exp)
{
    ASSERT_DEBUG(is_keyword(exp));
    emit_char(':');
    emit_str(keyword_name(exp));
}

static void render_pair(Expr exp)
{
    ASSERT_DEBUG(is_pair(exp));
    Expr head = car(exp);
    if (head == intern("object"))
    {
        Expr rest = cdr(exp);
        if (rest)
        {
            emit_str("{\n");
            indent();
            while (rest)
            {
                Expr key = car(rest);
                Expr val = cadr(rest);
                if (is_keyword(key))
                {
                    emit_str("\"");
                    emit_str(keyword_name(key));
                    emit_str("\": ");
                }
                else
                {
                    FAIL("cannot render object key of type %s\n", expr_type_name(key));
                }
                render_expr(val);
                rest = cddr(rest);
                if (rest)
                {
                    emit_char(',');
                }
                emit_char('\n');
            }
            dedent();
            emit_str("}\n");
        }
        else
        {
            emit_str("{}");
        }
    }
    else if (head == intern("array"))
    {
        emit_char('[');
        render_expr(head);
        for (Expr iter = cdr(exp); iter; iter = cdr(iter))
        {
            emit_str(", ");
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
        emit_char(']');
    }
    else
    {
        FAIL("cannot render pair %016" PRIx64 "\n", exp);
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
    case TYPE_KEYWORD:
        render_keyword(exp);
        break;
    case TYPE_PAIR:
        render_pair(exp);
        break;
    default:
        FAIL("cannot render expression of type %s\n", expr_type_name(exp));
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
    emit_char('\n');
}

int main(int argc, char ** argv)
{
    sexp2json();
    return 0;
}

#define LISP_IMPLEMENTATION
#include "lisp.h"
