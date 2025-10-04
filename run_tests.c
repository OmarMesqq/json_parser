#include <stdio.h>
#include "lexer.h"
#include "parser.h"
#include "json_parser.h"
#include "build_config.h"

static void run_test(const char* testName, const char* jsonFilePath, int expectedResult);

int main() {
    run_test("Step 1, valid JSON", "tests/step1/valid.json", 0);
    run_test("Step 1, invalid JSON", "tests/step1/invalid.json", -1);
    run_test("Step 2, valid JSON", "tests/step2/valid.json", 0);
    run_test("Step 2, invalid JSON", "tests/step2/invalid.json", -1);
    // run_test("Step 2, valid JSON 2", "tests/step2/valid2.json", 0);
    // run_test("Step 2, invalid JSON 2", "tests/step2/invalid2.json", 0);
    return 0;
}

static void run_test(const char* testName, const char* jsonFilePath, int expectedResult) {
    printf("Running test %s on file %s\n...", testName, jsonFilePath);
    
    FILE* fp = fopen(jsonFilePath, "r");
    if (!fp) {
        fprintf(stderr, RED "run_test: failed to open file %s on test %s\n" RESET_COLOR, jsonFilePath, testName);
        return;
    }
    
    int actualResult = ValidateJson(fp);

    if (actualResult == expectedResult) {
        printf(GREEN "Test %s on file %s passed.\n" RESET_COLOR, testName, jsonFilePath);
    } else {
        fprintf(stderr, RED "Test %s on file %s FAILED!\n" RESET_COLOR, testName, jsonFilePath);
    }

    fclose(fp);
}