#include "parser.h"
#include "lexer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void InitParser(Parser *parser) {
    parser->curTok = GetNextToken(parser->lexer);
    parser->peekTok = GetNextToken(parser->lexer);
}

static void parser_next_token(Parser* p) {
    p->curTok = p->peekTok;
    p->peekTok = GetNextToken(p->lexer);
}

ParseNodeResult ParseNode(Parser* p, Node* node) {
    ParseNodeResult result = {0};

    if (p->curTok.type != TOKEN_OPEN_TAG) {
        result.type = ERR;
        result.value.err = PARSER_ERR_EXPECTED_TAG;
        return result;
    }
    parser_next_token(p);

    if (p->curTok.type != TOKEN_IDENT) {
        result.type = ERR;
        result.value.err = PARSER_ERR_UNEXPECTED_TOKEN;
        return result;
    }
    Slice tag = p->curTok.literal;
    parser_next_token(p);

    // Props
    while (p->curTok.type == TOKEN_IDENT) {
        Slice key = p->curTok.literal;
        const char* value = NULL;

        parser_next_token(p);
        if (p->curTok.type == TOKEN_EQUAL) {
            parser_next_token(p);
            if (p->curTok.type == TOKEN_STRING) {
                value = p->curTok.literal.start;
                parser_next_token(p);
            } else if (p->curTok.type == TOKEN_EXPR) {
                value = p->curTok.literal.start;
                parser_next_token(p);
            } else {
                result.type = ERR;
                result.value.err = PARSER_ERR_EXPECTED_PROP;
                return result;
            }
        } else {
            value = "true";
        }

        if (p->prop_count >= p->prop_capacity) {
            if (p->realloc_fn) {
                p->props = (Prop*)p->realloc_fn(p->userdata, p->props, sizeof(Prop) * (p->prop_capacity * 2));
                if (!p->props) {
                    result.type = ERR;
                    result.value.err = PARSER_ERR_MEMORY_ALLOCATION;
                    return result;
                }
                p->prop_capacity *= 2;
            } else {
                result.type = ERR;
                result.value.err = PARSER_ERR_MEMORY_ALLOCATION;
                return result;
            }
        }

        Prop* prop = &p->props[p->prop_count++];
        prop->key = key;
        prop->value.start = value;
        prop->value.len = strlen(value);
    }

    if (p->curTok.type == TOKEN_SLASH && p->peekTok.type == TOKEN_CLOSE_TAG) {
        parser_next_token(p);
        parser_next_token(p);
        node->Tag = tag;
        node->Props = p->props;
        node->Children = NULL;
        result.type = OK;
        result.value.ok = node;
        return result;
    }

    if (p->curTok.type != TOKEN_CLOSE_TAG) {
        result.type = ERR;
        result.value.err = PARSER_ERR_UNEXPECTED_TOKEN;
        return result;
    }
    parser_next_token(p);

    size_t child_capacity = 4;
    size_t child_count = 0;
    Child* children = (Child*)malloc(sizeof(Child) * child_capacity);
    if (!children) {
        result.type = ERR;
        result.value.err = PARSER_ERR_MEMORY_ALLOCATION;
        return result;
    }

    while (p->curTok.type != TOKEN_OPEN_TAG || (p->curTok.type == TOKEN_OPEN_TAG && p->peekTok.type != TOKEN_SLASH)) {
        switch (p->curTok.type) {
            case TOKEN_TEXT:
                if (child_count >= child_capacity) {
                    child_capacity *= 2;
                    children = (Child*)realloc(children, sizeof(Child) * child_capacity);
                    if (!children) {
                        result.type = ERR;
                        result.value.err = PARSER_ERR_MEMORY_ALLOCATION;
                        return result;
                    }
                }
                children[child_count].type = EXPR_TYPE;
                children[child_count].value.expr = p->curTok.literal;
                children[child_count].next = NULL;
                child_count++;
                parser_next_token(p);
                break;
            case TOKEN_EXPR:
                if (child_count >= child_capacity) {
                    child_capacity *= 2;
                    children = (Child*)realloc(children, sizeof(Child) * child_capacity);
                    if (!children) {
                        result.type = ERR;
                        result.value.err = PARSER_ERR_MEMORY_ALLOCATION;
                        return result;
                    }
                }
                children[child_count].type = EXPR_TYPE;
                children[child_count].value.expr = p->curTok.literal;
                children[child_count].next = NULL;
                child_count++;
                parser_next_token(p);
                break;
            case TOKEN_OPEN_TAG: {
                Node* child_node = (Node*)malloc(sizeof(Node));
                if (!child_node) {
                    result.type = ERR;
                    result.value.err = PARSER_ERR_MEMORY_ALLOCATION;
                    return result;
                }

                ParseNodeResult child_result = ParseNode(p, child_node);
                if (child_result.type != OK) {
                    free(child_node);
                    free(children);
                    return child_result;
                }

                if (child_count >= child_capacity) {
                    child_capacity *= 2;
                    children = (Child*)realloc(children, sizeof(Child) * child_capacity);
                    if (!children) {
                        result.type = ERR;
                        result.value.err = PARSER_ERR_MEMORY_ALLOCATION;
                        return result;
                    }
                }

                children[child_count].type = NODE_TYPE;
                children[child_count].value.node = *child_node;
                children[child_count].next = NULL;
                child_count++;
                break;
            }
            case TOKEN_EOF:
                free(children);
                result.type = ERR;
                result.value.err = PARSER_ERR_UNEXPECTED_TOKEN;
                return result;
            default:
                free(children);
                result.type = ERR;
                result.value.err = PARSER_ERR_UNEXPECTED_TOKEN;
                return result;
        }
    }

    if (p->curTok.type == TOKEN_OPEN_TAG && p->peekTok.type == TOKEN_SLASH) {
        parser_next_token(p);
        parser_next_token(p);
        if (p->curTok.type != TOKEN_IDENT || strncmp(p->curTok.literal.start, tag.start, tag.len) != 0) {
            free(children);
            result.type = ERR;
            result.value.err = PARSER_ERR_UNEXPECTED_TOKEN;
            return result;
        }
        parser_next_token(p);
        if (p->curTok.type != TOKEN_CLOSE_TAG) {
            free(children);
            result.type = ERR;
            result.value.err = PARSER_ERR_UNEXPECTED_TOKEN;
            return result;
        }
        parser_next_token(p);
    }

    node->Tag = tag;
    node->Props = p->props;
    node->Children = children;

    result.type = OK;
    result.value.ok = node;
    return result;
}
