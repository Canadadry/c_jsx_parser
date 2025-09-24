#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "token.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char* parser_error_to_string(ParseErrorCode err){
    switch(err){
        case PARSER_OK: return "no error" ;
        case PARSER_ERR_CHILDREN_UNEXPECTED_TOKEN: return "children_unexpected_token";
        case PARSER_ERR_CLOSING_UNEXPECTED_TOKEN_1: return "closing_unexpected_token_1 expect </" ;
        case PARSER_ERR_CLOSING_UNEXPECTED_TOKEN_2: return "closing_unexpected_token_2 expect good tagname" ;
        case PARSER_ERR_CLOSING_UNEXPECTED_TOKEN_3: return "closing_unexpected_token_3 expect >" ;
        case PARSER_ERR_EXPECTED_OPENTAG_NAME: return "expected_opentag_name";
        case PARSER_ERR_EXPECTED_OPENTAG: return "expected_opentag" ;
        case PARSER_ERR_EXPECTED_ENDTAG: return "expected_endtag" ;
        case PARSER_ERR_EXPECTED_PROP: return "expected_prop" ;
        case PARSER_ERR_MEMORY_ALLOCATION: return "memory_allocation" ;
    }
    return "";
}

void InitParser(Parser *parser) {
    parser->curTok = GetNextToken(parser->lexer);
    parser->peekTok = GetNextToken(parser->lexer);
}

static inline int parser_grow_children(Parser* p){
    if(p->realloc_fn == NULL){
        return 0;
    }
    int next_capacity = (p->child_capacity*2)+1;
    p->children = p->realloc_fn(p->userdata,p->children,next_capacity*sizeof(Child));
    if(p->children == NULL){
        return 0;
    }
    p->child_capacity=next_capacity;
    return 1;
}

static inline int parser_grow_prop(Parser* p){
    if(p->realloc_fn == NULL){
        return 0;
    }
    int next_capacity = (p->prop_capacity*2)+1;
    p->props = p->realloc_fn(p->userdata,p->props,next_capacity*sizeof(Prop));
    if(p->props == NULL){
        return 0;
    }
    p->prop_capacity=next_capacity;
    return 1;
}

static inline Child* get_next_child(Parser* p){
    if(p->child_capacity == 0 || p->child_count == p->child_capacity){
        int ok = parser_grow_children(p);
        if(ok ==0){
            return  NULL;
        }
    }
    Child* c = &p->children[p->child_count];
    p->child_count++;
    return c;
}

static inline Prop* get_next_prop(Parser* p){
    if(p->prop_capacity == 0 || p->prop_count == p->prop_capacity){
        int ok = parser_grow_prop(p);
        if(ok ==0){
            return  NULL;
        }
    }
    Prop* prop = &p->props[p->prop_count];
    p->prop_count++;
    return prop;
}

static void parser_next_token(Parser* p) {
    p->curTok = p->peekTok;
    p->peekTok = GetNextToken(p->lexer);
}


typedef struct {
    ResultType type;
    union {
        Prop* ok;
        Error err;
    } value;
} PropsResults;


static inline PropsResults parse_props(Parser* p) {
    PropsResults result = {0};
    result.type=OK;

    while (p->curTok.type == TOKEN_IDENT) {
        Prop* prop = get_next_prop(p);
        prop->key = p->curTok.literal;
        prop->value = slice_from("true");
        prop->type = EXPR_PROP_TYPE;
        parser_next_token(p);

        if (p->curTok.type == TOKEN_EQUAL) {
            parser_next_token(p);
            if (p->curTok.type == TOKEN_STRING) {
                prop->value = p->curTok.literal;
                prop->type = TEXT_PROP_TYPE;
                parser_next_token(p);
            } else if (p->curTok.type == TOKEN_EXPR) {
                prop->value = p->curTok.literal;
                prop->type = EXPR_PROP_TYPE;
                parser_next_token(p);
            } else {
                result.type = ERR;
                result.value.err.code = PARSER_ERR_EXPECTED_PROP;
                result.value.err.at = p->curTok.pos;
                result.value.err.token = p->curTok.type;
                return result;
            }
        }
        prop->next=result.value.ok;
        result.value.ok=prop;
    }

    return result;
}

typedef struct {
    ResultType type;
    union {
        Child* ok;
        Error err;
    } value;
} ChildrenResults;

static void set_error(ChildrenResults* result, ParseErrorCode err,int at,TokenType type){
    result->type = ERR;
    result->value.err.code = err;
    result->value.err.token = type;
    result->value.err.at=at;
}


Error parse_child_node(Parser* p,Child* child) ;

Child* reverse_children(Child* head) {
    Child* prev = NULL;
    Child* current = head;
    Child* next = NULL;

    while (current != NULL) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }

    return prev;
}

static inline ChildrenResults parse_children(Parser* p) {
    ChildrenResults result={0};
    result.type=OK;

    while (p->curTok.type != TOKEN_OPEN_TAG || (p->curTok.type == TOKEN_OPEN_TAG && p->peekTok.type != TOKEN_SLASH)) {
        Child* child =  get_next_child(p);
        switch (p->curTok.type) {
            case TOKEN_TEXT:
                child->type = TEXT_NODE_TYPE;
                child->value.text= p->curTok.literal;
                parser_next_token(p);
                break;
            case TOKEN_EXPR:
                child->type = EXPR_NODE_TYPE;
                child->value.text= p->curTok.literal;
                parser_next_token(p);
                break;
            case TOKEN_OPEN_TAG: {
                Error err = parse_child_node(p,child);
                if (err.code != PARSER_OK) {
                    result.value.err= err;
                    return result;
                }
                break;
            }
            default:
                set_error(&result,PARSER_ERR_CHILDREN_UNEXPECTED_TOKEN,p->curTok.pos,p->curTok.type);
                return result;
        }
        child->next=result.value.ok;
        result.value.ok=child;
    }
    result.value.ok=reverse_children(result.value.ok);
    return result;
}

static inline Error parse_closing_tag(Parser* p,Slice tag){
    if (p->curTok.type != TOKEN_OPEN_TAG || p->peekTok.type != TOKEN_SLASH) {
        return (Error){.code=PARSER_ERR_CLOSING_UNEXPECTED_TOKEN_1,.at=p->curTok.pos,.token=p->curTok.type};
    }
    parser_next_token(p);
    parser_next_token(p);

    if (p->curTok.type != TOKEN_IDENT || slice_equal(p->curTok.literal, tag) != 0) {

        // printf("end tag dont match with (%d) %.*s\n",tag.len,tag.len,tag.start);
        // printf("got (%d) %.*s\n",p->curTok.literal.len,p->curTok.literal.len,p->curTok.literal.start);
        return (Error){.code=PARSER_ERR_CLOSING_UNEXPECTED_TOKEN_2,.at=p->curTok.pos,.token=p->curTok.type};
    }
    parser_next_token(p);
    if (p->curTok.type != TOKEN_CLOSE_TAG) {
        return (Error){.code=PARSER_ERR_CLOSING_UNEXPECTED_TOKEN_3,.at=p->curTok.pos,.token=p->curTok.type};
    }
    parser_next_token(p);
    return (Error){.code=PARSER_OK,.at=0};
}

Error parse_child_node(Parser* p,Child* child) {
    child->type = NODE_NODE_TYPE;
    Node* node= &child->value.node;

    if (node == NULL){
        return (Error){.code=PARSER_ERR_MEMORY_ALLOCATION,.at=p->curTok.pos,.token=p->curTok.type};
    }

    if (p->curTok.type != TOKEN_OPEN_TAG) {
        return (Error){.code=PARSER_ERR_EXPECTED_OPENTAG,.at=p->curTok.pos,.token=p->curTok.type};
    }
    parser_next_token(p);

    if (p->curTok.type != TOKEN_IDENT) {
        return (Error){.code=PARSER_ERR_EXPECTED_OPENTAG_NAME,.at=p->curTok.pos,.token=p->curTok.type};
    }
    node->Tag = p->curTok.literal;
    // printf("start tag(%d) %.*s\n",node->Tag.len,node->Tag.len,node->Tag.start);
    parser_next_token(p);

    PropsResults props_result = parse_props(p);
    if (props_result.type == ERR){
        return props_result.value.err;
    }
    node->Props = props_result.value.ok;

    if (p->curTok.type == TOKEN_SLASH && p->peekTok.type == TOKEN_CLOSE_TAG) {
        parser_next_token(p);
        parser_next_token(p);
        return (Error){.code=PARSER_OK,.at=p->curTok.pos};
    }

    if (p->curTok.type != TOKEN_CLOSE_TAG) {
        return (Error){.code=PARSER_ERR_EXPECTED_ENDTAG,.at=p->curTok.pos,.token=p->curTok.type};
    }
    parser_next_token(p);

    ChildrenResults children_result = parse_children(p);
    if (children_result.type == ERR){
        return children_result.value.err;
    }
    node->Children = children_result.value.ok;

    return parse_closing_tag(p,node->Tag);
}

ParseNodeResult ParseNode(Parser* p) {
    Child* child =  get_next_child(p);
    ParseNodeResult result = {0};
    result.type=OK;
    result.value.ok=&child->value.node;
    Error err = parse_child_node(p,child);
    if (err.code!=PARSER_OK){
        result.type=ERR;
        result.value.err=err;
    }
    return result;
}
