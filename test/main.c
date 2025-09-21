#define MINITEST_IMPL
#include "minitest.h"
#include "lexer_test.h"
#include "parser_test.h"

int main(int argc,char ** argv){
    TEST_GROUP(test_lexer);
    TEST_GROUP(test_parser);
    test_done();
    return 0;
}
