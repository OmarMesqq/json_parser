#include "json_parser.h"

#include <stdio.h>

#include "build_config.h"
#include "lexer.h"
#include "parser.h"

int ValidateJson(FILE* jsonFilePtr) {
  TokenStream* ts = tokenize(jsonFilePtr);
  if (!ts) {
    fprintf(stderr, RED "ValidateJson: failed to tokenize file!\n" RESET_COLOR);
    return -1;
  }

  int res = ParseJson(ts);
  return res;
}