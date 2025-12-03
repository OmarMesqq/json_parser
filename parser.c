#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

static void FreeTokenStream(TokenStream* ts);

/**
 * @returns 0 if JSON is valid, -1 otherwise
 */
int Parse(TokenStream* ts) {
  if (!ts || !ts->tokenArray) {
    fprintf(stderr, "parseJson: empty token stream or token list.refusing to parse.\n");
    return -1;
  }

  // /**
  //  * Empty file is not valid JSON as per the RFC
  //  * A JSON value MUST be an object, array, number, or string, or one of
  //  * the following three literal names:
  //  * 'false', 'true', 'null'
  //  */
  // if (ts->size == 0) {
  //   return -1;
  // }

  // size_t pos = 0;
  // size_t currentParseStatus = 0;
  // for (pos = 0; pos < ts->size; pos++) {
  //   TOKEN currentToken = ts->tokenArray[pos];
  //   switch (currentToken) {
  //     default: {
  //       fprintf(stderr, "ParseJson: unexpected token: %d (dec), %02x (hex), %c (char)\n", currentToken, currentToken, currentToken);
  //       break;
  //     }
  //   }
  // }

  FreeTokenStream(ts);
  return 0;
}

static void FreeTokenStream(TokenStream* ts) {
  if (!ts) {
    return;
  }
  free(ts->tokenArray);
  free(ts);
}
