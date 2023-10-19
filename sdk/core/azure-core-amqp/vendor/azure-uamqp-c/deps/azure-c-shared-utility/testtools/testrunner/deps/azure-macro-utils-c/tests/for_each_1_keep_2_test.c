// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "test_helper.h"

#include "azure_macro_utils/macro_utils.h"

#include "for_each_1_keep_2_test.h"

// build variables out of a list of 3 elements
#define LIST_3_ELEMENTS \
    41, 42, 43

#define MY_MACRO_X(val1, val2, var) \
    int MU_C2(x_value_1_, var) = val1; \
    int MU_C2(x_value_2_, var) = val2;

MU_FOR_EACH_1_KEEP_2(MY_MACRO_X, 5, 6, LIST_3_ELEMENTS)

// build variables out of a list of 1 element
#define LIST_1_ELEMENT \
    41

#define MY_MACRO_Y(val1, val2, var) \
    int MU_C2(y_value_1_, var) = val1; \
    int MU_C2(y_value_2_, var) = val2;

MU_FOR_EACH_1_KEEP_2(MY_MACRO_Y, 5, 6, LIST_1_ELEMENT)

int run_for_each_1_keep_2_tests(void)
{
    // check that the appropriate value has been placed in each variable
    POOR_MANS_ASSERT(x_value_1_41 == 5);
    POOR_MANS_ASSERT(x_value_2_41 == 6);
    POOR_MANS_ASSERT(x_value_1_42 == 5);
    POOR_MANS_ASSERT(x_value_2_42 == 6);
    POOR_MANS_ASSERT(x_value_1_43 == 5);
    POOR_MANS_ASSERT(x_value_2_43 == 6);

    POOR_MANS_ASSERT(y_value_1_41 == 5);
    POOR_MANS_ASSERT(y_value_2_41 == 6);

    return 0;
}
