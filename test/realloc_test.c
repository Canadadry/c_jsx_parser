#include "realloc_test.h"
#include <execinfo.h>
#include <string.h>
#include <stdio.h>

#define MAX_CALLSTACK_DEPTH 64

void print_callstack(int skip) {
    void *buffer[MAX_CALLSTACK_DEPTH];
    int nptrs = backtrace(buffer, MAX_CALLSTACK_DEPTH);
    char **symbols = backtrace_symbols(buffer, nptrs);

    if (symbols == NULL) {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

    printf("Call stack:\n");
    for (int i = skip; i < nptrs; i++) {
        printf("  [%d] %s\n", i, symbols[i]);
    }

    free(symbols);
}

void* fn_realloc(void* userdata,void* ptr, size_t size){
    // print_callstack(2);
    // printf("allocating memory %lu\n",size);
    return realloc(ptr, size);
}

void fn_free(void* userdata,void* ptr){
    free(ptr);
}
