#include <stdio.h>
#include "lexer.h"
#include "parser.h"
#include "json_parser.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: ./json_parser <filename.json>\n");
        return -1;
    }
    return 0;
}


int ValidateJson(FILE* jsonFilePtr) {

}