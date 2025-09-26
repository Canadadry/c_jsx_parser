#include "token.h"

const char* token_type_to_string(TokenType type){
    switch(type){
        case TOKEN_ILLEGAL: return "token_illegal";
        case TOKEN_EOF: return "token_eof";
        case TOKEN_OPEN_TAG: return "token_open_tag";
        case TOKEN_CLOSE_TAG: return "token_close_tag";
        case TOKEN_SLASH: return "token_slash";
        case TOKEN_EQUAL: return "token_equal";
        case TOKEN_IDENT: return "token_ident";
        case TOKEN_STRING: return "token_string";
        case TOKEN_TEXT: return "token_text";
        case TOKEN_EXPR: return "token_expr";
    }
    return "unknown token type";
}
