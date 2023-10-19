// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>

#include "test_helper.h"

#include "mu_define_enum_2_without_invalid_test.h"
#include "define_struct_test.h"
#include "for_each_1_keep_2_test.h"
#include "mu_eat_empty_args_test.h"
#include "mu_count_array_items_test.h"
#include "mu_define_enum_test.h"
#include "mu_define_local_enum_test.h"
#include "mu_define_enum_2_test.h"
#include "mu_pri_enum_test.h"

int main(void)
{
    int result;
    result = run_mu_define_enum_2_without_invalid_test();
    POOR_MANS_ASSERT(result == 0);
    
    result = run_mu_eat_empty_args_test();
    POOR_MANS_ASSERT(result == 0);

    result = run_define_struct_tests();
    POOR_MANS_ASSERT(result == 0);

    result = run_for_each_1_keep_2_tests();
    POOR_MANS_ASSERT(result == 0);

    result = run_mu_count_array_items_tests();
    POOR_MANS_ASSERT(result == 0);

    result = run_mu_define_enum_tests();
    POOR_MANS_ASSERT(result == 0);

    result = run_mu_define_local_enum_tests();
    POOR_MANS_ASSERT(result == 0);

    result = run_mu_define_enum_2_tests();
    POOR_MANS_ASSERT(result == 0);

    result = run_mu_pri_enum_tests();
    POOR_MANS_ASSERT(result == 0);

    return 0;
}

