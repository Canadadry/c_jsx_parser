#include "token.h"
#include <string.h>

Slice slice_from(const char* str){
    return (Slice){
        .start=str,
        .len=strlen(str),
    };
}
