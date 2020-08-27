#include "ast.h"

ast_expr unary(token **tokens) {
    ast_unary *expr = (ast_unary *)alloc(sizeof(ast_unary));
    expr->op = expect_op(tokens, 1, OP_NOT)->op;
    expr->right = parse_expr(tokens);
    return (ast_expr){EXPR_UNARY, expr};
}

ast_expr parse_expr(token **tokens) {
    return (ast_expr){0};
}

ast_stmt *parse_body(token **tokens) {
    expect(tokens, TOKEN_LBRACE);
    ast_stmt *stmts = 0;
    while (has(tokens, TOKEN_TERM)) {
        eat(tokens);
    }
    while (!has(tokens, TOKEN_RBRACE)) {
        while (has(tokens, TOKEN_TERM)) {
            eat(tokens);
        }
        ast_stmt s = parse_stmt(tokens);
        /* this wrap is a hack to get the linked list working */
        ast_stmt *stmt = (ast_stmt *)alloc(sizeof(ast_stmt));
        stmt->type = s.type;
        stmt->d = s.d;
        if (stmts) {
            stmts->next = stmt;
        } else {
            stmts = stmt;
        }
    }
    eat(tokens); /* eat right brace */
    return stmts;
}

ast_stmt parse_func(token **tokens, token *name, token *parent) {
    ast_func *func = (ast_func *)alloc(sizeof(ast_func));
    func->parent = parent;
    func->name = name;
    while (!has(tokens, TOKEN_RPAREN)) {
        ast_param *param = (ast_param *)alloc(sizeof(ast_param));
        param->name = expect(tokens, TOKEN_NAME);
        param->type = expect(tokens, TOKEN_NAME);
        if (func->params) {
            func->params->next = param;
        } else {
            func->params = param;
        }
    }
    eat(tokens); /* eat right paren */
    if (has(tokens, TOKEN_LPAREN)) {
        /* multi returns */
        eat(tokens); /* eat left paren */
        while (!has(tokens, TOKEN_RPAREN)) {
            ast_param *ret = (ast_param *)alloc(sizeof(ast_param));
            ret->name = expect(tokens, TOKEN_NAME);
            ret->type = expect(tokens, TOKEN_NAME);
            if (func->returns) {
                func->returns->next = ret;
            } else {
                func->returns = ret;
            }
        }
        eat(tokens); /* eat right paren */
    } else if (has(tokens, TOKEN_NAME)) {
        /* single return */
        ast_param *ret = (ast_param *)alloc(sizeof(ast_param));
        ret->type = eat(tokens);
        func->returns = ret;
    }
    func->body = parse_body(tokens);
    expect(tokens, TOKEN_TERM);
    return (ast_stmt){0, AST_DECL_FUNC, func};
}

ast_stmt parse_decl(token **tokens, token *name, token *parent) {
    token *symb = eat(tokens);
    switch (symb->type) {
        case TOKEN_LPAREN: /* function */
            return parse_func(tokens, name, parent);
        case TOKEN_LBRACE: /* struct */
            return (ast_stmt){0};
        default:
            break;
    }
    parse_error("unexpected declaration type %d\n", symb->type);
    return (ast_stmt){0};
}

ast_stmt parse_import(token **tokens) {
    ast_import *im = (ast_import *)alloc(sizeof(ast_import));
    im->path = expect(tokens, TOKEN_NAME);
    expect(tokens, TOKEN_TERM);
    return (ast_stmt){0, STMT_IMPORT, im};
}

ast_stmt parse_silo(token **tokens) {
    ast_silo *silo = (ast_silo *)alloc(sizeof(ast_silo));
    silo->path = expect(tokens, TOKEN_NAME);
    expect(tokens, TOKEN_TERM);
    return (ast_stmt){0, STMT_SILO, silo};
}

ast_stmt parse_root_keyword(token **tokens, token *keyword) {
    if (strcmp(keyword->text, "import") == 0) {
        return parse_import(tokens);
    } else if (strcmp(keyword->text, "silo") == 0) {
        return parse_silo(tokens);
    } else {
        parse_error("unexpected root keyword %s\n", keyword->text);
        return (ast_stmt){0};
    }
} 

ast_stmt parse_stmt(token **tokens) {
    if (has(tokens, TOKEN_NAME)) {
        token *name = eat(tokens);
        token *next = peek(tokens);
        switch (next->type) {
            case TOKEN_DECL:
            printf("here\n");
                eat(tokens); /* eat decl */
                return parse_decl(tokens, name, 0);
            case TOKEN_NAME:
            printf("test: %s\n", next->text);
                return parse_root_keyword(tokens, name);
            default:
                parse_error("unsupported token: %s\n", token_type_text[next->type]);
        }
    }
    parse_error("parse_stmt: cannot parse start token %s", token_type_text[(*tokens)->type]);
    return (ast_stmt){0}; 
}