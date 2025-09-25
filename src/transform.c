#include "transform.h"
#include "arena.h"
#include "ast.h"
#include "token.h"
#include <string.h>

static inline char isUpper(char ch) {
    return (ch >= 'A' && ch <= 'Z');
}

#define MIN(x,y) ((x)<(y)?(x):(y))

static inline int transformer_grow_buf(Transformer* t,size_t len){
    if(t->buf_count+len <t->buf_capacity){
        return 1;
    }
    if(t->realloc_fn == NULL){
        return 0;
    }
    int next_capacity = t->buf_capacity;
    if(next_capacity==0){
        next_capacity=1;
    }
    while(t->buf_count+len >= next_capacity){
        next_capacity = MIN(2*t->buf_capacity,len);
    }
    t->buf = t->realloc_fn(t->userdata,t->buf,next_capacity);
    if(t->buf == NULL){
        return 0;
    }
    t->buf_capacity=next_capacity;
    return 1;
}

static void write_slice(Transformer* t,Slice str){
    if(transformer_grow_buf(t,str.len)==0){
        return;
    }
    memcpy(t->buf+t->buf_count, str.start,str.len);
    t->buf_count+=str.len;
}

static void write_string(Transformer* t,const char* str){
    write_slice(t,slice_from(str));
}

static void write_char(Transformer* t,char c){
    if(transformer_grow_buf(t,1)==0){
        return;
    }
    t->buf[t->buf_count]=c;
    t->buf_count++;
}

void genNode(Transformer* t,Arena* arena,ValueIndex node_idx){
    Node* node = &arena->values[node_idx].value.node;
    write_slice(t,t->createElem);
    if(isUpper(node->Tag.start[0])){
        write_slice(t,node->Tag);
    }else{
        write_char(t, '"');
        write_slice(t,node->Tag);
        write_char(t, '"');
    }

    write_string(t, ", ");

    if(node->Props<0){
        write_string(t, "null");
    }else{
        Prop* p = &arena->props[node->Props];

        write_string(t, "{ ");
        write_slice(t, p->key);
        write_string(t, " : ");
        if(p->type==EXPR_PROP_TYPE){
            write_slice(t, p->value);
        }else if(p->type==TEXT_PROP_TYPE){
            write_string(t, "\"");
            write_slice(t, p->value);
            write_string(t, "\"");
        }
        PropIndex p_idx = p->next;
        while(p_idx>=0){
            p= &arena->props[p_idx];
            write_string(t, ", ");
            write_slice(t, p->key);
            write_string(t, " : ");
            if(p->type==EXPR_PROP_TYPE){
                write_slice(t, p->value);
            }else if(p->type==TEXT_PROP_TYPE){
                write_string(t, "\"");
                write_slice(t, p->value);
                write_string(t, "\"");
            }
            p_idx = p->next;
        }
        write_string(t, " }");
    }

    ValueIndex child_idx =node->Children;
    while(child_idx>=0){
        Value* c = &arena->values[child_idx];
        write_string(t, ", ");
        switch(c->type){
            case TEXT_NODE_TYPE:
                write_char(t, '"');
                write_slice(t,c->value.text);
                write_char(t, '"');
                break;
            case EXPR_NODE_TYPE:
                write_slice(t,c->value.expr);
                break;
            case NODE_NODE_TYPE:
                genNode(t,arena,child_idx);
        }
        child_idx=c->next;
    }
    write_char(t,')');
}

void Transform(Transformer* t,Arena* arena,ValueIndex node){
    genNode(t,arena,node);
}
