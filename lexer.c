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

  size_t idx = 0;  // for inserting `Token`'s in `tokenList`
  size_t capacity = MAX_TOKENS;
  int ch = 0;              // current parsed unsigned character from JSON file
  char parsingString = 0;  // boolean to bypass some tokenization if reading a string
  while ((ch = fgetc(file)) != EOF) {
    if (idx == capacity) {
      capacity *= 1.5;
      TOKEN* temp = (TOKEN*)realloc(tokenList, MAX_TOKENS * sizeof(TOKEN));
      if (!temp) {
        fprintf(stderr, "tokenize: realloc failed!\n");
        return NULL;
      }
      tokenList = temp;
    }

    // Ignore whitespace
    if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
      continue;
    }

    if (isalpha(ch)) {
      // pretty much anything is allowed inside a quoted string, so just go on with the flow
      if (parsingString) {
        continue;
      };

      // attempt to lexify 'true', 'false', or 'null' individually
      switch (ch) {
        case 't': {
          if ((ch = fgetc(file)) == 'r' &&
              (ch = fgetc(file)) == 'u' &&
              (ch = fgetc(file)) == 'e') {
            tokenList[idx] = LITERAL_TRUE;
          } else {
            fprintf(stderr, "tokenize: malformed 'true' literal.\n");
            return -1;
          }
          break;
        }
        case 'f': {
          if ((ch = fgetc(file)) == 'a' &&
              (ch = fgetc(file)) == 'l' &&
              (ch = fgetc(file)) == 's' &&
              (ch = fgetc(file)) == 'e') {
            tokenList[idx] = LITERAL_FALSE;
          } else {
            fprintf(stderr, "tokenize: malformed 'false' literal.\n");
            return -1;
          }
          break;
        }
        case 'n': {
          if ((ch = fgetc(file)) == 'u' &&
              (ch = fgetc(file)) == 'l' &&
              (ch = fgetc(file)) == 'l') {
            tokenList[idx] = LITERAL_NULL;
          } else {
            fprintf(stderr, "tokenize: malformed 'null' literal.\n");
            return -1;
          }
          break;
        }
        default: {
          fprintf(stderr, "tokenize: unexpected alphabetic token: %c (char), %d (dec)\n", ch, ch);
          break;
        }
      }
    }

    switch (ch) {
      case BEGIN_OBJECT:
        tokenList[idx] = BEGIN_OBJECT;
        break;
      case END_OBJECT:
        tokenList[idx] = END_OBJECT;
        break;
      case STRING_START_END:
        tokenList[idx] = STRING_START_END;
        if (!parsingString) {
          parsingString = 1;
        } else {
          parsingString = 0;
        }
        break;
      case NAME_SEPARATOR:
        tokenList[idx] = NAME_SEPARATOR;
        break;
      default:
        fprintf(stderr, "tokenize: unknown token: %c (char), %d (decimal)\n", ch, ch);
        break;
    }

    idx++;
  }

  TokenStream* ts = (TokenStream*)malloc(idx * sizeof(TokenStream));
  if (!ts) {
    fprintf(stderr, "tokenize: malloc failed!\n");
    return NULL;
  }

  ts->size = idx;
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

  for (int idx = 0; idx < ts->size; idx++) {
    printf("TOKEN: %c\n", ts->tokenList[idx]);
  }

  printf("---- END TOKEN STREAM ----\n");
}