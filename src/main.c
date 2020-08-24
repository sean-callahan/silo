#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "lex.h"

void token_print(token t) {
    printf("%d (%d:%d) %zu ", t.type, t.line, t.col, t.text_size);
    if (t.text_size) {
        int i;
        for (i = 0; i < t.text_size; i++)
            printf("%c", t.text[i]);
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    int c;

    int l = 0;

    while ((c = getopt(argc, argv, "l")) != -1) {
        switch (c) {
            case 'l':
                l = 1;
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

    return 0;
}