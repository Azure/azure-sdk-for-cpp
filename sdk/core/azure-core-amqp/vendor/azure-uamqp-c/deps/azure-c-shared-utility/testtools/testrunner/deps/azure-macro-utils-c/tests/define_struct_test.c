// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "test_helper.h"

#include "azure_macro_utils/macro_utils.h"

/*this is bringing in the test structs*/
#include "define_struct_test.h"

int run_define_struct_tests(void)
{
    int result = 0;

    /* this test only makes sure that the structs can be generated */
    /* if it builds, it succeeds! */

    TEST_STRUCT_WITH_1_FIELD test_struct_with_1_field;
    test_struct_with_1_field.a = 42;

    TEST_STRUCT_WITH_2_FIELDS test_struct_with_2_fields;
    test_struct_with_2_fields.a = 42;
    test_struct_with_2_fields.x = NULL;

    TEST_NESTED_STRUCT test_nested_struct;
    test_nested_struct.a = 43;
    test_nested_struct.x = "a";
    test_nested_struct.z.a = 42;
    test_nested_struct.z.x = NULL;

    return result;
}
