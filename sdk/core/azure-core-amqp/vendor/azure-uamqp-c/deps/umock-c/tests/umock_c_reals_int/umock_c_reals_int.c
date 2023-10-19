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

// This include checks that a header with a MOCKABLE_INTERFACE in it can be expanded when no ENABLE_MOCKS is used
#include "test_unit_no_reals.h"

#include "test_dependency_no_enable_mocks.h"
#include "test_dependency_real_code_no_enable_mocks.c"

#define ENABLE_MOCKS

#include "test_dependency.h"
/* Tests_SRS_UMOCK_C_LIB_01_217: [ In the presence of the ENABLE_MOCKS define, IMPLEMENT_MOCKABLE_FUNCTION shall expand to the signature of the function, but the name shall be changed to be prefix with real_. ]*/
#include "test_dependency_real_code.c"

typedef struct test_on_umock_c_error_CALL_TAG
{
    UMOCK_C_ERROR_CODE error_code;
} test_on_umock_c_error_CALL;

static test_on_umock_c_error_CALL* test_on_umock_c_error_calls;
static size_t test_on_umock_c_error_call_count;

static void test_on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    test_on_umock_c_error_CALL* new_calls = (test_on_umock_c_error_CALL*)realloc(test_on_umock_c_error_calls, sizeof(test_on_umock_c_error_CALL) * (test_on_umock_c_error_call_count + 1));
    if (new_calls != NULL)
    {
        test_on_umock_c_error_calls = new_calls;
        test_on_umock_c_error_calls[test_on_umock_c_error_call_count].error_code = error_code;
        test_on_umock_c_error_call_count++;
    }
}

#ifdef _MSC_VER
#pragma warning(disable: 4505)
#endif

static TEST_MUTEX_HANDLE test_mutex;

BEGIN_TEST_SUITE(umock_c_reals_inttests)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    test_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_mutex);

    result = umock_c_init(test_on_umock_c_error);
    ASSERT_ARE_EQUAL(int, 0, result);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(test_mutex);
}

TEST_FUNCTION_INITIALIZE(test_function_init)
{
    int mutex_acquire_result = TEST_MUTEX_ACQUIRE(test_mutex);
    ASSERT_ARE_EQUAL(int, 0, mutex_acquire_result);

    test_on_umock_c_error_calls = NULL;
    test_on_umock_c_error_call_count = 0;
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
    umock_c_reset_all_calls();

    free(test_on_umock_c_error_calls);
    test_on_umock_c_error_calls = NULL;
    test_on_umock_c_error_call_count = 0;

    TEST_MUTEX_RELEASE(test_mutex);
}

/* STRICT_EXPECTED_CALL */

TEST_FUNCTION(real_is_called_for_test_dependency_no_args)
{
    // arrange
    int result;

    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_no_args, real_test_dependency_no_args);

    STRICT_EXPECTED_CALL(test_dependency_no_args());

    // act
    result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 42, result);
}

TEST_FUNCTION(real_is_not_called_for_test_dependency_no_args_no_real)
{
    // arrange
    int result;

    REGISTER_GLOBAL_MOCK_RETURN(test_dependency_no_args_no_real, 1);

    STRICT_EXPECTED_CALL(test_dependency_no_args_no_real());

    // act
    result = test_dependency_no_args_no_real();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 1, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_219: [ REGISTER_GLOBAL_INTERFACE_HOOKS shall register as mock hooks the real functions for all the functions in a mockable interface. ]*/
TEST_FUNCTION(reals_are_setup_at_interface_level)
{
    // arrange
    int result;

    REGISTER_GLOBAL_INTERFACE_HOOKS(test_interface);

    STRICT_EXPECTED_CALL(test_dependency_1_arg(45));

    // act
    result = test_dependency_1_arg(45);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 42, result);
}

TEST_FUNCTION(real_is_not_called_for_interface_without_reals)
{
    // arrange
    int result;

    REGISTER_GLOBAL_MOCK_RETURN(test_dependency_1_arg_no_real, 1);

    STRICT_EXPECTED_CALL(test_dependency_1_arg_no_real(45));

    // act
    result = test_dependency_1_arg_no_real(45);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 1, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_218: [ If ENABLE_MOCKS is not defined, IMPLEMENT_MOCKABLE_FUNCTION shall expand to the signature of the function. ]*/
TEST_FUNCTION(no_rename_to_Real_if_ENABLE_MOCKS_is_not_defined)
{
    // arrange
    int result;

    // act
    result = test_dependency_no_args_no_ENABLE_MOCKS();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 42, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_220: [ UMOCK_REAL shall produce the name of the real function generated by umock. ]*/
TEST_FUNCTION(calling_a_real_function_is_possible)
{
    // arrange
    int result;

    // act
    result = UMOCK_REAL(test_dependency_no_args)();

    // assert
    ASSERT_ARE_EQUAL(int, 42, result);
}

END_TEST_SUITE(umock_c_reals_inttests)
