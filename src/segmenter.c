#include "segmenter.h"

static inline int is_letter(char code){
    return (code > 'a' && code < 'z') || (code > 'A' && code < 'Z');
}

Segment read_JSX(Segmenter* t) {
    int start = t->pos;
    int depth = 0;
    int inExpr = 0;
    char inString = 0;

    while (t->pos < t->src.len) {
        char ch = t->src.start[t->pos];

        if (inString != 0) {
            if (ch == inString) {
                inString = 0;
            } else if (ch == '\\' && t->pos + 1 < t->src.len) {
                t->pos++;
            }
            t->pos++;
            continue;
        } else if (ch == '"' || ch == '\'' || ch == '`') {
            inString = ch;
            t->pos++;
            continue;
        }

        if (ch == '{') {
            inExpr++;
        } else if (ch == '}') {
            if (inExpr > 0) {
                inExpr--;
            }
        }

        if (ch == '<' && inExpr == 0) {
            if (t->pos + 1 < t->src.len && t->src.start[t->pos + 1] == '/') {
                depth--;
            } else {
                depth++;
            }
        } else if (ch == '>' && inExpr == 0) {
            if (depth == 0) {
                t->pos++;
                Slice content = { t->src.start + start, t->pos - start };
                return (Segment) { .content = content, .type = JSX };
            }
        }
        t->pos++;
    }

    Slice content = { t->src.start + start, t->src.len - start };
    t->pos = t->src.len;
    return (Segment) { .content = content, .type = JSX };
}

Segment get_next_segment(Segmenter* t) {
    int start = t->pos;
    if (start >= t->src.len) {
        return (Segment){ .type = END };
    }

    while (t->pos < t->src.len) {
        char ch = t->src.start[t->pos];
        char next = 0;
        if ((t->pos+1) < t->src.len){
            next = t->src.start[t->pos+1];
        }

        if(ch == '/' && next == '/'){
            while(t->pos < t->src.len){
                char ch = t->src.start[t->pos];
                if(ch=='\n'){
                    break;
                }
                t->pos++;
            }
            continue;
        }

        if(ch == '/' && next == '*'){
            char next = 0;
            if ((t->pos+1) < t->src.len){
                next = t->src.start[t->pos+1];
            }
            while(t->pos < t->src.len){
                char ch = t->src.start[t->pos];
                if(ch=='*'&&next=='/'){
                    break;
                }
                t->pos++;
            }
        }

        if (ch == '<') {
            if (is_letter(next)||next =='>'){
                if (t->pos > start) {
                    Slice content = { t->src.start + start, t->pos - start };
                    return (Segment) { .content = content, .type = JS };
                }

                return read_JSX(t);
            }
        }
        t->pos++;
    }

    if (start < t->src.len) {
        Slice content = { t->src.start + start, t->src.len - start };
        t->pos = t->src.len;
        return (Segment) { .content = content, .type = JS };
    }

    return (Segment){ .type = END };
}
