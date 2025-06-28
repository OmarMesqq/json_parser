#ifndef GRAMMAR_H
#define GRAMMAR_H

typedef enum {
  BEGIN_ARRAY = '[',
  BEGIN_OBJECT = '{',
  END_ARRAY = ']',
  END_OBJECT = '}',
  NAME_SEPARATOR = ':',
  VALUE_SEPARATOR = ',',
  STRING_START_END = '"',
  LITERAL_TRUE,
  LITERAL_FALSE,
  LITERAL_NULL,
  WHITE_SPACE,
} TOKEN;

#endif