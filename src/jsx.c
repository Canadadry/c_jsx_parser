#include "jsx.h"
#include "allocator.h"
#include "ast.h"
#include "parser.h"
#include "buffer.h"
#include "transform.h"
#include "segmenter.h"
#include <stdio.h>

#define MAX_ITER 100

typedef struct{
    ResultType type;
    union {
        Slice ok;
        Error err;
    } value;
}SliceResult;

void swap_buffer(Compiler* c);
SliceResult transform(Compiler* c,Slice content);

Compiler* jsx_new_compiler(const char* createElem,Allocator allocator){
    Compiler* c =allocator.realloc_fn(allocator.userdata,NULL,sizeof(Compiler));
    *c = (Compiler){0};
    c->createElem = slice_from(createElem);
    c->in.allocator =allocator;
    c->out.allocator =allocator;
    c->tmp.allocator =allocator;
    c->arena.allocator=allocator;
    return c;
}

void jsx_free_compiler(Compiler* c){
    if(c->in.buf!=NULL && c->in.allocator.free_fn !=NULL){
        c->in.allocator.free_fn(c->in.allocator.userdata,c->in.buf);
    }
    if(c->out.buf!=NULL && c->out.allocator.free_fn !=NULL){
        c->out.allocator.free_fn(c->out.allocator.userdata,c->out.buf);
    }
    if(c->tmp.buf!=NULL && c->tmp.allocator.free_fn !=NULL){
        c->tmp.allocator.free_fn(c->tmp.allocator.userdata,c->tmp.buf);
    }
    if(c->arena.values!=NULL && c->arena.allocator.free_fn !=NULL){
        c->arena.allocator.free_fn(c->arena.allocator.userdata,c->arena.values);
    }
    if(c->arena.props!=NULL && c->arena.allocator.free_fn !=NULL){
        c->arena.allocator.free_fn(c->arena.allocator.userdata,c->arena.props);
    }
    if(c->in.allocator.free_fn !=NULL){
        c->in.allocator.free_fn(c->in.allocator.userdata,c);
    }
}

bool jsx_compile(Compiler* c,const char* in,size_t len ){
    if(c->max_iter<=0){
        c->max_iter=MAX_ITER;
    }
    c->in.buf_count=0;
    c->out.buf_count=0;
    c->tmp.buf_count=0;
    c->arena.values_count=0;
    c->arena.prop_count=0;
    if(len>0){
        write_slice(&c->in,(Slice){.start=in,.len=len});
    }else{
        write_string(&c->in,in);
    }

    c->last =(CompileResult){0};
    c->last.type=OK;

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
                    c->last.type=ERR;
                    c->last.value.err=r.value.err;
                    c->last.value.err.at += got.content.start - c->in.buf;
                    return false;
                }
                changed = true;
                write_slice(&c->out, r.value.ok);
            }
        }
        swap_buffer(c);
    }
    c->last.value.ok = buffer_to_slice(&c->out);
    return true;
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
    transformer.buf.allocator=c->arena.allocator;
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

char* jsx_get_last_error(Compiler* c){
    if(c->last.type !=ERR){
        return NULL;
    }
    c->out.buf_count=0;
    write_string(&c->out,"compilation error at ");
    char at[10]={0};
    snprintf(at,10,"%d ",c->last.value.err.at);
    write_string(&c->out,at);
    write_string(&c->out," : ");
    write_string(&c->out,parser_error_to_string(c->last.value.err.code));
    write_char(&c->out,0);
    return c->out.buf;
}

char* jsx_get_output(Compiler* c){
    if(c->last.type !=OK){
        return NULL;
    }
    write_char(&c->out,0);
    return c->out.buf;
}
