#include "jsx_test.h"
#include "../src/jsx.h"
#include "minitest.h"
#include <string.h>
#include <execinfo.h>

#define BUF_CAPACITY 2048
#define ARENA_SIZE 10

#define MAX_CALLSTACK_DEPTH 64
void print_callstack(int skip) {
    void *buffer[MAX_CALLSTACK_DEPTH];
    int nptrs = backtrace(buffer, MAX_CALLSTACK_DEPTH);
    char **symbols = backtrace_symbols(buffer, nptrs);

    if (symbols == NULL) {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

    printf("Call stack:\n");
    for (int i = skip; i < nptrs; i++) {
        printf("  [%d] %s\n", i, symbols[i]);
    }

    free(symbols);
}

void test_jsx_case(Slice in, Slice exp) {
    Compiler c = {0};
    c.createElem = slice_from("React.createElement(");;
    char in_buf[BUF_CAPACITY] ={0};
    c.in_buf = in_buf;
    memcpy(c.in_buf, in.start, in.len);
    c.in_buf_count = in.len;
    c.in_buf_capacity = BUF_CAPACITY;
    char out_buf[BUF_CAPACITY] ={0};
    c.out_buf = out_buf;
    c.out_buf_count = 0;
    c.out_buf_capacity = BUF_CAPACITY;
    char transform_buf[BUF_CAPACITY] ={0};
    c.transform_buf = transform_buf;
    c.transform_buf_capacity = BUF_CAPACITY;
    Prop props_arena[ARENA_SIZE] = {0};
    c.props = props_arena;
    c.prop_capacity = ARENA_SIZE;
    Child children_arena[ARENA_SIZE] = {0};
    c.children = children_arena;
    c.child_capacity = ARENA_SIZE;

    CompileResult result = compile(&c);
    if (result.type != OK) {
        TEST_ERRORF("test_jsx_case","compile %s failed at %d : %s\n", in.start,result.value.err.at,parser_error_to_string(result.value.err.code));
    }
    if (slice_equal(result.value.ok, exp) != 0) {
        TEST_ERRORF("test_jsx_case","compile failed for input: %.*s\n\nexpected: %.*s\n\ngot: %.*s\n",
            in.len,in.start,
            exp.len,exp.start,
            result.value.ok.len, result.value.ok.start
        );
    }
}

void* fn_realloc(void* userdata,void* ptr, size_t size){
    print_callstack(2);
    printf("allocating memory %lu\n",size);
    return realloc(ptr, size);
}

void test_jsx_case_realloc(Slice in, Slice exp) {
    Compiler c = {0};
    c.createElem = slice_from("React.createElement(");;
    c.in_buf = realloc(c.in_buf,in.len);
    memcpy(c.in_buf, in.start, in.len);
    c.in_buf_count = in.len;
    c.in_buf_capacity = in.len;
    c.realloc_fn =fn_realloc;

    printf("test_jsx_case_realloc\n");
    CompileResult result = compile(&c);
    if (result.type != OK) {
        TEST_ERRORF("test_jsx_case","compile %.*s failed at %d %s : %.*s, got %s\n",
            in.len,in.start,result.value.err.at,parser_error_to_string(result.value.err.code),
            10,in.start+result.value.err.at-5,token_type_to_string(result.value.err.token)
        );
    }
    if (result.type == OK && slice_equal(result.value.ok, exp) != 0) {
        TEST_ERRORF("test_jsx_case","compile failed for input: %.*s\n\nexpected: %.*s\n\ngot: %.*s\n",
            in.len,in.start,
            exp.len,exp.start,
            result.value.ok.len, result.value.ok.start
        );
    }
    free(c.in_buf);
    if(c.out_buf!=NULL){
        free(c.out_buf);
    }
    if(c.children!=NULL){
        free(c.children);
    }
    if(c.props!=NULL){
        free(c.props);
    }
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
