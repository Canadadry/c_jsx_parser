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

        if (slice_equal(got.content, tt.exp[i].content) != 0) {
            TEST_ERRORF(tt.name, "expected content at segment %d: %s, got: %s", i, tt.exp[i].content.start, got.content.start);
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
                "        let myClass = \"open\"\n"
                "        return (\n"
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
                            "        let name = \"Fernando\"\n"
                            "            let myClass = \"open\"\n"
                            "        return ("),
                },
                (Segment){
                    .type=JSX,
                    .content=slice_from("<div className={myClass} ref={myRef}>\n"
                            "                <h1>Hello {name}!</h1>\n"
                            "            </div>"),
                },
                (Segment){
                    .type=JS,
                    .content=slice_from(")\n}"),
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

        // {
        //     .name="JSX with comments",
        //     .in="const el = (<div><!-- Comment --><span>ok</span></div>);",
        //     .exp=(Segment[]){
        //         (Segment){
        //             .type=JS,
        //             .content=slice_from("const el = ("),
        //         },
        //         (Segment){
        //             .type=JSX,
        //             .content=slice_from("<div><!-- Comment --><span>ok</span></div>"),
        //         },
        //         (Segment){
        //             .type=JS,
        //             .content=slice_from(");"),
        //         },
        //         (Segment){
        //             .type=END,
        //         },
        //     },
        // },

    };

    int test_count = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < test_count; i++) {
        mt_total++;
        test_segment_case(cases[i]);
    }
}
