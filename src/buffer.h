#ifndef _BUFFER_H
#define _BUFFER_H

#include "slice.h"
#include <stdio.h>

typedef struct {
    char* buf;
    size_t buf_count;
    size_t buf_capacity;
    void* (*realloc_fn)(void* userdata,void* ptr, size_t size);
    void* userdata;
} Buffer;

void write_slice(Buffer* p,Slice str);
void write_string(Buffer* p,const char* str);
void write_char(Buffer* p,char c);

#endif
