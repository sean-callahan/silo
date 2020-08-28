#ifndef AST_H
#define AST_H

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "lex.h"

typedef enum ast_type ast_type;

typedef struct ast_var       ast_var;
typedef struct ast_param     ast_param;
typedef struct ast_func      ast_func;
typedef struct ast_silo      ast_silo;
typedef struct ast_import    ast_import;
typedef struct ast_stmt      ast_stmt;
typedef struct ast_expr      ast_expr;
typedef struct ast_arg       ast_arg;
typedef struct ast_unary     ast_unary;
typedef struct ast_binary    ast_binary;
typedef struct ast_func_call ast_func_call;
typedef struct ast_literal   ast_literal;

enum ast_type {
    AST_UNKNOWN,
    AST_DECL_VAR,        /* variable declaration */
    AST_DECL_FUNC,       /* function declaration */
    AST_DECL_STRUCT,     /* struct declaration */
    AST_DECL_CONST,      /* constant declaration */
    STMT_ASSIGN,         /* variable assignment */
    STMT_IF,             /* if statement */
    STMT_SILO,           /* silo statement */
    STMT_IMPORT,         /* import statement */
    STMT_RETURN,         /* return statement */

    EXPR_LITERAL,
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_FUNC_CALL,
    EXPR_MEMBER,
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

struct ast_silo {
    token *path;
};

struct ast_import {
    /* TODO: imports should support directories, so we need a delimiter. maybe '.'? */
    token *path;
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
    ast_expr expr;
};

struct ast_unary {
    operator op;
    ast_expr right;
};

struct ast_binary {
    operator op;
    ast_expr left, right;
};

struct ast_literal {
    token *value;
};

struct ast_func_call {
    token *name;
    ast_arg *args;
};

struct ast_var {
    token *name;
    token *type;
    ast_expr expr;
};

ast_expr parse_expr(token **tokens);
ast_stmt parse_stmt(token **tokens);

static inline void parse_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(1);
}

static inline token *eat(token **tokens) {
    if (!tokens) {
        return 0;
    }
    token *t = *tokens;
    *tokens = t->right;
    return t;
}

static inline token *peek(token **tokens) {
    assert(tokens);
    return *tokens;
}

static inline token *expect(token **tokens, const token_type type) {
    token *t = eat(tokens);
    if (!t) {
        parse_error("unexpected EOF\n");
    }
    if (t->type != type) {
        parse_error("expected type %s; got %s\n", token_type_text[type], token_type_text[t->type]);
    }
    return t;
}

static inline token *expect_op(token **tokens, const int count, ...) {
    if (!tokens) {
        return 0;
    }
    token *t = *tokens;
    int i;
    va_list args;
    va_start(args, count);
    for (i = 0; i < count; i++) {
        if (va_arg(args, operator) == t->op) {
            *tokens = t->right;
            return t;
        }
    }
    va_end(args);
    return 0;
}

static inline int has(token **tokens, const token_type type) {
    if (!tokens) {
        return 0;
    }
    return (*tokens)->type == type;
}

static inline int has_op(token **tokens, const token_type type, const operator op) {
    if (!tokens) {
        return 0;
    }
    return ((*tokens)->type == type) && ((*tokens)->op == op);
}


#endif