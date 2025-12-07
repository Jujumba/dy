#include <stdio.h>

#include "src/array.h"
#include "src/string.h"
#include "src/token.h"

typedef struct Numbers {
    ArrayHeader header;
    int        *buffer;
} Numbers;

int main() {
    char *token_types[] = {"<none>", "keyword", "punctuation", "whitespace", "ident",
                           "string", "number",  "operator",    "newline",    "comment"};

    String    input = S("for a in range(0, 10e1 ).\n");
    Tokenizer tokenizer = {.input = input};
    Token     t = {0};
    printf("Input string: `%.*s`\n", input.len, input.buffer);
    while ((t = TokenizerNext(&tokenizer)).type) {
        printf("Token = %-10.*s type = %s\n", t.s.len, t.s.buffer, TTypeToString[t.type]);
    }
    printf("Tokenizer pos: %u, input len: %u\n", tokenizer.pos, tokenizer.input.len);
}
