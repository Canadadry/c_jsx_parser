#ifndef JSX_LEXER_H
#define JSX_LEXER_H

#include "token.h"

typedef struct {
    Slice source;
    int current;
    int read;
    char ch;
    char inTag;
} Lexer;

Lexer NewLexer(Slice source);
Token GetNextToken(Lexer *lexer);

#endif // JSX_TOKEN_H
