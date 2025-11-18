#define MINITEST_IMPL
#include "minitest.h"
#include "lexer_test.h"
#include "parser_test.h"
#include "transform_test.h"
#include "segmenter_test.h"
#include "jsx_test.h"
#include "arena_test.h"

int main(int argc,char ** argv){
    TEST_GROUP(test_lexer);
    TEST_GROUP(test_arena);
    TEST_GROUP(test_parser);
    TEST_GROUP(test_parser_no_error);
    TEST_GROUP(test_transform);
    TEST_GROUP(test_escape);
    TEST_GROUP(test_segment);
    TEST_GROUP(test_jsx);
    test_done();
    return 0;
}
