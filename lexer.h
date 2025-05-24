#ifndef LEXER_H
#define LEXER_H

typedef enum {
  BEGIN_ARRAY = '[',
  BEGIN_OBJECT = '{',
  END_ARRAY = ']',
  END_OBJECT = '}',
  NAME_SEPARATOR = ':',
  VALUE_SEPARATOR = ',',
  LITERAL_TRUE,
  LITERAL_FALSE,
  LITERAL_NULL,
  WHITE_SPACE,
} GRAMMAR;

typedef struct tokenStream {
  GRAMMAR *tokenList;
  int size;
} TokenStream;

TokenStream *tokenize(FILE *file);
void free_token_stream(TokenStream* ts);

#endif