#include "jsx.h"
#include "ast.h"
#include "parser.h"
#include <string.h>
#include "token.h"
#include "transform.h"
#include "segmenter.h"

typedef struct{
    ResultType type;
    union {
        Slice ok;
        Error err;
    } value;
}SliceResult;

void copy_slice(Compiler* c, char** dest,size_t *dest_size, size_t *dest_cap, Slice source);
void swap_buffer(Compiler* c);
SliceResult transform(Compiler* c,Slice content);

CompileResult compile(Compiler* c){
    CompileResult result ={0};
    result.type=OK;

    while(1){
        c->out_buf_count=0;
        Segmenter segmenter={0};
        segmenter.src = (Slice){.start=c->in_buf,.len=c->in_buf_count};
        int changed = 0;

        while(1){
            Segment got = get_next_segment(&segmenter);
            if (got.type == END) {
                break;
            }
            if (got.type == JS) {
                copy_slice(c,
                    &c->out_buf,&c->out_buf_count,&c->out_buf_capacity,
                    got.content
                );
            }
            if (got.type == JSX) {
                SliceResult r = transform(c,got.content);
                if(r.type==ERR){
                    result.type=ERR;
                    result.value.err=r.value.err;
                    result.value.err.at += got.content.start - c->in_buf;
                    return result;
                }
                changed = 1;

                copy_slice(c,
                    &c->out_buf,&c->out_buf_count,&c->out_buf_capacity,
                    r.value.ok
                );

            }
        }
        if(changed==0){
            result.value.ok = (Slice){.start=c->out_buf,.len=c->out_buf_count};
            return result;
        }
        swap_buffer(c);
    }
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
    Printer got_printer ={0};
    char got_buf[BUF_CAPACITY] ={0};
    got_printer.buf=got_buf;
    got_printer.buf_capacity=BUF_CAPACITY;
    value_print(&got_printer,&c->arena, parse_result.value.ok, 0);

    ValueIndex actual = parse_result.value.ok;
    Transformer transformer = {0};
    transformer.createElem=c->createElem;
    transformer.buf=c->transform_buf;
    transformer.buf_capacity=c->transform_buf_capacity;
    transformer.realloc_fn=c->arena.realloc_fn;
    transformer.userdata=c->arena.userdata;
    Transform(&transformer,&c->arena,actual);
    c->transform_buf=transformer.buf;
    c->transform_buf_capacity=transformer.buf_capacity;

    result.value.ok = (Slice){.start=transformer.buf,.len=transformer.buf_count};

    return result;
}

void swap_buffer(Compiler* c){
    char* tmp_buf = c->out_buf;
    c->out_buf=c->in_buf;
    c->in_buf=tmp_buf;
    size_t tmp_buf_cap = c->out_buf_capacity;
    c->out_buf_capacity=c->in_buf_capacity;
    c->in_buf_capacity=tmp_buf_cap;
    size_t tmp_buf_count = c->out_buf_count;
    c->out_buf_count=c->in_buf_count;
    c->in_buf_count=tmp_buf_count;
}

void copy_slice(Compiler* c, char** dest,size_t *dest_size, size_t *dest_cap, Slice source){
    while (*dest_size + source.len >= *dest_cap) {
        if (*dest_cap == 0) *dest_cap = source.len+1;
        else *dest_cap *= 2;
        *dest = c->arena.realloc_fn(c->arena.userdata,*dest,*dest_cap);
    }
    memcpy(*dest + *dest_size, source.start, source.len);
    *dest_size+=source.len;
}
