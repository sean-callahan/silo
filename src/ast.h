#ifndef AST_H
#define AST_H

#include <stdarg.h>

#include "lex.h"

#define expect(tokens, ...) _expect(tokens, COUNT(__VA_ARGS__), __VA_ARGS__)
#define has(tokens, ...) _has(tokens, COUNT(__VA_ARGS__), __VA_ARGS__)

typedef enum ast_type ast_type;

typedef struct ast_var       ast_var;
typedef struct ast_param     ast_param;
typedef struct ast_func      ast_func;
typedef struct ast_stmt      ast_stmt;
typedef struct ast_expr      ast_expr;
typedef struct ast_arg       ast_arg;
typedef struct ast_unary     ast_unary;
typedef struct ast_binary    ast_binary;
typedef struct ast_func_call ast_func_call;

enum ast_type {
    AST_DECL_VAR,        /* variable declaration */
    AST_DECL_FUNC,       /* function declaration */
    AST_DECL_STRUCT,     /* struct declaration */
    AST_DECL_CONST,      /* constant declaration */
    STMT_ASSIGN,         /* variable assignment */
    STMT_IF,             /* if statement */
    STMT_RETURN,         /* return statement */

    EXPR_NAME,
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_FUNC_CALL,
    EXPR_MEMBER,
};

struct ast_var {
    token *name;
    token *type;
};

struct ast_param {
    ast_param *next;
    token *name;
    token *type;
};

struct ast_func {
    token *parent;
    token *name;
    ast_param *params;
    ast_param *returns;
    ast_stmt *body;
};

struct ast_stmt {
    ast_stmt *next;
    ast_type type;
    void *d;
};

struct ast_expr {
    ast_type type;
    void *d;
};

struct ast_arg {
    ast_arg *next;
    ast_expr *expr;
};

struct ast_unary {
    operator op;
    ast_expr *left;
};

struct ast_binary {
    operator op;
    ast_expr *left, *right;
};

struct ast_func_call {
    token *name;
    ast_arg *args;
};

ast_expr *parse_expr(token *tokens);
ast_stmt *parse_stmt(token *tokens);

static inline token *_expect(token* tokens, const int count, ...) {
    if (!tokens) {
        return 0;
    }
    token *t = tokens;
    int i;
    va_list args;
    va_start(args, count);
    for (i = 0; i < count; i++) {
        if (va_arg(args, token_type) == t->type) {
            tokens = t->right;
            return t;
        }
    }
    va_end(args);
    return 0;
}

static inline int _vhas(token *tokens, const int count, va_list args) {
    if (!tokens) {
        return 0;
    }
    token t = *tokens;
    int i;
    for (i = 0; i < count; i++) {
        if (va_arg(args, token_type) == t.type) {
            return 1;
        }
    }
    return 0;
}

static inline int _has(token *tokens, const int count, ...) {
    int b;
    va_list args;
    va_start(args, count);
    b = _vhas(tokens, count, args);
    va_end(args);
    return 0;
}

static inline token *eat(token *tokens) {
    if (!tokens) {
        return 0;
    }
    token *t = tokens;
    tokens = t->right;
    return t;
}

#endif