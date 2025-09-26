#include "../src/parser.h"
#include "minitest.h"

#define BUF_CAPACITY 512
#define ARENA_SIZE 10

typedef struct {
    const char *input;
    ValueIndex (*gen_expected)(Arena* expected);
} ParserTestCase;

static inline void test_parser_case(ParserTestCase tt) {
    Arena arena ={0};
    Value values_arena[ARENA_SIZE] = {0};
    arena.values = values_arena;
    arena.values_capacity = ARENA_SIZE;
    Prop props_arena[ARENA_SIZE] = {0};
    arena.props = props_arena;
    arena.prop_capacity = ARENA_SIZE;
    Lexer lexer = NewLexer(slice_from(tt.input));
    Parser parser = (Parser){0};
    parser.lexer =&lexer;
    parser.arena=&arena;
    InitParser(&parser);

    ParseNodeResult result = ParseNode(&parser);
    if (result.type != OK) {
        TEST_ERRORF("test_parser_case","parsing %s failed at %d %s : %.*s got %s\n",
            tt.input,result.value.err.at,parser_error_to_string(result.value.err.code),
            5,tt.input+result.value.err.at,token_type_to_string(result.value.err.token)
        );
    }
    ValueIndex actual_idx =result.value.ok;

    Arena expected_arena ={0};
    Value expected_values[ARENA_SIZE]={0};
    expected_arena.values = expected_values;
    expected_arena.values_capacity = ARENA_SIZE;
    Prop expected_props[ARENA_SIZE]={0};
    expected_arena.props = expected_props;
    expected_arena.prop_capacity = ARENA_SIZE;

    ValueIndex expected_idx = tt.gen_expected(&expected_arena);
    if(expected_idx<0){
        TEST_ERRORF("test_parser_case", "cannot generate expected node\n");
    }

    if (!value_equal(&arena,actual_idx, &expected_arena,expected_idx)) {
        Buffer exptected_buffer ={0};
        char exptected_buf[BUF_CAPACITY] ={0};
        exptected_buffer.buf=exptected_buf;
        exptected_buffer.buf_capacity=BUF_CAPACITY;
        value_print(&exptected_buffer, &expected_arena,expected_idx, 0);

        Buffer got_buffer ={0};
        char got_buf[BUF_CAPACITY] ={0};
        got_buffer.buf=got_buf;
        got_buffer.buf_capacity=BUF_CAPACITY;
        value_print(&got_buffer, &arena,actual_idx, 0);

        TEST_ERRORF("test_parser_case",
            "parsing %s failed:\nexpected node:\n%s\ngot node:\n%s\n",
            tt.input,exptected_buf,got_buf);
    }
}

#define GEN_SIMPLE_TEXT_INPUT "<div>ok</div>"
ValueIndex GEN_SIMPLE_TEXT_FN(Arena* arena) {
    if (arena->values_capacity < 2) return -1;
    arena->values[0].type = NODE_NODE_TYPE;
    arena->values[0].next = -1;
    arena->values[0].value.node = (Node) {
        .Tag = slice_from("div"),
        .Props = -1,
        .Children = 1
    };
    arena->values[1].type = TEXT_NODE_TYPE;
    arena->values[1].value.text = slice_from("ok");
    arena->values[1].next = -1;
    return 0;
}

#define GEN_MULTIPLE_NODES_INPUT "<div><a></a><b></b></div>"
ValueIndex GEN_MULTIPLE_NODES_FN(Arena* arena) {
    if (arena->values_capacity < 3) return -1;
    arena->values[0].type = NODE_NODE_TYPE;
    arena->values[0].next = -1;
    arena->values[0].value.node = (Node) {
        .Tag = slice_from("div"),
        .Props = -1,
        .Children = 1,
    };
    arena->values[1].type = NODE_NODE_TYPE;
    arena->values[1].value.node = (Node) {
        .Tag = slice_from("a"),
        .Props = -1,
        .Children = 2
    };
    arena->values[2].type = NODE_NODE_TYPE;
    arena->values[2].value.node = (Node) {
        .Tag = slice_from("b"),
        .Props = -1,
        .Children = -1
    };
    return 0;
}

// Node* gen_button_disabled(Child* children_arena, size_t children_len, Prop* prop_arena, size_t prop_len) {
//     if (children_len < 2 || prop_len < 1) return NULL;
//     children_arena[0].type = NODE_NODE_TYPE;
//     children_arena[0].value.node = (Node) {
//         .Tag = slice_from("button"),
//         .Props = &prop_arena[0],
//         .Children = &children_arena[1]
//     };
//     children_arena[1].type = TEXT_NODE_TYPE;
//     children_arena[1].value.text = slice_from("press");
//     children_arena[1].next = NULL;
//     prop_arena[0] = (Prop) {
//         .key = slice_from("disabled"),
//         .value = slice_from("true"),
//         .type = EXPR_PROP_TYPE,
//     };

//     return &children_arena[0].value.node;
// }

// Node* gen_button_class(Child* children_arena, size_t children_len, Prop* prop_arena, size_t prop_len) {
//     if (children_len < 2 || prop_len < 1) return NULL;
//     children_arena[0].type = NODE_NODE_TYPE;
//     children_arena[0].value.node = (Node) {
//         .Tag = slice_from("button"),
//         .Props = &prop_arena[0],
//         .Children = &children_arena[1]
//     };
//     prop_arena[0] = (Prop) {
//         .key = slice_from("class"),
//         .value = slice_from("btn"),
//         .type = TEXT_PROP_TYPE,
//         .next = NULL
//     };
//     children_arena[1].type = TEXT_NODE_TYPE;
//     children_arena[1].value.text = slice_from("press");

//     return &children_arena[0].value.node;
// }

// Node* gen_span_onClick(Child* children_arena, size_t children_len, Prop* prop_arena, size_t prop_len) {
//     if (children_len < 2 || prop_len < 1) return NULL;
//     children_arena[0].type = TEXT_NODE_TYPE;
//     children_arena[0].value.text = slice_from("X");
//     children_arena[0].next = NULL;

//     prop_arena[0] = (Prop) {
//         .key = slice_from("onClick"),
//         .value = slice_from("handleClick"),
//         .type = EXPR_PROP_TYPE,
//         .next = NULL
//     };

//     children_arena[1].type = NODE_NODE_TYPE;
//     children_arena[1].value.node = (Node) {
//         .Tag = slice_from("span"),
//         .Props = &prop_arena[0],
//         .Children = &children_arena[0]
//     };
//     children_arena[1].next = NULL;

//     return &children_arena[1].value.node;
// }

// Node* gen_button_onClick(Child* children_arena, size_t children_len, Prop* prop_arena, size_t prop_len) {
//     if (children_len < 2 || prop_len < 1) return NULL;
//     children_arena[0].type = TEXT_NODE_TYPE;
//     children_arena[0].value.text = slice_from("Click me");
//     children_arena[0].next = NULL;

//     prop_arena[0] = (Prop) {
//         .key = slice_from("onClick"),
//         .value = slice_from("() => alert(\"hi\")"),
//         .type = EXPR_PROP_TYPE,
//         .next = NULL
//     };

//     children_arena[1].type = NODE_NODE_TYPE;
//     children_arena[1].value.node = (Node) {
//         .Tag = slice_from("button"),
//         .Props = &prop_arena[0],
//         .Children = &children_arena[0]
//     };
//     children_arena[1].next = NULL;

//     return &children_arena[1].value.node;
// }

// Node* gen_paragraph(Child* children_arena, size_t children_len, Prop* prop_arena, size_t prop_len) {
//     if (children_len < 2) return NULL;
//     children_arena[0].type = TEXT_NODE_TYPE;
//     children_arena[0].value.text = slice_from("Hello world, this is JSX!");
//     children_arena[0].next = NULL;

//     children_arena[1].type = NODE_NODE_TYPE;
//     children_arena[1].value.node = (Node) {
//         .Tag = slice_from("p"),
//         .Props = NULL,
//         .Children = &children_arena[0]
//     };
//     children_arena[1].next = NULL;

//     return &children_arena[1].value.node;
// }

// Node* gen_span_expr(Child* children_arena, size_t children_len, Prop* prop_arena, size_t prop_len) {
//     if (children_len < 2) return NULL;
//     children_arena[0].type = EXPR_NODE_TYPE;
//     children_arena[0].value.expr = slice_from("user.name + {count}");
//     children_arena[0].next = NULL;

//     children_arena[1].type = NODE_NODE_TYPE;
//     children_arena[1].value.node = (Node) {
//         .Tag = slice_from("span"),
//         .Props = NULL,
//         .Children = &children_arena[0]
//     };
//     children_arena[1].next = NULL;

//     return &children_arena[1].value.node;
// }

// Node* gen_multiline_text(Child* children_arena, size_t children_len, Prop* prop_arena, size_t prop_len) {
//     if (children_len < 2) return NULL;
//     children_arena[0].type = TEXT_NODE_TYPE;
//     children_arena[0].value.text = slice_from("\n    Multi-line\n    text content\n");
//     children_arena[0].next = NULL;

//     children_arena[1].type = NODE_NODE_TYPE;
//     children_arena[1].value.node = (Node) {
//         .Tag = slice_from("div"),
//         .Props = NULL,
//         .Children = &children_arena[0]
//     };
//     children_arena[1].next = NULL;

//     return &children_arena[1].value.node;
// }

// Node* gen_div_key_expr(Child* children_arena, size_t children_len, Prop* prop_arena, size_t prop_len) {
//     if (prop_len < 1) return NULL;
//     prop_arena[0] = (Prop) {
//         .key = slice_from("key"),
//         .value = slice_from("\"x\" + n"),
//         .type = EXPR_PROP_TYPE,
//         .next = NULL
//     };
//     children_arena[0].type = NODE_NODE_TYPE;
//     children_arena[0].value.node = (Node) {
//         .Tag = slice_from("div"),
//         .Props = &prop_arena[0],
//         .Children = NULL
//     };
//     children_arena[0].next = NULL;

//     return &children_arena[0].value.node;
// }
//

#define GEN_BLOCK(name)              \
    {                                \
        .input = GEN_##name##_INPUT, \
        .gen_expected = GEN_##name##_FN,   \
    }

void test_parser() {
    ParserTestCase tests[] = {
        GEN_BLOCK(SIMPLE_TEXT),
        // GEN_BLOCK(MULTIPLE_NODES),
        // {
        //     .input = "<div><a></a><b></b></div>",
        //     .gen_expected = gen_multiple_nodes,
        // },
        // {
        //     .input = "<button disabled>press</button>",
        //     .gen_expected = gen_button_disabled,
        // },
        // {
        //     .input = "<button class=\"btn\">press</button>",
        //     .gen_expected = gen_button_class,
        // },
        // {
        //     .input = "<span onClick={handleClick}>X</span>",
        //     .gen_expected = gen_span_onClick,
        // },
        // {
        //     .input = "<button onClick={() => alert(\"hi\")}>Click me</button>",
        //     .gen_expected = gen_button_onClick,
        // },
        // {
        //     .input = "<p>Hello world, this is JSX!</p>",
        //     .gen_expected = gen_paragraph,
        // },
        // {
        //     .input = "<span>{user.name + {count}}</span>",
        //     .gen_expected = gen_span_expr,
        // },
        // {
        //     .input = "<div>\n    Multi-line\n    text content\n</div>",
        //     .gen_expected = gen_multiline_text,
        // },
        // {
        //     .input = "<div key={\"x\" + n}></div>",
        //     .gen_expected = gen_div_key_expr,
        // }
    };

    int test_count = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < test_count; i++) {
        mt_total++;
        test_parser_case(tests[i]);
    }
}
