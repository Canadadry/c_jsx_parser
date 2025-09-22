#ifndef JSX_TRANSFORM_H
#define JSX_TRANSFORM_H

#include "token.h"
#include "parser.h"

typedef struct {
    char* buf;
    size_t buf_count;
    size_t buf_capacity;
    void* (*realloc_fn)(void* userdata,void* ptr, size_t size);
    void* userdata;
} Transformer;

void Transform(Transformer* t,Node* in);

#endif
