#ifndef JSX_TOKEN_H
#define JSX_TOKEN_H

typedef enum {
    TOKEN_ILLEGAL,
    TOKEN_EOF,
    TOKEN_OPEN_TAG,   // <
    TOKEN_CLOSE_TAG,  // >
    TOKEN_SLASH,      // /
    TOKEN_EQUAL,      // =
    TOKEN_IDENT,      // tag or prop name
    TOKEN_STRING,     // "foo"
    TOKEN_TEXT,       // raw text between tags
    TOKEN_EXPR        // JS expr inside { ... }
} TokenType;

typedef struct{
    const char* start;
    int len;
} Slice;

Slice slice_from(const char* str);

typedef struct {
    TokenType type;
    Slice literal;
    int pos;
} Token;

#endif // JSX_TOKEN_H
