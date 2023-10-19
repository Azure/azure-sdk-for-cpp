// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "test_helper.h"

#include "azure_macro_utils/macro_utils.h"

#include "mu_count_array_items_test.h"

static const int test_array_1_item[1];
static const int test_array_3_items[3];
static const double test_array_4_items[] = { 0x42, 0x43, 0x44, 0x45 };

int run_mu_count_array_items_tests(void)
{
    // check that counting works appropriately
    POOR_MANS_ASSERT(MU_COUNT_ARRAY_ITEMS(test_array_1_item) == 1);
    POOR_MANS_ASSERT(MU_COUNT_ARRAY_ITEMS(test_array_3_items) == 3);
    POOR_MANS_ASSERT(MU_COUNT_ARRAY_ITEMS(test_array_4_items) == 4);

    return 0;
}
