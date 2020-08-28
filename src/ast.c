#include "ast.h"

ast_expr primary(token **tokens) {
    if (has(tokens, TOKEN_NAME)) {
        ast_literal *lit = (ast_literal *)alloc(sizeof(ast_literal));
        lit->value = eat(tokens);
        return (ast_expr){EXPR_LITERAL, lit};
    }
    if (has(tokens, TOKEN_NUMBER) || has(tokens, TOKEN_STRING)) {
        ast_literal *lit = (ast_literal *)alloc(sizeof(ast_literal));
        lit->value = eat(tokens);
        return (ast_expr){EXPR_LITERAL, lit};
    }
    return (ast_expr){0};
}

ast_expr unary(token **tokens) {
    if (has_op(tokens, TOKEN_OP, OP_NOT)) {
        ast_unary *expr = (ast_unary *)alloc(sizeof(ast_unary));
        expr->op = expect_op(tokens, 1, OP_NOT)->op;
        expr->right = parse_expr(tokens);
        return (ast_expr){EXPR_UNARY, expr};
    }
    return primary(tokens);
}

ast_expr mul(token **tokens) {
    ast_expr left = unary(tokens);
    if (has_op(tokens, TOKEN_OP, OP_MUL) || has_op(tokens, TOKEN_OP, OP_DIV) || has_op(tokens, TOKEN_OP, OP_MOD)) {
        ast_binary *binary = (ast_binary *)alloc(sizeof(ast_binary));
        binary->op = eat(tokens)->op;
        binary->left = left;
        binary->right = unary(tokens);
        left = (ast_expr){EXPR_BINARY, binary};
    }
    return left;
}

ast_expr add(token **tokens) {
    ast_expr left = mul(tokens);
    if (has_op(tokens, TOKEN_OP, OP_ADD) || has_op(tokens, TOKEN_OP, OP_SUB)) {
        ast_binary *binary = (ast_binary *)alloc(sizeof(ast_binary));
        binary->op = eat(tokens)->op;
        binary->left = left;
        binary->right = mul(tokens);
        left = (ast_expr){EXPR_BINARY, binary};
    }
    return left;
}

ast_expr comp(token **tokens) {
    ast_expr left = add(tokens);
    if (has_op(tokens, TOKEN_OPOP, OP_AND) || has_op(tokens, TOKEN_OPOP, OP_OR) || 
            has_op(tokens, TOKEN_OP, OP_LT) || has_op(tokens, TOKEN_OP, OP_GT) || 
            has_op(tokens, TOKEN_OPASSIGN, OP_LT) || has_op(tokens, TOKEN_OP, OP_GT)) {
        ast_binary *binary = (ast_binary *)alloc(sizeof(ast_binary));
        binary->op = eat(tokens)->op;
        binary->left = left;
        binary->right = add(tokens);
        left = (ast_expr){EXPR_BINARY, binary};
    }
    return left;
}

ast_expr eq(token **tokens) {
    ast_expr left = comp(tokens);
    if (has_op(tokens, TOKEN_OPOP, OP_EQ) || has_op(tokens, TOKEN_OPASSIGN, OP_NOT)) {
        ast_binary *binary = (ast_binary *)alloc(sizeof(ast_binary));
        binary->op = eat(tokens)->op;
        binary->left = left;
        binary->right = comp(tokens);
        left = (ast_expr){EXPR_BINARY, binary};
    }
    return left;
}

ast_expr parse_expr(token **tokens) {
    return eq(tokens);
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
    if (has(tokens, TOKEN_TERM)) {
        eat(tokens);
    }
    return (ast_stmt){0, AST_DECL_FUNC, func};
}

ast_stmt parse_var_decl(token **tokens, token *name) {
    eat(tokens); /* eat colon */
    ast_var *var = (ast_var *)alloc(sizeof(ast_var));
    var->name = name;
    var->type = expect(tokens, TOKEN_NAME);
    var->expr = (ast_expr){0};
    if (has(tokens, TOKEN_ASSIGN)) {
        eat(tokens);
        var->expr = parse_expr(tokens);
    } else {
        var->expr = (ast_expr){0};
    }
    expect(tokens, TOKEN_TERM);
    return (ast_stmt){0, AST_DECL_VAR, var};
}

ast_stmt parse_assign(token **tokens, token *name) {
    eat(tokens); /* eat assign */
    ast_var *assign = (ast_var *)alloc(sizeof(ast_var));
    assign->name = name;
    assign->type = 0;
    assign->expr = parse_expr(tokens);
    expect(tokens, TOKEN_TERM);
    return (ast_stmt){0, STMT_ASSIGN, assign};
}

ast_stmt parse_define(token **tokens, token *name) {
    eat(tokens); /* eat define */
    ast_var *assign = (ast_var *)alloc(sizeof(ast_var));
    assign->name = name;
    assign->type = 0;
    assign->expr = parse_expr(tokens);
    expect(tokens, TOKEN_TERM);
    return (ast_stmt){0, STMT_ASSIGN, assign};
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
    if (has(tokens, TOKEN_EOF)) {
        return (ast_stmt){0};
    }
    if (has(tokens, TOKEN_NAME)) {
        token *name = eat(tokens);
        token *next = peek(tokens);
        switch (next->type) {
            case TOKEN_DECL:
                eat(tokens); /* eat decl */
                return parse_decl(tokens, name, 0);
            case TOKEN_NAME:
                return parse_root_keyword(tokens, name);
            case TOKEN_COLON:
                return parse_var_decl(tokens, name);
            case TOKEN_ASSIGN:
                return parse_assign(tokens, name);
            case TOKEN_DEFINE:
                return parse_define(tokens, name);
            default:
                parse_error("unsupported token: %s\n", token_type_text[next->type]);
        }
    }
    parse_error("parse_stmt: cannot parse start token %s", token_type_text[(*tokens)->type]);
    return (ast_stmt){0}; 
}