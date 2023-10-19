// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <string.h>
#include "test_helper.h"

#include "azure_macro_utils/macro_utils.h"

/*this is bringing in the enum called TEST3_ENUM and main will call the "stringify" of this enum*/
#include "mu_define_enum_2_without_invalid_test.h"

MU_DEFINE_ENUM_STRINGS_2(TEST3_ENUM, TEST3_ENUM_VALUES);

/*defining a simple enum with the constituend test1_a=2, test1_b=3, note: this tests that a DEFINE_ENUM_STRING_2 is not mandatory*/
MU_DEFINE_ENUM_2_WITHOUT_INVALID(test1, test1_a, 2, test1_b, 3);

/*this checks that an enum with 1 value can still be constructed*/
MU_DEFINE_ENUM_2_WITHOUT_INVALID(test2, test2_a, 45);

int run_mu_define_enum_2_without_invalid_test(void)
{
    int result = 0;

    test1 someUnusedVariable; /*but wants to see that the type "test1" is a real type*/
    (void)someUnusedVariable;

    const char* TEST2_value_0 = MU_ENUM_TO_STRING_2(TEST3_ENUM, 0);

    POOR_MANS_ASSERT(TEST2_value_0 != NULL);
    POOR_MANS_ASSERT(strcmp("NULL", TEST2_value_0) == 0);


    const char* TEST3_value_2 = MU_ENUM_TO_STRING_2(TEST3_ENUM, 2);
    POOR_MANS_ASSERT(TEST3_value_2 != NULL);
    POOR_MANS_ASSERT(strcmp("test3_a", TEST3_value_2) == 0);

    return result;
}
