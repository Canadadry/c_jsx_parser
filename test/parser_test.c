#include <stdio.h>
#include <string.h>
#include "../src/parser.h"
#include "minitest.h"


Parser NewParser(Lexer *lexer) {
    Parser parser = {
        .lexer = lexer,
        .props = NULL,
        .prop_count = 0,
        .prop_capacity = 0,
        .children = NULL,
        .child_count = 0,
        .child_capacity = 0,
        .realloc_fn = NULL,
        .userdata = NULL
    };

    InitParser(&parser);

    return parser;
}

#include <stdio.h>
#include <string.h>
#include "../src/parser.h"
#include "minitest.h"

typedef struct {
    const char *input;
    Node expected;
} ParserTestCase;

static inline void test_parser_case(ParserTestCase tt) {
    Lexer lexer = NewLexer(slice_from(tt.input));
    Parser parser = NewParser(&lexer);

    Node actual;
    ParseNodeResult result = ParseNode(&parser, &actual);
    if (result.type != OK) {
        TEST_ERRORF("parser", "input=\"%s\": unexpected parsing error: %d\n", tt.input, result.value.err);
        return;
    }

    if (!node_equal(&actual, &tt.expected)) {
        TEST_ERRORF("parser", "input=\"%s\" mismatch\n expected: %#v\n actual: %#v\n", tt.input, &tt.expected, &actual);
    }
}

void test_parser() {
    ParserTestCase tests[] = {
        {
            .input = "<div>ok</div>",
            .expected = (Node) {
                .Tag = slice_from("div"),
                .Props = NULL,
                .Children = (Child[]) {
                    node_to_child((Node) {
                        .Tag = slice_from("ok"),
                        .Props = NULL,
                        .Children = NULL
                    })
                }
            }
        },
        {
            .input = "<button disabled>press</button>",
            .expected = (Node) {
                .Tag = slice_from("button"),
                .Props = (Prop[]){{.key = slice_from("disabled"), .value = slice_from("true")}},
                .Children = (Child[]) {
                    node_to_child((Node) {
                        .Tag = slice_from("press"),
                        .Props = NULL,
                        .Children = NULL
                    })
                }
            }
        },
        {
            .input = "<button class=\"btn\">press</button>",
            .expected = (Node) {
                .Tag = slice_from("button"),
                .Props = (Prop[]){{.key = slice_from("class"), .value = slice_from("\"btn\"")}},
                .Children = (Child[]) {
                    node_to_child((Node) {
                        .Tag = slice_from("press"),
                        .Props = NULL,
                        .Children = NULL
                    })
                }
            }
        },
        {
            .input = "<span onClick={handleClick}>X</span>",
            .expected = (Node) {
                .Tag = slice_from("span"),
                .Props = (Prop[]){{.key = slice_from("onClick"), .value = slice_from("handleClick")}},
                .Children = (Child[]) {
                    node_to_child((Node) {
                        .Tag = slice_from("X"),
                        .Props = NULL,
                        .Children = NULL
                    })
                }
            }
        },
        {
            .input = "<button onClick={() => alert(\"hi\")}>Click me</button>",
            .expected = (Node) {
                .Tag = slice_from("button"),
                .Props = (Prop[]){{.key = slice_from("onClick"), .value = slice_from("() => alert(\"hi\")")}},
                .Children = (Child[]) {
                    node_to_child((Node) {
                        .Tag = slice_from("Click me"),
                        .Props = NULL,
                        .Children = NULL
                    })
                }
            }
        },
        {
            .input = "<p>Hello world, this is JSX!</p>",
            .expected = (Node) {
                .Tag = slice_from("p"),
                .Props = NULL,
                .Children = (Child[]) {
                    node_to_child((Node) {
                        .Tag = slice_from("Hello world, this is JSX!"),
                        .Props = NULL,
                        .Children = NULL
                    })
                }
            }
        },
        {
            .input = "<span>{user.name + {count}}</span>",
            .expected = (Node) {
                .Tag = slice_from("span"),
                .Props = NULL,
                .Children = (Child[]) {
                    expr_to_child(slice_from("user.name + {count}"))
                }
            }
        },
        {
            .input = "<div>\n    Multi-line\n    text content\n</div>",
            .expected = (Node) {
                .Tag = slice_from("div"),
                .Props = NULL,
                .Children = (Child[]) {
                    node_to_child((Node) {
                        .Tag = slice_from("\n    Multi-line\n    text content\n"),
                        .Props = NULL,
                        .Children = NULL
                    })
                }
            }
        },
        {
            .input = "<div key={\"x\" + n}></div>",
            .expected = (Node) {
                .Tag = slice_from("div"),
                .Props = (Prop[]){{.key = slice_from("key"), .value = slice_from("\"x\" + n")}},
                .Children = NULL
            }
        }
    };

    int test_count = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < test_count; i++) {
        test_parser_case(tests[i]);
    }
}
