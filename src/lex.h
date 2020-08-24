#ifndef LEX_H
#define LEX_H

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "mem.h"

#define SOURCE_BUFFER_SIZE 16384 /* 16KB */
#define TEXT_BUFFER_SIZE   1024  /* largest name or literal size */

typedef enum token_type token_type;
enum token_type {
    TOKEN_EOF,         /* end of file */
    TOKEN_TERM,        /* line terminator: '\n' */
    
    /* names and literals */
    TOKEN_NAME,        /* symbol or keyword */
    TOKEN_NUMBER,      /* number literal: 42, 3.14, -5, 0x2a */
    TOKEN_STRING,      /* string literal: "foo" */
    
    /* operators and operations */
    TOKEN_DECL,        /* declaration: '::' */
    TOKEN_ASSIGN,      /* assignment: = */
    TOKEN_DEFINE,      /* definition: := */
    TOKEN_OP,          /* single operator: +, -, *, /, %, !, &, etc. */
    TOKEN_OPOP,        /* double operator: ++, ==, &&, etc. */
    TOKEN_OPASSIGN,    /* operator equals: +=, !=, %=, etc. */

    /* delimiters */
    TOKEN_LPAREN,      /* ( */
    TOKEN_LBRACK,      /* [ */
    TOKEN_LBRACE,      /* { */
    TOKEN_RPAREN,      /* ) */
    TOKEN_RBRACK,      /* ] */
    TOKEN_RBRACE,      /* } */
    TOKEN_COLON,       /* : */
    TOKEN_COMMA,       /* , */
    TOKEN_DOT,         /* . */
};

typedef enum operator operator;
enum operator {
    OP_NONE,   /* no-op */

    OP_NOT,    /* ! */

    /* comparison */
    OP_EQ,     /* = */
    OP_LT,     /* < */
    OP_GT,     /* > */

    /* addition */
    OP_ADD,    /* + */
    OP_SUB,    /* - */
    OP_OR,     /* | */

    /* multiplication */
    OP_MUL,    /* * */
    OP_DIV,    /* / */
    OP_MOD,    /* % */
    OP_AND,    /* & */
};

typedef struct token token;
struct token {
    /* tokens to the left and right of this one. */
    token *left, *right;

    token_type type;

    /* operator if the token is an operator type. */
    operator op;

    /* line number and column offset. */
    uint32_t line, col;

    /* number of bytes in text. */
    size_t text_size;

    /* text value of the token. */
    char *text;
};

typedef struct source_file source_file;
struct source_file {
    /* relative path to the source file. */
    char *path;

    /* file descriptor. */
    int fd; 

    /* total bytes read from the file. */
    off_t read;

    /* saved character to use when peeking. */
    char peek;

    /* line number and column offset. */
    uint32_t line, col;

    /* start and end tokens. */
    token *start, *end;

    /* current buffer read and write pointers. */
    char *br, *bw;

    /* file buffer. */
    char b[SOURCE_BUFFER_SIZE];
};

typedef struct lexer lexer;
struct lexer {
    source_file *src;

    /* line number and column offset to the start of the text. */
    uint32_t tline, tcol;

    /* text write pointer. */
    char *tw;

    /* text buffer. */
    char text[TEXT_BUFFER_SIZE];
};

int source_open(source_file *src, const char *path);

token *next_token(lexer *lex);

/* Pushes a character to the text buffer. */
static inline void text_push(lexer *lex, char c) {
    assert(lex->tw - lex->text < TEXT_BUFFER_SIZE);
    *lex->tw++ = c;
}

/* Removes the last character from the text buffer. */
static inline char text_pop(lexer *lex) {
    if (lex->tw > lex->text) {
        return *--lex->tw;
    }
    return 0;
}

/* Marks the beginning of a text token. */
static inline void text_start(lexer *lex, char c) {
    lex->tline = lex->src->line;
    lex->tcol = lex->src->col;
    lex->tw = lex->text;
    text_push(lex, c);
}

/* Emits a token that has an operator. */
static inline token *emit_op(lexer *lex, token_type type, operator op) {
    token *t = (token *)alloc(sizeof(token));
    t->type = type;
    t->op = op;
    t->line = lex->src->line;
    t->col = lex->src->col;
    if (lex->tw > lex->text) {
        t->line = lex->tline;
        t->col = lex->tcol;
        t->text_size = lex->tw - lex->text;
        t->text = (char *)alloc(t->text_size);
        memcpy(t->text, lex->text, t->text_size);
        lex->tw = lex->text;
    }
    lex->tline = lex->tcol = 0;
    t->left = lex->src->end;
    if (lex->src->end) {
        lex->src->end->right = t;
    }
    if (!lex->src->start) {
        lex->src->start = t;
    }
    lex->src->end = t;
    return t;
}

/* Emits a token. */
static inline token *emit(lexer *lex, token_type type) {
    return emit_op(lex, type, OP_NONE);
}

/* Refills the source buffer. */
static inline int source_fill(source_file *src) {
    if (!src->fd) {
        return -1; /* file is closed. */
    }
    ssize_t n = read(src->fd, src->b, SOURCE_BUFFER_SIZE);
    if (n <= 0) {
        close(src->fd);
        return -1;
    }
    src->read += n;
    src->bw = src->b + n;
    src->br = src->b;
    return *src->br++ & 0xff;
}

static inline int get(source_file *src) {
    if (src->peek) {
        char c = src->peek;
        src->peek = 0;
        return c;
    }
    if (src->br == src->bw) {
        return source_fill(src);
    }
    src->col++;
    return *src->br++ & 0xff;
}

static inline void unget(source_file *src, char c) {
    src->peek = c;
    /*if (c == '\n') {
        src->line--;
    }*/
}

#endif