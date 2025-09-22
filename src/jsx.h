#ifndef JSX_H
#define JSX_H

#include "parser.h"

typedef struct {
    ResultType type;
    union {
        Slice ok;
        ParseErrorCode err;
    } value;
} CompileResult;

typedef struct{
    Slice createElem;
    char* buf;
    size_t buf_capacity;
    Prop *props;
    size_t prop_capacity;
    Child *children;
    size_t child_capacity;
    void* (*realloc_fn)(void* userdata,void* ptr, size_t size);
    void* userdata;
} Compiler;


CompileResult compile(Compiler* c,Slice in);

#endif
