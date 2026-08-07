#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "build_config.h"

#define INITIAL_MAX_TOKENS 500  // acceptable number of tokens to initially read from the text file

static char* read_file(FILE* f);
static inline int8_t is_whitespace(int ch);
static inline int8_t is_control_character(int ch);
static int8_t lexify_primitive_value(int currentChar, const char* buf, size_t* buf_idx, TOKEN* tokenArray, size_t* tok_idx);
static int8_t lexify_string(const char* buf, size_t* buf_idx, TOKEN* tokenArray, size_t* tok_idx);
static int8_t lexify_number(int currentChar, const char* buf, size_t* buf_idx, TOKEN* tokenArray, size_t* tok_idx);
static int8_t lexify_true(const char* buf, size_t* buf_idx, TOKEN* tokenArray, size_t* tok_idx);
static int8_t lexify_false(const char* buf, size_t* buf_idx, TOKEN* tokenArray, size_t* tok_idx);
static int8_t lexify_null(const char* buf, size_t* buf_idx, TOKEN* tokenArray, size_t* tok_idx);

/**
 * Converts individual characters of `file`
 * into a meaningful stream of JSON tokens.
 *
 * @returns Heap allocated pointer to `TokenStream` on success, `NULL` on failure
 */
TokenStream* Tokenize(FILE* file) {
  TOKEN* tokenArray = NULL;
  TokenStream* ts = NULL;

  tokenArray = (TOKEN*)calloc(INITIAL_MAX_TOKENS, sizeof(TOKEN));
  if (!tokenArray) {
    fprintf(stderr, "tokenize: failed to calloc TOKEN* array!\n");
    goto on_error;
  }

  char* buf = read_file(file);
  if (!buf) {
    fprintf(stderr, "tokenize: failed to allocate buffer for JSON file!\n");
    goto on_error;
  }

  size_t buf_idx = 0;  // Tracks position in char* buf
  size_t tok_idx = 0;  // Tracks position in TOKEN* tokenArray
  size_t capacity = INITIAL_MAX_TOKENS;

  // Iterate based on the BUFFER index
  for (int ch = buf[buf_idx]; ch != '\0'; ch = buf[buf_idx]) {
    // Resize based on the TOKEN index
    if (tok_idx >= capacity) {
      capacity *= 1.5;
      TOKEN* temp = (TOKEN*)realloc(tokenArray, capacity * sizeof(TOKEN));
      if (!temp) {
        fprintf(stderr, "tokenize: failed to realloc TOKEN* array!\n");
        free(buf);
        goto on_error;
      }
      tokenArray = temp;
    }

    // Ignore whitespace
    if (is_whitespace(ch)) {
      buf_idx++;  // Advance buffer only
      continue;
    }

    // Handle "primitives": string, number, boolean and null
    char status = lexify_primitive_value(ch, buf, &buf_idx, tokenArray, &tok_idx);
    if (status == 0) {
      goto on_error;
    } else if (status == 1) {
      // If lexify primitive returned 1, then a primitive was lexed.
      // Go to iteration to eventually trigger realloc
      // Otherwise, it will fall-through into the structural character switch below
      // without risk of heap overflow
      continue;
    }

    // Handle structural characters
    switch (ch) {
      case BEGIN_ARRAY:
        tokenArray[tok_idx++] = BEGIN_ARRAY;
        break;
      case BEGIN_OBJECT:
        tokenArray[tok_idx++] = BEGIN_OBJECT;
        break;
      case END_ARRAY:
        tokenArray[tok_idx++] = END_ARRAY;
        break;
      case END_OBJECT:
        tokenArray[tok_idx++] = END_OBJECT;
        break;
      case NAME_SEPARATOR:
        tokenArray[tok_idx++] = NAME_SEPARATOR;
        break;
      case VALUE_SEPARATOR:
        tokenArray[tok_idx++] = VALUE_SEPARATOR;
        break;
      default:
        fprintf(stderr, "tokenize: unexpected token: %c (char), %d (decimal)\n", ch, ch);
        goto on_error;
    }

    buf_idx++;  // Advance buffer after consuming structural char
  }

  // avoid reading heap I don't own even though malloc(0) is valid (?) thanks valgrind
  if (tok_idx == 0) goto on_error;

  ts = (TokenStream*)malloc(sizeof(TokenStream));
  if (!ts) {
    fprintf(stderr, "tokenize: failed to malloc TokenStream!\n");
    goto on_error;
  }

  ts->size = tok_idx;
  ts->tokenArray = tokenArray;

  free(buf);

  return ts;

on_error:
  if (tokenArray) free(tokenArray);
  if (ts) free(ts);
  if (buf) free(buf);
  return NULL;
}

/**
 * Reads `ch` and decides which primitive to lex:
 * - number
 * - string
 * - 'true'
 * - 'false'
 * - 'null'
 *
 * Returns 0 on error, 1 on success, and -1 if the char didn't correspond to a primitive
 */
static int8_t lexify_primitive_value(int currentChar, const char* buf, size_t* buf_idx, TOKEN* tokenArray, size_t* tok_idx) {
  int8_t status = -1;

  if (isdigit(currentChar) || currentChar == '-') {
    status = lexify_number(currentChar, buf, buf_idx, tokenArray, tok_idx);
  } else if (currentChar == '"') {
    status = lexify_string(buf, buf_idx, tokenArray, tok_idx);
  } else if (currentChar == 't') {
    status = lexify_true(buf, buf_idx, tokenArray, tok_idx);
  } else if (currentChar == 'f') {
    status = lexify_false(buf, buf_idx, tokenArray, tok_idx);
  } else if (currentChar == 'n') {
    status = lexify_null(buf, buf_idx, tokenArray, tok_idx);
  }

  return status;
}

/**
 * Attempts to lexify a number.
 * In JSON, a number follows the schema:
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
 *
 * @returns 1 on success, 0 on error
 */
static int8_t lexify_number(int currentChar, const char* buf, size_t* buf_idx, TOKEN* tokenArray, size_t* tok_idx) {
  int ch = buf[*buf_idx + 1];

  // 1. Check for trailing minus at end of file
  if (ch == '\0' && currentChar == '-') {
    fprintf(stderr, "Trailing '-' at end of file.\n");
    return 0;
  }

  // 2. Handle leading minus explicitly
  if (currentChar == '-') {
    (*buf_idx)++;        // Consume the minus
    ch = buf[*buf_idx];  // Look at the digit following it

    if (!isdigit(ch)) {
      fprintf(stderr, "Minus sign must be followed by a digit.\n");
      return 0;
    }

    // Check for leading zero rule for negative numbers: e.g. -01 is invalid
    if (ch == '0' && isdigit(buf[*buf_idx + 1])) {
      fprintf(stderr, "No leading zeroes allowed in a number.\n");
      return 0;
    }
  } else {
    // No minus, currentChar is a digit. Check leading zero rule: e.g. 01 is invalid
    if (currentChar == '0' && isdigit(buf[*buf_idx + 1])) {
      fprintf(stderr, "No leading zeroes allowed in a number.\n");
      return 0;
    }
  }

  char foundDecimalPoint = 0;
  char fracPartHasNumbers = 0;
  char foundExpStart = 0;
  char expHasNumbers = 0;
  char isScanningExp = 0;
  int previousCh = 0;

  // Start loop at CURRENT buffer position (already past the leading minus if there was one)
  ch = buf[*buf_idx];

  while (ch != '\0') {
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
      break;
    }

    previousCh = ch;
    (*buf_idx)++;
    ch = buf[*buf_idx];
  }

  if (foundDecimalPoint && !fracPartHasNumbers) {
    fprintf(stderr, "Unterminated number's fractional part!\n");
    return 0;
  }

  if (foundExpStart && !expHasNumbers) {
    fprintf(stderr, "Unterminated number's exponent part!\n");
    return 0;
  }

  tokenArray[*tok_idx] = NUMBER;
  (*tok_idx)++;
  return 1;
}

/**
 * Attempts to lexify a string.
 * In JSON, a string is of type:
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
 *
 * @returns 1 on success, 0 on error
 */
static int8_t lexify_string(const char* buf, size_t* buf_idx, TOKEN* tokenArray, size_t* tok_idx) {
  int ch = buf[*buf_idx];
  char foundStrEnd = 0;

  // Consume opening quote
  (*buf_idx)++;
  ch = buf[*buf_idx];

  while (ch != '\0') {
    if (ch == '\\') {
      // Peek at the next char (the escaped char)
      char escaped = buf[*buf_idx + 1];

      if (escaped == 'u') {
        // Handle Unicode: \uXXXX
        // We need to consume: \ (1) + u (1) + 4 digits = 6 chars total

        // Verify we have enough chars ahead:
        // we need buf_idx + 1 ('u') + 4 digits.

        size_t temp_idx = *buf_idx + 2;  // Start at first hex digit
        for (int i = 0; i < 4; i++) {
          if (!isxdigit(buf[temp_idx])) {
            fprintf(stderr, "Invalid unicode escape hex digit: %c\n", buf[temp_idx]);
            return 0;
          }
          temp_idx++;
        }

        // Advance main buffer index by 6
        (*buf_idx) += 6;
        ch = buf[*buf_idx];
        continue;
      } else {
        // Simple escapes: \", \n, \\, etc.
        char isEscapeOk = 0;
        switch (escaped) {
          case '"':
          case '\\':
          case '/':
          case 'b':
          case 'f':
          case 'n':
          case 'r':
          case 't':
            isEscapeOk = 1;
            break;
          default:
            break;
        }

        if (isEscapeOk) {
          (*buf_idx) += 2;  // Consume '\' and the escaped char
          ch = buf[*buf_idx];
          continue;
        } else {
          fprintf(stderr, "Bad escape character: %c\n", escaped);
          return 0;
        }
      }
    } else if (is_control_character(ch)) {
      fprintf(stderr, "Control characters not allowed in string (found %d)\n", ch);
      return 0;
    } else if (ch == '"') {
      foundStrEnd = 1;
      (*buf_idx)++;  // Consume closing quote
      break;
    }

    // Normal character
    (*buf_idx)++;
    ch = buf[*buf_idx];
  }

  if (foundStrEnd) {
    tokenArray[*tok_idx] = STRING;
    (*tok_idx)++;
    return 1;
  }

  fprintf(stderr, "String was not terminated!\n");
  return 0;
}

/**
 * Attempts to lexify the `true` JSON literal.
 * @returns 1 on success, 0 on error
 */
static int8_t lexify_true(const char* buf, size_t* buf_idx, TOKEN* tokenArray, size_t* tok_idx) {
  if (buf[*buf_idx] == 't' &&
      buf[*buf_idx + 1] == 'r' &&
      buf[*buf_idx + 2] == 'u' &&
      buf[*buf_idx + 3] == 'e') {
    tokenArray[*tok_idx] = LITERAL_TRUE;
    (*tok_idx)++;
    (*buf_idx) += 4;
    return 1;
  }
  return 0;
}

/**
 * Attempts to lexify the `false` JSON literal.
 * @returns 1 on success, 0 on error
 */
static int8_t lexify_false(const char* buf, size_t* buf_idx, TOKEN* tokenArray, size_t* tok_idx) {
  if (buf[*buf_idx] == 'f' &&
      buf[*buf_idx + 1] == 'a' &&
      buf[*buf_idx + 2] == 'l' &&
      buf[*buf_idx + 3] == 's' &&
      buf[*buf_idx + 4] == 'e') {
    tokenArray[*tok_idx] = LITERAL_FALSE;
    (*tok_idx)++;
    (*buf_idx) += 5;
    return 1;
  }
  return 0;
}

/**
 * Attempts to lexify the `null` JSON literal.
 * @returns 1 on success, 0 on error
 */
static int8_t lexify_null(const char* buf, size_t* buf_idx, TOKEN* tokenArray, size_t* tok_idx) {
  if (buf[*buf_idx] == 'n' &&
      buf[*buf_idx + 1] == 'u' &&
      buf[*buf_idx + 2] == 'l' &&
      buf[*buf_idx + 3] == 'l') {
    tokenArray[*tok_idx] = LITERAL_NULL;
    (*tok_idx)++;
    (*buf_idx) += 4;
    return 1;
  }
  return 0;
}

/**
 * Returns true if `ch` is either:
 * - ' ' space
 * - '\t' tab
 * - '\n' line feed/newline
 * - '\r' carriage return
 */
static inline int8_t is_whitespace(int ch) {
  return (ch == 0x20) || (ch == 0x09) || (ch == 0x0A) || (ch == 0x0D);
}

/**
 * Returns true (1) if `ch` is a control character: `0x00` through `0x1F`
 */
static inline int8_t is_control_character(int ch) {
  return ((ch > 0) && (ch <= 0x1F));
}

/**
 * Reads the **open** file handle `f` and
 * copies the entirety of it to a heap allocated byte array
 * 
 * @returns valid pointer on success, `NULL` on failure
 */
static char* read_file(FILE* f) {
  // 1. Jump to the end of the file
  fseek(f, 0, SEEK_END);

  // 2. Get the current byte offset i.e the file size
  long length = ftell(f);

  // 3. Jump back to the beginning
  rewind(f);

  // 4. Allocate for whole file +1 for a null terminator
  char* buffer = (char*)malloc(length + 1);
  if (!buffer) {
    fprintf(stderr, "read_file: failed to malloc memory for file\n");
    return NULL;
  }

  // 5. Read the whole file
  size_t read_size = fread(buffer, 1, length, f);
  if (read_size != (size_t)length) {
    fprintf(stderr, "read_file: could not read entire file\n");
    free(buffer);
    return NULL;
  }

  // 6. Null-terminate
  buffer[length] = '\0';

  return buffer;
}
