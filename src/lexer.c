#include <stdio.h>
#include "lexer.h"

static char isWhiteSpace(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

static char isIdentStart(char ch) {
    return (ch >= 'a' && ch <= 'z') ||
           (ch >= 'A' && ch <= 'Z') ||
           ch == '_' || ch == '$';
}

static char isIdentPart(char ch) {
    return isIdentStart(ch) || (ch >= '0' && ch <= '9') || ch == '-';
}

static void readChar(Lexer *l) {
    if (l->read >= l->source.len) {
        l->ch = 0;
    } else {
        l->ch = l->source.start[l->read];
    }
    l->current = l->read;
    l->read++;
}

static Token readExpression(Lexer *l) {
    int start = l->current;
    readChar(l);
    int exprStart = l->current;
    int depth = 1;
    while (l->ch != 0 && depth > 0) {
        if (l->ch == '{') {
            depth++;
        } else if (l->ch == '}') {
            depth--;
            if (depth == 0) break;
        }
        readChar(l);
    }
    Slice literal;
    literal.start = l->source.start + exprStart;
    literal.len = l->current - exprStart;
    readChar(l);

    Token tok = {TOKEN_EXPR, literal, start};
    return tok;
}

static Token readString(Lexer *l) {
    char quote = l->ch;
    int start = l->current + 1;
    readChar(l);
    while (l->ch != 0 && l->ch != quote) {
        readChar(l);
    }
    Slice literal;
    literal.start = l->source.start + start;
    literal.len = l->current - start;
    Token tok = {TOKEN_STRING, literal, start};
    if (l->ch == quote) {
        readChar(l);
    }
    return tok;
}

static Token getNextTokenInTag(Lexer *l) {
    while (isWhiteSpace(l->ch)) {
        readChar(l);
    }

    Token tok;
    tok.type = TOKEN_ILLEGAL;
    tok.literal.start = l->source.start + l->current;
    tok.literal.len = 1;
    tok.pos = l->current;

    switch (l->ch) {
        case 0:
            tok.type = TOKEN_EOF;
            tok.literal.len = 0;
            break;
        case '<':
            readChar(l);
            tok.type = TOKEN_OPEN_TAG;
            l->inTag = 1;
            break;
        case '>':
            l->inTag = 0;
            readChar(l);
            tok.type = TOKEN_CLOSE_TAG;
            break;
        case '/':
            readChar(l);
            tok.type = TOKEN_SLASH;
            break;
        case '=':
            readChar(l);
            tok.type = TOKEN_EQUAL;
            break;
        case '{':
            return readExpression(l);
        case '"':
        case '\'':
            return readString(l);
        default:
            if (isIdentStart(l->ch)) {
                int start = l->current;
                while (isIdentPart(l->ch)) {
                    readChar(l);
                }
                tok.type = TOKEN_IDENT;
                tok.literal.start = l->source.start + start;
                tok.literal.len = l->current - start;
                tok.pos = start;
            } else {
                readChar(l);
            }
    }

    return tok;
}

static Token getNextTokenOutTag(Lexer *l) {
    if (l->ch == 0) {
        Token tok = {TOKEN_EOF, {NULL, 0}, l->current};
        return tok;
    }
    if (l->ch == '<' || l->ch == '{') {
        return getNextTokenInTag(l);
    }

    int start = l->current;
    while (l->ch != 0 && l->ch != '<' && l->ch != '{') {
        readChar(l);
    }
    Slice literal;
    literal.start = l->source.start + start;
    literal.len = l->current - start;

    Token tok = {TOKEN_TEXT, literal, start};
    return tok;
}

Lexer NewLexer(Slice source) {
    Lexer l;
    l.source = source;
    l.current = 0;
    l.read = 0;
    l.ch = 0;
    l.inTag = 0;
    readChar(&l);
    return l;
}

Token GetNextToken(Lexer *l) {
    if (l->inTag) {
        return getNextTokenInTag(l);
    } else {
        return getNextTokenOutTag(l);
    }
}
