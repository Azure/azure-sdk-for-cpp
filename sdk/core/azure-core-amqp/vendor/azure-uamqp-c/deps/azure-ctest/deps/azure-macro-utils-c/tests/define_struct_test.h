// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef DEFINE_STRUCT_TEST_H
#define DEFINE_STRUCT_TEST_H

#include "azure_macro_utils/macro_utils.h"

/*this construct defines a struct with 1 field*/

#define TEST_STRUCT_WITH_1_FIELD_FIELDS \
    int, a

MU_DEFINE_STRUCT(TEST_STRUCT_WITH_1_FIELD, TEST_STRUCT_WITH_1_FIELD_FIELDS);

/*this construct defines a struct with 2 fields*/

#define TEST_STRUCT_WITH_2_FIELDS_FIELDS \
    int, a, \
    char*, x

MU_DEFINE_STRUCT(TEST_STRUCT_WITH_2_FIELDS, TEST_STRUCT_WITH_2_FIELDS_FIELDS);

/*this construct defines a struct with another struct in it, u la la*/

#define TEST_NESTED_STRUCT_FIELDS \
    int, a, \
    TEST_STRUCT_WITH_2_FIELDS, z, \
    char*, x

MU_DEFINE_STRUCT(TEST_NESTED_STRUCT, TEST_NESTED_STRUCT_FIELDS);

int run_define_struct_tests(void);

#endif // DEFINE_STRUCT_TEST_H
