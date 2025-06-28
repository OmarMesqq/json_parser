#include <stdio.h>
#include "lexer.h"
#include "parser.h"
#include "json_parser.h"
#include "color_codes.h"


int ValidateJson(FILE* jsonFilePtr) {
    TokenStream* ts= tokenize(jsonFilePtr);
    if (!ts) {
        fprintf(stderr, RED "ValidateJson: failed to tokenize file!\n" RESET_COLOR);
        return -1;
    }

    int res = parseJson(ts);
    return res;
}