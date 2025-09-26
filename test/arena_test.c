#include "arena_test.h"
#include "minitest.h"
#include "../src/arena.h"

#define BUF_CAPACITY 512

typedef struct{
    const char* name;
    Arena left;
    ValueIndex left_head;
    Arena right;
    ValueIndex right_head;
    bool should_match;
} TestCase;

void testcase_value_equal(TestCase* tt){
    bool match = value_equal(&tt->left, tt->left_head,&tt->right,tt->right_head);
    if(match != tt->should_match){
        TEST_ERRORF(tt->name, "expect %d got %d",tt->should_match,match);

        Printer left_printer ={0};
        char left_buf[BUF_CAPACITY] ={0};
        left_printer.buf=left_buf;
        left_printer.buf_capacity=BUF_CAPACITY;
        value_print(&left_printer, &tt->left,tt->left_head, 0);

        Printer right_printer ={0};
        char right_buf[BUF_CAPACITY] ={0};
        right_printer.buf=right_buf;
        right_printer.buf_capacity=BUF_CAPACITY;
        value_print(&right_printer, &tt->right,tt->right_head, 0);

        TEST_ERRORF(tt->name,"left node \n%s\nright node \n%s",left_buf,right_buf);
    }
}

void test_value_equal(){
    TestCase cases[]={
        {
            .name = "equal_text_values",
            .left = {
                .values = (Value[]) {
                    {
                        .type = TEXT_NODE_TYPE,
                        .value.text = slice_from("hello"),
                        .next = -1
                    }
                },
                .values_count = 1,
                .values_capacity = 1
            },
            .left_head = 0,
            .right = {
                .values = (Value[]) {
                    {
                        .type = TEXT_NODE_TYPE,
                        .value.text = slice_from("hello"),
                        .next = -1
                    }
                },
                .values_count = 1,
                .values_capacity = 1
            },
            .right_head = 0,
            .should_match = true
        },
        {
            .name = "different_text_values",
            .left = {
                .values = (Value[]) {
                    {
                        .type = TEXT_NODE_TYPE,
                        .value.text = slice_from("hello"),
                        .next = -1
                    }
                },
                .values_count = 1,
                .values_capacity = 1
            },
            .left_head = 0,
            .right = {
                .values = (Value[]) {
                    {
                        .type = TEXT_NODE_TYPE,
                        .value.text = slice_from("world"),
                        .next = -1
                    }
                },
                .values_count = 1,
                .values_capacity = 1
            },
            .right_head = 0,
            .should_match = false
        },
        {
            .name = "equal_expr_values",
            .left = {
                .values = (Value[]) {
                    {
                        .type = EXPR_NODE_TYPE,
                        .value.expr = slice_from("1 + 2"),
                        .next = -1
                    }
                },
                .values_count = 1,
                .values_capacity = 1
            },
            .left_head = 0,
            .right = {
                .values = (Value[]) {
                    {
                        .type = EXPR_NODE_TYPE,
                        .value.expr = slice_from("1 + 2"),
                        .next = -1
                    }
                },
                .values_count = 1,
                .values_capacity = 1
            },
            .right_head = 0,
            .should_match = true
        },
        {
            .name = "equal_node_with_child",
            .left = {
                .values = (Value[]) {
                    {
                        .type = NODE_NODE_TYPE,
                        .value.node = {
                            .Tag = slice_from("div"),
                            .Props = -1,
                            .Children = 1
                        },
                        .next = -1
                    },
                    {
                        .type = TEXT_NODE_TYPE,
                        .value.text = slice_from("child"),
                        .next = -1
                    }
                },
                .values_count = 2,
                .values_capacity = 2
            },
            .left_head = 0,
            .right = {
                .values = (Value[]) {
                    {
                        .type = NODE_NODE_TYPE,
                        .value.node = {
                            .Tag = slice_from("div"),
                            .Props = -1,
                            .Children = 1
                        },
                        .next = -1
                    },
                    {
                        .type = TEXT_NODE_TYPE,
                        .value.text = slice_from("child"),
                        .next = -1
                    }
                },
                .values_count = 2,
                .values_capacity = 2
            },
            .right_head = 0,
            .should_match = true
        },
        {
            .name = "equal_node_with_props",
            .left = {
                .values = (Value[]) {
                    {
                        .type = NODE_NODE_TYPE,
                        .value.node = {
                            .Tag = slice_from("span"),
                            .Props = 0,
                            .Children = -1
                        },
                        .next = -1
                    }
                },
                .values_count = 1,
                .values_capacity = 1,
                .props = (Prop[]) {
                    {
                        .key = slice_from("class"),
                        .value = slice_from("highlight"),
                        .type = TEXT_PROP_TYPE,
                        .next = -1
                    }
                },
                .prop_count = 1,
                .prop_capacity = 1
            },
            .left_head = 0,
            .right = {
                .values = (Value[]) {
                    {
                        .type = NODE_NODE_TYPE,
                        .value.node = {
                            .Tag = slice_from("span"),
                            .Props = 0,
                            .Children = -1
                        },
                        .next = -1
                    }
                },
                .values_count = 1,
                .values_capacity = 1,
                .props = (Prop[]) {
                    {
                        .key = slice_from("class"),
                        .value = slice_from("highlight"),
                        .type = TEXT_PROP_TYPE,
                        .next = -1
                    }
                },
                .prop_count = 1,
                .prop_capacity = 1
            },
            .right_head = 0,
            .should_match = true
        },
        {
            .name = "equal_node_with_multiple_children",
            .left = {
                .values = (Value[]) {
                    {
                        .type = NODE_NODE_TYPE,
                        .value.node = {
                            .Tag = slice_from("div"),
                            .Props = -1,
                            .Children = 1
                        },
                        .next = -1
                    },
                    {
                        .type = TEXT_NODE_TYPE,
                        .value.text = slice_from("one"),
                        .next = 2
                    },
                    {
                        .type = TEXT_NODE_TYPE,
                        .value.text = slice_from("two"),
                        .next = -1
                    }
                },
                .values_count = 3,
                .values_capacity = 3
            },
            .left_head = 0,
            .right = {
                .values = (Value[]) {
                    {
                        .type = NODE_NODE_TYPE,
                        .value.node = {
                            .Tag = slice_from("div"),
                            .Props = -1,
                            .Children = 1
                        },
                        .next = -1
                    },
                    {
                        .type = TEXT_NODE_TYPE,
                        .value.text = slice_from("one"),
                        .next = 2
                    },
                    {
                        .type = TEXT_NODE_TYPE,
                        .value.text = slice_from("two"),
                        .next = -1
                    }
                },
                .values_count = 3,
                .values_capacity = 3
            },
            .right_head = 0,
            .should_match = true
        },
        {
            .name = "different_props",
            .left = {
                .values = (Value[]) {
                    {
                        .type = NODE_NODE_TYPE,
                        .value.node = {
                            .Tag = slice_from("span"),
                            .Props = 0,
                            .Children = -1
                        },
                        .next = -1
                    }
                },
                .values_count = 1,
                .values_capacity = 1,
                .props = (Prop[]) {
                    {
                        .key = slice_from("class"),
                        .value = slice_from("highlight"),
                        .type = TEXT_PROP_TYPE,
                        .next = -1
                    }
                },
                .prop_count = 1,
                .prop_capacity = 1
            },
            .left_head = 0,
            .right = {
                .values = (Value[]) {
                    {
                        .type = NODE_NODE_TYPE,
                        .value.node = {
                            .Tag = slice_from("span"),
                            .Props = 0,
                            .Children = -1
                        },
                        .next = -1
                    }
                },
                .values_count = 1,
                .values_capacity = 1,
                .props = (Prop[]) {
                    {
                        .key = slice_from("class"),
                        .value = slice_from("other"),
                        .type = TEXT_PROP_TYPE,
                        .next = -1
                    }
                },
                .prop_count = 1,
                .prop_capacity = 1
            },
            .right_head = 0,
            .should_match = false
        },
        {
            .name = "different_children_count",
            .left = {
                .values = (Value[]) {
                    {
                        .type = NODE_NODE_TYPE,
                        .value.node = {
                            .Tag = slice_from("div"),
                            .Props = -1,
                            .Children = 1
                        },
                        .next = -1
                    },
                    {
                        .type = TEXT_NODE_TYPE,
                        .value.text = slice_from("one"),
                        .next = -1
                    }
                },
                .values_count = 2,
                .values_capacity = 2
            },
            .left_head = 0,
            .right = {
                .values = (Value[]) {
                    {
                        .type = NODE_NODE_TYPE,
                        .value.node = {
                            .Tag = slice_from("div"),
                            .Props = -1,
                            .Children = 1
                        },
                        .next = -1
                    },
                    {
                        .type = TEXT_NODE_TYPE,
                        .value.text = slice_from("one"),
                        .next = 2
                    },
                    {
                        .type = TEXT_NODE_TYPE,
                        .value.text = slice_from("extra"),
                        .next = -1
                    }
                },
                .values_count = 3,
                .values_capacity = 3
            },
            .right_head = 0,
            .should_match = false
        },
        {
            .name = "different_node_tags",
            .left = {
                .values = (Value[]) {
                    {
                        .type = NODE_NODE_TYPE,
                        .value.node = {
                            .Tag = slice_from("div"),
                            .Props = -1,
                            .Children = -1
                        },
                        .next = -1
                    }
                },
                .values_count = 1,
                .values_capacity = 1
            },
            .left_head = 0,
            .right = {
                .values = (Value[]) {
                    {
                        .type = NODE_NODE_TYPE,
                        .value.node = {
                            .Tag = slice_from("span"),
                            .Props = -1,
                            .Children = -1
                        },
                        .next = -1
                    }
                },
                .values_count = 1,
                .values_capacity = 1
            },
            .right_head = 0,
            .should_match = false
        },
        {
            .name = "equal_children_same_content_different_order_in_arena",
            .left = {
                .values = (Value[]) {
                    {
                        .type = NODE_NODE_TYPE,
                        .value.node = {
                            .Tag = slice_from("ul"),
                            .Props = -1,
                            .Children = 1
                        },
                        .next = -1
                    },
                    {
                        .type = TEXT_NODE_TYPE,
                        .value.text = slice_from("first"),
                        .next = 2
                    },
                    {
                        .type = TEXT_NODE_TYPE,
                        .value.text = slice_from("second"),
                        .next = -1
                    }
                },
                .values_count = 3,
                .values_capacity = 3
            },
            .left_head = 0,
            .right = {
                .values = (Value[]) {
                    {
                        .type = NODE_NODE_TYPE,
                        .value.node = {
                            .Tag = slice_from("ul"),
                            .Props = -1,
                            .Children = 2
                        },
                        .next = -1
                    },
                    {
                        .type = TEXT_NODE_TYPE,
                        .value.text = slice_from("second"),
                        .next = -1
                    },
                    {
                        .type = TEXT_NODE_TYPE,
                        .value.text = slice_from("first"),
                        .next = 1
                    }
                },
                .values_count = 3,
                .values_capacity = 3
            },
            .right_head = 0,
            .should_match = true
        },
        {
            .name = "equal_props_same_content_different_order_in_arena",
            .left = {
                .values = (Value[]) {
                    {
                        .type = NODE_NODE_TYPE,
                        .value.node = {
                            .Tag = slice_from("div"),
                            .Props = 0,
                            .Children = -1
                        },
                        .next = -1
                    }
                },
                .props = (Prop[]) {
                    {
                        .key = slice_from("id"),
                        .value = slice_from("main"),
                        .type = TEXT_PROP_TYPE,
                        .next = 1
                    },
                    {
                        .key = slice_from("class"),
                        .value = slice_from("container"),
                        .type = TEXT_PROP_TYPE,
                        .next = -1
                    }
                },
                .values_count = 1,
                .values_capacity = 1,
                .prop_count = 2,
                .prop_capacity = 2
            },
            .left_head = 0,
            .right = {
                .values = (Value[]) {
                    {
                        .type = NODE_NODE_TYPE,
                        .value.node = {
                            .Tag = slice_from("div"),
                            .Props = 1,
                            .Children = -1
                        },
                        .next = -1
                    }
                },
                .props = (Prop[]) {
                    {
                        .key = slice_from("class"),
                        .value = slice_from("container"),
                        .type = TEXT_PROP_TYPE,
                        .next = -1
                    },
                    {
                        .key = slice_from("id"),
                        .value = slice_from("main"),
                        .type = TEXT_PROP_TYPE,
                        .next = 0
                    }
                },
                .values_count = 1,
                .values_capacity = 1,
                .prop_count = 2,
                .prop_capacity = 2
            },
            .right_head = 0,
            .should_match = true
        }

    };
    int test_count = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < test_count; i++) {
        testcase_value_equal(&cases[i]);
        mt_total++;
    }
}

void test_arena(){
    test_value_equal();
}
