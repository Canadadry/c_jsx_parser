#ifndef JSX_TRANSFORM_H
#define JSX_TRANSFORM_H

#include "token.h"

typedef  struct {
	Slice src ;
	int pos;
}Segmenter;

typedef  struct {
	Slice content;
	enum{JS,JSX,END} type;
}Segment;

Segment get_next_segment(Segmenter* t);

#endif
