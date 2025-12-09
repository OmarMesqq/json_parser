#include <stdio.h>

#include "build_config.h"
#include "lexer.h"
#include "parser.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, RED "usage: ./json_parser <filename.json>\n" RESET_COLOR);
    return -1;
  }
  const char* jsonFilePath = argv[1];

  FILE* fp = fopen(jsonFilePath, "r");
  if (!fp) {
    fprintf(stderr, RED "Failed to open JSON file %s\n" RESET_COLOR, jsonFilePath);
    return -1;
  }

  TokenStream* ts = Tokenize(fp);
  int parsingResult = Parse(ts);

  if (parsingResult == 0) {
    printf(GREEN "%s is valid JSON.\n" RESET_COLOR, jsonFilePath);
  } else if (parsingResult == -1) {
    printf(RED "%s is NOT valid JSON.\n" RESET_COLOR, jsonFilePath);
  } else {
    fprintf(stderr, RED "Unknown error. json_parser returned status code %d\n" RESET_COLOR, parsingResult);
    return -1;
  }

  fclose(fp);
  return 0;
}
