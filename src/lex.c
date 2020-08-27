#include <fcntl.h>
#include <ctype.h>

#include "lex.h"

/* TODO: we can probably move this back to the header file. */
const char * const token_type_text[] = {
    "TOKEN_EOF",
    "TOKEN_TERM",
    "TOKEN_NAME",
    "TOKEN_NUMBER",
    "TOKEN_STRING",
    "TOKEN_DECL",
    "TOKEN_ASSIGN",
    "TOKEN_DEFINE",
    "TOKEN_OP",
    "TOKEN_OPOP",
    "TOKEN_OPASSIGN",
    "TOKEN_LPAREN",
    "TOKEN_LBRACK",
    "TOKEN_LBRACE",
    "TOKEN_RPAREN",
    "TOKEN_RBRACK",
    "TOKEN_RBRACE",
    "TOKEN_COLON",
    "TOKEN_COMMA",
    "TOKEN_DOT",
};

int source_open(source_file *src, const char *path) {
    src->fd = open(path, 0);
    if (!src->fd) {
        return -1;
    }
    src->line = src->col = 1;
    return 0;
}

token *next_token(lexer *lex) {
    int c, nc;
fetch:
    c = get(lex->src);
lex:
    if (c == -1) {
        return emit(lex, TOKEN_EOF);
    }
    if (isspace(c)) {
        if (c == '\n') {
            token *t = emit(lex, TOKEN_TERM);
            lex->src->line++;
            lex->src->col = 0;
            return t;
        }
        goto fetch;
    }
    if (isalpha(c)) {
        goto name;
    }
    if (isdigit(c)) {
        goto digit;
    }
    switch (c) {
        case '"':
            c = get(lex->src);
            goto string;
        case '(':
            return emit(lex, TOKEN_LPAREN);
        case '[':
            return emit(lex, TOKEN_LBRACK);
        case '{':
            return emit(lex, TOKEN_LBRACE);
        case ')':
            return emit(lex, TOKEN_RPAREN);
        case ']':
            return emit(lex, TOKEN_RBRACK);
        case '}':
            return emit(lex, TOKEN_RBRACE);
        case ',':
            return emit(lex, TOKEN_COMMA);
        case '.':
            return emit(lex, TOKEN_DOT);
        case ':':
            nc = get(lex->src);
            if (nc == ':') {
                return emit(lex, TOKEN_DECL);
            }
            unget(lex->src, nc);
            return emit(lex, TOKEN_COLON);
        case '=':
            nc = get(lex->src);
            if (nc == '=') {
                return emit_op(lex, TOKEN_OPOP, OP_EQ);
            }
            unget(lex->src, nc);
            return emit_op(lex, TOKEN_OP, OP_EQ);
        case '!':
            nc = get(lex->src);
            if (nc == '=') {
                return emit_op(lex, TOKEN_OPASSIGN, OP_NOT);
            }
            unget(lex->src, nc);
            return emit_op(lex, TOKEN_OP, OP_NOT);
        case '<':
            nc = get(lex->src);
            if (nc == '<') {
                return emit_op(lex, TOKEN_OPOP, OP_LT);
            }
            unget(lex->src, nc);
            return emit_op(lex, TOKEN_OP, OP_LT);
        case '>':
            nc = get(lex->src);
            if (nc == '>') {
                return emit_op(lex, TOKEN_OPOP, OP_GT);
            }
            unget(lex->src, nc);
            return emit_op(lex, TOKEN_OP, OP_GT);
        case '+':
            nc = get(lex->src);
            if (nc == '+') {
                return emit_op(lex, TOKEN_OPOP, OP_ADD);
            } else if (nc == '=') {
                return emit_op(lex, TOKEN_OPASSIGN, OP_ADD);
            }
            unget(lex->src, nc);
            return emit_op(lex, TOKEN_OP, OP_ADD);
        case '-':
            nc = get(lex->src);
            if (nc == '-') {
                return emit_op(lex, TOKEN_OPOP, OP_SUB);
            } else if (nc == '=') {
                return emit_op(lex, TOKEN_OPASSIGN, OP_SUB);
            }
            unget(lex->src, nc);
            return emit_op(lex, TOKEN_OP, OP_SUB);
        case '*':
            nc = get(lex->src);
            if (nc == '=') {
                return emit_op(lex, TOKEN_OPASSIGN, OP_MUL);
            }
            unget(lex->src, nc);
            return emit_op(lex, TOKEN_OP, OP_MUL);
        case '/':
            nc = get(lex->src);
            if (nc == '=') {
                return emit_op(lex, TOKEN_OPASSIGN, OP_DIV);
            }
            unget(lex->src, nc);
            return emit_op(lex, TOKEN_OP, OP_DIV);
        case '%':
            nc = get(lex->src);
            if (nc == '=') {
                return emit_op(lex, TOKEN_OPASSIGN, OP_MOD);
            }
            unget(lex->src, nc);
            return emit_op(lex, TOKEN_OP, OP_MOD);
        case '&':
            nc = get(lex->src);
            if (nc == '&') {
                return emit_op(lex, TOKEN_OPOP, OP_AND);
            }
            unget(lex->src, nc);
            return emit_op(lex, TOKEN_OP, OP_AND);
        case '|':
            nc = get(lex->src);
            if (nc == '|') {
                return emit_op(lex, TOKEN_OPOP, OP_OR);
            }
            unget(lex->src, nc);
            return emit_op(lex, TOKEN_OP, OP_OR);
        default:
            return 0;
    }
    return 0;
name:
    text_start(lex, c);
    while (isalnum(c) || c == '_') {
        c = get(lex->src);
        text_push(lex, c);
    }
    unget(lex->src, c);
    text_pop(lex);
    return emit(lex, TOKEN_NAME);
digit:
    text_start(lex, c);
    while (isdigit(c)) {
        c = get(lex->src);
    }
    unget(lex->src, c);
    return emit(lex, TOKEN_NUMBER);
string:
    text_start(lex, c);
    while (c != '"') {
        c = get(lex->src);
        text_push(lex, c);
    }
    text_pop(lex);
    return emit(lex, TOKEN_STRING);
}