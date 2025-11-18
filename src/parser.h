#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include "token.h"
#include "arena.h"

typedef struct {
    Lexer *lexer;
    Token curTok;
    Token peekTok;
    Arena* arena;
} Parser;

typedef enum {
    PARSER_OK,
    PARSER_ERR_CHILDREN_UNEXPECTED_TOKEN,
    PARSER_ERR_CLOSING_UNEXPECTED_TOKEN_1,
    PARSER_ERR_CLOSING_UNEXPECTED_TOKEN_2,
    PARSER_ERR_CLOSING_UNEXPECTED_TOKEN_3,
    PARSER_ERR_EXPECTED_OPENTAG_NAME,
    PARSER_ERR_EXPECTED_OPENTAG,
    PARSER_ERR_EXPECTED_ENDTAG,
    PARSER_ERR_EXPECTED_PROP,
    PARSER_ERR_MEMORY_ALLOCATION
} ParseErrorCode;

typedef struct{
    ParseErrorCode code;
    int at;
    TokenType token;
    Slice litt;
}Error;

const char* parser_error_to_string(ParseErrorCode err);

typedef enum{NONE,OK,ERR} ResultType;

typedef struct {
    ResultType type;
    union {
        ValueIndex ok;
        Error err;
    } value;
} ParseNodeResult;

void InitParser(Parser *parser);
ParseNodeResult ParseNode(Parser* p);

#endif
