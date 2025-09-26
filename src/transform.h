#ifndef JSX_TRANSFORM_H
#define JSX_TRANSFORM_H

#include "ast.h"
#include "token.h"
#include "parser.h"

typedef struct {
    Slice createElem;
    Buffer buf;
} Transformer;

void Transform(Transformer* t,Arena* arena,ValueIndex node);

#endif
