#include <stdio.h>
#include "lexer.h"
#include "parser.h"

static void print_result(int res, char* filename);
int main() {
    FILE* f1 = fopen("tests/step1/valid.json", "r");
    FILE* f2 = fopen("tests/step1/invalid.json", "r");
    if (!f1 || !f2) {
        printf("failed to open json files!\n");
        return -1;
    }


    TokenStream* ts1= tokenize(f1);
    int j = 0;
    while (j < ts1->size) {
        printf("tokenList[%d]: %c\n", j, ts1->tokenList[j]);
        j++;
    }

    int resf1 = parseJson(ts1);
    print_result(resf1, "tests/step1/valid.json");

    free_token_stream(ts1);
    fclose(f1);
    fclose(f2);
    return 0;
}

static void print_result(int res, char* filename) {
    if (!filename) {
        return;
    }

    if (res == 1) {
        printf("Valid JSON: %s.\n", filename);
    } else {
        printf("Invalid JSON: %s.\n", filename);
    }
}