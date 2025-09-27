#ifndef JSX_H
#define JSX_H

#include "allocator.h"
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

Compiler* new_compiler(const char* createElem,Allocator allocator);
void free_compiler(Compiler* c);
// Compiler static_memory_compiler(const char* createElem);
CompileResult compile(Compiler* c,Slice in );

#endif
