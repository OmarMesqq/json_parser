#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

static void FreeTokenStream(TokenStream* ts);
static inline char is_simple_value(TOKEN tk);
static inline char is_json_value(TOKEN tk);
static char parse_object_member(TOKEN* ta, size_t* pos);
static char parse_array_element(TOKEN* ta, size_t* pos);

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
  unsigned objectStarts = 0;
  unsigned objectEnds = 0;
  unsigned arrayStarts = 0;
  unsigned arrayEnds = 0;
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
        objectStarts++;
        pos++;  // walk past BEGIN_OBJECT

        // empty object immediately closing itself
        if (ts->tokenArray[pos] == END_OBJECT) {
          objectEnds++;
          break;
        }
        // else case: there is a member inside the object
        parseStatus = parse_object_member(ts->tokenArray, &pos);
        if (!parseStatus) return -1;
        break;
      }
      case END_OBJECT: {
        objectEnds++;
        break;
      }
      case BEGIN_ARRAY: {
        arrayStarts++;
        pos++;  // walk past BEGIN_ARRAY

        // empty array immediately closing itself
        if (ts->tokenArray[pos] == END_ARRAY) {
          arrayEnds++;
          break;
        }

        parseStatus = parse_array_element(ts->tokenArray, &pos);
        if (!parseStatus) return -1;
        break;
      }
      case END_ARRAY: {
        arrayEnds++;
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

  if (objectStarts != objectEnds) {
    printf("objectStarts: %d\n", objectStarts);
    printf("objectEnds: %d\n", objectEnds);
    fprintf(stderr, "Parse: unbalanced amount of object start and end tokens!\n");
    return -1;
  }

  if (arrayStarts != arrayEnds) {
    printf("arrayStarts: %d\n", arrayStarts);
    printf("arrayEnds: %d\n", arrayEnds);
    fprintf(stderr, "Parse: unbalanced amount of array start and end tokens!\n");
    return -1;
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

  (*pos)++;                                // walk past STRING (member key)
  TOKEN tokenAfterNameSep = ta[*pos + 1];  // VALUE is after NAME_SEPARATOR
  const char isSingleTokenValue = is_simple_value(tokenAfterNameSep);
  if (isSingleTokenValue) {
    (*pos)++;  // walk past NAME_SEPARATOR (:)
    // loop will already skip VALUE, so don't do it here
  }
  // else branch: loop will skip NAME_SEPARATOR, landing at BEGIN_ARRAY or BEGIN_OBJECT (hopefully)

  return 1;
}

static char parse_array_element(TOKEN* ta, size_t* pos) {
  TOKEN actualValue = ta[*pos];

  if (!is_json_value(actualValue)) {
    fprintf(stderr, "expected value inside array!\n");
    return 0;
  }
  (*pos)++;  // walk past VALUE (assumes single token VALUE)
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
