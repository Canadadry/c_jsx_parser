#include "arena.h"
#include "ast.h"
#include "string.h"

int parser_grow_values(Arena* a){
    if(a->realloc_fn == NULL){
        return 0;
    }
    int next_capacity = (a->values_capacity*2)+1;
    a->values = a->realloc_fn(a->userdata,a->values,next_capacity*sizeof(Value));
    if(a->values == NULL){
        return 0;
    }
    a->values_capacity=next_capacity;
    return 1;
}

int parser_grow_prop(Arena* a){
    if(a->realloc_fn == NULL){
        return 0;
    }
    int next_capacity = (a->prop_capacity*2)+1;
    a->props = a->realloc_fn(a->userdata,a->props,next_capacity*sizeof(Prop));
    if(a->props == NULL){
        return 0;
    }
    a->prop_capacity=next_capacity;
    return 1;
}

ValueIndex get_next_value(Arena* a){
    if(a->values_count >= a->values_capacity){
        int ok = parser_grow_values(a);
        if(ok==0){
            return  -1;
        }
    }
    a->values_count++;
    return a->values_count-1;
}

PropIndex get_next_prop(Arena* a){
    if(a->prop_count >= a->prop_capacity){
       int ok = parser_grow_prop(a);
        if(ok==0){
            return  -1;
        }
    }
    a->prop_count++;
    return a->prop_count-1;
}

bool value_equal(Arena* arena_a,ValueIndex idx_a,Arena* arena_b,ValueIndex idx_b){
    if (
            idx_a < 0 || idx_a >= arena_a->values_count
        ||  idx_b < 0 || idx_b >= arena_b->values_count
    ) {
        return false;
    }
    Value* val_a = arena_a->values+idx_a;
    Value* val_b = arena_b->values+idx_b;

    if (val_a->type != val_b->type) return false;

    if (val_a->type == EXPR_NODE_TYPE) {
        return slice_equal(val_a->value.expr, val_b->value.expr);
    }
    if (val_a->type==TEXT_NODE_TYPE) {
        return slice_equal(val_a->value.text, val_b->value.text);
    }

    Node* a = &val_a->value.node;
    Node* b = &val_b->value.node;

    if (!slice_equal(a->Tag, b->Tag)) return false;

    PropIndex prop_a_idx = a->Props;
    PropIndex prop_b_idx = b->Props;
    SAFE_WHILE (prop_a_idx>=0 && prop_b_idx>=0,arena_a->prop_count) { //TODO fix prop_a not init at -1
        Prop prop_a = arena_a->props[prop_a_idx];
        Prop prop_b = arena_b->props[prop_b_idx];

        if (!slice_equal(prop_a.key, prop_b.key ) ||
            !slice_equal(prop_a.value,prop_b.value)) {
            return false;
        }
        if (prop_a.type != prop_b.type){
            return false;
        }
        prop_a_idx = prop_a.next;
        prop_b_idx = prop_b.next;
    }
    if (prop_a_idx != prop_b_idx) return false;


    ValueIndex child_a_idx = a->Children;
    ValueIndex child_b_idx = b->Children;
    SAFE_WHILE (child_a_idx >= 0 && child_b_idx >= 0,arena_a->values_count) { //TODO fix child_a_idx not init at -1
        if(!value_equal(arena_a,  child_a_idx,arena_b, child_b_idx)){
            return false;
        }
        child_a_idx = arena_a->values[child_a_idx].next;
        child_b_idx = arena_b->values[child_b_idx].next;
    }
    if (child_a_idx != child_b_idx) return false;

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

void value_print(Printer* p,Arena* arena,ValueIndex index,int indent) {
    if(index < 0 || index >= arena->values_count){
        return;
    }

    if (arena->values[index].type == EXPR_NODE_TYPE) {
        write_indent(p,indent+1);
        write_slice(p,arena->values[index].value.expr);
        write_string(p,"\n");
        return;
    }

    if (arena->values[index].type == TEXT_NODE_TYPE) {
        write_indent(p,indent+1);
        write_slice(p,arena->values[index].value.text);
        write_string(p,"\n");
        return;
    }

    Node* node = &arena->values[index].value.node;
    write_indent(p,indent);
    write_char(p,'<');
    write_slice(p,node->Tag);
    if (node->Props >= 0) {
        PropIndex prop = node->Props;
        SAFE_WHILE (prop >= 0,arena->prop_count) {
            write_string(p," ");
            write_slice(p,arena->props[prop].key);
            write_string(p," = ");
            if(arena->props[prop].type==EXPR_PROP_TYPE){
                write_slice(p,arena->props[prop].value);
            }else if(arena->props[prop].type==TEXT_PROP_TYPE){
                write_char(p,'"');
                write_slice(p,arena->props[prop].value);
                write_char(p,'"');
            }
            prop = arena->props[prop].next;
        }
    }
    write_string(p,">\n");

    if (node->Children >= 0) {
        ValueIndex child = node->Children;
        SAFE_WHILE (child >= 0,arena->values_count) {
            value_print(p,arena,child,indent+1);
            child = arena->values[child].next;
        }
    }

    write_indent(p,indent);
    write_string(p,"</");
    write_slice(p,node->Tag);
    write_string(p,">\n");
}
