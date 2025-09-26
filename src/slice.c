#include "slice.h"
#include <string.h>

Slice slice_from(const char* str){
    return (Slice){
        .start=str,
        .len=strlen(str),
    };
}

bool slice_equal(Slice left, Slice right){
    if (left.len != right.len) return false;
    return strncmp(left.start, right.start, left.len) == 0 ;
}
