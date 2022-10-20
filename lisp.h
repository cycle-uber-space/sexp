
#ifndef _LISP_H_
#define _LISP_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifndef DEBUG
#define DEBUG 1
#endif

#define FAIL(...) fail(__FILE__, __LINE__, __VA_ARGS__)

#define ASSERT(x) do { if (!(x)) { fail(__FILE__, __LINE__, "assertion failed: %s\n", #x); } } while (0)

#if DEBUG
#define ASSERT_DEBUG(x) ASSERT(x)
#else
#define ASSERT_DEBUG(x) ((void) x)
#endif

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
    TYPE_KEYWORD,
    TYPE_PAIR,
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

inline static bool is_keyword(Expr exp)
{
    return expr_type(exp) == TYPE_KEYWORD;
}

Expr make_keyword(char const * name);
char const * keyword_name(Expr exp);

inline static bool is_pair(Expr exp)
{
    return expr_type(exp) == TYPE_PAIR;
}

Expr make_pair(Expr a, Expr b);
Expr pair_first(Expr exp);
Expr pair_second(Expr exp);

void pair_set_first(Expr exp, Expr val);
void pair_set_second(Expr exp, Expr val);

inline static bool is_string(Expr exp)
{
    return expr_type(exp) == TYPE_STRING;
}

Expr intern(char const * name);

Expr cons(Expr a, Expr b);
Expr car(Expr exp);
Expr cdr(Expr exp);

void rplaca(Expr exp, Expr val);
void rplacd(Expr exp, Expr val);

inline static Expr cadr(Expr exp)
{
    return car(cdr(exp));
}

inline static Expr cddr(Expr exp)
{
    return cdr(cdr(exp));
}

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

#define MAX_KEYWORDS 100

static char * g_keyword_names[MAX_KEYWORDS];
static u64 g_keyword_count = 0;

Expr make_keyword(char const * name)
{
    u64 index;
    for (index = 0; index < g_keyword_count; index++)
    {
        if (!strcmp(g_keyword_names[index], name))
        {
            break;
        }
    }

    if (index == g_keyword_count)
    {
        ASSERT(g_keyword_count < MAX_KEYWORDS);
        size_t len = strlen(name);
        char * copy = (char *) malloc(len + 1);
        memcpy(copy, name, len + 1);
        g_keyword_names[g_keyword_count++] = copy;
    }

    return make_expr(TYPE_KEYWORD, index);
}

char const * keyword_name(Expr exp)
{
    ASSERT(is_keyword(exp));
    u64 const index = expr_data(exp);
    ASSERT(index < g_keyword_count);
    return g_keyword_names[index];
}

typedef struct
{
    Expr first, second;
} Pair;

#define MAX_PAIRS 1000

Pair g_pairs[MAX_PAIRS];
u64 g_num_pairs = 0;

Expr make_pair(Expr a, Expr b)
{
    ASSERT(g_num_pairs < MAX_PAIRS);
    u64 const index = g_num_pairs++;
    g_pairs[index].first = a;
    g_pairs[index].second = b;
    return make_expr(TYPE_PAIR, index);
}

static u64 _pair_index(Expr exp)
{
    ASSERT(is_pair(exp));
    u64 const index = expr_data(exp);
    ASSERT(index < g_num_pairs);
    return index;
}

Expr pair_first(Expr exp)
{
    return g_pairs[_pair_index(exp)].first;
}

Expr pair_second(Expr exp)
{
    return g_pairs[_pair_index(exp)].second;
}

void pair_set_first(Expr exp, Expr val)
{
    g_pairs[_pair_index(exp)].first = val;
}

void pair_set_second(Expr exp, Expr val)
{
    g_pairs[_pair_index(exp)].second = val;
}

Expr intern(char const * name)
{
    if (!strcmp("nil", name))
    {
        return nil;
    }
    else if (name[0] == ':' && name[1] != '\0')
    {
        return make_keyword(name + 1);
    }
    else
    {
        return make_symbol(name);
    }
}

Expr cons(Expr a, Expr b)
{
    return make_pair(a, b);
}

Expr car(Expr exp)
{
    if (is_nil(exp))
    {
        return exp;
    }
    else
    {
        return pair_first(exp);
    }
}

Expr cdr(Expr exp)
{
    if (is_nil(exp))
    {
        return exp;
    }
    else
    {
        return pair_second(exp);
    }
}

void rplaca(Expr exp, Expr val)
{
    pair_set_first(exp, val);
}

void rplacd(Expr exp, Expr val)
{
    pair_set_second(exp, val);
}

#endif /* _LISP_C_ */

#endif
