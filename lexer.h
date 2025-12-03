#ifndef LEXER_H
#define LEXER_H
#include <stdio.h>

#include "token.h"

TokenStream* Tokenize(FILE* file);

#endif