#include "token.h"
#include <string.h>

Slice slice_from_null_temrinated_string(const char* str){
    return (Slice){
        .start=str,
        .len=strlen(str),
    };
}
