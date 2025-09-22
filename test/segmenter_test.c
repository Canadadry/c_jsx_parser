#include "../src/segmenter.h"
#include "minitest.h"


typedef struct {
    const char * name;
    const char *in;
    Segment* exp;
}SegmentCase;

void test_segment_case(SegmentCase tt){
}

void test_segment() {
    SegmentCase cases[] = {
        {
            .name="component with props and children",
            .in= "import * as MyLib from './MyLib.js'\
\
    export function Component() {\
        let myRef = null\
        let name = \"Fernando\"\
        let myClass = \"open\"\
        return (\
            <div className={myClass} ref={myRef}>\
                <h1>Hello {name}!</h1>\
            </div>\
    )\
}",
            .exp=(Segment[]){
                (Segment){
                    .type=JS,
                    .content="import * as MyLib from './MyLib.js'\
\
    export function Component() {\
        let myRef = null\
        let name = \"Fernando\"\
        let myClass = \"open\"\
        return (",
                },
                (Segment){
                    .type=JSX,
                    .content="<div className={myClass} ref={myRef}>\
                <h1>Hello {name}!</h1>\
            </div>",
                },
                (Segment){
                    .type=JS,
                    .content=")\
}",
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
