#include <stdio.h>
#include "lexer.h"

int main() {
    FILE* f1 = fopen("tests/step1/valid.json", "r");
    FILE* f2 = fopen("tests/step1/invalid.json", "r");
    if (!f1 || !f2) {
        printf("failed to open json files!\n");
        return -1;
    }

    TokenStream* tl = tokenize(f1);

    free_token_stream(tl);
    fclose(f1);
    fclose(f2);
    return 0;
}
