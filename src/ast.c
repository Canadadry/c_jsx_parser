#include "ast.h"
#include <string.h>
#include <stdlib.h>

Node node_create(Slice tag) {
    Node node;
    node.Tag = tag;
    node.Props = NULL;
    node.Children = NULL;
    return node;
}

void node_add_prop(Node* node, Prop* prop) {
    if (node == NULL || prop == NULL) return;

    prop->next = node->Props;
    node->Props = prop;
}

void node_add_child(Node* node, Child* child) {
    if (node == NULL || child == NULL) return;

    child->next = node->Children;
    node->Children = child;
}

Child node_to_child(Node child) {
    Child new_child;
    new_child.type = NODE_TYPE;
    new_child.value.node = child;
    new_child.next = NULL;
    return new_child;
}

Child expr_to_child(Slice expr) {
    Child new_child;
    new_child.type = EXPR_TYPE;
    new_child.value.expr = expr;
    new_child.next = NULL;
    return new_child;
}

bool node_equal(Node* a, Node* b) {
    if (a == NULL || b == NULL) return false;

    if (memcmp(&a->Tag, &b->Tag, sizeof(Slice)) != 0) return false;

    Prop* prop_a = a->Props;
    Prop* prop_b = b->Props;
    while (prop_a && prop_b) {
        if (memcmp(&prop_a->key, &prop_b->key, sizeof(Slice)) != 0 ||
            memcmp(&prop_a->value, &prop_b->value, sizeof(Slice)) != 0) {
            return false;
        }
        prop_a = prop_a->next;
        prop_b = prop_b->next;
    }

    if (prop_a || prop_b) return false;

    Child* child_a = a->Children;
    Child* child_b = b->Children;
    while (child_a && child_b) {
        if (child_a->type != child_b->type) return false;

        if (child_a->type == NODE_TYPE) {
            if (!node_equal(&child_a->value.node, &child_b->value.node)) return false;
        } else {
            if (memcmp(&child_a->value.expr, &child_b->value.expr, sizeof(Slice)) != 0) return false;
        }

        child_a = child_a->next;
        child_b = child_b->next;
    }

    if (child_a || child_b) return false;

    return true;
}
