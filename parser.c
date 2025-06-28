#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "grammar.h"

static int delegate_parser(TokenStream* ts, int* pos);
static int parse_object(TokenStream* ts, int* pos);

int parseJson(TokenStream* ts) {
    if (!ts || !ts->tokenList) {
        fprintf(stderr, "parseJson: empty token stream or token list.refusing to parse.\n");
        return -1;
    }

    int pos, parseStatus;
    for (pos = 0; pos < ts->size; pos++) {
        parseStatus = delegate_parser(ts, &pos);

        if (parseStatus == -1) {
            return -1;
        }
    }

    free_token_stream(ts);
    return 0;
}

static int delegate_parser(TokenStream* ts, int* pos) {
    GRAMMAR token = ts->tokenList[*pos];
    switch (token) {
        case BEGIN_OBJECT:
            return parse_object(ts, pos);
            break;
        default:
            fprintf(stderr, "delegate_parser: Unexpected token: %c (char), %d (decimal)\n", token, token);
            break;
    }
    return -1;
}

static int parse_object(TokenStream* ts, int* pos) {
    if (!ts || !ts->tokenList) {
        printf("parse_object: empty token stream or token list.refusing to parse.\n");
        return -1;
    }

    while (*pos < ts->size) {
        GRAMMAR token = ts->tokenList[*pos];
        if (token == END_OBJECT) {
            return 1;
        }
        (*pos)++;
        delegate_parser(ts, pos);
    }
    printf("parse_object failed. expected END_OBJECT");
    return -1;
}
