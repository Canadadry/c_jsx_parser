#include "../src/parser.h"
#include "minitest.h"
#include "realloc_test.h"


#define BUF_CAPACITY 512
#define ARENA_SIZE 10

typedef struct {
    const char *input;
    ValueIndex (*gen_expected)(Arena* expected);
} ParserTestCase;

static inline void test_parser_case(ParserTestCase tt) {
    Arena arena ={0};
    arena.allocator.realloc_fn =fn_realloc;
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
        if(arena.values != NULL){
            free(arena.values);
        }
        if(arena.props != NULL){
            free(arena.props);
        }
        return;
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
        if(arena.values != NULL){
            free(arena.values);
        }
        if(arena.props != NULL){
            free(arena.props);
        }
        return;
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

    if(arena.values != NULL){
        free(arena.values);
    }
    if(arena.props != NULL){
        free(arena.props);
    }
}

static inline void test_parser_case_realloc(ParserTestCase tt) {
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
    arena->values_count=2;
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
    arena->values_count=3;
    arena->values[0].type = NODE_NODE_TYPE;
    arena->values[0].next = -1;
    arena->values[0].value.node = (Node) {
        .Tag = slice_from("div"),
        .Props = -1,
        .Children = 1,
    };
    arena->values[1].type = NODE_NODE_TYPE;
    arena->values[1].next = 2;
    arena->values[1].value.node = (Node) {
        .Tag = slice_from("a"),
        .Props = -1,
        .Children = -1
    };
    arena->values[2].type = NODE_NODE_TYPE;
    arena->values[2].next = -1;
    arena->values[2].value.node = (Node) {
        .Tag = slice_from("b"),
        .Props = -1,
        .Children = -1
    };
    return 0;
}

#define GEN_BUTTON_DISABLED_INPUT "<button disabled>press</button>"
ValueIndex GEN_BUTTON_DISABLED_FN(Arena* arena) {
    if (arena->values_capacity < 2 || arena->prop_capacity < 1) return -1;
    arena->values_count=2;
    arena->prop_count=1;
    arena->values[0].type = NODE_NODE_TYPE;
    arena->values[0].next = -1;
    arena->values[0].value.node = (Node) {
        .Tag = slice_from("button"),
        .Props = 0,
        .Children = 1,
    };
    arena->values[1].type = TEXT_NODE_TYPE;
    arena->values[1].value.text = slice_from("press");
    arena->values[1].next = -1;
    arena->props[0] = (Prop) {
        .key = slice_from("disabled"),
        .value = slice_from("true"),
        .type = EXPR_PROP_TYPE,
        .next = -1,
    };

    return 0;
}

#define GEN_BUTTON_CLASS_INPUT "<button class=\"btn\">press</button>"
ValueIndex GEN_BUTTON_CLASS_FN(Arena* arena) {
    if (arena->values_capacity < 2 || arena->prop_capacity < 1) return -1;
    arena->values_count=2;
    arena->prop_count=1;
    arena->values[0].type = NODE_NODE_TYPE;
    arena->values[0].next = -1;
    arena->values[0].value.node = (Node) {
        .Tag = slice_from("button"),
        .Props = 0,
        .Children = 1,
    };
    arena->props[0] = (Prop) {
        .key = slice_from("class"),
        .value = slice_from("btn"),
        .type = TEXT_PROP_TYPE,
        .next = -1
    };
    arena->values[1].type = TEXT_NODE_TYPE;
    arena->values[1].next = -1;
    arena->values[1].value.text = slice_from("press");

    return 0;
}

#define GEN_SPAN_ONCLICK_INPUT "<span onClick={handleClick}>X</span>"
ValueIndex GEN_SPAN_ONCLICK_FN(Arena* arena) {
    if (arena->values_capacity < 2 || arena->prop_capacity < 1) return -1;
    arena->values_count=2;
    arena->prop_count=1;
    arena->values[0].type = TEXT_NODE_TYPE;
    arena->values[0].value.text = slice_from("X");
    arena->values[0].next = -1;

    arena->props[0] = (Prop) {
        .key = slice_from("onClick"),
        .value = slice_from("handleClick"),
        .type = EXPR_PROP_TYPE,
        .next = -1
    };

    arena->values[1].type = NODE_NODE_TYPE;
    arena->values[1].next = -1;
    arena->values[1].value.node = (Node) {
        .Tag = slice_from("span"),
        .Props = 0,
        .Children = 0
    };

    return 1;
}

#define GEN_BUTTON_ONCLICK_INPUT "<button onClick={() => alert(\"hi\")}>Click me</button>"
ValueIndex GEN_BUTTON_ONCLICK_FN(Arena* arena) {
    if (arena->values_capacity < 2 || arena->prop_capacity < 1) return -1;
    arena->values_count=2;
    arena->prop_count=1;
    arena->values[0].type = TEXT_NODE_TYPE;
    arena->values[0].value.text = slice_from("Click me");
    arena->values[0].next = -1;

    arena->props[0] = (Prop) {
        .key = slice_from("onClick"),
        .value = slice_from("() => alert(\"hi\")"),
        .type = EXPR_PROP_TYPE,
        .next = -1
    };

    arena->values[1].type = NODE_NODE_TYPE;
    arena->values[1].value.node = (Node) {
        .Tag = slice_from("button"),
        .Props = 0,
        .Children = 0
    };
    arena->values[1].next = -1;

    return 1;
}

#define GEN_PARAGRAPH_INPUT "<p>Hello world, this is JSX!</p>"
ValueIndex GEN_PARAGRAPH_FN(Arena* arena) {
    if (arena->values_capacity < 2) return -1;
    arena->values_count=2;
    arena->prop_count=0;
    arena->values[0].type = TEXT_NODE_TYPE;
    arena->values[0].value.text = slice_from("Hello world, this is JSX!");
    arena->values[0].next = -1;

    arena->values[1].type = NODE_NODE_TYPE;
    arena->values[1].value.node = (Node) {
        .Tag = slice_from("p"),
        .Props = -1,
        .Children = 0
    };
    arena->values[1].next = -1;

    return 1;
}

#define GEN_SPAN_EXPR_INPUT "<span>{user.name + {count}}</span>"
ValueIndex GEN_SPAN_EXPR_FN(Arena* arena) {
    if (arena->values_capacity < 2) return -1;
    arena->values_count=2;
    arena->prop_count=0;
    arena->values[0].type = EXPR_NODE_TYPE;
    arena->values[0].value.expr = slice_from("user.name + {count}");
    arena->values[0].next = -1;

    arena->values[1].type = NODE_NODE_TYPE;
    arena->values[1].value.node = (Node) {
        .Tag = slice_from("span"),
        .Props = -1,
        .Children = 0
    };
    arena->values[1].next = -1;

    return 1;
}

#define GEN_MULTILINE_TEXT_INPUT "<div>\n    Multi-line\n    text content\n</div>"
ValueIndex GEN_MULTILINE_TEXT_FN(Arena* arena) {
    if (arena->values_capacity < 2) return -1;
    arena->values_count=2;
    arena->prop_count=0;
    arena->values[0].type = TEXT_NODE_TYPE;
    arena->values[0].value.text = slice_from("\n    Multi-line\n    text content\n");
    arena->values[0].next = -1;

    arena->values[1].type = NODE_NODE_TYPE;
    arena->values[1].value.node = (Node) {
        .Tag = slice_from("div"),
        .Props = -1,
        .Children = 0
    };
    arena->values[1].next = -1;

    return 1;
}

#define GEN_DIV_KEY_EXPR_INPUT "<div key={\"x\" + n}></div>"
ValueIndex GEN_DIV_KEY_EXPR_FN(Arena* arena) {
    if (arena->prop_capacity < 1) return -1;
    arena->values_count=1;
    arena->prop_count=0;
    arena->props[0] = (Prop) {
        .key = slice_from("key"),
        .value = slice_from("\"x\" + n"),
        .type = EXPR_PROP_TYPE,
        .next = -1
    };
    arena->values[0].type = NODE_NODE_TYPE;
    arena->values[0].value.node = (Node) {
        .Tag = slice_from("div"),
        .Props = 0,
        .Children = -1
    };
    arena->values[0].next = -1;

    return 0;
}


#define GEN_BLOCK(name)              \
    {                                \
        .input = GEN_##name##_INPUT, \
        .gen_expected = GEN_##name##_FN,   \
    }

void test_parser() {
    ParserTestCase tests[] = {
        GEN_BLOCK(SIMPLE_TEXT),
        GEN_BLOCK(MULTIPLE_NODES),
        GEN_BLOCK(BUTTON_DISABLED),
        GEN_BLOCK(BUTTON_CLASS),
        GEN_BLOCK(SPAN_ONCLICK),
        GEN_BLOCK(BUTTON_ONCLICK),
        GEN_BLOCK(PARAGRAPH),
        GEN_BLOCK(SPAN_EXPR),
        GEN_BLOCK(MULTILINE_TEXT),
        GEN_BLOCK(DIV_KEY_EXPR),
    };

    int test_count = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < test_count; i++) {
        test_parser_case(tests[i]);
        mt_total++;
        test_parser_case_realloc(tests[i]);
        mt_total++;
    }
}
