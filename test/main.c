#define MINITEST_IMPL
#include "minitest.h"
#include "lexer_test.h"
#include "parser_test.h"
#include "transform_test.h"
#include "segmenter_test.h"

int main(int argc,char ** argv){
    TEST_GROUP(test_lexer);
    TEST_GROUP(test_parser);
    TEST_GROUP(test_transform);
    TEST_GROUP(test_segment);
    test_done();
    return 0;
}
