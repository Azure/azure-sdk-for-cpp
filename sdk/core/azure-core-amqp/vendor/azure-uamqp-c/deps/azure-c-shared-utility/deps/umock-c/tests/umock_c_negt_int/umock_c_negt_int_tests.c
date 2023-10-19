// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdint>
#include <cstdlib>
#else
#include <stdint.h>
#include <stdlib.h>
#endif

#include "testrunnerswitcher.h"

#include "umock_c/umock_c.h"
#include "umock_c/umock_c_negative_tests.h"

#define ENABLE_MOCKS
#include "test_dependency.h"
#include "umock_c/umocktypes_charptr.h"

static TEST_MUTEX_HANDLE test_mutex;

static void test_on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    (void)error_code;
}

int function_under_test_1_call(void)
{
    int result;
    
    if (function_1() != 0)
    {
        result = __LINE__;
    }
    else
    {
        result = 0;
    }

    return result;
}

int function_under_test_2_calls(void)
{
    int result;

    if (function_1() != 0)
    {
        result = __LINE__;
    }
    else
    {
        if (function_2() != 0)
        {
            result = __LINE__;
        }
        else
        {
            result = 0;
        }
    }

    return result;
}

int function_under_test_1_call_dep_void_return(void)
{
    function_3_void_return();
    return 0;
}

int function_under_test_3_call_dep_void_ptr_return(void)
{
    int result;

    if (function_3_void_ptr_return((void*)0x42) == NULL)
    {
        result = __LINE__;
    }
    else
    {
        result = 0;
    }

    return result;
}

MOCK_FUNCTION_WITH_CODE(, void*, function_4_void_ptr_return_non_NULL, void*, a)
    void* my_result = (void*)0x42;
MOCK_FUNCTION_END(my_result)

typedef void* SOME_HANDLE;
static const SOME_HANDLE test_handle = (SOME_HANDLE)0x4242;

MOCK_FUNCTION_WITH_CODE(, SOME_HANDLE, some_create, int, a);
MOCK_FUNCTION_END(test_handle)
MOCK_FUNCTION_WITH_CODE(, void, some_destroy, SOME_HANDLE, h);
MOCK_FUNCTION_END()

int function_under_test_4_call_dep_void_ptr_return_non_NULL(void)
{
    int result;

    if (function_4_void_ptr_return_non_NULL((void*)0x42) == NULL)
    {
        result = __LINE__;
    }
    else
    {
        if (function_4_void_ptr_return_non_NULL((void*)0x42) == NULL)
        {
            result = __LINE__;
        }
        else
        {
            if (function_4_void_ptr_return_non_NULL((void*)0x42) == NULL)
            {
                result = __LINE__;
            }
            else
            {
                if (function_4_void_ptr_return_non_NULL((void*)0x42) == NULL)
                {
                    result = __LINE__;
                }
                else
                {
                    if (function_4_void_ptr_return_non_NULL((void*)0x42) == NULL)
                    {
                        result = __LINE__;
                    }
                    else
                    {
                        if (function_4_void_ptr_return_non_NULL((void*)0x42) == NULL)
                        {
                            result = __LINE__;
                        }
                        else
                        {
                            if (function_4_void_ptr_return_non_NULL((void*)0x42) == NULL)
                            {
                                result = __LINE__;
                            }
                            else
                            {
                                if (function_4_void_ptr_return_non_NULL((void*)0x42) == NULL)
                                {
                                    result = __LINE__;
                                }
                                else
                                {
                                    if (function_4_void_ptr_return_non_NULL((void*)0x42) == NULL)
                                    {
                                        result = __LINE__;
                                    }
                                    else
                                    {
                                        result = 0;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return result;
}

BEGIN_TEST_SUITE(umock_c_negative_tests_integrationtests)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    test_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_mutex);

    result = umock_c_init(test_on_umock_c_error);
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_UMOCK_ALIAS_TYPE(SOME_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SOME_OTHER_HANDLE, void*);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(test_mutex);
}

TEST_FUNCTION_INITIALIZE(test_function_init)
{
    int result = TEST_MUTEX_ACQUIRE(test_mutex);
    ASSERT_ARE_EQUAL(int, 0, result);

    result = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, result);
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
    umock_c_negative_tests_deinit();
    umock_c_reset_all_calls();
    TEST_MUTEX_RELEASE(test_mutex);
}

/* Tests_SRS_UMOCK_C_LIB_01_167: [ umock_c_negative_tests_snapshot shall take a snapshot of the current setup of expected calls (a.k.a happy path). ]*/
/* Tests_SRS_UMOCK_C_LIB_01_170: [ umock_c_negative_tests_reset shall bring umock_c expected and actual calls to the state recorded when umock_c_negative_tests_snapshot was called. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_173: [ umock_c_negative_tests_fail_call shall instruct the negative tests module to fail a specific call. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_176: [ umock_c_negative_tests_call_count shall provide the number of expected calls, so that the test code can iterate through all negative cases. ]*/
TEST_FUNCTION(negative_tests_with_one_call)
{
    size_t i;
    STRICT_EXPECTED_CALL(function_1())
        .SetReturn(0).SetFailReturn(1);
    umock_c_negative_tests_snapshot();

    for (i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        // arrange
        char temp_str[128];
        int result;
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        // act
        result = function_under_test_1_call();

        // assert
        sprintf(temp_str, "On failed call %zu", i + 1);
        ASSERT_ARE_NOT_EQUAL(int, 0, result, temp_str);
    }
}

/* Tests_SRS_UMOCK_C_LIB_01_167: [ umock_c_negative_tests_snapshot shall take a snapshot of the current setup of expected calls (a.k.a happy path). ]*/
/* Tests_SRS_UMOCK_C_LIB_01_170: [ umock_c_negative_tests_reset shall bring umock_c expected and actual calls to the state recorded when umock_c_negative_tests_snapshot was called. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_173: [ umock_c_negative_tests_fail_call shall instruct the negative tests module to fail a specific call. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_176: [ umock_c_negative_tests_call_count shall provide the number of expected calls, so that the test code can iterate through all negative cases. ]*/
TEST_FUNCTION(negative_tests_with_2_calls)
{
    size_t i;
    STRICT_EXPECTED_CALL(function_1())
        .SetReturn(0).SetFailReturn(1);
    STRICT_EXPECTED_CALL(function_2())
        .SetReturn(0).SetFailReturn(1);
    umock_c_negative_tests_snapshot();

    for (i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        // arrange
        char temp_str[128];
        int result;
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        // act
        result = function_under_test_2_calls();

        // assert
        sprintf(temp_str, "On failed call %zu", i + 1);
        ASSERT_ARE_NOT_EQUAL(int, 0, result, temp_str);
    }
}

/* Tests_SRS_UMOCK_C_LIB_01_167: [ umock_c_negative_tests_snapshot shall take a snapshot of the current setup of expected calls (a.k.a happy path). ]*/
/* Tests_SRS_UMOCK_C_LIB_01_170: [ umock_c_negative_tests_reset shall bring umock_c expected and actual calls to the state recorded when umock_c_negative_tests_snapshot was called. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_173: [ umock_c_negative_tests_fail_call shall instruct the negative tests module to fail a specific call. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_176: [ umock_c_negative_tests_call_count shall provide the number of expected calls, so that the test code can iterate through all negative cases. ]*/
TEST_FUNCTION(negative_tests_with_1_call_with_void_return_dependency)
{
    size_t i;
    STRICT_EXPECTED_CALL(function_3_void_return());
    umock_c_negative_tests_snapshot();

    for (i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        // arrange
        char temp_str[128];
        int result;
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        // act
        result = function_under_test_1_call_dep_void_return();

        // assert
        sprintf(temp_str, "On failed call %zu", i + 1);
        ASSERT_ARE_EQUAL(int, 0, result, temp_str);
    }
}

/* Tests_SRS_UMOCK_C_LIB_01_167: [ umock_c_negative_tests_snapshot shall take a snapshot of the current setup of expected calls (a.k.a happy path). ]*/
/* Tests_SRS_UMOCK_C_LIB_01_170: [ umock_c_negative_tests_reset shall bring umock_c expected and actual calls to the state recorded when umock_c_negative_tests_snapshot was called. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_173: [ umock_c_negative_tests_fail_call shall instruct the negative tests module to fail a specific call. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_176: [ umock_c_negative_tests_call_count shall provide the number of expected calls, so that the test code can iterate through all negative cases. ]*/
TEST_FUNCTION(negative_tests_with_1_call_with_void_ptr_return_dependency)
{
    size_t i;
    STRICT_EXPECTED_CALL(function_3_void_ptr_return(IGNORED_PTR_ARG))
        .SetReturn((void*)0x42).SetFailReturn(NULL).IgnoreArgument_a();
    umock_c_negative_tests_snapshot();

    for (i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        // arrange
        char temp_str[128];
        int result;
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        // act
        result = function_under_test_3_call_dep_void_ptr_return();

        // assert
        sprintf(temp_str, "On failed call %zu", i + 1);
        ASSERT_ARE_NOT_EQUAL(int, 0, result, temp_str);
    }
}

/* Tests_SRS_UMOCK_C_LIB_01_167: [ umock_c_negative_tests_snapshot shall take a snapshot of the current setup of expected calls (a.k.a happy path). ]*/
/* Tests_SRS_UMOCK_C_LIB_01_170: [ umock_c_negative_tests_reset shall bring umock_c expected and actual calls to the state recorded when umock_c_negative_tests_snapshot was called. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_173: [ umock_c_negative_tests_fail_call shall instruct the negative tests module to fail a specific call. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_176: [ umock_c_negative_tests_call_count shall provide the number of expected calls, so that the test code can iterate through all negative cases. ]*/
TEST_FUNCTION(negative_tests_with_1_call_with_ignored_arguments_takes_the_ignoreargument_flags_into_account)
{
    size_t i;
    STRICT_EXPECTED_CALL(function_4_void_ptr_return_non_NULL(IGNORED_PTR_ARG))
        .SetFailReturn(NULL).IgnoreArgument(1);
    umock_c_negative_tests_snapshot();

    for (i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        // arrange
        char temp_str[128];
        int result;
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        // act
        result = function_under_test_4_call_dep_void_ptr_return_non_NULL();

        // assert
        sprintf(temp_str, "On failed call %zu", i + 1);
        ASSERT_ARE_NOT_EQUAL(int, 0, result, temp_str);
    }
}

/* Tests_SRS_UMOCK_C_LIB_01_167: [ umock_c_negative_tests_snapshot shall take a snapshot of the current setup of expected calls (a.k.a happy path). ]*/
/* Tests_SRS_UMOCK_C_LIB_01_170: [ umock_c_negative_tests_reset shall bring umock_c expected and actual calls to the state recorded when umock_c_negative_tests_snapshot was called. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_173: [ umock_c_negative_tests_fail_call shall instruct the negative tests module to fail a specific call. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_176: [ umock_c_negative_tests_call_count shall provide the number of expected calls, so that the test code can iterate through all negative cases. ]*/
TEST_FUNCTION(negative_tests_with_9_calls_works)
{
    size_t i;
    for (i = 0; i < 9; i++)
    {
        STRICT_EXPECTED_CALL(function_4_void_ptr_return_non_NULL(IGNORED_PTR_ARG))
            .SetFailReturn(NULL).IgnoreArgument(1);
    }

    umock_c_negative_tests_snapshot();

    for (i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        // arrange
        char temp_str[128];
        int result;
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        // act
        result = function_under_test_4_call_dep_void_ptr_return_non_NULL();

        // assert
        sprintf(temp_str, "On failed call %zu", i + 1);
        ASSERT_ARE_NOT_EQUAL(int, 0, result, temp_str);
    }
}

/* Tests_SRS_UMOCK_C_LIB_01_204: [ Tracking of paired calls shall not be done if the actual call to the create_call is using the SetFailReturn call modifier. ]*/
TEST_FUNCTION(SetFailReturn_suppresses_paired_calls_tracking)
{
    REGISTER_UMOCKC_PAIRED_CREATE_DESTROY_CALLS(some_create, some_destroy);

    STRICT_EXPECTED_CALL(some_create(42))
        .SetFailReturn(NULL);

    umock_c_negative_tests_snapshot();

    umock_c_negative_tests_reset();
    umock_c_negative_tests_fail_call(0);

    // act
    (void)some_create(42);

    // assert
    // no explicit assert, no leak expected
}

/* Tests_SRS_UMOCK_C_LIB_01_204: [ Tracking of paired calls shall not be done if the actual call to the create_call is using the SetFailReturn call modifier. ]*/
TEST_FUNCTION(SetFailReturn_suppresses_paired_calls_tracking_for_mockable_functions)
{
    REGISTER_UMOCKC_PAIRED_CREATE_DESTROY_CALLS(some_other_create, some_other_destroy);

    STRICT_EXPECTED_CALL(some_other_create(42))
        .SetFailReturn(NULL);

    umock_c_negative_tests_snapshot();

    umock_c_negative_tests_reset();
    umock_c_negative_tests_fail_call(0);

    // act
    (void)some_other_create(42);

    // assert
    // no explicit assert, no leak expected
}

/* Tests_SRS_UMOCK_C_LIB_31_209: [ call_cannot_fail_func__{name} call modifier shall record that when performing failure case run, this call should be skipped. ]*/
TEST_FUNCTION(umock_c_negative_tests_can_call_fail_test)
{
    // arrange   
    STRICT_EXPECTED_CALL(void_function_no_args());
    STRICT_EXPECTED_CALL(function_mark_cannot_fail_no_args()).CallCannotFail();
    STRICT_EXPECTED_CALL(void_function_with_args(12));
    STRICT_EXPECTED_CALL(function_default_no_args());
    STRICT_EXPECTED_CALL(function_default_with_args(34));
    STRICT_EXPECTED_CALL(function_mark_cannot_fail_with_args(78)).CallCannotFail();

    // act
    umock_c_negative_tests_snapshot();

    // assert
    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_can_call_fail(0), "void_function_no_args indicated it can fail");
    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_can_call_fail(1), "function_mark_cannot_fail_no_args indicated it can fail");
    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_can_call_fail(2), "void_function_with_args indicated it can fail");
    ASSERT_ARE_EQUAL(int, 1, umock_c_negative_tests_can_call_fail(3), "function_default_no_args wrongly indicated it cannot fail");
    ASSERT_ARE_EQUAL(int, 1, umock_c_negative_tests_can_call_fail(4), "function_default_with_args wrongly indicated it cannot fail");
    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_can_call_fail(5), "function_mark_cannot_fail_with_args indicated it can fail");
}

/* Tests_SRS_UMOCK_C_LIB_01_214: [ Specifying the return values for success and failure shall be equivalent to calling REGISTER_GLOBAL_MOCK_RETURNS. ]*/
TEST_FUNCTION(fail_return_value_specified_in_MOCKABLE_FUNCTION_WITH_RETURNS_is_returned)
{
    int result;
    STRICT_EXPECTED_CALL(function_with_returns());
    umock_c_negative_tests_snapshot();

    umock_c_negative_tests_reset();
    umock_c_negative_tests_fail_call(0);

    // act
    result = function_with_returns();

    // assert
    ASSERT_ARE_EQUAL(int, 43, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_214: [ Specifying the return values for success and failure shall be equivalent to calling REGISTER_GLOBAL_MOCK_RETURNS. ]*/
TEST_FUNCTION(SetFailReturns_overrides_MOCKABLE_FUNCTION_WITH_RETURNS)
{
    int result;
    STRICT_EXPECTED_CALL(function_with_returns())
        .SetFailReturn(44);
    umock_c_negative_tests_snapshot();

    umock_c_negative_tests_reset();
    umock_c_negative_tests_fail_call(0);

    // act
    result = function_with_returns();

    // assert
    ASSERT_ARE_EQUAL(int, 44, result);
}

END_TEST_SUITE(umock_c_negative_tests_integrationtests)
