#include <stdio.h>
#include <stdlib.h>

#include "build_config.h"
#include "lexer.h"
#include "parser.h"

static void run_test(const char* testName, const char* jsonFilePath, const int expected);

int main() {
  run_test("Step 1, valid JSON", "tests/step1/valid.json", 0);
  run_test("Step 1, invalid JSON", "tests/step1/invalid.json", -1);

  run_test("Step 2, valid JSON", "tests/step2/valid.json", 0);
  run_test("Step 2, invalid JSON", "tests/step2/invalid.json", -1);
  run_test("Step 2, valid JSON 2", "tests/step2/valid2.json", 0);
  run_test("Step 2, invalid JSON 2", "tests/step2/invalid2.json", -1);

  run_test("Step 3, valid JSON", "tests/step3/valid.json", 0);
  run_test("Step 3, invalid JSON", "tests/step3/invalid.json", -1);

  run_test("Step 4, valid JSON", "tests/step4/valid.json", 0);
  run_test("Step 4, invalid JSON", "tests/step4/invalid.json", -1);
  run_test("Step 4, valid JSON 2", "tests/step4/valid2.json", 0);

  run_test("Step 5 pass1", "tests/step5/pass1.json", 0);
  run_test("Step 5 pass2", "tests/step5/pass2.json", 0);
  run_test("Step 5 pass3", "tests/step5/pass3.json", 0);

  run_test("Step 5 fail1", "tests/step5/fail1.json", -1);
  run_test("Step 5 fail2", "tests/step5/fail2.json", -1);
  run_test("Step 5 fail3", "tests/step5/fail3.json", -1);
  run_test("Step 5 fail4", "tests/step5/fail4.json", -1);
  run_test("Step 5 fail5", "tests/step5/fail5.json", -1);
  run_test("Step 5 fail6", "tests/step5/fail6.json", -1);
  run_test("Step 5 fail7", "tests/step5/fail7.json", -1);
  run_test("Step 5 fail8", "tests/step5/fail8.json", -1);
  run_test("Step 5 fail9", "tests/step5/fail9.json", -1);
  run_test("Step 5 fail10", "tests/step5/fail10.json", -1);
  run_test("Step 5 fail11", "tests/step5/fail11.json", -1);
  run_test("Step 5 fail12", "tests/step5/fail12.json", -1);
  run_test("Step 5 fail13", "tests/step5/fail13.json", -1);
  run_test("Step 5 fail14", "tests/step5/fail14.json", -1);
  run_test("Step 5 fail15", "tests/step5/fail15.json", -1);
  run_test("Step 5 fail16", "tests/step5/fail16.json", -1);
  run_test("Step 5 fail17", "tests/step5/fail17.json", -1);
  run_test("Step 5 fail18", "tests/step5/fail18.json", -1);
  run_test("Step 5 fail19", "tests/step5/fail19.json", -1);
  run_test("Step 5 fail20", "tests/step5/fail20.json", -1);
  run_test("Step 5 fail21", "tests/step5/fail21.json", -1);
  run_test("Step 5 fail22", "tests/step5/fail22.json", -1);
  run_test("Step 5 fail23", "tests/step5/fail23.json", -1);
  run_test("Step 5 fail24", "tests/step5/fail24.json", -1);
  run_test("Step 5 fail25", "tests/step5/fail25.json", -1);
  run_test("Step 5 fail26", "tests/step5/fail26.json", -1);
  run_test("Step 5 fail27", "tests/step5/fail27.json", -1);
  run_test("Step 5 fail28", "tests/step5/fail28.json", -1);
  run_test("Step 5 fail29", "tests/step5/fail29.json", -1);
  run_test("Step 5 fail30", "tests/step5/fail30.json", -1);
  run_test("Step 5 fail31", "tests/step5/fail31.json", -1);
  run_test("Step 5 fail32", "tests/step5/fail32.json", -1);
  run_test("Step 5 fail33", "tests/step5/fail33.json", -1);

  return 0;
}

static void run_test(const char* testName, const char* jsonFilePath, const int expected) {
  printf("Running test %s on file %s\n...", testName, jsonFilePath);

  FILE* fp = fopen(jsonFilePath, "r");
  if (!fp) {
    fprintf(stderr, RED "run_test: failed to open file %s on test %s\n" RESET_COLOR, jsonFilePath, testName);
    return;
  }

  TokenStream* ts = Tokenize(fp);  // tokenization
  int actual = Parse(ts);          // parsing

  if (actual == expected) {
    printf(GREEN "Test %s on file %s passed.\n" RESET_COLOR, testName, jsonFilePath);
    fclose(fp);
  } else {
    fprintf(stderr, RED "Test %s on file %s FAILED. Expected %d, got %d!\n" RESET_COLOR, testName, jsonFilePath, expected, actual);
    fclose(fp);
    exit(-1);
  }
}
