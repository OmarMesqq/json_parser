#ifndef LEXER_H
#define LEXER_H
#include <stdio.h>

#include "grammar.h"
typedef struct tokenStream {
  TOKEN* tokenList;
  int size;
} TokenStream;

TokenStream* Tokenize(FILE* file);
void FreeTokenStream(TokenStream* ts);

#endif