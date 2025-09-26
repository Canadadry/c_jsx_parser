#ifndef JSX_H
#define JSX_H

#include "buffer.h"
#include "parser.h"

typedef struct {
    ResultType type;
    union {
        Slice ok;
        Error err;
    } value;
} CompileResult;

typedef struct{
    int max_iter;
    Slice createElem;
    Buffer in;
    Buffer out;
    Buffer tmp;
    Arena arena;
} Compiler;


CompileResult compile(Compiler* c);

#endif
