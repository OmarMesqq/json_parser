#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

#define MAX_DEPTH 19  // acceptable number of nested arrays and objects

static inline char is_simple_value(TOKEN tk);
static char eat(TOKEN expectedToken, TOKEN* ta);
static char parse_value(TOKEN* tokenArray);
static char parse_object(TOKEN* ta);
static char parse_array(TOKEN* ta);
static void free_token_stream(TokenStream* ts);

static size_t cursor = 0;  // tracks position in the `TOKEN*` array
static size_t depth = 0;   // tracks how deep the parser is in the call stack due to its parsing static funcs

/**
 * Parses and validates a JSON file described by the
 * token stream `ts` using recursive descent.
 *
 * @returns 0 for valid JSONs, -1 otherwise
 */
int Parse(TokenStream* ts) {
  // An empty file is not valid JSON
  if (!ts || !ts->tokenArray || ts->size == 0) {
    fprintf(stderr, "Parse: no tokens in JSON file!\n");
    return -1;
  }

  char res = 0;
  TOKEN* ta = ts->tokenArray;

  /**
   * Although the RFC states that a valid JSON text is of type:
   * `ws value ws`
   *
   * where `ws` is whitespace and value is a simple or complex JSON value,
   *
   * the test file `tests/step5/fail1.json` from json.org says
   * "A JSON payload should be an object or array, not a string.",
   * so I am outright rejecting edge cases like a JSON file
   * that's a single boolean, string or 'null'
   */
  if (is_simple_value(ta[cursor]) && ts->size == 1) {
    res = -1;
    goto on_cleanup;
  }

  res = parse_value(ta);
  if (res == -1) goto on_cleanup;

  if (cursor != ts->size) {
    fprintf(stderr, "Parse: only a single root value allowed in JSON!\n");
    // fprintf(stderr, "cursor: %ld\n", cursor);
    // fprintf(stderr, "ts->size: %ld\n", ts->size);
    res = -1;
  }

on_cleanup:
  // printf("final depth: %ld\n", depth);
  free_token_stream(ts);
  depth = 0;
  cursor = 0;
  return res;
}

/**
 * Returns true if the token `tk` is a JSON value that can be represented in a single token i.e:
 *
 * string || number || 'true || 'false' || 'null'
 */
static inline char is_simple_value(TOKEN tk) {
  return (tk == STRING) || (tk == NUMBER) || (tk == LITERAL_TRUE) || (tk == LITERAL_FALSE) || (tk == LITERAL_NULL);
}

/**
 * Attempts to consume an `expectedToken`
 * in the currently parsed token array `ta`.
 *
 * This function uses the static variable `cursor` to track
 * current token in the stream.
 *
 * Returns 0 on success, incrementing `cursor`
 * Returns -1 on failure
 */
static char eat(TOKEN expectedToken, TOKEN* ta) {
  if (ta[cursor] == expectedToken) {
    cursor++;
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
  if (depth > MAX_DEPTH) {
    fprintf(stderr, "Nesting in JSON file exceeds safe limit (%d). Aborting!\n", MAX_DEPTH);
    return -1;
  }
  TOKEN currentToken = tokenArray[cursor];
  char res = 0;

  if (is_simple_value(currentToken)) {
    res = eat(currentToken, tokenArray);
  } else if (currentToken == BEGIN_OBJECT) {
    depth++;
    res = parse_object(tokenArray);
  } else if (currentToken == BEGIN_ARRAY) {
    depth++;
    res = parse_array(tokenArray);
  } else {
    fprintf(stderr, "parse_value: unexpected token: %c\n", currentToken);
    res = -1;
  }
  if (depth > 0) depth--;
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
  if (depth > MAX_DEPTH) {
    fprintf(stderr, "Nesting in JSON file exceeds safe limit (%d). Aborting!\n", MAX_DEPTH);
    return -1;
  }

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
        fprintf(stderr, "Trailing comma in object!\n");
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
  if (depth > MAX_DEPTH) {
    fprintf(stderr, "Nesting in JSON file exceeds safe limit (%d). Aborting!\n", MAX_DEPTH);
    return -1;
  }

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

static void free_token_stream(TokenStream* ts) {
  if (!ts) {
    return;
  }
  free(ts->tokenArray);
  free(ts);
}
