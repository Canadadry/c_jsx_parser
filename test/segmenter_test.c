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

        if (got.type == EOF) {
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
                    .type=EOF,
                },
            },
        }
    };

    int test_count = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < test_count; i++) {
        mt_total++;
        test_segment_case(cases[i]);
    }
}
