#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

static void FreeTokenStream(TokenStream* ts);
static inline char is_simple_value(TOKEN tk);
static inline char is_json_value(TOKEN tk);
static char parse_object_member(TOKEN* ta, size_t* pos);
static char parse_array_element(TOKEN* ta, size_t* pos);

static void eat(TOKEN expectedToken, TOKEN* ta);
static void parse_value(TOKEN* tokenArray);
static void parse_object(TOKEN* ta);
static void parse_array(TOKEN* ta);

static size_t cursor = 0;

/**
 * @returns 0 if JSON is valid, -1 otherwise
 */
int Parse(TokenStream* ts) {
  int status = 0;
  if (!ts || !ts->tokenArray || ts->size == 0) {
    fprintf(stderr, "Parse: no tokens in JSON file!\n");
    status = -1;
    goto on_cleanup;
  }

  size_t pos = 0;
  TOKEN currentToken = 0;

  currentToken = ts->tokenArray[pos];

  // Edge case: standalone simple values in JSON
  const char isStandaloneSimpleValue = (is_simple_value(currentToken) && ts->size == 1);
  if (isStandaloneSimpleValue) {
    status = 0;
    goto on_cleanup;
  }

  // Edge case: standalone composite values in JSON
  if (ts->size == 2) {
    if (currentToken == BEGIN_OBJECT && (ts->tokenArray[pos + 1] == END_OBJECT)) {
      // JSON is `{}`
      status = 0;
      goto on_cleanup;
    }
    if (currentToken == BEGIN_ARRAY && (ts->tokenArray[pos + 1] == END_ARRAY)) {
      // JSON is `[]`
      status = 0;
      goto on_cleanup;
    }
  }

  // Edge case: bad tokens starting JSON
  if (ts->size == 1) {
    if (currentToken == NAME_SEPARATOR ||
        currentToken == VALUE_SEPARATOR ||
        currentToken == END_OBJECT ||
        currentToken == END_ARRAY) {
      fprintf(stderr, "Parse: expected JSON object, array, or literal!\n");
      status = -1;
      goto on_cleanup;
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
        status = -1;
        goto on_cleanup;
      } else {
        parseStatus = parse_object_member(ts->tokenArray, &pos);
        if (!parseStatus) {
          status = -1;
          goto on_cleanup;
        }
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
        if (!parseStatus) {
          status = -1;
          goto on_cleanup;
        }
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
        if (!parseStatus) {
          status = -1;
          goto on_cleanup;
        }
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
          status = -1;
          goto on_cleanup;
        }
        break;
      }
      default: {
        fprintf(stderr, "Parse: unexpected token: %d (dec), %02x (hex), %c (char)\n", currentToken, currentToken, currentToken);
        status = -1;
        goto on_cleanup;
      }
    }
  }

  if (objectStarts != objectEnds) {
    printf("objectStarts: %d\n", objectStarts);
    printf("objectEnds: %d\n", objectEnds);
    fprintf(stderr, "Parse: unbalanced amount of object start and end tokens!\n");
    status = -1;
    goto on_cleanup;
  }

  if (arrayStarts != arrayEnds) {
    printf("arrayStarts: %d\n", arrayStarts);
    printf("arrayEnds: %d\n", arrayEnds);
    fprintf(stderr, "Parse: unbalanced amount of array start and end tokens!\n");
    status = -1;
    goto on_cleanup;
  }

on_cleanup:
  FreeTokenStream(ts);
  return status;
}

/**
 * An object in JSON is of type:
 * `BEGIN_OBJECT *(member *(VALUE_SEPARATOR member)) END_OBJECT`
 *
 * where its possible member(s) - aka name/value pair(s) -  are defined as:
 * `STRING NAME_SEPARATOR VALUE`
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

/**
 * An array in JSON is of type:
 * `BEGIN_ARRAY *(VALUE *(VALUE_SEPARATOR VALUE)) END_ARRAY`
 */
static char parse_array_element(TOKEN* ta, size_t* pos) {
  TOKEN actualValue = ta[*pos];

  if (!is_json_value(actualValue)) {
    fprintf(stderr, "expected value inside array!\n");
    return 0;
  }

  // composite start values (array and object need to handled in loop)
  const char isSingleTokenValue = is_simple_value(actualValue);
  if (!isSingleTokenValue) {
    (*pos)--;
  }

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

static void eat(TOKEN expectedToken, TOKEN* ta) {
  if (ta[cursor] == expectedToken) {
    cursor++;  // advance cursor
  } else {
    fprintf(stderr, "eat: expected %c, got %c\n", expectedToken, ta[cursor]);
    exit(-1);
  }
}

/**
 * `parse_value(tk) {
 if tk == is_simple_value()
  eat(SIMPLE_VALUE)
 elif tk == BEGIN_OBJECT 
  parse_object()
elif tk == BEGIN_ARRAY
  parse_array()
else error
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
 * parse_object() {
  while (tk != END_OBJECT)
   eat(STRING)
   eat(NAME_SEP)
   parse_value(tk)
if input[OBJECT_END] 
  break
elif input[VALUE_SEP] 
 eat(VALUE_SEP)
 // implicit continue
  if input[cursor] == END_OBJECT
    Error("trailing comma")
}
eat(OBJECT_END)
 */
static void parse_object(TOKEN* ta) {
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
        exit(-1);
      }
    }
    currentToken = ta[cursor];
  }
  eat(END_OBJECT, ta);
}

/**
 * parse_array() {
  while (tk != END_ARRAY)
   parse_value(tk)
   if input[END_ARRAY] 
    break
   elif input[VALUE_SEP] 
    eat(VALUE_SEP)
       if input[cursor] == END_ARRAY
         Error("trailing comma")
}
eat(END_ARRAY)
 */
static void parse_array(TOKEN* ta) {
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
        exit(-1);
      }
    }
  }
  eat(END_ARRAY, ta);
}

/**
 * json_parse() {
  // edge cases
  if input[cursor] == is_simple_value && inputSize = 1
   return ok


 parse_value(tk)

if cursor != inputSize
 error

return ok 
}
 */
int JsonParse(TokenStream* ts) {
  if (!ts || !ts->tokenArray || ts->size == 0) {
    fprintf(stderr, "JsonParse: no tokens in JSON file!\n");
    return -1;
  }

  // edge cases
  TOKEN* ta = ts->tokenArray;
  if (is_json_value(ta[cursor]) && ts->size == 1) {
    return 0;
  }

  parse_value(ta);

  if (cursor != ts->size) {
    fprintf(stderr, "Only a single root value allowed in JSON!\n");
    return -1;
  }
  cursor = 0;
  return 0;
}