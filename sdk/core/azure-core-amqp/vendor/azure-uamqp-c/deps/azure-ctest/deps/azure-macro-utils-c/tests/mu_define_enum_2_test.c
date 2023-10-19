// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <string.h>
#include "test_helper.h"

#include "azure_macro_utils/macro_utils.h"

#include "mu_define_enum_2_test.h"

// This lists the values in the enum
#define TEST5_ENUM_VALUES \
    test5_a, 2, \
    test5_b, 42

// This will define an enum that has an INVALID value as first enum value and then it has the enum values
// mentioned in TEST5_ENUM_VALUES
MU_DEFINE_ENUM_2(TEST5_ENUM, TEST5_ENUM_VALUES);

MU_DEFINE_ENUM_STRINGS_2(TEST5_ENUM, TEST5_ENUM_VALUES);

int run_mu_define_enum_2_tests(void)
{
    int result = 0;

    TEST5_ENUM someUnusedVariable; /*but wants to see that the type "TEST5_ENUM" is a real type*/
    (void)someUnusedVariable;

    POOR_MANS_ASSERT(TEST5_ENUM_INVALID == (int)0xDDDDDDDD);
    POOR_MANS_ASSERT(test5_a == 2);
    POOR_MANS_ASSERT(test5_b == 42);

    const char* TEST5_test5_a = MU_ENUM_TO_STRING_2(TEST5_ENUM, 2);
    POOR_MANS_ASSERT(TEST5_test5_a != NULL);
    POOR_MANS_ASSERT(strcmp("test5_a", TEST5_test5_a) == 0);

    const char* TEST5_test5_b = MU_ENUM_TO_STRING_2(TEST5_ENUM, 42);
    POOR_MANS_ASSERT(TEST5_test5_b != NULL);
    POOR_MANS_ASSERT(strcmp("test5_b", TEST5_test5_b) == 0);

    POOR_MANS_ASSERT(MU_ENUM_2_VALUE_COUNT(TEST5_ENUM_VALUES) == 3);

    return result;
}
