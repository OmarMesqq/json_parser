#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "grammar.h"

static int parseObject(TokenStream* ts, int* pos);
static int parseArray(TokenStream* ts, int* pos);
static int parseNumber(TokenStream* ts, int* pos);
static int parseString(TokenStream* ts, int* pos);
static int parseLiteral(TokenStream* ts, int* pos);


/**
 * @returns 0 if JSON is valid, -1 otherwise
 */
int ParseJson(TokenStream* ts) {
    if (!ts || !ts->tokenList) {
        fprintf(stderr, "parseJson: empty token stream or token list.refusing to parse.\n");
        return -1;
    }

    /**
     *  Empty file is not valid JSON. From RFC:
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
            if (currentParseStatus == -1) {
                return currentParseStatus;
            }
            break;
        case STRING_START_END:
            currentParseStatus = parseString(ts, &pos);
            if (currentParseStatus == -1) {
                return currentParseStatus;
            }
            break;
        default:
            fprintf(stderr, "ParseJson: unexpected token: %d (decimal), %c (char)\n", currentToken, currentToken);
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

    int foundObjectEnd = 0;
    int parseStatus = 0;
    TOKEN currentToken = ts->tokenList[*pos];
    do {
        currentToken = ts->tokenList[*pos];
        (*pos)++;
        
        switch (currentToken) {
            case END_OBJECT:
                foundObjectEnd = 1;
                break;
            case STRING_START_END:
                parseStatus = parseString(ts, pos);
                if (parseStatus == -1) {
                    return parseStatus;
                }
                break;
            default:
                fprintf(stderr, "parseObject: unexpected token: %d (decimal), %c (char)\n", currentToken, currentToken);
                return -1;
        }
    }
    while (currentToken != END_OBJECT && !foundObjectEnd);

    if (foundObjectEnd) {
        return 0;
    };

    fprintf(stderr, "Expected a } for closing object.");
    return -1;
}

static int parseString(TokenStream* ts, int* pos) {
    (*pos)++;   // parse current STRING_START_END

    int foundMatchingEndDoubleQuote = 0;
    TOKEN currentToken = ts->tokenList[*pos];
    do {
        currentToken = ts->tokenList[*pos];
        (*pos)++;
        
        switch (currentToken) {
            case STRING_START_END:
                foundMatchingEndDoubleQuote = 1;
                (*pos)++;   // mark current double quote as parsed
                break;
            default:
                fprintf(stderr, "parseString: unexpected token: %d (decimal), %c (char)\n", currentToken, currentToken);
                return -1;
        }
    }
    while (currentToken != STRING_START_END && !foundMatchingEndDoubleQuote);

    if (foundMatchingEndDoubleQuote) {
        return 0;
    };

    fprintf(stderr, "Expected closing \" for closing string.");
    return -1;
}