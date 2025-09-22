#ifndef AST_H
#define AST_H

#include <stddef.h>
#include <stdbool.h>
#include "token.h"

typedef struct Prop {
    Slice key;
    Slice value;
    struct Prop* next;
} Prop;

typedef struct Node {
    Slice Tag;
    Prop* Props;
    struct Child* Children;
} Node;

typedef struct Child{
    enum {
        NODE_TYPE,
        EXPR_TYPE,
        TEXT_TYPE
    } type;
    union {
        Node node;
        Slice expr;
        Slice text;
    } value;
    struct Child* next;
} Child;

bool node_equal(Node* a, Node* b);

typedef struct {
    char* buf;
    size_t buf_count;
    size_t buf_capacity;
    void* (*realloc_fn)(void* userdata,void* ptr, size_t size);
    void* userdata;
} Printer;

void node_print(Printer* printer,Node* node,int indent);

#endif
