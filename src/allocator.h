#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdlib.h>

typedef struct{
    void* (*realloc_fn)(void* userdata,void* ptr, size_t size);
    void (*free_fn)(void* userdata,void* ptr);
    void* userdata;
} Allocator;

#endif
