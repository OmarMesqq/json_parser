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
  TokenStream* ts = Tokenize(jsonFilePtr);
  if (ts == NULL) {
    // fprintf(stderr, "tokenizer found errors during tokenization!\n");
    return -1;
  }

  // parsing
  int parsingResult = Parse(ts);
  return parsingResult;
}