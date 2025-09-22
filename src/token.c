#include "token.h"
#include <string.h>

Slice slice_from(const char* str){
    return (Slice){
        .start=str,
        .len=strlen(str),
    };
}

int slice_equal(Slice left, Slice right){
    if (left.len != right.len) return 0;
    return strncmp(left.start, right.start, left.len);
}
