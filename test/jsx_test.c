#include "jsx_test.h"
#include "../src/jsx.h"
#include "minitest.h"
#include "realloc_test.h"

#define BUF_CAPACITY 2048
#define ARENA_SIZE 10

void test_jsx_case(Slice in, Slice exp) {
    Compiler c = {0};
    c.createElem = slice_from("React.createElement(");;
    c.in.buf = (char[BUF_CAPACITY]){0};
    c.in.buf_capacity = BUF_CAPACITY;
    c.out.buf = (char[BUF_CAPACITY]){0};
    c.out.buf_capacity = BUF_CAPACITY;
    c.tmp.buf = (char[BUF_CAPACITY]){0};
    c.tmp.buf_capacity = BUF_CAPACITY;
    c.arena.props = (Prop[ARENA_SIZE]){0};
    c.arena.prop_capacity = ARENA_SIZE;
    c.arena.values = (Value[ARENA_SIZE]){0};
    c.arena.values_capacity = ARENA_SIZE;

    bool ok= jsx_compile(&c,in.start,in.len);
    if (!ok) {
        TEST_ERRORF("test_jsx_case","compile %s  %s\n", in.start,jsx_get_last_error(&c));
        return;
    }
    Slice result = slice_from(jsx_get_output(&c));
    if (!slice_equal(result,exp)) {
        TEST_ERRORF("test_jsx_case","compile failed for input: %.*s\n\nexpected: %.*s\n\ngot: %.*s\n",
            in.len,in.start,
            exp.len,exp.start,
            result.len, result.start
        );
    }
}

void test_jsx_case_realloc(Slice in, Slice exp) {
    Compiler* c = jsx_new_compiler("React.createElement(", (Allocator){
        .realloc_fn=fn_realloc,
        .free_fn=fn_free,
    });
    bool ok= jsx_compile(c,in.start,in.len);
    if (!ok) {
        TEST_ERRORF("test_jsx_case","compile %s  %s\n", in.start,jsx_get_last_error(c));
    }else{
        Slice result = slice_from(jsx_get_output(c));
        if (!slice_equal(result,exp)) {
            TEST_ERRORF("test_jsx_case","compile failed for input: %.*s\n\nexpected: %.*s\n\ngot: %.*s\n",
                in.len,in.start,
                exp.len,exp.start,
                result.len, result.start
            );
        }
    }
    jsx_free_compiler(c);
}


void test_jsx() {
    struct{Slice in;Slice exp;} cases[] = {
        {
            .in=slice_from("import * as MyLib from './MyLib.js'\n"
                "\n"
                "export function Component() {\n"
                "    let myRef = null\n"
                "    let name = \"Fernando\"\n"
                "        let myClass = \"open\"\n"
                "        return (\n"
                "        <div id=\"test\" className={myClass} ref={myRef}>\n"
                "        <h1>Hello {name}!</h1>\n"
                "    </div>\n"
                "    )\n"
                "}"),
            .exp=slice_from("import * as MyLib from './MyLib.js'\n"
                "\n"
                "export function Component() {\n"
                "    let myRef = null\n"
                "    let name = \"Fernando\"\n"
                "        let myClass = \"open\"\n"
                "        return (\n"
                "        React.createElement(\"div\", { ref : myRef, className : myClass, id : \"test\" }, \"\n"
                "        \", React.createElement(\"h1\", null, \"Hello \", name, \"!\"), \"\n"
                "    \")\n"
                "    )\n"
                "}"),
        },
    };

    int test_count = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < test_count; i++) {
        mt_total++;
        test_jsx_case(cases[i].in,cases[i].exp);
        mt_total++;
        test_jsx_case_realloc(cases[i].in,cases[i].exp);
    }
}
