#include "../src/transform.h"
#include "minitest.h"

#define ARENA_SIZE 10
#define BUF_CAPACITY 512

void test_transform_case(Slice in, Slice exp){
    Arena arena = {0};
    Value children_arena[ARENA_SIZE] = {0};
    arena.values = children_arena;
    arena.values_capacity = ARENA_SIZE;
    Prop     props_arena[ARENA_SIZE] = {0};
    arena.props = props_arena;
    arena.prop_capacity = ARENA_SIZE;
    Lexer lexer = NewLexer(in);
    Parser parser = {0};
    parser.lexer=&lexer;
    parser.arena=&arena;
    Transformer transformer = {0};
    char buf[BUF_CAPACITY] ={0};
    transformer.createElem=slice_from("React.createElement(");
    transformer.buf.buf=buf;
    transformer.buf.buf_capacity=BUF_CAPACITY;

    InitParser(&parser);
    ParseNodeResult result = ParseNode(&parser);
    if (result.type != OK) {
        TEST_ERRORF("test_transform_case","parsing %s failed: %d\n", in.start,result.value.err);
    }

    Transform(&transformer,&arena,result.value.ok);
    if (!slice_equal((Slice){.start=buf,.len=transformer.buf.buf_count}, exp)) {
        TEST_ERRORF("test_transform_case","Test failed for input: %s\nExp: -%s-\nGot: -%s-\n",
            in.start, exp.start, buf
        );
    }
}

void test_transform() {
    struct {
        const char *in;
        const char *exp;
    } cases[] = {
        {
            .in = "<div></div>",
            .exp = "React.createElement(\"div\", null, [])"
        },
        {
            .in = "<div>ok</div>",
            .exp = "React.createElement(\"div\", null, [\"ok\"])"
        },
        {
            .in = "<div className=\"x\">ok</div>",
            .exp = "React.createElement(\"div\", { className : \"x\" }, [\"ok\"])"
        },
        {
            .in = "<button disabled>click</button>",
            .exp = "React.createElement(\"button\", { disabled : true }, [\"click\"])"
        },
        {
            .in = "<span>{user.name}</span>",
            .exp = "React.createElement(\"span\", null, [user.name])"
        },
        {
            .in = "<ul><li>one</li><li>two</li></ul>",
            .exp = "React.createElement(\"ul\", null, [React.createElement(\"li\", null, [\"one\"]), React.createElement(\"li\", null, [\"two\"])])"
        },
        {
            .in = "<p>Hello {name}</p>",
            .exp = "React.createElement(\"p\", null, [\"Hello \", name])"
        },
        {
            .in = "<Button primary>Click</Button>",
            .exp = "React.createElement(Button, { primary : true }, [\"Click\"])"
        },
        {
            .in = "<Button></Button>",
            .exp = "React.createElement(Button, null, [])"
        },
        {
            .in = "<Button>Click</Button>",
            .exp = "React.createElement(Button, null, [\"Click\"])"
        },
        {
            .in = "<button onClick={handleClick}>press</button>",
            .exp = "React.createElement(\"button\", { onClick : handleClick }, [\"press\"])"
        },
        {
            .in = "<button onClick={() => alert(\"hi\")}>press</button>",
            .exp = "React.createElement(\"button\", { onClick : () => alert(\"hi\") }, [\"press\"])"
        },
        {
            .in = "<button onClick={() => alert(\"hi\")}>\n\tpress\n</button>",
            .exp = "React.createElement(\"button\", { onClick : () => alert(\"hi\") }, [\"\\n\tpress\\n\"])"
        },
    };

    int test_count = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < test_count; i++) {
        mt_total++;
        test_transform_case(slice_from(cases[i].in),slice_from(cases[i].exp));
    }
}


void test_escape_case(Slice in, Slice exp){

    char buf_content[BUF_CAPACITY] ={0};
    Buffer buf = (Buffer){
        .buf =buf_content,
        .buf_count=0,
        .buf_capacity=BUF_CAPACITY,
        .allocator=(Allocator){},
    };
    escape(in,&buf);
    Slice got = buffer_to_slice(&buf);
    if (!slice_equal(got, exp)) {
        TEST_ERRORF("test_escape_case","Test failed for input: %s\nExp: -%s-\nGot: -%s-\n",
            in.start, exp.start, got.start
        );
    }
}

void test_escape() {
    struct {
        const char *in;
        const char *exp;
    } cases[] = {
        {
            .in = "no line return",
            .exp = "no line return"
        },
        {
            .in = "line return at the end \n",
            .exp = "line return at the end \\n"
        },
        {
            .in = "line return \n in the middle",
            .exp = "line return \\n in the middle"
        },
        {
            .in = "two line \n return in \n the middle",
            .exp = "two line \\n return in \\n the middle"
        },
    };

    int test_count = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < test_count; i++) {
        mt_total++;
        test_escape_case(slice_from(cases[i].in),slice_from(cases[i].exp));
    }
}
