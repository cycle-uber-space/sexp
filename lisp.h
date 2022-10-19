
#ifndef _LISP_H_
#define _LISP_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define FAIL(...) fail(__FILE__, __LINE__, __VA_ARGS__)

#define ASSERT(x) do { if (!(x)) { fail(__FILE__, __LINE__, "assertion failed: %s\n", #x); } } while (0)

typedef uint64_t u64;

void fail(char const * file, int line, char const * fmt, ...);

typedef u64 Expr;

Expr make_expr(u64 type, u64 data);
u64 expr_type(Expr exp);
u64 expr_data(Expr exp);

enum
{
    TYPE_NIL = 0,
    TYPE_SYMBOL,
    TYPE_CONS,
    TYPE_STRING,
};

enum
{
    DATA_NIL = 0,
};

#define nil 0

inline static bool is_nil(Expr exp)
{
    return exp == nil;
}

inline static bool is_symbol(Expr exp)
{
    return expr_type(exp) == TYPE_SYMBOL;
}

Expr make_symbol(char const * name);
char const * symbol_name(Expr exp);

inline static bool is_cons(Expr exp)
{
    return expr_type(exp) == TYPE_CONS;
}

inline static bool is_string(Expr exp)
{
    return expr_type(exp) == TYPE_STRING;
}

Expr intern(char const * name);

#endif /* _LISP_H_ */

#ifdef LISP_IMPLEMENTATION

#ifndef _LISP_C_
#define _LISP_C_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

void fail(char const * file, int line, char const * fmt, ...)
{
    FILE * out = stderr;
    fprintf(out, "%s:%d: [FAIL] ", file, line);
    va_list args;
    va_start(args, fmt);
    vfprintf(out, fmt, args);
    va_end(args);
    exit(1);
}

Expr make_expr(u64 type, u64 data)
{
    return (data << 8) | (type & 255);
}

u64 expr_type(Expr exp)
{
    return exp & 255;
}

u64 expr_data(Expr exp)
{
    return exp >> 8;
}

#define MAX_SYMBOLS 100

static char * g_symbol_names[MAX_SYMBOLS];
static u64 g_symbol_count = 0;

Expr make_symbol(char const * name)
{
    u64 index;
    for (index = 0; index < g_symbol_count; index++)
    {
        if (!strcmp(g_symbol_names[index], name))
        {
            break;
        }
    }

    if (index == g_symbol_count)
    {
        ASSERT(g_symbol_count < MAX_SYMBOLS);
        size_t len = strlen(name);
        char * copy = (char *) malloc(len + 1);
        memcpy(copy, name, len + 1);
        g_symbol_names[g_symbol_count++] = copy;
    }

    return make_expr(TYPE_SYMBOL, index);
}

char const * symbol_name(Expr exp)
{
    ASSERT(is_symbol(exp));
    u64 const index = expr_data(exp);
    ASSERT(index < g_symbol_count);
    return g_symbol_names[index];
}

Expr intern(char const * name)
{
    if (!strcmp("nil", name))
    {
        return nil;
    }
    else
    {
        return make_symbol(name);
    }
}

#endif /* _LISP_C_ */

#endif
