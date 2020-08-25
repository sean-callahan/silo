#include "ast.h"

ast_expr unary(token *tokens) {
    ast_unary *expr = (ast_unary *)alloc(sizeof(ast_unary));
    expr->op = expect_op(tokens, 1, OP_NOT)->op;
    expr->right = parse_expr(tokens);
    return (ast_expr){EXPR_UNARY, expr};
}

ast_expr parse_expr(token *tokens) {
    return (ast_expr){0};
}

ast_stmt *parse_body(token *tokens) {
    return 0;
}

ast_stmt parse_func(token *tokens, token *name, token *parent) {
    ast_func *func = (ast_func *)alloc(sizeof(ast_func));
    func->parent = parent;
    func->name = name;
    while (!has(tokens, 1, TOKEN_RPAREN)) {
        ast_param *param = (ast_param *)alloc(sizeof(ast_param));
        param->name = expect(tokens, 1, TOKEN_NAME);
        param->type = expect(tokens, 1, TOKEN_NAME);
        if (func->params) {
            func->params->next = param;
        } else {
            func->params = param;
        }
    }
    eat(tokens); /* eat right paren */
    if (has(tokens, 1, TOKEN_LPAREN)) {
        /* multi returns */
        eat(tokens); /* eat left paren */
        while (!has(tokens, 1, TOKEN_RPAREN)) {
            ast_param *ret = (ast_param *)alloc(sizeof(ast_param));
            ret->name = expect(tokens, 1, TOKEN_NAME);
            ret->type = expect(tokens, 1, TOKEN_NAME);
            if (func->returns) {
                func->returns->next = ret;
            } else {
                func->returns = ret;
            }
        }
        eat(tokens); /* eat right paren */
    } else if (has(tokens, 1, TOKEN_NAME)) {
        /* single return */
        ast_param *ret = (ast_param *)alloc(sizeof(ast_param));
        ret->type = eat(tokens);
        func->returns = ret;
    }
    func->body = parse_body(tokens);
    return (ast_stmt){0, AST_DECL_FUNC, func};
}

ast_stmt parse_decl(token *tokens, token *name, token *parent) {
    token *symb = eat(tokens);
    switch (symb->type) {
        case TOKEN_LPAREN: /* function */
            return parse_func(tokens, name, parent);
            break;
        case TOKEN_LBRACE: /* struct */
            return (ast_stmt){0};
            break;
        default:
            break;
    }
    parse_error("unexpected declaration type %d", symb->type);
    return (ast_stmt){0};
}

ast_stmt parse_stmt(token *tokens) {
    if (has(tokens, 1, TOKEN_NAME)) {
        token *name = eat(tokens);
        if (has(tokens, 1, TOKEN_DECL)) {
            eat(tokens);
            return parse_decl(tokens, name, 0); 
        }
    }
    return (ast_stmt){0};
}