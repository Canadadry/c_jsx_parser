#include <stdio.h>
#include <string.h>
#include "../src/lexer.h"
#include "minitest.h"

static int token_equal(Token a, Token b) {
    if (a.type != b.type) return 1;
    if (a.pos != b.pos) return 2;
    if (slice_equal(a.literal, a.literal)!=0) return 3;
    return 0;
}

typedef struct{
    const char *input;
    Token expected[20];
    int expected_len;
} LexerTestCase;


static inline void test_lexer_case(LexerTestCase tt) {
    Slice src = {tt.input, (int)strlen(tt.input)};
    Lexer lexer = NewLexer(src);

    for (int i = 0; i < tt.expected_len; i++) {
        Token actual = GetNextToken(&lexer);
        Token expected = tt.expected[i];
        int match = token_equal(actual, expected);
        if (match != 0) {
            TEST_ERRORF("test_lexer_case",
                "input=\"%s\", token[%d] mismatch %d\n expected: type=%d, literal(%d)=\"%.*s\","\
                " pos=%d\n   actual: type=%d, literal(%d)=\"%.*s\", pos=%d",
                tt.input, i,match,
                expected.type, expected.literal.len, expected.literal.len, expected.literal.start, expected.pos,
                actual.type, actual.literal.len,actual.literal.len, actual.literal.start, actual.pos
            );
        }
    }
}

 void test_lexer() {
    LexerTestCase tests[] = {
        {
            "<div>ok</div>",
            {
                {TOKEN_OPEN_TAG, {"<", 1}, 0},
                {TOKEN_IDENT, {"div", 3}, 1},
                {TOKEN_CLOSE_TAG, {">", 1}, 4},
                {TOKEN_TEXT, {"ok", 2}, 5},
                {TOKEN_OPEN_TAG, {"<", 1}, 7},
                {TOKEN_SLASH, {"/", 1}, 8},
                {TOKEN_IDENT, {"div", 3}, 9},
                {TOKEN_CLOSE_TAG, {">", 1}, 12},
                {TOKEN_EOF, {NULL, 0}, 13},
            },
            9
        },
        {
            "<button disabled>press</button>",
            {
                {TOKEN_OPEN_TAG, {"<", 1}, 0},
                {TOKEN_IDENT, {"button", 6}, 1},
                {TOKEN_IDENT, {"disabled", 8}, 8},
                {TOKEN_CLOSE_TAG, {">", 1}, 16},
                {TOKEN_TEXT, {"press", 5}, 17},
                {TOKEN_OPEN_TAG, {"<", 1}, 22},
                {TOKEN_SLASH, {"/", 1}, 23},
                {TOKEN_IDENT, {"button", 6}, 24},
                {TOKEN_CLOSE_TAG, {">", 1}, 30},
                {TOKEN_EOF, {NULL, 0}, 31},
            },
            10
        },
        {
            "<span onClick={handleClick}>X</span>",
            {
                {TOKEN_OPEN_TAG, {"<", 1}, 0},
                {TOKEN_IDENT, {"span", 4}, 1},
                {TOKEN_IDENT, {"onClick", 7}, 6},
                {TOKEN_EQUAL, {"=", 1}, 13},
                {TOKEN_EXPR, {"handleClick", 11}, 14},
                {TOKEN_CLOSE_TAG, {">", 1}, 27},
                {TOKEN_TEXT, {"X", 1}, 28},
                {TOKEN_OPEN_TAG, {"<", 1}, 29},
                {TOKEN_SLASH, {"/", 1}, 30},
                {TOKEN_IDENT, {"span", 4}, 31},
                {TOKEN_CLOSE_TAG, {">", 1}, 35},
                {TOKEN_EOF, {NULL, 0}, 36},
            },
            12
        },
        {
            "<button onClick={() => alert(\"hi\")}>Click me</button>",
            {
                {TOKEN_OPEN_TAG, {"<", 1}, 0},
                {TOKEN_IDENT, {"button", 6}, 1},
                {TOKEN_IDENT, {"onClick", 7}, 8},
                {TOKEN_EQUAL, {"=", 1}, 15},
                {TOKEN_EXPR, slice_from("() => alert(\"hi\")"), 16},
                {TOKEN_CLOSE_TAG, {">", 1}, 35},
                {TOKEN_TEXT, {"Click me", 8}, 36},
                {TOKEN_OPEN_TAG, {"<", 1}, 44},
                {TOKEN_SLASH, {"/", 1}, 45},
                {TOKEN_IDENT, {"button", 6}, 46},
                {TOKEN_CLOSE_TAG, {">", 1}, 52},
                {TOKEN_EOF, {NULL, 0}, 53},
            },
            12
        },
        {
            "<p>Hello world, this is JSX!</p>",
            {
                {TOKEN_OPEN_TAG, {"<", 1}, 0},
                {TOKEN_IDENT, {"p", 1}, 1},
                {TOKEN_CLOSE_TAG, {">", 1}, 2},
                {TOKEN_TEXT, slice_from("Hello world, this is JSX!"), 3},
                {TOKEN_OPEN_TAG, {"<", 1}, 28},
                {TOKEN_SLASH, {"/", 1}, 29},
                {TOKEN_IDENT, {"p", 1}, 30},
                {TOKEN_CLOSE_TAG, {">", 1}, 31},
                {TOKEN_EOF, {NULL, 0}, 32},
            },
            9
        },
        {
            "<span>{user.name + {count}}</span>",
            {
                {TOKEN_OPEN_TAG, {"<", 1}, 0},
                {TOKEN_IDENT, {"span", 4}, 1},
                {TOKEN_CLOSE_TAG, {">", 1}, 5},
                {TOKEN_EXPR, {"user.name + {count}", 19}, 6},
                {TOKEN_OPEN_TAG, {"<", 1}, 27},
                {TOKEN_SLASH, {"/", 1}, 28},
                {TOKEN_IDENT, {"span", 4}, 29},
                {TOKEN_CLOSE_TAG, {">", 1}, 33},
                {TOKEN_EOF, {NULL, 0}, 34},
            },
            9
        },
        {
            "<div>\n    Multi-line\n    text content\n</div>",
            {
                {TOKEN_OPEN_TAG, {"<", 1}, 0},
                {TOKEN_IDENT, {"div", 3}, 1},
                {TOKEN_CLOSE_TAG, {">", 1}, 4},
                {TOKEN_TEXT, slice_from("\n    Multi-line\n    text content\n"), 5},
                {TOKEN_OPEN_TAG, {"<", 1}, 38},
                {TOKEN_SLASH, {"/", 1}, 39},
                {TOKEN_IDENT, {"div", 3}, 40},
                {TOKEN_CLOSE_TAG, {">", 1}, 43},
                {TOKEN_EOF, {NULL, 0}, 44},
            },
            9
        },
    };

    for (int t = 0; t < sizeof(tests)/sizeof(tests[0]); t++) {
        mt_total++;
        test_lexer_case(tests[t]);
    }
}
