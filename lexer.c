#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "build_config.h"

#define INITIAL_MAX_TOKENS 500

static void print_token_stream(TokenStream* ts);
static inline char is_whitespace(int ch);
static inline char is_control_character(int ch);
static char lexify_string(FILE* f, TOKEN* tokenArray, size_t* idx);
static char lexify_number(int currentChar, FILE* f, TOKEN* tokenArray, size_t* idx);
static char lexify_true(FILE* f, TOKEN* tokenArray, size_t* idx);
static char lexify_false(FILE* f, TOKEN* tokenArray, size_t* idx);
static char lexify_null(FILE* f, TOKEN* tokenArray, size_t* idx);
int peek_next_char(FILE* file);

/**
 * A valid JSON text is a sequence of tokens such that:
 * `ws value ws`
 *
 * where `ws` is ignored whitespace and `value` is one of the below:
 * - string
 * - number
 * - boolean ('true'/'false')
 * - 'null'
 * - object
 * - array
 *
 * This function converts the individual characters of the file
 * into a meaningful token stream for later use by the parser
 */
TokenStream* Tokenize(FILE* file) {
  TOKEN* tokenArray = NULL;
  TokenStream* ts = NULL;

  tokenArray = (TOKEN*)calloc(INITIAL_MAX_TOKENS, sizeof(TOKEN));
  if (!tokenArray) {
    fprintf(stderr, "tokenize: failed to calloc TOKEN* array!\n");
    goto on_error;
  }

  size_t idx = 0;  // index of current `Token` in `tokenArray`
  size_t capacity = INITIAL_MAX_TOKENS;

  int ch = 0;  // current parsed unsigned character from JSON file
  char lexifyStatus = 0;
  char lexedValue = 0;  // value = str || number || boolean || null || object || array

  while ((ch = fgetc(file)) != EOF) {
    if (idx == capacity) {
      capacity *= 1.5;
      TOKEN* temp = (TOKEN*)realloc(tokenArray, capacity * sizeof(TOKEN));
      if (!temp) {
        fprintf(stderr, "tokenize: failed to realloc TOKEN* array!\n");
        goto on_error;
      }
      tokenArray = temp;
    }

    // Ignore whitespace
    if (is_whitespace(ch)) {
      continue;
    }

    // Numbers = [-] int [frac] [exp]
    if (isdigit(ch) || ch == '-') {
      lexifyStatus = lexify_number(ch, file, tokenArray, &idx);
      if (!lexifyStatus) {
        goto on_error;
      }
      lexedValue = 1;
    }

    // Strings = quotation-mark *char quotation-mark
    if (ch == '"') {
      lexifyStatus = lexify_string(file, tokenArray, &idx);
      if (!lexifyStatus) {
        goto on_error;
      }
      lexedValue = 1;
    }

    // Literal 'true'
    if (ch == 't') {
      lexifyStatus = lexify_true(file, tokenArray, &idx);
      if (!lexifyStatus) {
        goto on_error;
      }
      lexedValue = 1;
    }

    // Literal 'false'
    if (ch == 'f') {
      lexifyStatus = lexify_false(file, tokenArray, &idx);
      if (!lexifyStatus) {
        goto on_error;
      }
      lexedValue = 1;
    }

    // Literal 'null'
    if (ch == 'n') {
      lexifyStatus = lexify_null(file, tokenArray, &idx);
      if (!lexifyStatus) {
        goto on_error;
      }
      lexedValue = 1;
    }

    // Walk +1 position after lexifying a "primitive" value
    if (lexedValue) {
      lexedValue = 0;
      ch = fgetc(file);
      if (ch == EOF) break;
      if (is_whitespace(ch)) continue;
    }

    switch (ch) {
      case BEGIN_ARRAY:
        tokenArray[idx] = BEGIN_ARRAY;
        break;
      case BEGIN_OBJECT:
        tokenArray[idx] = BEGIN_OBJECT;
        break;
      case END_ARRAY:
        tokenArray[idx] = END_ARRAY;
        break;
      case END_OBJECT:
        tokenArray[idx] = END_OBJECT;
        break;
      case NAME_SEPARATOR:
        tokenArray[idx] = NAME_SEPARATOR;
        break;
      case VALUE_SEPARATOR:
        tokenArray[idx] = VALUE_SEPARATOR;
        break;
      default:
        fprintf(stderr, "tokenize: unexpected token: %c (char), %d (decimal)\n", ch, ch);
        goto on_error;
    }

    idx++;
  }

  // avoid reading heap I don't own even though malloc(0) is valid (?) thanks valgrind
  if (idx == 0) goto on_error;

  ts = (TokenStream*)malloc(sizeof(TokenStream));
  if (!ts) {
    fprintf(stderr, "tokenize: failed to malloc TokenStream!\n");
    goto on_error;
  }

  ts->size = idx;
  ts->tokenArray = tokenArray;

#ifdef DEBUG
  print_token_stream(ts);
#endif

  return ts;
on_error:
  if (tokenArray) free(tokenArray);
  if (ts) free(ts);
  return NULL;
}

/**
 * A number in JSON is of type:
 * `[ minus ] int [ frac ] [ exp ]`, where:
 *
 * - `minus` is: `-` (hex 0x2D)
 * - `int` is: `zero / ( digit1-9 *DIGIT )`
 * - `frac` is: `decimal-point 1*DIGIT`
 * - `exp` is: `e [ minus / plus ] 1*DIGIT`
 *
 * and:
 *
 * - `decimal-point` is: `.` (hex 0x2E)
 * - `digit1-9` is: `1-9` (hex 0x31-39)
 * - `e` is: `e/E` (hex 0x65/0x45)
 * - `plus` is: `+` (hex 0x2B)
 * - `zero` is: `0` (hex 0x30)
 */
static char lexify_number(int currentChar, FILE* f, TOKEN* tokenArray, size_t* idx) {
  int ch = 0;

  // peek at the char after `currentChar`
  ch = peek_next_char(f);
  if (ch == EOF) {
    if (currentChar == '-') {
      fprintf(stderr, "Trailing '-' at end of file.\n");
      return 0;
    }
  }

  if (currentChar == '0' && isdigit(ch)) {
    fprintf(stderr, "No leading zeroes allowed in a number.\n");
    return 0;
  }

  char foundDecimalPoint = 0;
  char fracPartHasNumbers = 0;
  char foundExpStart = 0;
  char expHasNumbers = 0;
  char isScanningExp = 0;
  int previousCh = 0;

  ch = fgetc(f);
  while (ch != EOF) {
    // probable start of number's `frac`, lex the following chars as part of the number's fractional part
    if (ch == '.') {
      if (!foundDecimalPoint) {
        foundDecimalPoint = 1;
      } else {
        fprintf(stderr, "Not allowed more than one decimal point in number.\n");
        return 0;
      }
      if (isScanningExp) {
        fprintf(stderr, "Decimal point not allowed in number's exponent.\n");
        return 0;
      }
    }
    // probably the fractional part's `exp`
    else if (ch == 'e' || ch == 'E') {
      if (!foundExpStart) {
        foundExpStart = 1;
        isScanningExp = 1;
      } else {
        fprintf(stderr, "Not allowed more than of exponent start 'e'/'E' in number.\n");
        return 0;
      }
    }
    // probably `exp`'s signal
    else if (ch == '-' || ch == '+') {
      if (previousCh != 'e' && previousCh != 'E') {
        fprintf(stderr, "Misplaced %c sign. Expected to be after 'e' or 'E'.\n", ch);
        return 0;
      }
    } else if (isdigit(ch)) {
      if (foundDecimalPoint && !fracPartHasNumbers) {
        fracPartHasNumbers = 1;  // there are numbers after the point in X.YZ
      }
      if (foundExpStart && !expHasNumbers) {
        expHasNumbers = 1;  // there are numbers after the exponent in XeYZ
      }

    } else {
      if (!is_whitespace(ch)) {
        // end of the number: put back read char and stop lexing it
        ungetc(ch, f);
        break;
      }
    }

    previousCh = ch;
    ch = fgetc(f);
  }

  if (foundDecimalPoint && !fracPartHasNumbers) {
    fprintf(stderr, "Unterminated number's fractional part!\n");
    return 0;
  }

  if (foundExpStart && !expHasNumbers) {
    fprintf(stderr, "Unterminated number's exponent part!\n");
    return 0;
  }

  tokenArray[*idx] = NUMBER;
  (*idx)++;
  return 1;
}

/**
 * A string in JSON is of type:
 * `quotation-mark *char quotation-mark`, where:
 *
 * `quotation-mark` is `"` (hex 0x22)
 * and `char` is:
 ```
 unescaped / escape (
    0x22 /          ; "    quotation mark
    0x5C /          ; \    reverse solidus
    0x2F /          ; /    solidus
    0x62 /          ; b    backspace
    0x66 /          ; f    form feed
    0x6E /          ; n    line feed
    0x72 /          ; r    carriage return
    0x74 /          ; t    tab
    0x75 4HEXDIG )  ; uXXXX
  ```
 *
 * where:
 * `escape` is `\` (hex 0x5C) and
 * `unescaped = 0x20-21 / 0x23-5B / 0x5D-10FFFF`, i.e
 * any Unicode character EXCEPT:
 * - the quotation mark (`"`)
 * - control characters (`0x00` through `0x1F`)
 * - backslash (`\`)
 */
static char lexify_string(FILE* f, TOKEN* tokenArray, size_t* idx) {
  if (!f || !tokenArray) return 0;

  int ch = 0;
  char foundStrEnd = 0;

  while ((ch = fgetc(f)) != EOF) {
    // Escapes
    if (ch == '\\') {
      ch = peek_next_char(f);
      char isEscapeOk = 0;

      switch (ch) {
        case '"':   // quotation mark
        case '\\':  // reverse solidus
        case '/':   // solidus
        case 'b':   // backspace
        case 'f':   // form feed
        case 'n':   // line feed
        case 'r':   // carriage return
        case 't':   // tab
          isEscapeOk = 1;
          break;
        case 'u':  // uXXXX
          // expect 4 hexadecimal digits for Unicode
          ch = fgetc(f);  // consume 'u'
          for (int i = 0; i < 4; i++) {
            ch = fgetc(f);

            if (!isxdigit(ch)) {
              fprintf(stderr, "Invalid character in Unicode escape sequence: '%c' (expected hex digit).\n", ch);
              return 0;
            }
          }

          continue;
        default:
          fprintf(stderr, "Unexpected character after escape character ('\\'): %c.\n", ch);
          break;
      }

      if (isEscapeOk) {
        fgetc(f);  // consume it
        continue;
      } else {
        fprintf(stderr, "Bad escape in string!\n");
        break;
      }

    }

    else if (is_control_character(ch)) {
      fprintf(stderr, "control characters must be escaped!\n");
      break;
    }
    
    else if (ch == '"') {
      foundStrEnd = 1;
      break;
    }
  }

  if (foundStrEnd) {
    tokenArray[*idx] = STRING;
    (*idx)++;
    return 1;
  } else {
    fprintf(stderr, "String was not terminated! Aborting.\n");
    return 0;
  }
}

static char lexify_true(FILE* f, TOKEN* tokenArray, size_t* idx) {
  int ch = 0;
  if ((ch = fgetc(f)) == 'r' &&
      (ch = fgetc(f)) == 'u' &&
      (ch = fgetc(f)) == 'e') {
    tokenArray[*idx] = LITERAL_TRUE;
    (*idx)++;
    return 1;
  }
  fprintf(stderr, "expected 'true' literal. Was malformed.\n");
  return 0;
}

static char lexify_false(FILE* f, TOKEN* tokenArray, size_t* idx) {
  int ch = 0;
  if ((ch = fgetc(f)) == 'a' &&
      (ch = fgetc(f)) == 'l' &&
      (ch = fgetc(f)) == 's' &&
      (ch = fgetc(f)) == 'e') {
    tokenArray[*idx] = LITERAL_FALSE;
    (*idx)++;
    return 1;
  }
  fprintf(stderr, "expected 'false' literal. Was malformed.\n");
  return 0;
}

static char lexify_null(FILE* f, TOKEN* tokenArray, size_t* idx) {
  int ch = 0;
  if ((ch = fgetc(f)) == 'u' &&
      (ch = fgetc(f)) == 'l' &&
      (ch = fgetc(f)) == 'l') {
    tokenArray[*idx] = LITERAL_NULL;
    (*idx)++;
    return 1;
  }
  fprintf(stderr, "expected 'null' literal. Was malformed.\n");
  return 0;
}

int peek_next_char(FILE* file) {
  int ch;
  ch = fgetc(file);

  if (ch == EOF) {
    return EOF;
  }

  ungetc(ch, file);  // put char back
  return ch;
}

/**
 * Returns true if `ch` is either:
 * - ' ' space
 * - '\t' tab
 * - '\n' line feed/newline
 * - '\r' carriage return
 */
static inline char is_whitespace(int ch) {
  return (ch == 0x20) || (ch == 0x09) || (ch == 0x0A) || (ch == 0x0D);
}

/**
 * Returns true (1) if `ch` is a control character: `0x00` through `0x1F`
 */
static inline char is_control_character(int ch) {
  return ((ch > 0) && (ch <= 0x1F));
}

static void print_token_stream(TokenStream* ts) {
  if (!ts || !ts->tokenArray) {
    return;
  }

  printf("Stream has %ld tokens.\n", ts->size);
  printf("---- START TOKEN STREAM ----\n");

  for (size_t idx = 0; idx < ts->size; idx++) {
    printf("%c (char), %02x (hex), %d (dec)\n", ts->tokenArray[idx], ts->tokenArray[idx], ts->tokenArray[idx]);
  }

  printf("---- END TOKEN STREAM ----\n");
}