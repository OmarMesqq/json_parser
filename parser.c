#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "grammar.h"

static int delegate_parser(TokenStream* ts);
static int parse_object(TokenStream* ts);

static int POSITION;

int parseJson(TokenStream* ts) {
    if (!ts || !ts->tokenList) {
        printf("parseJson: empty token stream or token list.refusing to parse.\n");
        return -1;
    }
    int ret;
    for (POSITION = 0; POSITION < ts->size; POSITION++) {
        ret = delegate_parser(ts);
        if (ret == -1) {
            printf("parseJson: invalid JSON.\n");
            return -1;
        }
    }
    printf("parseJson: valid JSON.\n");
    return 0;
}

static int delegate_parser(TokenStream* ts) {
    GRAMMAR token = ts->tokenList[POSITION];
    switch (token) {
        case BEGIN_OBJECT:
            return parse_object(ts);
            break;
        default:
            printf("unexpected token: %c\n", token);
            break;
    }
    return -1;
}
static int parse_object(TokenStream* ts) {
    if (!ts || !ts->tokenList) {
        printf("parse_object: empty token stream or token list.refusing to parse.\n");
        return -1;
    }
    while (POSITION < ts->size) {
        GRAMMAR token = ts->tokenList[POSITION];
        if (token == END_OBJECT) {
            return 1;
        }
        POSITION++;
        delegate_parser(ts);
    }
    printf("parse_object failed. expected END_OBJECT");
    return -1;
}
