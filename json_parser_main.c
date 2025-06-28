#include <stdio.h>
#include "lexer.h"
#include "parser.h"
#include "json_parser.h"
#include "color_codes.h"

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

    int res = ValidateJson(fp);

    if (res == 1) {
        printf(GREEN "%s is valid JSON.\n" RESET_COLOR, jsonFilePath);
    }
    else if (res == 0) {
        printf(RED "%s is NOT valid JSON.\n" RESET_COLOR, jsonFilePath);
    }
    else {
        fprintf(stderr, RED "Unknown error. Parser returned status code %d\n" RESET_COLOR, res);
        return -1;
    }

    return 0;
}
