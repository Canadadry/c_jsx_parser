#include <stdio.h>
#include <stdlib.h>
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

typedef struct {
    const char *input;
    Node expected;
} ParserTestCase;

#define ARENA_SIZE 10


static inline void test_parser_case(ParserTestCase tt) {
    Lexer lexer = NewLexer(slice_from(tt.input));
    Parser parser = NewParser(&lexer);
    Child children_arena[ARENA_SIZE] = {0};
    parser.children = children_arena;
    parser.child_capacity = ARENA_SIZE;
    Prop     props_arena[ARENA_SIZE] = {0};
    parser.props = props_arena;
    parser.prop_capacity = ARENA_SIZE;

    ParseNodeResult result = ParseNode(&parser);
    if (result.type != OK) {
        printf("parsing %s failed: %d\n", tt.input,result.value.err);
        return;
    }
    Node* actual = result.value.ok;

    if (!node_equal(actual, &tt.expected)) {
        printf("parsing %s failed:\n", tt.input);
        printf("expected node:\n");
        node_print(&tt.expected,0);
        printf("actual node:\n");
        node_print(actual,0);
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
                    text_to_child(slice_from("ok"))
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
        // {
        //     .input = "<button class=\"btn\">press</button>",
        //     .expected = (Node) {
        //         .Tag = slice_from("button"),
        //         .Props = (Prop[]){{.key = slice_from("class"), .value = slice_from("\"btn\"")}},
        //         .Children = (Child[]) {
        //             node_to_child((Node) {
        //                 .Tag = slice_from("press"),
        //                 .Props = NULL,
        //                 .Children = NULL
        //             })
        //         }
        //     }
        // },
        // {
        //     .input = "<span onClick={handleClick}>X</span>",
        //     .expected = (Node) {
        //         .Tag = slice_from("span"),
        //         .Props = (Prop[]){{.key = slice_from("onClick"), .value = slice_from("handleClick")}},
        //         .Children = (Child[]) {
        //             node_to_child((Node) {
        //                 .Tag = slice_from("X"),
        //                 .Props = NULL,
        //                 .Children = NULL
        //             })
        //         }
        //     }
        // },
        // {
        //     .input = "<button onClick={() => alert(\"hi\")}>Click me</button>",
        //     .expected = (Node) {
        //         .Tag = slice_from("button"),
        //         .Props = (Prop[]){{.key = slice_from("onClick"), .value = slice_from("() => alert(\"hi\")")}},
        //         .Children = (Child[]) {
        //             node_to_child((Node) {
        //                 .Tag = slice_from("Click me"),
        //                 .Props = NULL,
        //                 .Children = NULL
        //             })
        //         }
        //     }
        // },
        // {
        //     .input = "<p>Hello world, this is JSX!</p>",
        //     .expected = (Node) {
        //         .Tag = slice_from("p"),
        //         .Props = NULL,
        //         .Children = (Child[]) {
        //             node_to_child((Node) {
        //                 .Tag = slice_from("Hello world, this is JSX!"),
        //                 .Props = NULL,
        //                 .Children = NULL
        //             })
        //         }
        //     }
        // },
        // {
        //     .input = "<span>{user.name + {count}}</span>",
        //     .expected = (Node) {
        //         .Tag = slice_from("span"),
        //         .Props = NULL,
        //         .Children = (Child[]) {
        //             expr_to_child(slice_from("user.name + {count}"))
        //         }
        //     }
        // },
        // {
        //     .input = "<div>\n    Multi-line\n    text content\n</div>",
        //     .expected = (Node) {
        //         .Tag = slice_from("div"),
        //         .Props = NULL,
        //         .Children = (Child[]) {
        //             node_to_child((Node) {
        //                 .Tag = slice_from("\n    Multi-line\n    text content\n"),
        //                 .Props = NULL,
        //                 .Children = NULL
        //             })
        //         }
        //     }
        // },
        // {
        //     .input = "<div key={\"x\" + n}></div>",
        //     .expected = (Node) {
        //         .Tag = slice_from("div"),
        //         .Props = (Prop[]){{.key = slice_from("key"), .value = slice_from("\"x\" + n")}},
        //         .Children = NULL
        //     }
        // }
    };

    int test_count = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < test_count; i++) {
        test_parser_case(tests[i]);
    }
}
