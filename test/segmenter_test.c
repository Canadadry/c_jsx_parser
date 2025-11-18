#include "../src/segmenter.h"
#include "minitest.h"


typedef struct {
    const char * name;
    const char *in;
    Segment* exp;
}SegmentCase;

#define MAX_SEGMENT 10

void test_segment_case(SegmentCase tt) {
    Segmenter segmenter={0};
    segmenter.src = slice_from(tt.in);

    for(int i=0;i<MAX_SEGMENT;i++) {
        Segment got = get_next_segment(&segmenter);
        if (got.type != tt.exp[i].type) {
            TEST_ERRORF(tt.name, "expected type %d, got %d at segment %d", tt.exp[i].type, got.type, i);
            return;
        }

        if (got.type == END) {
            break;
        }

        if (!slice_equal(got.content, tt.exp[i].content)) {
            TEST_ERRORF(tt.name, "expected content at segment %d: \n-%.*s-\n, got: \n-%.*s-\n", i,
                tt.exp[i].content.len ,  tt.exp[i].content.start,
                got.content.len ,  got.content.start
            );
            return;
        }
    }
}

void test_segment() {
    SegmentCase cases[] = {
        {
            .name="component with props and children",
            .in="import * as MyLib from './MyLib.js'\n"
                "\n"
                "export function Component() {\n"
                "    let myRef = null\n"
                "    let name = \"Fernando\"\n"
                "    let myClass = \"open\"\n"
                "    return (\n"
                "        <div className={myClass} ref={myRef}>\n"
                "        <h1>Hello {name}!</h1>\n"
                "    </div>\n"
                "    )\n"
                "}",
            .exp=(Segment[]){
                (Segment){
                    .type=JS,
                    .content=slice_from("import * as MyLib from './MyLib.js'\n"
                            "\n"
                            "export function Component() {\n"
                            "    let myRef = null\n"
                            "    let name = \"Fernando\"\n"
                            "    let myClass = \"open\"\n"
                            "    return (\n"
                            "        "),
                },
                (Segment){
                    .type=JSX,
                    .content=slice_from("<div className={myClass} ref={myRef}>\n"
                            "        <h1>Hello {name}!</h1>\n"
                            "    </div>"),
                },
                (Segment){
                    .type=JS,
                    .content=slice_from("\n    )\n}"),
                },
                (Segment){
                    .type=END,
                },
            },
        },
        {
            .name="simple div",
            .in="const el = (<div>ok</div>);",
            .exp=(Segment[]){
                (Segment){
                    .type=JS,
                    .content=slice_from("const el = ("),
                },
                (Segment){
                    .type=JSX,
                    .content=slice_from("<div>ok</div>"),
                },
                (Segment){
                    .type=JS,
                    .content=slice_from(");"),
                },
                (Segment){
                    .type=END,
                },
            },
        },
        {
            .name="button with boolean prop",
            .in="export const Component = () => (<button disabled>press</button>);",
            .exp=(Segment[]){
                (Segment){
                    .type=JS,
                    .content=slice_from("export const Component = () => ("),
                },
                (Segment){
                    .type=JSX,
                    .content=slice_from("<button disabled>press</button>"),
                },
                (Segment){
                    .type=JS,
                    .content=slice_from(");"),
                },
                (Segment){
                    .type=END,
                },
            },
        },
        {
            .name="span with expression",
            .in="function handleClick() {}\n"
                "export function X() {\n"
                "    return <span onClick={handleClick}>X</span>\n"
                "}",
            .exp=(Segment[]){
                (Segment){
                    .type=JS,
                    .content=slice_from("function handleClick() {}\n"
                            "export function X() {\n"
                            "    return "),
                },
                (Segment){
                    .type=JSX,
                    .content=slice_from("<span onClick={handleClick}>X</span>"),
                },
                (Segment){
                    .type=JS,
                    .content=slice_from("\n}"),
                },
                (Segment){
                    .type=END,
                },
            },
        },
        {
            .name="inline closure",
            .in="export default function Foo() {\n"
                "    return (<button onClick={() => alert(\"hi\")}>Click me</button>);\n"
                "}",
            .exp=(Segment[]){
                (Segment){
                    .type=JS,
                    .content=slice_from("export default function Foo() {\n"
                            "    return ("),
                },
                (Segment){
                    .type=JSX,
                    .content=slice_from("<button onClick={() => alert(\"hi\")}>Click me</button>"),
                },
                (Segment){
                    .type=JS,
                    .content=slice_from(");\n}"),
                },
                (Segment){
                    .type=END,
                },
            },
        },
        {
            .name="multi-line text",
            .in="export function M() {\n"
                "    return (\n"
                "        <div>\n"
                "            Multi-line\n"
                "            text content\n"
                "        </div>\n"
                "    )\n"
                "}",
            .exp=(Segment[]){
                (Segment){
                    .type=JS,
                    .content=slice_from("export function M() {\n"
                            "    return (\n"
                            "        "),
                },
                (Segment){
                    .type=JSX,
                    .content=slice_from("<div>\n"
                            "            Multi-line\n"
                            "            text content\n"
                            "        </div>"),
                },
                (Segment){
                    .type=JS,
                    .content=slice_from("\n    )\n}"),
                },
                (Segment){
                    .type=END,
                },
            },
        },
        {
            .name="empty JSX",
            .in="const el = (<></>);",
            .exp=(Segment[]){
                (Segment){
                    .type=JS,
                    .content=slice_from("const el = ("),
                },
                (Segment){
                    .type=JSX,
                    .content=slice_from("<></>"),
                },
                (Segment){
                    .type=JS,
                    .content=slice_from(");"),
                },
                (Segment){
                    .type=END,
                },
            },
        },
        {
            .name="JSX with nested elements",
            .in="const el = (<div><span>Nested</span></div>);",
            .exp=(Segment[]){
                (Segment){
                    .type=JS,
                    .content=slice_from("const el = ("),
                },
                (Segment){
                    .type=JSX,
                    .content=slice_from("<div><span>Nested</span></div>"),
                },
                (Segment){
                    .type=JS,
                    .content=slice_from(");"),
                },
                (Segment){
                    .type=END,
                },
            },
        },
        {
            .name="JS with < ",
            .in="function render() {"
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
            "}",
            .exp=(Segment[]){
                (Segment){
                    .type=JS,
                    .content=slice_from(
                        "function render() {"
                        "  ClearBackground(\"#fff\");"
                        "  ui_clear();"
                        "  var root = ui_compute("
                    ),
                },
                (Segment){
                    .type=JSX,
                    .content=slice_from(
                        "<item w={200}>{"
                        "    Slider({ id: \"line\", name: \"test\", val: val })"
                        "  }</item>"
                    ),
                },
                (Segment){
                    .type=JS,
                    .content=slice_from(
                        ");"
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
                },
                (Segment){
                    .type=END,
                }
            }
        },
        {
        .name="bug eof",
        .in="var Slider = function(props){"
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
        "}",
        .exp=(Segment[]){
            (Segment){
                .type=JS,
                .content=slice_from(
                    "var Slider = function(props){"
                    "  return "
                )
            },
            (Segment){
                .type=JSX,
                .content=slice_from(
                    "<rectangle class=\"fit-y grow-x lh p-1 m-1\" color=\"#aa0\">"
                    "    <txt msg={props.name} class=\"grow max-bound-x\" font_size={20} color=\"#000\"></txt>"
                    "    <rectangle id={props.id+\"-box\"} class=\"fit-y grow-x ls\" radius={1} color=\"#00a\">"
                    "      {/* <rectangle x={props.pos} w={20} h={20} radius={1} color=\"#f0a\">*/}"
                    "      <rectangle w={20} h={20} radius={1} color=\"#f0a\">"
                    "      </rectangle>"
                    "    </rectangle>"
                    "    <txt msg={\"\"+props.val} class=\"grow max-bound-x\" font_size={20} color=\"#000\"></txt>"
                    "</rectangle>"
                )
            },
            (Segment){
                .type=JS,
                .content=slice_from(
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
                    "  var root = ui_compute("
                )
            },
            (Segment){
                .type=JSX,
                .content=slice_from(
                    "<item w={200}>{"
                    "    // Slider({id:\"line\",name:\"test\",val:val,pos:pos})"
                    "    Slider({ id: \"line\", name: \"test\", val: val })"
                    "  }</item>"
                ),
            },
            (Segment){
                .type=JS,
                .content=slice_from(
                    ");"
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
            },
            (Segment){
                .type=END,
            }
        }
    },

    {
        .name="JSX in line comments",
        .in="//const el = (<div><span>ok</span></div>);",
        .exp=(Segment[]){
            (Segment){
                .type=JS,
                .content=slice_from("//const el = (<div><span>ok</span></div>);"),
            },
            (Segment){
                .type=END,
            },
        },
    },
    {
        .name="JSX in block comments",
        .in="/*const el = (<div><span>ok</span></div>); */",
        .exp=(Segment[]){
            (Segment){
                .type=JS,
                .content=slice_from("/*const el = (<div><span>ok</span></div>); */"),
            },
            (Segment){
                .type=END,
            },
        },
    },

    };

    int test_count = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < test_count; i++) {
        mt_total++;
        test_segment_case(cases[i]);
    }
}
