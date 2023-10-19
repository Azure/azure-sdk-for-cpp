// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <string.h>
#include "test_helper.h"

#include "azure_macro_utils/macro_utils.h"

#include "mu_define_local_enum_test.h"

// This lists the values in the enum
#define TEST42_LOCAL_ENUM_VALUES \
    test42_a, \
    test42_b

// This will define an enum that has an INVALID value as first enum value and then it has the enum values
// mentioned in TEST42_ENUM_VALUES
// This also generates the strings for it ...
MU_DEFINE_LOCAL_ENUM(TEST42_LOCAL_ENUM, TEST42_LOCAL_ENUM_VALUES);

int run_mu_define_local_enum_tests(void)
{
    int result = 0;

    TEST42_LOCAL_ENUM someUnusedVariable; /*but wants to see that the type "TEST42_ENUM" is a real type*/
    (void)someUnusedVariable;

    POOR_MANS_ASSERT(TEST42_LOCAL_ENUM_INVALID == 0);
    POOR_MANS_ASSERT(test42_a == 1);

    const char* TEST42_LOCAL_test42_a = MU_ENUM_TO_STRING(TEST42_LOCAL_ENUM, 1);
    POOR_MANS_ASSERT(TEST42_LOCAL_test42_a != NULL);
    POOR_MANS_ASSERT(strcmp("test42_a", TEST42_LOCAL_test42_a) == 0);

    return result;
}
