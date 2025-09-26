#include "jsx.h"
#include "ast.h"
#include "parser.h"
#include "buffer.h"
#include "transform.h"
#include "segmenter.h"

typedef struct{
    ResultType type;
    union {
        Slice ok;
        Error err;
    } value;
}SliceResult;

void swap_buffer(Compiler* c);
SliceResult transform(Compiler* c,Slice content);

CompileResult compile(Compiler* c){
    if(c->max_iter<=0){
        c->max_iter=100;
    }
    CompileResult result ={0};
    result.type=OK;

    bool changed = true;
    SAFE_WHILE(changed, c->max_iter){
        changed=false;
        buffer_clear(&c->out);
        Segmenter segmenter={0};
        segmenter.src = buffer_to_slice(&c->in);

        Segment got = {.type=JS};
        SAFE_WHILE(got.type != END, c->max_iter){
            Segment got = get_next_segment(&segmenter);
            if (got.type == JS) {
                write_slice(&c->out, got.content);
            }
            if (got.type == JSX) {
                SliceResult r = transform(c,got.content);
                if(r.type==ERR){
                    result.type=ERR;
                    result.value.err=r.value.err;
                    result.value.err.at += got.content.start - c->in.buf;
                    return result;
                }
                changed = true;
                write_slice(&c->out, r.value.ok);
            }
        }
        swap_buffer(c);
    }
    result.value.ok = buffer_to_slice(&c->out);
    return result;
}

SliceResult transform(Compiler* c,Slice content){
    SliceResult result={0};
    result.type=OK;

    Lexer lexer = NewLexer(content);
    lexer.source = content;

    Parser parser = {0};
    parser.lexer=&lexer;
    InitParser(&parser);
    parser.arena = &c->arena;

    ParseNodeResult parse_result = ParseNode(&parser);
    if (parse_result.type != OK) {
        result.type=ERR;
        result.value.err=parse_result.value.err;
        return result;
    }
    #define BUF_CAPACITY 1024
    Buffer got_buffer ={0};
    char got_buf[BUF_CAPACITY] ={0};
    got_buffer.buf=got_buf;
    got_buffer.buf_capacity=BUF_CAPACITY;
    value_print(&got_buffer,&c->arena, parse_result.value.ok, 0);

    ValueIndex actual = parse_result.value.ok;
    Transformer transformer = {0};
    transformer.createElem=c->createElem;
    transformer.buf.buf=c->tmp.buf;
    transformer.buf.buf_capacity=c->tmp.buf_capacity;
    transformer.buf.realloc_fn=c->arena.realloc_fn;
    transformer.buf.userdata=c->arena.userdata;
    Transform(&transformer,&c->arena,actual);
    c->tmp.buf=transformer.buf.buf;
    c->tmp.buf_capacity=transformer.buf.buf_capacity;

    result.value.ok = (Slice){.start=transformer.buf.buf,.len=transformer.buf.buf_count};

    return result;
}

void swap_buffer(Compiler* c){
    char* tmp_buf = c->out.buf;
    c->out.buf=c->in.buf;
    c->in.buf=tmp_buf;
    size_t tmp_buf_cap = c->out.buf_capacity;
    c->out.buf_capacity=c->in.buf_capacity;
    c->in.buf_capacity=tmp_buf_cap;
    size_t tmp_buf_count = c->out.buf_count;
    c->out.buf_count=c->in.buf_count;
    c->in.buf_count=tmp_buf_count;
}
