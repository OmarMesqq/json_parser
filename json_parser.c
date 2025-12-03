#include "json_parser.h"

#include <stdio.h>

#include "build_config.h"
#include "lexer.h"
#include "parser.h"

/**
 * Tokenizes and parses a JSON file at the open file handle
 * returning status codes of the tokenizer or the parser.
 *
 * @param jsonFilePtr **open** file pointer/handle of the JSON file
 * @returns 0 on success or -1 on failure
 */
int ValidateJson(FILE* jsonFilePtr) {
  // tokenization
  TokenStream* ts = tokenize(jsonFilePtr);
  if (ts == NULL) {
    fprintf(stderr, RED "tokenizer failed allocate some structures!\n" RESET_COLOR);
    return -1;
  } else if (ts == -1) {
    fprintf(stderr, RED "tokenizer found errors during tokenization!\n" RESET_COLOR);
    return -1;
  }

  // parsing
  int parsingResult = ParseJson(ts);
  if (parsingResult == -1) {
    fprintf(stderr, RED "parser found errors during parsing!\n" RESET_COLOR);
    return -1;
  }
  return parsingResult;
}