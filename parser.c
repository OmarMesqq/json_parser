#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

static void FreeTokenStream(TokenStream* ts);
static char is_simple_value(TOKEN tk);

/**
 * @returns 0 if JSON is valid, -1 otherwise
 */
int Parse(TokenStream* ts) {
  if (!ts || !ts->tokenArray || ts->size == 0) {
    fprintf(stderr, "Parse: No tokens in JSON file!\n");
    return -1;
  }

  size_t pos = 0;
  TOKEN currentToken = 0;

  currentToken = ts->tokenArray[pos];

  // Edge case: standalone simple values in JSON
  const char isStandaloneSimpleValue = (is_simple_value(currentToken) && ts->size == 1);
  if (isStandaloneSimpleValue) return 0;

  // Edge case: standalone composite values in JSON
  if (ts->size == 2) {
    if (currentToken == BEGIN_OBJECT && (ts->tokenArray[pos + 1] == END_OBJECT)) {
      // JSON is `{}`
      return 0;
    }
    if (currentToken == BEGIN_ARRAY && (ts->tokenArray[pos + 1] == END_ARRAY)) {
      // JSON is `[]`
      return 0;
    }
  }

  if (ts->size == 1) {
    if (currentToken == NAME_SEPARATOR || currentToken == VALUE_SEPARATOR) {
      fprintf(stderr, "Parse: expected JSON object, array, or literal!\n");
      return -1;
    }
  }

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

static char is_simple_value(TOKEN tk) {
  return (tk == STRING) || (tk == NUMBER) || (tk == LITERAL_TRUE) || (tk == LITERAL_FALSE) || (tk == LITERAL_NULL);
}

static void FreeTokenStream(TokenStream* ts) {
  if (!ts) {
    return;
  }
  free(ts->tokenArray);
  free(ts);
}
