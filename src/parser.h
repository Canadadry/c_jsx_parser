#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

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
    PARSER_ERR_UNEXPECTED_TOKEN,
    PARSER_ERR_EXPECTED_TAG,
    PARSER_ERR_EXPECTED_PROP,
    PARSER_ERR_MEMORY_ALLOCATION
} ParseErrorCode;


typedef enum{OK,ERR} ResultType;

typedef struct {
    ResultType type;
    union {
        Node* ok;
        ParseErrorCode err;
    } value;
} ParseNodeResult;

void InitParser(Parser *parser);
ParseNodeResult ParseNode(Parser* p);

#endif
