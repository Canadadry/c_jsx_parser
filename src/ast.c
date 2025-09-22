#include "ast.h"
#include "token.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Node node_create(Slice tag) {
    Node node;
    node.Tag = tag;
    node.Props = NULL;
    node.Children = NULL;
    return node;
}

bool node_equal(Node* a, Node* b) {
    if (a == NULL || b == NULL) return false;
    if (slice_equal(a->Tag, b->Tag) != 0) return false;

    Prop* prop_a = a->Props;
    Prop* prop_b = b->Props;
    while (prop_a && prop_b) {
        if (slice_equal(prop_a->key, prop_b->key ) != 0 ||
            slice_equal(prop_a->value, prop_b->value) != 0) {
            return false;
        }
        prop_a = prop_a->next;
        prop_b = prop_b->next;
    }
    if (prop_a || prop_b) return false;

    Child* child_a = a->Children;
    Child* child_b = b->Children;
    while (child_a && child_b) {
        if (child_a->type != child_b->type) return false;

        if (child_a->type == NODE_TYPE) {
            if (!node_equal(&child_a->value.node, &child_b->value.node)) return false;
        } else {
            if (slice_equal(child_a->value.expr, child_b->value.expr) != 0) return false;
        }

        child_a = child_a->next;
        child_b = child_b->next;
    }

    if (child_a || child_b) return false;

    return true;
}






#define MIN(x,y) ((x)<(y)?(x):(y))

static inline int transformer_grow_buf(Printer* p,size_t len){
    if(p->buf_count+len <p->buf_capacity){
        return 1;
    }
    if(p->realloc_fn == NULL){
        return 0;
    }
    int next_capacity = MIN(2*p->buf_capacity,len);
    p->buf = p->realloc_fn(p->userdata,p->buf,next_capacity);
    if(p->buf == NULL){
        return 0;
    }
    p->buf_capacity=next_capacity;
    return 1;
}

static void write_slice(Printer* p,Slice str){
    if(transformer_grow_buf(p,str.len)==0){
        return;
    }
    memcpy(p->buf+p->buf_count, str.start,str.len);
    p->buf_count+=str.len;
}

static void write_string(Printer* p,const char* str){
    write_slice(p,slice_from(str));
}

static void write_char(Printer* p,char c){
    if(transformer_grow_buf(p,1)==0){
        return;
    }
    p->buf[p->buf_count]=c;
    p->buf_count++;
}

static void write_indent(Printer* p,int indent){
    for (int i=0;i<indent;i++){
        write_char(p,' ');
    }
}


void node_print(Printer* p,Node* node,int indent) {
    write_indent(p,indent);
    write_char(p,'<');
    write_slice(p,node->Tag);
    if (node->Props != NULL) {
        Prop* prop = node->Props;
        while (prop != NULL) {
            write_string(p," ");
            write_slice(p,prop->key);
            write_string(p," = ");
            write_slice(p,prop->value);
            prop = prop->next;
        }
    }
    write_string(p,">\n");


    if (node->Children != NULL) {
        Child* child = node->Children;
        while (child != NULL) {
            if (child->type == NODE_TYPE) {
                node_print(p,&child->value.node,indent+1);
            } else {
                write_indent(p,indent+1);
                write_slice(p,child->value.expr);
                write_string(p,"\n");
            }
            child = child->next;
        }
    }

    write_indent(p,indent);
    write_string(p,"</");
    write_slice(p,node->Tag);
    write_string(p,">\n");
}
