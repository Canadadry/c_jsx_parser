#include "jsx.h"
#include "parser.h"
#include "transform.h"
#include "segmenter.h"

CompileResult compile(Compiler* c,Slice in){
    CompileResult result ={0};
    Slice content =  in;

    while(1){
        Segmenter segmenter={0};
        segmenter.src = in;
        int changed = 0;

        while(1){
            Segment got = get_next_segment(&segmenter);
            if (got.type == END) {
                break;
            }
            if (got.type == JSX) {
                Lexer lexer = NewLexer(in);
                lexer.source = got.content;
                Parser parser = {0};
                parser.lexer=&lexer;
                InitParser(&parser);
                parser.children = c->children;
                parser.child_capacity = c->child_capacity;
                parser.props = c->props;
                parser.prop_capacity = c->prop_capacity;
                parser.realloc_fn=c->realloc_fn;
                parser.userdata=c->userdata;

                Transformer transformer = {0};
                transformer.createElem=c->createElem;
                transformer.buf=c->buf;
                transformer.buf_capacity=c->buf_capacity;

                ParseNodeResult parse_result = ParseNode(&parser);
                if (parse_result.type != OK) {
                    result.type=ERR;
                    result.value.err=parse_result.value.err;
                }
                Node* actual = parse_result.value.ok;
                Transform(&transformer,actual);
                changed = 1;
                break;
            }
            if (got.type == JS) {

            }
        }
        if(changed==0){
            return result;
        }
        content = (Slice){0};
    }
}
