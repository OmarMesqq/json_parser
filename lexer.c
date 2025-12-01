#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "build_config.h"

#define MAX_TOKENS 500

static void print_token_stream(TokenStream* ts);

TokenStream* tokenize(FILE* file) {
  TOKEN* tokenList = (TOKEN*)calloc(MAX_TOKENS, sizeof(TOKEN));
  if (!tokenList) {
    fprintf(stderr, "tokenize: calloc failed!\n");
    return NULL;
  }

  int i = 0;
  int capacity = MAX_TOKENS;
  int c;
  int parsingString = 0;
  while ((c = fgetc(file)) != EOF) {
    if (i == capacity) {
      capacity *= 1.5;
      TOKEN* temp = (TOKEN*)realloc(tokenList, MAX_TOKENS * sizeof(TOKEN));
      if (!temp) {
        fprintf(stderr, "tokenize: realloc failed!\n");
        return NULL;
      }
      tokenList = temp;
    }

    // Ignore whitespace
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
      continue;
    }
    if (isalpha(c)) {
      if (parsingString) {
        // pretty much anything is allowed inside a quoted string, so just go on with the flow
        continue;
      };
      switch (c) {
        case 't': {
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
        }
        case 'f': {
          // attempt to lexify 'false' into a token
          if ((c = fgetc(file)) == 'a' &&
              (c = fgetc(file)) == 'l' &&
              (c = fgetc(file)) == 's' &&
              (c = fgetc(file)) == 'e') {
            tokenList[i] = LITERAL_FALSE;
          } else {
            fprintf(stderr, "tokenize: malformed 'false' literal.\n");
            return NULL;
          }
          break;
        }
        case 'n': {
          // attempt to lexify 'null' into a token
          if ((c = fgetc(file)) == 'u' &&
              (c = fgetc(file)) == 'l' &&
              (c = fgetc(file)) == 'l') {
            tokenList[i] = LITERAL_NULL;
          } else {
            fprintf(stderr, "tokenize: malformed 'null' literal.\n");
            return NULL;
          }
          break;
        }
        default: {
          fprintf(stderr, "tokenize: unexpected alphanumeric token: %c (char), %d (dec)\n", c, c);
          break;
        }
      }
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
        if (!parsingString) {
          parsingString = 1;
        } else {
          parsingString = 0;
        }
        break;
      case ':':
        tokenList[i] = NAME_SEPARATOR;
        break;
      default:
        fprintf(stderr, "tokenize: unknown token: %c (char), %d (decimal)\n", c, c);
        break;
    }
    i++;
  }
  TokenStream* ts = (TokenStream*)malloc(i * sizeof(TokenStream));
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