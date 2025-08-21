#ifndef GRAMMAR_H
#define GRAMMAR_H

typedef enum {
  // structural chars
  BEGIN_ARRAY = '[',
  BEGIN_OBJECT = '{',
  END_ARRAY = ']',
  END_OBJECT = '}',
  NAME_SEPARATOR = ':',
  VALUE_SEPARATOR = ',',
  // strings
  STRING_START_END = '"',
  // literal names
  LITERAL_TRUE,
  LITERAL_FALSE,
  LITERAL_NULL,
} TOKEN;

#endif