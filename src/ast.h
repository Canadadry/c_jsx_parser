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

Node node_create(Slice tag);
void node_add_prop(Node* node, Prop* prop);
void node_add_child(Node* node, Child* child);
Child node_to_child(Node child);
Child expr_to_child(Slice expr);
bool node_equal(Node* a, Node* b);
void node_print(Node* node,int indent);

#endif
