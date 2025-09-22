#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/parser.h"
#include "minitest.h"

#define BUF_CAPACITY 512

Child node_to_child(Node child) {
    Child new_child;
    new_child.type = NODE_TYPE;
    new_child.value.node = child;
    new_child.next = NULL;
    return new_child;
}

Child text_to_child(Slice text) {
    Child new_child;
    new_child.type = TEXT_TYPE;
    new_child.value.text = text;
    new_child.next = NULL;
    return new_child;
}

Child expr_to_child(Slice expr) {
    Child new_child;
    new_child.type = EXPR_TYPE;
    new_child.value.expr = expr;
    new_child.next = NULL;
    return new_child;
}

typedef struct {
    const char *input;
    Node* (*gen_expected)(Child* children_arena,size_t children_len,Prop* prop_arena,size_t prop_len);
} ParserTestCase;

#define ARENA_SIZE 10


static inline void test_parser_case(ParserTestCase tt) {
    Lexer lexer = NewLexer(slice_from(tt.input));
    Parser parser = (Parser){0};
    parser.lexer =&lexer;
    InitParser(&parser);
    Child children_arena[ARENA_SIZE] = {0};
    parser.children = children_arena;
    parser.child_capacity = ARENA_SIZE;
    Prop     props_arena[ARENA_SIZE] = {0};
    parser.props = props_arena;
    parser.prop_capacity = ARENA_SIZE;

    ParseNodeResult result = ParseNode(&parser);
    if (result.type != OK) {
        TEST_ERRORF("parsing %s failed: %d\n", tt.input,result.value.err);
    }
    Node* actual = result.value.ok;

    if (!node_equal(actual, &tt.expected)) {
        Printer exptected_printer ={0};
        char exptected_buf[BUF_CAPACITY] ={0};
        exptected_printer.buf=exptected_buf;
        exptected_printer.buf_capacity=BUF_CAPACITY;
        node_print(&exptected_printer, &tt.expected, 0);

        Printer got_printer ={0};
        char got_buf[BUF_CAPACITY] ={0};
        got_printer.buf=got_buf;
        got_printer.buf_capacity=BUF_CAPACITY;
        node_print(&got_printer, actual, 0);

        TEST_ERRORF("test_parser_case",
            "parsing %s failed:\nexpected node:\n%sexpected node:\n%sexpected node:\n",
            tt.input,exptected_buf,got_buf);
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
            .input = "<div><a></a><b></b></div>",
            .expected = (Node) {
                .Tag = slice_from("div"),
                .Props = NULL,
                .Children = (Child[]) {
                    node_to_child((Node){.Tag=slice_from("a")}),
                    node_to_child((Node){.Tag=slice_from("b")}),
                }
            }
        },
        {
            .input = "<button disabled>press</button>",
            .expected = (Node) {
                .Tag = slice_from("button"),
                .Props = (Prop[]){{.key = slice_from("disabled"), .value = slice_from("true")}},
                .Children = (Child[]) {
                    text_to_child(slice_from("press"))
                }
            }
        },
        {
            .input = "<button class=\"btn\">press</button>",
            .expected = (Node) {
                .Tag = slice_from("button"),
                .Props = (Prop[]){{.key = slice_from("class"), .value = slice_from("\"btn\"")}},
                .Children = (Child[]) {
                    text_to_child(slice_from("press"))
                }
            }
        },
        {
            .input = "<span onClick={handleClick}>X</span>",
            .expected = (Node) {
                .Tag = slice_from("span"),
                .Props = (Prop[]){{.key = slice_from("onClick"), .value = slice_from("handleClick")}},
                .Children = (Child[]) {
                    text_to_child(slice_from("X"))
                }
            }
        },
        {
            .input = "<button onClick={() => alert(\"hi\")}>Click me</button>",
            .expected = (Node) {
                .Tag = slice_from("button"),
                .Props = (Prop[]){{.key = slice_from("onClick"), .value = slice_from("() => alert(\"hi\")")}},
                .Children = (Child[]) {
                    text_to_child(slice_from("Click me"))
                }
            }
        },
        {
            .input = "<p>Hello world, this is JSX!</p>",
            .expected = (Node) {
                .Tag = slice_from("p"),
                .Props = NULL,
                .Children = (Child[]) {
                    text_to_child(slice_from("Hello world, this is JSX!"))
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
                    text_to_child(slice_from("\n    Multi-line\n    text content\n"))
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
        mt_total++;
        test_parser_case(tests[i]);
    }
}
