#ifndef JSX_PARSER_H
#define JSX_PARSER_H

#include <stdlib.h>

typedef struct{
    void* (*realloc_fn)(void* userdata,void* ptr, size_t size);
    void (*free_fn)(void* userdata,void* ptr);
    void* userdata;
} Allocator;

typedef struct Compiler Compiler;

Compiler* jsx_new_compiler(const char* createElem,Allocator allocator);
void jsx_free_compiler(Compiler* c);
bool jsx_compile(Compiler* c,const char* in,size_t len );
char* jsx_get_last_error(Compiler* c);
char* jsx_get_output(Compiler* c);

#endif
