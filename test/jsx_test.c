#include "jsx_test.h"
#include "../src/jsx.h"
#include "minitest.h"
#include "realloc_test.h"

#define BUF_CAPACITY 4096
#define ARENA_SIZE 30

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
    if(exp.len==0){
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
        if(exp.len!=0){
            Slice result = slice_from(jsx_get_output(c));
            if (!slice_equal(result,exp)) {
                TEST_ERRORF("test_jsx_case","compile failed for input: %.*s\n\nexpected: %.*s\n\ngot: %.*s\n",
                    in.len,in.start,
                    exp.len,exp.start,
                    result.len, result.start
                );
            }
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
                "        React.createElement(\"div\", { ref : myRef, className : myClass, id : \"test\" }, [\"\\n"
                "        \", React.createElement(\"h1\", null, [\"Hello \", name, \"!\"]), \"\\n"
                "    \"])\n"
                "    )\n"
                "}"),
        },
        {
            .in=slice_from(
                "function render() {"
                "  ClearBackground(\"#fff\");"
                "  ui_clear();"
                "  var root = ui_compute(<item w={200}>{"
                "    Slider({ id: \"line\", name: \"test\", val: val })"
                "  }</item>);"
                "  ui_draw(root);"
                ""
                "  if(is_mouse_button_pressed(\"left\")){"
                "    var node = ui_pick(get_mouse_x(), get_mouse_y());"
                "    if(node==\"line-box\"){"
                "      pressed = true;"
                "    }"
                "  }else if(is_mouse_button_released(\"left\")){"
                "    pressed = false;"
                "  }"
                "  if(pressed){"
                "    var b = ui_bb(\"line-box\");"
                "    var local_x = get_mouse_x() - b.x;"
                "    if (local_x < 0) { local_x = 0; }"
                "    if (local_x > b.w) { local_x = b.w; }"
                "    pos = local_x;"
                "  }"
                "}"
            ),
            .exp=slice_from(""),
        },
        {
            .in=slice_from(
            "var Slider = function(props){"
            "  return <rectangle class=\"fit-y grow-x lh p-1 m-1\" color=\"#aa0\">"
            "    <txt msg={props.name} class=\"grow max-bound-x\" font_size={20} color=\"#000\"></txt>"
            "    <rectangle id={props.id+\"-box\"} class=\"fit-y grow-x ls\" radius={1} color=\"#00a\">"
            "      {/* <rectangle x={props.pos} w={20} h={20} radius={1} color=\"#f0a\">*/}"
            "      <rectangle w={20} h={20} radius={1} color=\"#f0a\">"
            "      </rectangle>"
            "    </rectangle>"
            "    <txt msg={\"\"+props.val} class=\"grow max-bound-x\" font_size={20} color=\"#000\"></txt>"
            "</rectangle>"
            "}"
            ""
            "var val = 100;"
            "var pos = 0;"
            "var pressed = false;"
            ""
            "function conf() {"
            "  window.width = 800;"
            "  window.height = 600;"
            "  window.title = \"widget\";"
            "}"
            ""
            "function init() {"
            "}"
            ""
            "function render() {"
            "  ClearBackground(\"#fff\");"
            "  ui_clear();"
            "  var root = ui_compute(<item w={200}>{"
            "    // Slider({id:\"line\",name:\"test\",val:val,pos:pos})"
            "    Slider({ id: \"line\", name: \"test\", val: val })"
            "  }</item>);"
            "  ui_draw(root);"
            ""
            "  // if(is_mouse_button_pressed(\"left\")){"
            "  //   var node = ui_pick(get_mouse_x(), get_mouse_y());"
            "  //   if(node==\"line-box\"){"
            "  //     pressed = true;"
            "  //   }"
            "  // }else if(is_mouse_button_released(\"left\")){"
            "  //   pressed = false;"
            "  // }"
            "  // if(pressed){"
            "  //   var b = ui_bb(\"line-box\");"
            "  //   var local_x = get_mouse_x() - b.x;"
            "  //   if (0 > local_x) { local_x = 0; }"
            "  //   if (local_x > b.w) { local_x = b.w; }"
            "  //   pos = local_x;"
            "  // }"
            "}"

            ),
            .exp=slice_from(""),
        }
    };

    int test_count = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < test_count; i++) {
        mt_total++;
        test_jsx_case(cases[i].in,cases[i].exp);
        mt_total++;
        test_jsx_case_realloc(cases[i].in,cases[i].exp);
    }
}
