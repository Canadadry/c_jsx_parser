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

const char* token_type_to_string(TokenType type);

typedef struct{
    const char* start;
    int len;
} Slice;

Slice slice_from(const char* str);
int slice_equal(Slice left, Slice right);

typedef struct {
    TokenType type;
    Slice literal;
    int pos;
} Token;

#endif // JSX_TOKEN_H
