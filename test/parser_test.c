#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/parser.h"
#include "minitest.h"

#define BUF_CAPACITY 512
#define ARENA_SIZE 10

typedef struct {
    const char *input;
    Node* (*gen_expected)(Child* children_arena,size_t children_len,Prop* prop_arena,size_t prop_len);
} ParserTestCase;

static inline void test_parser_case(ParserTestCase tt) {
    Lexer lexer = NewLexer(slice_from(tt.input));
    Parser parser = (Parser){0};
    parser.lexer =&lexer;
    InitParser(&parser);
    Child children_arena[ARENA_SIZE] = {0};
    parser.children = children_arena;
    parser.child_capacity = ARENA_SIZE;
    Prop props_arena[ARENA_SIZE] = {0};
    parser.props = props_arena;
    parser.prop_capacity = ARENA_SIZE;

    ParseNodeResult result = ParseNode(&parser);
    if (result.type != OK) {
        TEST_ERRORF("test_parser_case","parsing %s failed: %d\n", tt.input,result.value.err);
    }
    Node* actual = result.value.ok;

    Child expected_child[ARENA_SIZE]={0};
    Prop expected_props[ARENA_SIZE]={0};
    Node* expected = tt.gen_expected(expected_child,ARENA_SIZE,expected_props,ARENA_SIZE);
    if(expected==NULL){
        TEST_ERRORF("test_parser_case", "cannot generate expected node\n");
    }

    if (!node_equal(actual, expected)) {
        Printer exptected_printer ={0};
        char exptected_buf[BUF_CAPACITY] ={0};
        exptected_printer.buf=exptected_buf;
        exptected_printer.buf_capacity=BUF_CAPACITY;
        node_print(&exptected_printer, expected, 0);

        Printer got_printer ={0};
        char got_buf[BUF_CAPACITY] ={0};
        got_printer.buf=got_buf;
        got_printer.buf_capacity=BUF_CAPACITY;
        node_print(&got_printer, actual, 0);

        TEST_ERRORF("test_parser_case",
            "parsing %s failed:\nexpected node:\n%s\ngot node:\n%s\n",
            tt.input,exptected_buf,got_buf);
    }
}

Node* gen_simple_text(Child* children_arena, size_t children_len, Prop* prop_arena, size_t prop_len) {
    if (children_len < 2) return NULL;
    children_arena[0].type = NODE_NODE_TYPE;
    children_arena[0].value.node = (Node) {
        .Tag = slice_from("div"),
        .Props = NULL,
        .Children = &children_arena[1]
    };
    children_arena[1].type = TEXT_NODE_TYPE;
    children_arena[1].value.text = slice_from("ok");
    children_arena[1].next = NULL;
    return &children_arena[0].value.node;
}

Node* gen_multiple_nodes(Child* children_arena, size_t children_len, Prop* prop_arena, size_t prop_len) {
    if (children_len < 3) return NULL;
    children_arena[0].type = NODE_NODE_TYPE;
    children_arena[0].value.node = (Node) {
        .Tag = slice_from("div"),
        .Props = NULL,
        .Children = &children_arena[1]
    };
    children_arena[1].type = NODE_NODE_TYPE;
    children_arena[1].value.node = (Node) {
        .Tag = slice_from("a"),
        .Props = NULL,
        .Children = NULL
    };
    children_arena[1].next = &children_arena[2];
    children_arena[2].type = NODE_NODE_TYPE;
    children_arena[2].value.node = (Node) {
        .Tag = slice_from("b"),
        .Props = NULL,
        .Children = NULL
    };
    return &children_arena[0].value.node;
}

Node* gen_button_disabled(Child* children_arena, size_t children_len, Prop* prop_arena, size_t prop_len) {
    if (children_len < 2 || prop_len < 1) return NULL;
    children_arena[0].type = NODE_NODE_TYPE;
    children_arena[0].value.node = (Node) {
        .Tag = slice_from("button"),
        .Props = &prop_arena[0],
        .Children = &children_arena[1]
    };
    children_arena[1].type = TEXT_NODE_TYPE;
    children_arena[1].value.text = slice_from("press");
    children_arena[1].next = NULL;
    prop_arena[0] = (Prop) {
        .key = slice_from("disabled"),
        .value = slice_from("true"),
        .type = EXPR_PROP_TYPE,
    };

    return &children_arena[0].value.node;
}

Node* gen_button_class(Child* children_arena, size_t children_len, Prop* prop_arena, size_t prop_len) {
    if (children_len < 2 || prop_len < 1) return NULL;
    children_arena[0].type = NODE_NODE_TYPE;
    children_arena[0].value.node = (Node) {
        .Tag = slice_from("button"),
        .Props = &prop_arena[0],
        .Children = &children_arena[1]
    };
    prop_arena[0] = (Prop) {
        .key = slice_from("class"),
        .value = slice_from("btn"),
        .type = TEXT_PROP_TYPE,
        .next = NULL
    };
    children_arena[1].type = TEXT_NODE_TYPE;
    children_arena[1].value.text = slice_from("press");

    return &children_arena[0].value.node;
}

Node* gen_span_onClick(Child* children_arena, size_t children_len, Prop* prop_arena, size_t prop_len) {
    if (children_len < 2 || prop_len < 1) return NULL;
    children_arena[0].type = TEXT_NODE_TYPE;
    children_arena[0].value.text = slice_from("X");
    children_arena[0].next = NULL;

    prop_arena[0] = (Prop) {
        .key = slice_from("onClick"),
        .value = slice_from("handleClick"),
        .type = EXPR_PROP_TYPE,
        .next = NULL
    };

    children_arena[1].type = NODE_NODE_TYPE;
    children_arena[1].value.node = (Node) {
        .Tag = slice_from("span"),
        .Props = &prop_arena[0],
        .Children = &children_arena[0]
    };
    children_arena[1].next = NULL;

    return &children_arena[1].value.node;
}

Node* gen_button_onClick(Child* children_arena, size_t children_len, Prop* prop_arena, size_t prop_len) {
    if (children_len < 2 || prop_len < 1) return NULL;
    children_arena[0].type = TEXT_NODE_TYPE;
    children_arena[0].value.text = slice_from("Click me");
    children_arena[0].next = NULL;

    prop_arena[0] = (Prop) {
        .key = slice_from("onClick"),
        .value = slice_from("() => alert(\"hi\")"),
        .type = EXPR_PROP_TYPE,
        .next = NULL
    };

    children_arena[1].type = NODE_NODE_TYPE;
    children_arena[1].value.node = (Node) {
        .Tag = slice_from("button"),
        .Props = &prop_arena[0],
        .Children = &children_arena[0]
    };
    children_arena[1].next = NULL;

    return &children_arena[1].value.node;
}

Node* gen_paragraph(Child* children_arena, size_t children_len, Prop* prop_arena, size_t prop_len) {
    if (children_len < 2) return NULL;
    children_arena[0].type = TEXT_NODE_TYPE;
    children_arena[0].value.text = slice_from("Hello world, this is JSX!");
    children_arena[0].next = NULL;

    children_arena[1].type = NODE_NODE_TYPE;
    children_arena[1].value.node = (Node) {
        .Tag = slice_from("p"),
        .Props = NULL,
        .Children = &children_arena[0]
    };
    children_arena[1].next = NULL;

    return &children_arena[1].value.node;
}

Node* gen_span_expr(Child* children_arena, size_t children_len, Prop* prop_arena, size_t prop_len) {
    if (children_len < 2) return NULL;
    children_arena[0].type = EXPR_NODE_TYPE;
    children_arena[0].value.expr = slice_from("user.name + {count}");
    children_arena[0].next = NULL;

    children_arena[1].type = NODE_NODE_TYPE;
    children_arena[1].value.node = (Node) {
        .Tag = slice_from("span"),
        .Props = NULL,
        .Children = &children_arena[0]
    };
    children_arena[1].next = NULL;

    return &children_arena[1].value.node;
}

Node* gen_multiline_text(Child* children_arena, size_t children_len, Prop* prop_arena, size_t prop_len) {
    if (children_len < 2) return NULL;
    children_arena[0].type = TEXT_NODE_TYPE;
    children_arena[0].value.text = slice_from("\n    Multi-line\n    text content\n");
    children_arena[0].next = NULL;

    children_arena[1].type = NODE_NODE_TYPE;
    children_arena[1].value.node = (Node) {
        .Tag = slice_from("div"),
        .Props = NULL,
        .Children = &children_arena[0]
    };
    children_arena[1].next = NULL;

    return &children_arena[1].value.node;
}

Node* gen_div_key_expr(Child* children_arena, size_t children_len, Prop* prop_arena, size_t prop_len) {
    if (prop_len < 1) return NULL;
    prop_arena[0] = (Prop) {
        .key = slice_from("key"),
        .value = slice_from("\"x\" + n"),
        .type = EXPR_PROP_TYPE,
        .next = NULL
    };
    children_arena[0].type = NODE_NODE_TYPE;
    children_arena[0].value.node = (Node) {
        .Tag = slice_from("div"),
        .Props = &prop_arena[0],
        .Children = NULL
    };
    children_arena[0].next = NULL;

    return &children_arena[0].value.node;
}

void test_parser() {
    ParserTestCase tests[] = {
        {
            .input = "<div>ok</div>",
            .gen_expected = gen_simple_text,
        },
        {
            .input = "<div><a></a><b></b></div>",
            .gen_expected = gen_multiple_nodes,
        },
        {
            .input = "<button disabled>press</button>",
            .gen_expected = gen_button_disabled,
        },
        {
            .input = "<button class=\"btn\">press</button>",
            .gen_expected = gen_button_class,
        },
        {
            .input = "<span onClick={handleClick}>X</span>",
            .gen_expected = gen_span_onClick,
        },
        {
            .input = "<button onClick={() => alert(\"hi\")}>Click me</button>",
            .gen_expected = gen_button_onClick,
        },
        {
            .input = "<p>Hello world, this is JSX!</p>",
            .gen_expected = gen_paragraph,
        },
        {
            .input = "<span>{user.name + {count}}</span>",
            .gen_expected = gen_span_expr,
        },
        {
            .input = "<div>\n    Multi-line\n    text content\n</div>",
            .gen_expected = gen_multiline_text,
        },
        {
            .input = "<div key={\"x\" + n}></div>",
            .gen_expected = gen_div_key_expr,
        }
    };

    int test_count = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < test_count; i++) {
        mt_total++;
        test_parser_case(tests[i]);
    }
}
