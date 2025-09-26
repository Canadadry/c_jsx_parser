#ifndef AST_H
#define AST_H

#include <stddef.h>
#include <stdbool.h>
#include "token.h"


typedef int PropIndex;

typedef struct Prop {
    Slice key;
    Slice value;
    enum {
        UNDEF_PROP_TYPE,
        EXPR_PROP_TYPE,
        TEXT_PROP_TYPE
    } type;
    PropIndex next;
} Prop;


typedef int ValueIndex;

typedef struct Node {
    Slice Tag;
    PropIndex Props;
    ValueIndex Children;

} Node;

typedef struct Value{
    enum {
        NODE_NODE_TYPE,
        EXPR_NODE_TYPE,
        TEXT_NODE_TYPE
    } type;
    union {
        Node node;
        Slice expr;
        Slice text;
    } value;
    ValueIndex next;
} Value;

#define SAFE_WHILE(cond, max_iters) \
    for (int _sw_guard = 0; (cond) && _sw_guard < (max_iters); _sw_guard++)

#endif
