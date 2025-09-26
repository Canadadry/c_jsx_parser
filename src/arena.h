#ifndef ARENA_H
#define ARENA_H

#include "ast.h"
#include "buffer.h"

typedef struct{
    ValueIndex root;
    Value* values;
    size_t values_count;
    size_t values_capacity;
    Prop* props;
    size_t prop_count;
    size_t prop_capacity;
    void* (*realloc_fn)(void* userdata,void* ptr, size_t size);
    void* userdata;
} Arena;

bool value_equal(Arena* arena_a,ValueIndex a,Arena* arena_b,ValueIndex b);

void value_print(Buffer* printer,Arena* arena,ValueIndex index,int indent);

int parser_grow_values(Arena* p);
int parser_grow_prop(Arena* p);
ValueIndex get_next_value(Arena* p);
PropIndex get_next_prop(Arena* p);

#endif // ARENA_H
