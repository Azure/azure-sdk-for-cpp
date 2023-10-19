// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/lock.h"
#include "umock_c/umock_c.h"

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#undef ENABLE_MOCKS

TEST_DEFINE_ENUM_TYPE(LOCK_RESULT, LOCK_RESULT_VALUES);

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

static TEST_MUTEX_HANDLE g_testByTest;

BEGIN_TEST_SUITE(LOCK_UnitTests)

TEST_SUITE_INITIALIZE(a)
{
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
}

TEST_SUITE_CLEANUP(b)
{
    umock_c_deinit();
}

TEST_FUNCTION_INITIALIZE(f)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(cleans)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* Tests_SRS_LOCK_10_002: [Lock_Init on success shall return a valid lock handle which should be a non NULL value] */
TEST_FUNCTION(LOCK_Lock_Init_succeeds)
{
    //arrange
#ifdef WIN32
    STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
#endif

    //act
    LOCK_HANDLE handle = Lock_Init();

    //assert
    ASSERT_IS_NOT_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    (void)Lock_Deinit(handle);
}

/* Tests_SRS_LOCK_10_005: [Lock on success shall return LOCK_OK] */
TEST_FUNCTION(LOCK_Init_Lock_succeeds)
{
    //arrange
    LOCK_HANDLE handle = Lock_Init();

    //act
    LOCK_RESULT result = Lock(handle);

    //assert
    ASSERT_ARE_EQUAL(LOCK_RESULT, LOCK_OK, result);

    //cleanup
    (void)Unlock(handle);
    (void)Lock_Deinit(handle);
}

/* Tests_SRS_LOCK_10_009: [Unlock on success shall return LOCK_OK] */
TEST_FUNCTION(LOCK_Init_Lock_Unlock_succeeds)
{
    //arrange
    LOCK_RESULT result;
    LOCK_HANDLE handle = Lock_Init();
    (void)Lock(handle);

    //act
    result = Unlock(handle);

    //assert
    ASSERT_ARE_EQUAL(LOCK_RESULT, LOCK_OK, result);

    //cleanup
    (void)Lock_Deinit(handle);
}

/* Tests_SRS_LOCK_10_002: [Lock_Init on success shall return a valid lock handle which should be a non NULL value] */
TEST_FUNCTION(LOCK_Init_DeInit_succeeds)
{
    //arrange
    LOCK_HANDLE handle = Lock_Init();

    umock_c_reset_all_calls();

#ifdef WIN32
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));
#endif

    //act
    LOCK_RESULT result = Lock_Deinit(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //assert
    ASSERT_ARE_EQUAL(LOCK_RESULT, LOCK_OK, result);
}

/* Tests_SRS_LOCK_10_007: [Lock_Deinit on NULL handle passed returns LOCK_ERROR] */
TEST_FUNCTION(LOCK_Lock_NULL_fails)
{
    //arrange

    //act
    LOCK_RESULT result = Lock(NULL);

    ASSERT_ARE_EQUAL(LOCK_RESULT, LOCK_ERROR, result);
}

/* Tests_SRS_LOCK_10_011: [Unlock on NULL handle passed returns LOCK_ERROR] */
TEST_FUNCTION(LOCK_Unlock_NULL_fails)
{
    //arrange

    //act
    LOCK_RESULT result = Unlock(NULL);

    /*Tests_SRS_LOCK_10_011:[ This API on NULL handle passed returns LOCK_ERROR]*/
    ASSERT_ARE_EQUAL(LOCK_RESULT, LOCK_ERROR, result);
}

TEST_FUNCTION(LOCK_DeInit_NULL_fails)
{
    //arrange

    //act
    LOCK_RESULT result = Lock_Deinit(NULL);

    //assert
    ASSERT_ARE_EQUAL(LOCK_RESULT, LOCK_ERROR, result);
}

/* Extra negative tests - only supported on Win32. */
#ifdef WIN32
TEST_FUNCTION(LOCK_Lock_Init_fails_if_malloc_fails)
{
    //arrange
    STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    //act
    LOCK_HANDLE handle = Lock_Init();

    //assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    (void)Lock_Deinit(handle);
}
#endif

END_TEST_SUITE(LOCK_UnitTests);
