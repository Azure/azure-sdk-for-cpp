// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef MU_DEFINE_ENUM_2_WITHOUT_INVALID_TEST_H
#define MU_DEFINE_ENUM_2_WITHOUT_INVALID_TEST_H

#include "azure_macro_utils/macro_utils.h"

/*this construct defines an enum called "enum AB_TAG", makes a typedef called "AB" for it, and sticks a=3 and b = 3 in it*/
/*also declares a function that can be called to get a->"a"*/

#define TEST3_ENUM_VALUES test3_a, 2, test3_b, 3

MU_DEFINE_ENUM_2_WITHOUT_INVALID(TEST3_ENUM, TEST3_ENUM_VALUES);

int run_mu_define_enum_2_without_invalid_test(void);

#endif // MU_DEFINE_ENUM_2_WITHOUT_INVALID_TEST_H
