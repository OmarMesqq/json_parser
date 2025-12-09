#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

static void FreeTokenStream(TokenStream* ts);
static inline char is_simple_value(TOKEN tk);
static char eat(TOKEN expectedToken, TOKEN* ta);
static char parse_value(TOKEN* tokenArray);
static char parse_object(TOKEN* ta);
static char parse_array(TOKEN* ta);

static size_t cursor = 0;

int Parse(TokenStream* ts) {
  if (!ts || !ts->tokenArray || ts->size == 0) {
    fprintf(stderr, "Parse: no tokens in JSON file!\n");
    return -1;
  }

  TOKEN* ta = ts->tokenArray;
  // "A JSON payload should be an object or array, not a string."
  if (is_simple_value(ta[cursor]) && ts->size == 1) {
    return -1;
  }

  char res = 0;

  res = parse_value(ta);

  if (cursor != ts->size) {
    fprintf(stderr, "Parse: only a single root value allowed in JSON!\n");
    // fprintf(stderr, "cursor: %ld\n", cursor);
    // fprintf(stderr, "ts->size: %ld\n", ts->size);
    res = -1;
  }
  cursor = 0;
  return res;
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

static char eat(TOKEN expectedToken, TOKEN* ta) {
  if (ta[cursor] == expectedToken) {
    // printf("eaten!\n");
    cursor++;  // advance cursor
    return 0;
  } else {
    fprintf(stderr, "eat: expected %c, got %c\n", expectedToken, ta[cursor]);
    return -1;
  }
}

/**
 * A JSON value is the production:
 *
 * `Value -> Object ∣ Array ∣ String ∣ Number | Boolean | Null`
 *
 * where `Value`, `Object`, and `Array` are nonterminals.
 */
static char parse_value(TOKEN* tokenArray) {
  TOKEN currentToken = tokenArray[cursor];
  char res = 0;

  if (is_simple_value(currentToken)) {
    res = eat(currentToken, tokenArray);
  } else if (currentToken == BEGIN_OBJECT) {
    res = parse_object(tokenArray);
  } else if (currentToken == BEGIN_ARRAY) {
    res = parse_array(tokenArray);
  } else {
    fprintf(stderr, "parse_value: unexpected token: %c\n", currentToken);
    res = -1;
  }
  return res;
}

/**
 * Parses a JSON object:
 * `BEGIN_OBJECT *(member *(VALUE_SEPARATOR member)) END_OBJECT`
 *
 * where its possible member(s) - aka name/value pair(s) -  are defined as:
 * `STRING NAME_SEPARATOR VALUE`
 */
static char parse_object(TOKEN* ta) {
  char res = 0;
  res = eat(BEGIN_OBJECT, ta);
  if (res == -1) return -1;

  TOKEN currentToken = ta[cursor];

  while (currentToken != END_OBJECT) {
    res = eat(STRING, ta);  // key
    if (res == -1) return -1;
    res = eat(NAME_SEPARATOR, ta);  // :
    if (res == -1) return -1;
    res = parse_value(ta);  // JSON value
    if (res == -1) return -1;

    // object is over
    if (ta[cursor] == END_OBJECT) {
      break;
    }

    // object has more entries
    if (ta[cursor] == VALUE_SEPARATOR) {
      res = eat(VALUE_SEPARATOR, ta);
      if (res == -1) return -1;

      if (ta[cursor] == END_OBJECT) {
        fprintf(stderr, "trailing comma in object!\n");
        return -1;
      }
    }
    currentToken = ta[cursor];
  }
  res = eat(END_OBJECT, ta);
  if (res == -1) return -1;
  return 0;
}

/**
 * An array in JSON is of type:
 * `BEGIN_ARRAY *(VALUE *(VALUE_SEPARATOR VALUE)) END_ARRAY`
 */
static char parse_array(TOKEN* ta) {
  char res = 0;
  res = eat(BEGIN_ARRAY, ta);
  if (res == -1) return -1;

  TOKEN currentToken = ta[cursor];
  while (currentToken != END_ARRAY) {
    res = parse_value(ta);
    if (res == -1) return -1;

    // array is over
    if (ta[cursor] == END_ARRAY) {
      break;
    }

    // array has more entries
    if (ta[cursor] == VALUE_SEPARATOR) {
      res = eat(VALUE_SEPARATOR, ta);
      if (res == -1) return -1;

      if (ta[cursor] == END_ARRAY) {
        fprintf(stderr, "trailing comma in array!\n");
        return -1;
      }
    }
    currentToken = ta[cursor];
  }
  res = eat(END_ARRAY, ta);
  if (res == -1) return -1;
  return 0;
}
