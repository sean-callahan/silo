#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "ast.h"
#include "lex.h"
#include "obj.h"

void token_print(token t) {
    printf("%s (%d:%d) ", token_type_text[t.type], t.line, t.col);
    if (t.text) {
        printf("%s", t.text);
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    int c;

    int p = 0;
    int l = 0;

    while ((c = getopt(argc, argv, "lp")) != -1) {
        switch (c) {
            case 'l':
                l = 1;
                break;
            case 'p':
                p = 1;
                break;
            default:
                abort();
        }
    }

    if (optind == argc) {
        printf("missing path argument\n");
        return 1;
    }

    const char *path = argv[optind];
    
    source_file src = {0};
    int err = source_open(&src, path);
    if (err == -1) {
        printf("could not opne\n");
        return 1;
    }

    if (l) {
        lexer lex = {0};
        lex.src = &src;
        token *t;
        do {
            t = next_token(&lex);
            token_print(*t);
        } while (t->type != TOKEN_EOF);
    }

    if (p) {
        lexer lex = {0};
        lex.src = &src;
        token *tokens;
        token *t;
        do {
            if (t) {
                t->right = next_token(&lex);
                t = t->right;
            } else {
                t = next_token(&lex);
                tokens = t;
            }
        } while (t->type != TOKEN_EOF);

        ast_stmt stmt;
        do {
            while (has(&tokens, TOKEN_TERM)) {
                eat(&tokens);
            }
            stmt = parse_stmt(&tokens);
            printf("stmt: %d\n", stmt.type);
            if (stmt.type == AST_DECL_FUNC) {
                ast_stmt *s = ((ast_func *)stmt.d)->body;
                while (s) {
                    printf("\tbody: %d\n", s->type);
                    s = s->next;
                }
            }
        } while (stmt.d);
    }

    return 0;
}