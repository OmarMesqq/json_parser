#ifndef LEXER_H
#define LEXER_H

typedef struct tokenStream {
  GRAMMAR *tokenList;
  int size;
} TokenStream;

TokenStream *tokenize(FILE *file);
void free_token_stream(TokenStream* ts);

#endif