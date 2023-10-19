// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TEST_DEPENDENCY_H
#define TEST_DEPENDENCY_H

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TEST_STRUCT_TAG
    {
        int x;
    } TEST_STRUCT;

    typedef struct TEST_STRUCT_WITH_2_MEMBERS_TAG
    {
        int x;
        int y;
    } TEST_STRUCT_WITH_2_MEMBERS;

    typedef struct TEST_STRUCT_COPY_FAILS_TAG
    {
        int x;
    } TEST_STRUCT_COPY_FAILS;

    typedef struct TEST_STRUCT_NOT_REGISTERED
    {
        int x;
    } TEST_STRUCT_NOT_REGISTERED;

    typedef unsigned char ARRAY_TYPE[16];

    /* Tests_SRS_UMOCK_C_LIB_01_001: [MOCKABLE_FUNCTION shall be used to wrap function definition allowing the user to declare a function that can be mocked.]*/
    /* Tests_SRS_UMOCK_C_LIB_01_004: [If ENABLE_MOCKS is defined, MOCKABLE_FUNCTION shall generate the declaration of the function and code for the mocked function, thus allowing setting up of expectations in test functions.] */
    MOCKABLE_FUNCTION(, int, test_dependency_no_args);
    MOCKABLE_FUNCTION(, int, test_dependency_1_arg, int, a);
    MOCKABLE_FUNCTION(, void, test_dependency_1_arg_no_return, int, a);
    MOCKABLE_FUNCTION(, int, test_dependency_2_args, int, a, int, b);
    MOCKABLE_FUNCTION(, int, test_dependency_struct_arg, TEST_STRUCT, s);
    MOCKABLE_FUNCTION(, const TEST_STRUCT*, test_dependency_return_const_struct_ptr);
    MOCKABLE_FUNCTION(, int, test_dependency_struct_with_2_members, TEST_STRUCT_WITH_2_MEMBERS, s, int, a);
    MOCKABLE_FUNCTION(, int, test_dependency_char_star_arg, char*, s);
    MOCKABLE_FUNCTION(, int, test_dependency_1_out_arg, int*, a);
    MOCKABLE_FUNCTION(, int, test_dependency_2_out_args, int*, a, int*, b);
    MOCKABLE_FUNCTION(, void, test_dependency_void_return);
    MOCKABLE_FUNCTION(, int*, test_dependency_int_ptr_return);
    MOCKABLE_FUNCTION(, void, test_dependency_buffer_arg, unsigned char*, a);
    MOCKABLE_FUNCTION(, int, test_dependency_global_mock_return_test);
    MOCKABLE_FUNCTION(, TEST_STRUCT_COPY_FAILS, test_dependency_global_mock_return_copy_fails);
    MOCKABLE_FUNCTION(, void, test_dependency_type_with_space, char *,s);
    MOCKABLE_FUNCTION(, void, test_dependency_all_types, char, char_arg, unsigned char, unsignedchar_arg, short, short_arg, unsigned short, unsignedshort_arg, int, int_arg, unsigned int, unsignedint_arg, long, long_arg, unsigned long, unsignedlong_arg, long long, longlong_arg, unsigned long long, unsignedlonglong_arg, float, float_arg, double, double_arg, long double, longdouble_arg, size_t, size_t_arg, void*, void_ptr_arg, const void*, const_void_ptr_arg);
    MOCKABLE_FUNCTION(, void, test_dependency_type_not_registered, TEST_STRUCT_NOT_REGISTERED, a);
    MOCKABLE_FUNCTION(, int, test_dependency_with_global_mock_hook);
    MOCKABLE_FUNCTION(, int, test_dependency_with_global_return);
    MOCKABLE_FUNCTION(, int, test_dependency_returning_int);
    MOCKABLE_FUNCTION(, char*, test_mock_function_returning_string);
    MOCKABLE_FUNCTION(, char*, test_mock_function_returning_string_with_macro);
    MOCKABLE_FUNCTION(, void, test_dependency_with_void_ptr, void*, argument);
    MOCKABLE_FUNCTION(, void, test_dependency_with_const_void_ptr, const void*, argument);
    MOCKABLE_FUNCTION(, void, test_dependency_with_array_arg, ARRAY_TYPE, argument);
    MOCKABLE_FUNCTION(, void, test_dependency_with_volatile_arg, volatile int, argument);
    MOCKABLE_FUNCTION(, void, test_dependency_with_volatile_pointer_arg, int volatile*, argument);
    MOCKABLE_FUNCTION(, volatile void*, test_dependency_with_volatile_pptr_return);

    typedef enum TEST_ENUM_TAG
    {
        TEST_ENUM_VALUE_1,
        TEST_ENUM_VALUE_2
    } TEST_ENUM;

    MOCKABLE_FUNCTION(, void, test_mock_function_with_enum_type, TEST_ENUM, enum_type);

    MOCKABLE_FUNCTION_WITH_RETURNS(, int, test_dependency_with_returns_no_args_returning_int)(42, 43);
    MOCKABLE_FUNCTION_WITH_RETURNS(, void, test_dependency_with_no_return_works);
    MOCKABLE_FUNCTION_WITH_RETURNS(, void*, test_dependency_with_returns_no_args_returning_void_ptr)((void*)0x4242, (void*)0x4243);
    MOCKABLE_FUNCTION_WITH_RETURNS(, TEST_STRUCT, test_dependency_with_returns_no_args_returning_struct)({ 0x42 }, { 0x43 });

#ifdef __cplusplus
}
#endif

#endif /* TEST_DEPENDENCY_H */
