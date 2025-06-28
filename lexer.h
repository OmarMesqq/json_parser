#ifndef LEXER_H
#define LEXER_H
#include "grammar.h"

typedef struct tokenStream {
  GRAMMAR *tokenList;
  int size;
} TokenStream;

TokenStream *tokenize(FILE *file);
void free_token_stream(TokenStream* ts);

#endif