#ifndef JSX_H
#define JSX_H

#include "parser.h"

typedef struct {
    ResultType type;
    union {
        Slice ok;
        Error err;
    } value;
} CompileResult;

typedef struct{
    Slice createElem;
    char*  in_buf;
    size_t in_buf_count;
    size_t in_buf_capacity;
    char*  out_buf;
    size_t out_buf_count;
    size_t out_buf_capacity;
    char*  transform_buf;
    size_t transform_buf_capacity;
    Arena arena;
} Compiler;


CompileResult compile(Compiler* c);

#endif
