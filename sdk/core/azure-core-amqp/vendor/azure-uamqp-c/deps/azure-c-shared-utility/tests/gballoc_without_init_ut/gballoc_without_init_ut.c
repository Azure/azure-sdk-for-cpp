// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if defined(GB_MEASURE_MEMORY_FOR_THIS)
#undef GB_MEASURE_MEMORY_FOR_THIS
#endif

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/gballoc.h"
#include "testrunnerswitcher.h"
#include "azure_c_shared_utility/lock.h"

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)~(size_t)0)
#endif

static TEST_MUTEX_HANDLE g_testByTest;

static void* TEST_ALLOC_PTR1 = (void*)0x4242;
static const LOCK_HANDLE TEST_LOCK_HANDLE = (LOCK_HANDLE)0x4244;

#define ENABLE_MOCKS

#include "umock_c/umock_c.h"
#include "umock_c/umock_c_prod.h"

//TEST_DEFINE_ENUM_TYPE(LOCK_RESULT, LOCK_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(LOCK_RESULT, LOCK_RESULT_VALUES);

#ifdef __cplusplus
extern "C" {
#endif
    MOCKABLE_FUNCTION(, void*, mock_malloc, size_t, size);
    MOCKABLE_FUNCTION(, void*, mock_calloc, size_t, nmemb, size_t, size);
    MOCKABLE_FUNCTION(, void*, mock_realloc, void*, ptr, size_t, size);
    MOCKABLE_FUNCTION(, void, mock_free, void*, ptr);

    MOCKABLE_FUNCTION(, LOCK_HANDLE, Lock_Init);
    MOCKABLE_FUNCTION(, LOCK_RESULT, Lock_Deinit, LOCK_HANDLE, handle);
    MOCKABLE_FUNCTION(, LOCK_RESULT, Lock, LOCK_HANDLE, handle);
    MOCKABLE_FUNCTION(, LOCK_RESULT, Unlock, LOCK_HANDLE, handle);
#ifdef __cplusplus
}
#endif

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(GBAlloc_For_Init_UnitTests)

TEST_SUITE_INITIALIZE(TestClassInitialize)
{
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    REGISTER_UMOCK_ALIAS_TYPE(LOCK_HANDLE, void*);
    REGISTER_TYPE(LOCK_RESULT, LOCK_RESULT);

    REGISTER_GLOBAL_MOCK_RETURN(mock_malloc, TEST_ALLOC_PTR1);
    REGISTER_GLOBAL_MOCK_RETURN(mock_realloc, TEST_ALLOC_PTR1);
    REGISTER_GLOBAL_MOCK_RETURN(mock_calloc, TEST_ALLOC_PTR1);

    REGISTER_GLOBAL_MOCK_RETURN(Lock_Init, TEST_LOCK_HANDLE);
    REGISTER_GLOBAL_MOCK_RETURN(Lock, LOCK_OK);
    REGISTER_GLOBAL_MOCK_RETURN(Unlock, LOCK_OK);
}

TEST_SUITE_CLEANUP(TestClassCleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* gballoc_deinit */

/* Tests_SRS_GBALLOC_01_029: [if gballoc is not initialized gballoc_deinit shall do nothing.] */
TEST_FUNCTION(when_gballoc_is_not_initialized_gballoc_deinit_doesnot_free_lock)
{
    //arrange

    // act
    gballoc_deinit();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* gballoc_malloc */

/* Tests_SRS_GBALLOC_01_039: [If gballoc was not initialized gballoc_malloc shall simply call malloc without any memory tracking being performed.] */
TEST_FUNCTION(when_gballoc_is_not_initialized_gballoc_malloc_calls_crt_malloc)
{
    // arrange
    void* result;
    STRICT_EXPECTED_CALL(mock_malloc(1));

    //act
    result = gballoc_malloc(1);

    //assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* gballoc_calloc */

/* Tests_SRS_GBALLOC_01_040: [If gballoc was not initialized gballoc_calloc shall simply call calloc without any memory tracking being performed.] */
TEST_FUNCTION(when_gballoc_is_not_initialized_then_gballoc_calloc_calls_crt_calloc)
{
    // arrange
    void* result;
    STRICT_EXPECTED_CALL(mock_calloc(1, 1));

    // act
    result = gballoc_calloc(1, 1);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* gballoc_realloc */

/* Tests_SRS_GBALLOC_01_041: [If gballoc was not initialized gballoc_realloc shall shall simply call realloc without any memory tracking being performed.] */
TEST_FUNCTION(when_gballoc_is_not_initialized_then_gballoc_realloc_calls_crt_realloc)
{
    // arrange
    void* result;
    STRICT_EXPECTED_CALL(mock_realloc(NULL, 1));

    // act
    result = gballoc_realloc(NULL, 1);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* gballoc_free */

/* Tests_SRS_GBALLOC_01_042: [If gballoc was not initialized gballoc_free shall shall simply call free.] */
TEST_FUNCTION(when_gballoc_is_not_initialized_then_gballoc_free_does_nothing)
{
    // arrange
    STRICT_EXPECTED_CALL(mock_free((void*)0x4242));

    // act
    gballoc_free((void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* gballoc_getMaximumMemoryUsed */

/* Tests_SRS_GBALLOC_01_038: [If gballoc was not initialized gballoc_getMaximumMemoryUsed shall return MAX_INT_SIZE.]  */
TEST_FUNCTION(without_gballoc_being_initialized_gballoc_getMaximumMemoryUsed_fails)
{
    // arrange

    // act
    size_t result = gballoc_getMaximumMemoryUsed();

    // assert
    ASSERT_ARE_EQUAL(size_t, SIZE_MAX, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* gballoc_getCurrentMemoryUsed */

/* Tests_SRS_GBALLOC_01_044: [If gballoc was not initialized gballoc_getCurrentMemoryUsed shall return SIZE_MAX.] */
TEST_FUNCTION(without_gballoc_being_initialized_gballoc_getCurrentMemoryUsed_fails)
{
    // arrange

    // act
    size_t result = gballoc_getCurrentMemoryUsed();

    // assert
    ASSERT_ARE_EQUAL(size_t, SIZE_MAX, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(GBAlloc_For_Init_UnitTests)
