#include "parser.h"
#include "arena.h"
#include "ast.h"
#include "lexer.h"
#include "token.h"

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


static void parser_next_token(Parser* p) {
    p->curTok = p->peekTok;
    p->peekTok = GetNextToken(p->lexer);
}

typedef struct {
    ResultType type;
    union {
        PropIndex ok;
        Error err;
    } value;
} PropsResults;

static inline PropsResults parse_props(Parser* p) {
    PropsResults result = {0};
    result.type=OK;
    result.value.ok=-1;

    while (p->curTok.type == TOKEN_IDENT) {
        PropIndex prop_index = get_next_prop(p->arena);
        p->arena->props[prop_index].next=-1;
        p->arena->props[prop_index].key = p->curTok.literal;
        p->arena->props[prop_index].value = slice_from("true");
        p->arena->props[prop_index].type = EXPR_PROP_TYPE;
        parser_next_token(p);

        if (p->curTok.type == TOKEN_EQUAL) {
            parser_next_token(p);
            if (p->curTok.type == TOKEN_STRING) {
                p->arena->props[prop_index].value = p->curTok.literal;
                p->arena->props[prop_index].type = TEXT_PROP_TYPE;
                parser_next_token(p);
            } else if (p->curTok.type == TOKEN_EXPR) {
                p->arena->props[prop_index].value = p->curTok.literal;
                p->arena->props[prop_index].type = EXPR_PROP_TYPE;
                parser_next_token(p);
            } else {
                result.type = ERR;
                result.value.err.code = PARSER_ERR_EXPECTED_PROP;
                result.value.err.at = p->curTok.pos;
                result.value.err.token = p->curTok.type;
                return result;
            }
        }
        p->arena->props[prop_index].next=result.value.ok;
        result.value.ok=prop_index;
    }

    return result;
}

typedef struct {
    ResultType type;
    union {
        ValueIndex ok;
        Error err;
    } value;
} ChildrenResults;

static void set_error(ChildrenResults* result, ParseErrorCode err,int at,TokenType type){
    result->type = ERR;
    result->value.err.code = err;
    result->value.err.token = type;
    result->value.err.at=at;
}


Error parse_child_node(Parser* p,ValueIndex child) ;

ValueIndex reverse_children(Arena* arena,ValueIndex head) {
    if(head<0){
        return head;
    }
    ValueIndex prev = -1;
    ValueIndex current = head;
    ValueIndex next = -1;

    while (current >= 0) {
        next = arena->values[current].next;
        arena->values[current].next = prev;
        prev = current;
        current = next;
    }
    return prev;
}

static inline ChildrenResults parse_children(Parser* p) {
    ChildrenResults result={0};
    result.type=OK;
    result.value.ok=-1;

    while (p->curTok.type != TOKEN_OPEN_TAG || (p->curTok.type == TOKEN_OPEN_TAG && p->peekTok.type != TOKEN_SLASH)) {
        ValueIndex child_index =  get_next_value(p->arena);
        p->arena->values[child_index].next=-1;
        switch (p->curTok.type) {
            case TOKEN_TEXT:
                p->arena->values[child_index].type = TEXT_NODE_TYPE;
                p->arena->values[child_index].value.text= p->curTok.literal;
                parser_next_token(p);
                break;
            case TOKEN_EXPR:
                p->arena->values[child_index].type = EXPR_NODE_TYPE;
                p->arena->values[child_index].value.text= p->curTok.literal;
                parser_next_token(p);
                break;
            case TOKEN_OPEN_TAG: {
                Error err = parse_child_node(p,child_index);
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
        p->arena->values[child_index].next=result.value.ok;
        result.value.ok=child_index;
    }
    result.value.ok=reverse_children(p->arena,result.value.ok);
    return result;
}

static inline Error parse_closing_tag(Parser* p,Slice tag){
    if (p->curTok.type != TOKEN_OPEN_TAG || p->peekTok.type != TOKEN_SLASH) {
        return (Error){.code=PARSER_ERR_CLOSING_UNEXPECTED_TOKEN_1,.at=p->curTok.pos,.token=p->curTok.type};
    }
    parser_next_token(p);
    parser_next_token(p);

    if (p->curTok.type != TOKEN_IDENT || !slice_equal(p->curTok.literal, tag)) {

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

Error parse_child_node(Parser* p,ValueIndex child_idx) {
    if (child_idx <0){
        return (Error){.code=PARSER_ERR_MEMORY_ALLOCATION,.at=p->curTok.pos,.token=p->curTok.type};
    }
    p->arena->values[child_idx].type = NODE_NODE_TYPE;
    p->arena->values[child_idx].value.node.Children=-1;
    p->arena->values[child_idx].value.node.Props=-1;


    if (p->curTok.type != TOKEN_OPEN_TAG) {
        return (Error){.code=PARSER_ERR_EXPECTED_OPENTAG,.at=p->curTok.pos,.token=p->curTok.type};
    }
    parser_next_token(p);

    if (p->curTok.type != TOKEN_IDENT) {
        return (Error){.code=PARSER_ERR_EXPECTED_OPENTAG_NAME,.at=p->curTok.pos,.token=p->curTok.type};
    }
    p->arena->values[child_idx].value.node.Tag = p->curTok.literal;
    // printf("start tag(%d) %.*s\n",node->Tag.len,node->Tag.len,node->Tag.start);
    parser_next_token(p);

    PropsResults props_result = parse_props(p);
    if (props_result.type == ERR){
        return props_result.value.err;
    }
    p->arena->values[child_idx].value.node.Props = props_result.value.ok;

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
    p->arena->values[child_idx].value.node.Children = children_result.value.ok;

    return parse_closing_tag(p,p->arena->values[child_idx].value.node.Tag);
}

ParseNodeResult ParseNode(Parser* p) {
    ParseNodeResult result = {0};
    result.type=OK;
    ValueIndex child_idx =  get_next_value(p->arena);
    if (child_idx<0){
        result.type=ERR;
        result.value.err=(Error){.code=PARSER_ERR_MEMORY_ALLOCATION,.at=p->curTok.pos,.token=p->curTok.type};
        return result;
    }
    p->arena->values[child_idx].next = -1;
    result.value.ok=child_idx;
    Error err = parse_child_node(p,child_idx);
    if (err.code!=PARSER_OK){
        result.type=ERR;
        result.value.err=err;
    }
    return result;
}
