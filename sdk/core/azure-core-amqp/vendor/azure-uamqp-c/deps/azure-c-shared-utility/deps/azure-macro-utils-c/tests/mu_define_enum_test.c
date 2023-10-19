// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <string.h>
#include "test_helper.h"

#include "azure_macro_utils/macro_utils.h"

#include "mu_define_enum_test.h"

// This lists the values in the enum
#define TEST4_ENUM_VALUES \
    test4_a, \
    test4_b

// This will define an enum that has an INVALID value as first enum value and then it has the enum values
// mentioned in TEST4_ENUM_VALUES
MU_DEFINE_ENUM(TEST4_ENUM, TEST4_ENUM_VALUES);
MU_DEFINE_ENUM_STRINGS(TEST4_ENUM, TEST4_ENUM_VALUES);

int run_mu_define_enum_tests(void)
{
    int result = 0;

    TEST4_ENUM someUnusedVariable; /*but wants to see that the type "TEST4_ENUM" is a real type*/
    (void)someUnusedVariable;

    POOR_MANS_ASSERT(TEST4_ENUM_INVALID == 0);
    POOR_MANS_ASSERT(test4_a == 1);

    const char* TEST4_test4_a = MU_ENUM_TO_STRING(TEST4_ENUM, 1);
    POOR_MANS_ASSERT(TEST4_test4_a != NULL);
    POOR_MANS_ASSERT(strcmp("test4_a", TEST4_test4_a) == 0);

    POOR_MANS_ASSERT(MU_ENUM_VALUE_COUNT(TEST4_ENUM_VALUES) == 3);

    return result;
}
