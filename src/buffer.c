#include "buffer.h"
#include <string.h>

#define MIN(x,y) ((x)<(y)?(x):(y))

static inline bool grow_buf(Buffer* b,size_t len){
    if(b->buf_count+len <b->buf_capacity){
        return true;
    }
    if(b->realloc_fn == NULL){
        return false;
    }
    int next_capacity = b->buf_capacity;
    if(next_capacity==0){
        next_capacity=1;
    }
    while(b->buf_count+len >= next_capacity){
        next_capacity = 2*next_capacity;
    }
    b->buf = b->realloc_fn(b->userdata,b->buf,next_capacity);
    if(b->buf == NULL){
        return false;
    }
    b->buf_capacity=next_capacity;
    return true;
}

 void write_slice(Buffer* b,Slice str){
    if(!grow_buf(b,str.len)){
        return;
    }
    memcpy(b->buf+b->buf_count, str.start,str.len);
    b->buf_count+=str.len;
}

 void write_string(Buffer* b,const char* str){
    write_slice(b,slice_from(str));
}

 void write_char(Buffer* b,char c){
    if(!grow_buf(b,1)){
        return;
    }
    b->buf[b->buf_count]=c;
    b->buf_count++;
}


void buffer_clear(Buffer* b){
    b->buf_count=0;
}

Slice buffer_to_slice(Buffer* b){
    return (Slice){.start=b->buf,.len=b->buf_count};
}
