#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

static void FreeTokenStream(TokenStream* ts);
static inline char is_simple_value(TOKEN tk);
static inline char is_json_value(TOKEN tk);
static char parse_object_member(TOKEN* ta, size_t* pos);

/**
 * @returns 0 if JSON is valid, -1 otherwise
 */
int Parse(TokenStream* ts) {
  if (!ts || !ts->tokenArray || ts->size == 0) {
    fprintf(stderr, "Parse: no tokens in JSON file!\n");
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

  // Edge case: bad tokens starting JSON
  if (ts->size == 1) {
    if (currentToken == NAME_SEPARATOR ||
        currentToken == VALUE_SEPARATOR ||
        currentToken == END_OBJECT ||
        currentToken == END_ARRAY) {
      fprintf(stderr, "Parse: expected JSON object, array, or literal!\n");
      return -1;
    }
  }

  size_t parseStatus = 0;
  char isParsingObject = 0;
  char isExpectingMember = 0;
  for (; pos < ts->size; pos++) {
    currentToken = ts->tokenArray[pos];
    if (isExpectingMember) {
      if (currentToken != STRING) {
        fprintf(stderr, "Parse: trailing comma! Expected object member start!\n");
        return -1;
      } else {
        parseStatus = parse_object_member(ts->tokenArray, &pos);
        if (!parseStatus) return -1;
        isExpectingMember = 0;
        continue;
      }
    }

    switch (currentToken) {
      case BEGIN_OBJECT: {
        if (!isParsingObject) {
          isParsingObject = 1;
        } else {
          fprintf(stderr, "Parse: unexpected object start '{' while already parsing an object!\n");
          return -1;
        }

        pos++;  // walk past BEGIN_OBJECT, so inner function only deals with member tokens
        parseStatus = parse_object_member(ts->tokenArray, &pos);
        if (!parseStatus) return -1;
        break;
      }
      case END_OBJECT: {
        if (isParsingObject) {
          isParsingObject = 0;
        } else {
          fprintf(stderr, "Parse: unexpected object end '}' while already parsing an object!\n");
          return -1;
        }
        break;
      }
      case BEGIN_ARRAY: {
        break;
      }
      case VALUE_SEPARATOR: {
        if (!isExpectingMember) {
          isExpectingMember = 1;
        } else {
          fprintf(stderr, "Parse: unexpected value separator ',' while already expecting another object member!\n");
          return -1;
        }
        break;
      }
      default: {
        fprintf(stderr, "Parse: unexpected token: %d (dec), %02x (hex), %c (char)\n", currentToken, currentToken, currentToken);
        return -1;
      }
    }
  }

  FreeTokenStream(ts);
  return 0;
}

/**
 * An object member (a.k.a name/value pair) is defined as:
 * `member = STRING NAME_SEPARATOR VALUE`
 */
static char parse_object_member(TOKEN* ta, size_t* pos) {
  TOKEN actualName = ta[*pos];
  TOKEN actualNameSep = ta[*pos + 1];
  TOKEN actualValue = ta[*pos + 2];

  if (actualName != STRING) {
    fprintf(stderr, "expected quoted string as object member name (property key)!\n");
    return 0;
  }
  if (actualNameSep != NAME_SEPARATOR) {
    fprintf(stderr, "expected colon (:) for separating names in an object member!\n");
    return 0;
  }
  if (!is_json_value(actualValue)) {
    fprintf(stderr, "expected value in the object member!\n");
    return 0;
  }

  (*pos)++;  // walk past STRING (member key)
  (*pos)++;  // walk past NAME_SEPARATOR (:)
  // loop will already skip VALUE (considering it's made of a single token, don't do it here)
  return 1;
}

/**
 * Checks if the `TOKEN` `tk` is a JSON value that can be represented in a single token i.e:
 *
 * string || number || 'true || 'false' || 'null'
 */
static inline char is_simple_value(TOKEN tk) {
  return (tk == STRING) || (tk == NUMBER) || (tk == LITERAL_TRUE) || (tk == LITERAL_FALSE) || (tk == LITERAL_NULL);
}

/**
 * A JSON value is either:
 * string || number || 'true || 'false' || 'null' || object || array
 */
static inline char is_json_value(TOKEN tk) {
  return (tk == STRING) ||
         (tk == NUMBER) ||
         (tk == LITERAL_TRUE) ||
         (tk == LITERAL_FALSE) ||
         (tk == LITERAL_NULL) ||
         (tk == BEGIN_OBJECT) ||
         (tk == BEGIN_ARRAY);
}

static void FreeTokenStream(TokenStream* ts) {
  if (!ts) {
    return;
  }
  free(ts->tokenArray);
  free(ts);
}
