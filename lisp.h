
#ifndef _LISP_H_
#define _LISP_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef uint64_t u64;

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

inline static bool is_cons(Expr exp)
{
    return expr_type(exp) == TYPE_CONS;
}

inline static bool is_string(Expr exp)
{
    return expr_type(exp) == TYPE_STRING;
}

#endif /* _LISP_H_ */

#ifdef LISP_IMPLEMENTATION

#ifndef _LISP_C_
#define _LISP_C_

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

#endif /* _LISP_C_ */

#endif
