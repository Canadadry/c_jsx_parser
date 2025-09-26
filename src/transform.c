#include "transform.h"
#include "arena.h"
#include "ast.h"

static inline char isUpper(char ch) {
    return (ch >= 'A' && ch <= 'Z');
}

void genNode(Transformer* t,Arena* arena,ValueIndex node_idx){
    Node* node = &arena->values[node_idx].value.node;
    write_slice(&t->buf,t->createElem);
    if(isUpper(node->Tag.start[0])){
        write_slice(&t->buf,node->Tag);
    }else{
        write_char(&t->buf, '"');
        write_slice(&t->buf,node->Tag);
        write_char(&t->buf, '"');
    }

    write_string(&t->buf, ", ");

    if(node->Props<0){
        write_string(&t->buf, "null");
    }else{
        Prop* p = &arena->props[node->Props];

        write_string(&t->buf, "{ ");
        write_slice(&t->buf, p->key);
        write_string(&t->buf, " : ");
        if(p->type==EXPR_PROP_TYPE){
            write_slice(&t->buf, p->value);
        }else if(p->type==TEXT_PROP_TYPE){
            write_string(&t->buf, "\"");
            write_slice(&t->buf, p->value);
            write_string(&t->buf, "\"");
        }
        PropIndex p_idx = p->next;
        SAFE_WHILE(p_idx>=0,arena->prop_count){
            p= &arena->props[p_idx];
            write_string(&t->buf, ", ");
            write_slice(&t->buf, p->key);
            write_string(&t->buf, " : ");
            if(p->type==EXPR_PROP_TYPE){
                write_slice(&t->buf, p->value);
            }else if(p->type==TEXT_PROP_TYPE){
                write_string(&t->buf, "\"");
                write_slice(&t->buf, p->value);
                write_string(&t->buf, "\"");
            }
            p_idx = p->next;
        }
        write_string(&t->buf, " }");
    }

    ValueIndex child_idx =node->Children;
    SAFE_WHILE(child_idx>=0,arena->values_count){
        Value* c = &arena->values[child_idx];
        write_string(&t->buf, ", ");
        switch(c->type){
            case TEXT_NODE_TYPE:
                write_char(&t->buf, '"');
                write_slice(&t->buf,c->value.text);
                write_char(&t->buf, '"');
                break;
            case EXPR_NODE_TYPE:
                write_slice(&t->buf,c->value.expr);
                break;
            case NODE_NODE_TYPE:
                genNode(t,arena,child_idx);
        }
        child_idx=c->next;
    }
    write_char(&t->buf,')');
}

void Transform(Transformer* t,Arena* arena,ValueIndex node){
    genNode(t,arena,node);
}
