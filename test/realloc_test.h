#ifndef REALLOC_TEST_H
#define REALLOC_TEST_H
#include <stdlib.h>

void* fn_realloc(void* userdata,void* ptr, size_t size);
void  fn_free(void* userdata,void* ptr);

#endif //REALLOC_TEST_H
