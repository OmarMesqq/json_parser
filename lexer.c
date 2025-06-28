#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "build_config.h"

#define MAX_TOKENS 500

static void print_token_stream(TokenStream* ts);

TokenStream *tokenize(FILE *file) {
  TOKEN* tokenList = (TOKEN*) calloc(MAX_TOKENS, sizeof(TOKEN));
  if (!tokenList) {
    fprintf(stderr, "tokenize: calloc failed!\n");
    return NULL;
  }

  int i = 0;
  int capacity = MAX_TOKENS;
  int c;
  while ((c = fgetc(file)) != EOF) {
    if (i == capacity) {
      capacity *= 1.5;
      TOKEN *temp = (TOKEN *) realloc(tokenList, MAX_TOKENS * sizeof(TOKEN));
      if (!temp) {
        fprintf(stderr, "tokenize: realloc failed!\n");
        return NULL;
      }
      tokenList = temp;
    }

    switch (c) {
      case BEGIN_OBJECT:
        tokenList[i] = BEGIN_OBJECT;
        break;
      case END_OBJECT:
        tokenList[i] = END_OBJECT;
        break;
      case STRING_START_END:
        tokenList[i] = STRING_START_END;
        break;
      case 't':
        // attempt to lexify 'true' into a token
        if ((c = fgetc(file)) == 'r' &&
          (c = fgetc(file)) == 'u' &&
          (c = fgetc(file)) == 'e') {
          tokenList[i] = LITERAL_TRUE;
        } else {
          fprintf(stderr, "tokenize: malformed 'true' literal.\n");
          return NULL;
        }
        break;
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        tokenList[i] = WHITE_SPACE;
        break;
      default:
        fprintf(stderr, "tokenize: unknown token: %c (char), %d (decimal)\n", c, c);
        break;
    }
    i++;
  }
  TokenStream *ts = (TokenStream *) malloc(i * sizeof(TokenStream));
  if (!ts) {
    fprintf(stderr, "tokenize: malloc failed!\n");
    return NULL;
  }

  ts->size = i;
  ts->tokenList = tokenList;

  #ifdef DEBUG
  print_token_stream(ts);
  #endif

  return ts;
}

void free_token_stream(TokenStream* ts) {
  if (!ts) {
    return;
  }
  free(ts->tokenList);
  free(ts);
}

static void print_token_stream(TokenStream* ts) {
  if (!ts || !ts->tokenList) {
    return;
  }

  printf("---- START TOKEN STREAM ----\n");
  printf("Stream has %d tokens.\n", ts->size);
  
  for (int i = 0; i < ts->size; i++) {
    printf("TOKEN: %c\n", ts->tokenList[i]);
  }


  printf("---- END TOKEN STREAM ----\n");
}