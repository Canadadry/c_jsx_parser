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
    CompileResult last;
} Compiler;

Compiler* jsx_new_compiler(const char* createElem,Allocator allocator);
void jsx_free_compiler(Compiler* c);
bool jsx_compile(Compiler* c,const char* in,size_t len );
char* jsx_get_last_error(Compiler* c);
char* jsx_get_output(Compiler* c);

#endif
