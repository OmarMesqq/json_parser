#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>

/**
 * JSON tokens per RFC definition
 */
typedef enum {
  // structural chars
  BEGIN_ARRAY = '[',
  BEGIN_OBJECT = '{',
  END_ARRAY = ']',
  END_OBJECT = '}',
  NAME_SEPARATOR = ':',
  VALUE_SEPARATOR = ',',

  STRING = 'S',
  NUMBER = 'N',
  LITERAL_TRUE = 'T',
  LITERAL_FALSE = 'F',
  LITERAL_NULL = 'U',
} TOKEN;

/**
 * Represents the collected tokens from a JSON file.
 * Fields:
 * - `tokenArray` a pointer to `TOKEN`, showing JSON tokens in the order they were lexified
 * - `size` how large the array is
 */
typedef struct {
  TOKEN* tokenArray;
  size_t size;
} TokenStream;

#endif
