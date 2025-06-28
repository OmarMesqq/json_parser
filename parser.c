#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "grammar.h"

static int parseObject(TokenStream* ts, int* pos);


/**
 * @returns 0 if JSON is valid, -1 otherwise
 */
int ParseJson(TokenStream* ts) {
    if (!ts || !ts->tokenList) {
        fprintf(stderr, "parseJson: empty token stream or token list.refusing to parse.\n");
        return -1;
    }

    /**
     *  Empty file is not valid JSON
     * A JSON value MUST be an object, array, number, or string, or one of
     * the following three literal names:
     * 'false', 'true', 'null'
     */
    if (ts->size == 0) {
        return -1;
    }

    int pos, currentParseStatus;
    for (pos = 0; pos < ts->size; pos++) {
       TOKEN currentToken =  ts->tokenList[pos];
       switch (currentToken) {
        case BEGIN_OBJECT:
            currentParseStatus = parseObject(ts, &pos);
            if (!currentParseStatus) {
                return currentParseStatus;
            }
            break;
        default:
            fprintf(stderr, "parseJson: unexpected token: %d (decimal), %c (char)\n", currentToken, currentToken);
            break;
       }
    }

    free_token_stream(ts);
    return 0;
}

/**
 * object = begin-object [ member *( value-separator member ) ] end-object
 * 
 * member = string name-separator value
 */
static int parseObject(TokenStream* ts, int* pos) {
    (*pos)++;   // parse current BEGIN_OBJECT

    int foundEnd = 0;
    while (ts->tokenList[*pos] != END_OBJECT) {
        (*pos)++;
        if (ts->tokenList[*pos] == END_OBJECT) {
            foundEnd = 1;
            break;
        }
    }

    if (foundEnd) return 0;
    return -1;
}