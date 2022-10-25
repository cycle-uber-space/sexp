
#include "lisp.h"

#include <inttypes.h>

static bool is_whitespace(int ch)
{
    return ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r';
}

static int peek_ch = -1;

static int peek()
{
    if (peek_ch == -1)
    {
        FILE * in = stdin;
        peek_ch = getc(in);
        ungetc(peek_ch, in);
    }
    //fprintf(stderr, "peek: '%c' (%d)\n", peek_ch, peek_ch);
    return peek_ch;
}

static void advance()
{
    (void) getc(stdin);
    //fprintf(stderr, "advance: '%c' (%d)\n", ch, ch);
    peek_ch = -1;
}

static bool at_whitespace()
{
    return is_whitespace(peek());
}

static void skip_whitespace()
{
    while (at_whitespace())
    {
        (void) advance();
    }
}

static bool at_eof()
{
    return peek() == -1;
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
    case TYPE_STRING:
        return "string";
    default:
        return "#:<unknown>";
    }
}

static Expr read_value();
static Expr read_string();
static Expr read_symbol();

static Expr read_object()
{
    //fprintf(stderr, "%s()\n", __FUNCTION__);
    ASSERT(peek() == '{');
    (void) advance();

    Expr head = nil, tail = nil;
    while (true)
    {
        skip_whitespace();
        int ch = peek();
        if (ch == '}')
        {
            (void) advance();
            break;
        }
        else if (ch == -1)
        {
            FAIL("unexpected end of stream in %s()\n", __FUNCTION__);
            return nil;
        }
        else
        {
            Expr key = read_string();
            Expr colon = read_symbol();
            ASSERT(colon == intern(":"));
            Expr val = read_value();

            Expr next = cons(make_keyword(string_value(key)), cons(val, nil));
            if (head)
            {
                rplacd(tail, next);
                tail = next;
            }
            else
            {
                head = tail = next;
            }
            skip_whitespace();
            bool have_comma = false;
            if (peek() == ',')
            {
                have_comma = true;
                advance();
            }
            if (have_comma && peek() == '}')
            {
                FAIL("unexpected '}' after ',' in %s()\n", __FUNCTION__);
            }
        }
    }
    return cons(intern("object"), head);
}

static Expr read_array()
{
    //fprintf(stderr, "%s()\n", __FUNCTION__);
    ASSERT(peek() == '[');
    (void) advance();

    Expr head = nil, tail = nil;
    while (true)
    {
        skip_whitespace();
        int ch = peek();
        if (ch == ']')
        {
            (void) advance();
            break;
        }
        else if (ch == -1)
        {
            FAIL("unexpected end of stream in %s()\n", __FUNCTION__);
            return nil;
        }
        else
        {
            Expr val = read_value();
            Expr next = cons(val, nil);
            if (head)
            {
                rplacd(tail, next);
                tail = next;
            }
            else
            {
                head = tail = next;
            }

            skip_whitespace();
            bool have_comma = false;
            if (peek() == ',')
            {
                have_comma = true;
                advance();
            }
            if (have_comma && peek() == ']')
            {
                FAIL("unexpected ']' after ',' in %s()\n", __FUNCTION__);
            }
        }
    }
    return cons(intern("array"), head);
}

static char * _store(size_t size, char * buffer, char ch, char * pout)
{
    //fprintf(stderr, "STORE: %c (%d)\n", ch, ch);
    ASSERT((size_t) (pout - buffer) < size);
    *pout++ = ch;
    return pout;
}

static Expr read_string()
{
    //fprintf(stderr, "%s()\n", __FUNCTION__);
    ASSERT(peek() == '"');
    (void) advance();

    char buffer[4096];
    char * pout = buffer;
    //fprintf(stderr, "%c", peek());
    while (true)
    {
        int ch = peek();
        if (ch == -1)
        {
            FAIL("unexpected end of stream in %s()\n", __FUNCTION__);
            return nil;
        }
        else if (ch == '"')
        {
            break;
        }
        else if (ch == '\\')
        {
            advance();
            ch = peek();
            switch (ch)
            {
            case '\\':
                pout = _store(4096, buffer, '\\', pout);
                advance();
                break;
            case '"':
                pout = _store(4096, buffer, '"', pout);
                advance();
                break;
            default:
                FAIL("illegal escape sequence %c\n", ch);
                return nil;
            }
        }
        else
        {
            //fprintf(stderr, "%c", peek());
            pout = _store(4096, buffer, ch, pout);
            advance();
        }
    }

    ASSERT(peek() == '"');
    (void) advance();

    pout = _store(4096, buffer, '\0', pout);
    //fprintf(stderr, "%s\n", buffer);

    return make_string(buffer);
}

static bool is_symbol_char(int ch)
{
    if (ch == -1)
    {
        return false;
    }

    if (is_whitespace(ch))
    {
        return false;
    }

    if (ch == '(' || ch == ')')
    {
        return false;
    }

    if (ch == '{' || ch == '}')
    {
        return false;
    }

    if (ch == '[' || ch == ']')
    {
        return false;
    }

    if (ch == ',')
    {
        return false;
    }

    if (ch == '"')
    {
        return false;
    }

    return true;
}

static Expr read_symbol()
{
    //fprintf(stderr, "%s()\n", __FUNCTION__);
    char buffer[4096];
    char * pout = buffer;
    //fprintf(stderr, "%c", peek());
    while (is_symbol_char(peek()))
    {
        int ch = peek();
        advance();
        //fprintf(stderr, "%c\n", ch);
        pout = _store(4096, buffer, ch, pout);
    }

    pout = _store(4096, buffer, '\0', pout);

    //fprintf(stderr, "BUFFER: %s\n", buffer);

    return intern(buffer);
}

static Expr read_value()
{
    //fprintf(stderr, "%s()\n", __FUNCTION__);
    Expr ret = nil;
    skip_whitespace();
    switch (peek())
    {
    case '{':
        ret = read_object();
        break;
    case '[':
        ret = read_array();
        break;
    case '"':
        ret = read_string();
        break;
    default:
        ret = read_symbol();
        break;
    }
    //fprintf(stderr, "READ => %016" PRIx64 " (%s)\n", ret, expr_type_name(ret));
    return ret;
}

static bool maybe_read_value(Expr * pexp)
{
    skip_whitespace();
    if (at_eof())
    {
        return false;
    }
    *pexp = read_value();
    return true;
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
            emit_str("(object");
            indent();
            while (rest)
            {
                emit_char('\n');

                Expr key = car(rest);
                Expr val = cadr(rest);
                render_expr(key);
                emit_str(" ");
                render_expr(val);
                rest = cddr(rest);
            }
            emit_str(")");
            dedent();
        }
        else
        {
            emit_str("(object)");
        }
    }
    else if (head == intern("array"))
    {
        Expr rest = cdr(exp);
        if (rest)
        {
            emit_str("(array\n");
            indent();
            for (Expr iter = rest; iter; iter = cdr(iter))
            {
                if (is_pair(iter))
                {
                    render_expr(car(iter));
                }
                else
                {
                    FAIL("cannot map dotted list to json\n");
                    break;
                }
                if (cdr(iter))
                {
                    emit_str("\n");
                }
            }
            emit_str(")");
            dedent();
        }
        else
        {
            emit_str("(array)");
        }
    }
    else
    {
        FAIL("cannot render pair %016" PRIx64 "\n", exp);
    }
}

static void render_string(Expr exp)
{
    ASSERT_DEBUG(is_string(exp));
    char const * str = string_value(exp);
    emit_char('"');
    for (char const * p = str; *p; p++)
    {
        char const ch = *p;
        switch (ch)
        {
        case '\\':
            emit_char('\\');
            emit_char('\\');
            break;
        case '"':
            emit_char('\\');
            emit_char('"');
            break;
        default:
            emit_char(ch);
            break;
        }
    }
    emit_char('"');
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
    case TYPE_STRING:
        render_string(exp);
        break;
    default:
        FAIL("cannot render expression of type %s\n", expr_type_name(exp));
        break;
    }
}

static void json2sexp()
{
    Expr exp;
    while (maybe_read_value(&exp))
    {
        render_expr(exp);
        emit_char('\n');
    }
}

int main(int argc, char ** argv)
{
    json2sexp();
    return 0;
}

#define LISP_IMPLEMENTATION
#include "lisp.h"
