#include "../src/transform.h"
#include "minitest.h"

#define ARENA_SIZE 10
#define BUF_CAPACITY 512

void test_transform_case(Slice in, Slice exp){
    Lexer lexer = NewLexer(in);
    Parser parser = {0};
    parser.lexer=&lexer;
    Child children_arena[ARENA_SIZE] = {0};
    parser.children = children_arena;
    parser.child_capacity = ARENA_SIZE;
    Prop     props_arena[ARENA_SIZE] = {0};
    parser.props = props_arena;
    parser.prop_capacity = ARENA_SIZE;
    Transformer transformer = {0};
    char buf[BUF_CAPACITY] ={0};
    transformer.createElem=slice_from("React.createElement(");
    transformer.buf=buf;
    transformer.buf_capacity=BUF_CAPACITY;

    InitParser(&parser);
    ParseNodeResult result = ParseNode(&parser);
    if (result.type != OK) {
        TEST_ERRORF("test_transform_case","parsing %s failed: %d\n", in.start,result.value.err);
    }
    Node* actual = result.value.ok;

    Transform(&transformer,actual);
    if (slice_equal((Slice){.start=buf,.len=transformer.buf_count}, exp) != 0) {
        TEST_ERRORF("test_transform_case","Test failed for input: %s\nExpected: %s\nGot: %s\n", in.start, exp.start, buf);
    }
}

void test_transform() {
    struct {
        const char *in;
        const char *exp;
    } cases[] = {
        {
            .in = "<div>ok</div>",
            .exp = "React.createElement(\"div\", null, \"ok\")"
        },
        {
            .in = "<div className=\"x\">ok</div>",
            .exp = "React.createElement(\"div\", { className: \"x\" }, \"ok\")"
        },
        {
            .in = "<button disabled>click</button>",
            .exp = "React.createElement(\"button\", { disabled: true }, \"click\")"
        },
        {
            .in = "<span>{user.name}</span>",
            .exp = "React.createElement(\"span\", null, user.name)"
        },
        {
            .in = "<ul><li>one</li><li>two</li></ul>",
            .exp = "React.createElement(\"ul\", null, React.createElement(\"li\", null, \"one\"), React.createElement(\"li\", null, \"two\"))"
        },
        {
            .in = "<p>Hello {name}</p>",
            .exp = "React.createElement(\"p\", null, \"Hello \", name)"
        },
        {
            .in = "<Button primary>Click</Button>",
            .exp = "React.createElement(Button, { primary: true }, \"Click\")"
        },
        {
            .in = "<button onClick={handleClick}>press</button>",
            .exp = "React.createElement(\"button\", { onClick: handleClick }, \"press\")"
        },
        {
            .in = "<button onClick={() => alert(\"hi\")}>press</button>",
            .exp = "React.createElement(\"button\", { onClick: () => alert(\"hi\") }, \"press\")"
        },
        {
            .in = "<div key={\"x\" + n}>\n    {cond && <span>ok</span>}\n</div>",
            .exp = "React.createElement(\"div\", { key: \"x\" + n }, \"\\n    \", cond && React.createElement(\"span\", null, \"ok\"), \"\\n\")"
        }
    };

    int test_count = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < test_count; i++) {
        mt_total++;
        test_transform_case(slice_from(cases[i].in),slice_from(cases[i].exp));
    }
}
