#ifndef _SLICE_H
#define _SLICE_H

#include <stdbool.h>

typedef struct{
    const char* start;
    int len;
} Slice;

bool slice_equal(Slice left, Slice right);
Slice slice_from(const char* str);

#endif // _SLICE_H
