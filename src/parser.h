#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include "token.h"

typedef struct {
    Lexer *lexer;
    Token curTok;
    Token peekTok;
    Prop *props;
    size_t prop_count;
    size_t prop_capacity;
    Child *children;
    size_t child_count;
    size_t child_capacity;
    void* (*realloc_fn)(void* userdata,void* ptr, size_t size);
    void* userdata;
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
}Error;

const char* parser_error_to_string(ParseErrorCode err);

typedef enum{OK,ERR} ResultType;

typedef struct {
    ResultType type;
    union {
        Node* ok;
        Error err;
    } value;
} ParseNodeResult;

void InitParser(Parser *parser);
ParseNodeResult ParseNode(Parser* p);

#endif
