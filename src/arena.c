#include "arena.h"
#include "ast.h"
#include "string.h"

int parser_grow_values(Arena* a){
    if(a->allocator.realloc_fn == NULL){
        return 0;
    }
    int next_capacity = a->values_capacity;
    if(next_capacity==0){
        next_capacity=1;
    }
    while(a->values_count+1 >= next_capacity){
        next_capacity = 2*next_capacity;
    }
    a->values = a->allocator.realloc_fn(a->allocator.userdata,a->values,next_capacity*sizeof(Value));
    if(a->values == NULL){
        return 0;
    }
    a->values_capacity=next_capacity;
    return 1;
}

int parser_grow_prop(Arena* a){
    if(a->allocator.realloc_fn == NULL){
        return 0;
    }
    int next_capacity = a->prop_capacity;
    if(next_capacity==0){
        next_capacity=1;
    }
    while(a->prop_count+1 >= next_capacity){
        next_capacity = 2*next_capacity;
    }
    a->props = a->allocator.realloc_fn(a->allocator.userdata,a->props,next_capacity*sizeof(Prop));
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

static void write_indent(Buffer* b,int indent){
    for (int i=0;i<indent;i++){
        write_char(b,' ');
    }
}

void value_print(Buffer* b,Arena* arena,ValueIndex index,int indent) {
    if(index < 0 || index >= arena->values_count){
        return;
    }

    if (arena->values[index].type == EXPR_NODE_TYPE) {
        write_indent(b,indent+1);
        write_slice(b,arena->values[index].value.expr);
        write_string(b,"\n");
        return;
    }

    if (arena->values[index].type == TEXT_NODE_TYPE) {
        write_indent(b,indent+1);
        write_slice(b,arena->values[index].value.text);
        write_string(b,"\n");
        return;
    }

    Node* node = &arena->values[index].value.node;
    write_indent(b,indent);
    write_char(b,'<');
    write_slice(b,node->Tag);
    if (node->Props >= 0) {
        PropIndex prop = node->Props;
        SAFE_WHILE (prop >= 0,arena->prop_count) {
            write_string(b," ");
            write_slice(b,arena->props[prop].key);
            write_string(b," = ");
            if(arena->props[prop].type==EXPR_PROP_TYPE){
                write_slice(b,arena->props[prop].value);
            }else if(arena->props[prop].type==TEXT_PROP_TYPE){
                write_char(b,'"');
                write_slice(b,arena->props[prop].value);
                write_char(b,'"');
            }
            prop = arena->props[prop].next;
        }
    }
    write_string(b,">\n");

    if (node->Children >= 0) {
        ValueIndex child = node->Children;
        SAFE_WHILE (child >= 0,arena->values_count) {
            value_print(b,arena,child,indent+1);
            child = arena->values[child].next;
        }
    }

    write_indent(b,indent);
    write_string(b,"</");
    write_slice(b,node->Tag);
    write_string(b,">\n");
}
