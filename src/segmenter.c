#include "segmenter.h"

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
        return (Segment){ .type = EOF };
    }

    while (t->pos < t->src.len) {
        char ch = t->src.start[t->pos];

        if (ch == '<') {
            if (t->pos > start) {
                Slice content = { t->src.start + start, t->pos - start };
                return (Segment) { .content = content, .type = JS };
            }

            return read_JSX(t);
        }
        t->pos++;
    }

    if (start < t->src.len) {
        Slice content = { t->src.start + start, t->src.len - start };
        t->pos = t->src.len;
        return (Segment) { .content = content, .type = JS };
    }

    return (Segment){ .type = EOF };
}
