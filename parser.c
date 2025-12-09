#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

static void FreeTokenStream(TokenStream* ts);
static inline char is_simple_value(TOKEN tk);
static void eat(TOKEN expectedToken, TOKEN* ta);
static void parse_value(TOKEN* tokenArray);
static char parse_object(TOKEN* ta);
static char parse_array(TOKEN* ta);

static size_t cursor = 0;

int Parse(TokenStream* ts) {
  if (!ts || !ts->tokenArray || ts->size == 0) {
    fprintf(stderr, "Parse: no tokens in JSON file!\n");
    return -1;
  }

  // edge cases
  TOKEN* ta = ts->tokenArray;
  if (is_simple_value(ta[cursor]) && ts->size == 1) {
    return 0;
  }

  parse_value(ta);

  if (cursor != ts->size) {
    fprintf(stderr, "Parse: only a single root value allowed in JSON!\n");
    // fprintf(stderr, "cursor: %ld\n", cursor);
    // fprintf(stderr, "ts->size: %ld\n", ts->size);
    cursor = 0;
    return -1;
  }
  cursor = 0;
  return 0;
}

/**
 * Checks if the `TOKEN` `tk` is a JSON value that can be represented in a single token i.e:
 *
 * string || number || 'true || 'false' || 'null'
 */
static inline char is_simple_value(TOKEN tk) {
  return (tk == STRING) || (tk == NUMBER) || (tk == LITERAL_TRUE) || (tk == LITERAL_FALSE) || (tk == LITERAL_NULL);
}

static void FreeTokenStream(TokenStream* ts) {
  if (!ts) {
    return;
  }
  free(ts->tokenArray);
  free(ts);
}

static void eat(TOKEN expectedToken, TOKEN* ta) {
  if (ta[cursor] == expectedToken) {
    // printf("eaten!\n");
    cursor++;  // advance cursor
  } else {
    fprintf(stderr, "eat: expected %c, got %c\n", expectedToken, ta[cursor]);
    exit(-1);
  }
}

/**
 * A JSON value is the production:
 *
 * `Value -> Object ∣ Array ∣ String ∣ Number | Boolean | Null`
 *
 * where `Value`, `Object`, and `Array` are nonterminals.
 */
static void parse_value(TOKEN* tokenArray) {
  TOKEN currentToken = tokenArray[cursor];

  if (is_simple_value(currentToken)) {
    eat(currentToken, tokenArray);
  } else if (currentToken == BEGIN_OBJECT) {
    parse_object(tokenArray);
  } else if (currentToken == BEGIN_ARRAY) {
    parse_array(tokenArray);
  } else {
    fprintf(stderr, "parse_value: unexpected token: %c\n", currentToken);
    exit(-1);
  }
}

/**
 * Parses a JSON object:
 * `BEGIN_OBJECT *(member *(VALUE_SEPARATOR member)) END_OBJECT`
 *
 * where its possible member(s) - aka name/value pair(s) -  are defined as:
 * `STRING NAME_SEPARATOR VALUE`
 */
static char parse_object(TOKEN* ta) {
  eat(BEGIN_OBJECT, ta);

  TOKEN currentToken = ta[cursor];

  while (currentToken != END_OBJECT) {
    eat(STRING, ta);          // key
    eat(NAME_SEPARATOR, ta);  // :
    parse_value(ta);          // JSON value

    // object is over
    if (ta[cursor] == END_OBJECT) {
      break;
    }

    // object has more entries
    if (ta[cursor] == VALUE_SEPARATOR) {
      eat(VALUE_SEPARATOR, ta);

      if (ta[cursor] == END_OBJECT) {
        fprintf(stderr, "trailing comma in object!\n");
        return -1;
      }
    }
    currentToken = ta[cursor];
  }
  eat(END_OBJECT, ta);
  return 0;
}

/**
 * An array in JSON is of type:
 * `BEGIN_ARRAY *(VALUE *(VALUE_SEPARATOR VALUE)) END_ARRAY`
 */
static char parse_array(TOKEN* ta) {
  eat(BEGIN_ARRAY, ta);

  TOKEN currentToken = ta[cursor];
  while (currentToken != END_ARRAY) {
    parse_value(ta);

    // array is over
    if (ta[cursor] == END_ARRAY) {
      break;
    }
    // array has more entries
    if (ta[cursor] == VALUE_SEPARATOR) {
      eat(VALUE_SEPARATOR, ta);

      if (ta[cursor] == END_ARRAY) {
        fprintf(stderr, "trailing comma in array!\n");
        return -1;
      }
    }
    currentToken = ta[cursor];
  }
  eat(END_ARRAY, ta);
  return 0;
}
