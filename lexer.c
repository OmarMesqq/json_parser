#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"

#define MAX_TOKENS 500
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

TokenStream *tokenize(FILE *file) {
  GRAMMAR *tl = (GRAMMAR *)calloc(MAX_TOKENS, sizeof(GRAMMAR));
  if (!tl) {
    printf("tokenize.error.failed.to.calloc\n");
    return NULL;
  }

  int i = 0;
  int capacity = MAX_TOKENS;
  int c;
  while ((c = fgetc(file)) != EOF) {
    if (i == capacity) {
      capacity *= 1.5;
      GRAMMAR *temp = (GRAMMAR *)realloc(tl, MAX_TOKENS * sizeof(GRAMMAR));
      if (!temp) {
        printf("tokenize.error.failed.to.realloc\n");
        return NULL;
      }
      tl = temp;
    }
    switch (c) {
      case BEGIN_OBJECT:
        tl[i] = BEGIN_OBJECT;
        break;
      case END_OBJECT:
        tl[i] = END_OBJECT;
        break;
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        tl[i] = WHITE_SPACE;
        break;
      default:
        break;
    }
    i++;
  }
  TokenStream *ts = (TokenStream *)malloc(i * sizeof(TokenStream));
  if (!ts) {
    printf("tokenize.error.failed.to.malloc\n");
    return NULL;
  }
  ts->size = i;
  ts->tokenList = tl;
  return ts;
}

void free_token_stream(TokenStream* ts) {
  if (!ts) {
    return;
  }
  free(ts->tokenList);
  free(ts);
}