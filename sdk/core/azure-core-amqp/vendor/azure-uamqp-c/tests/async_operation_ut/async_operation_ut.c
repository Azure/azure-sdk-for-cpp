// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#include <cstdio>
#include <cstdint>
#else
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_bool.h"

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void* my_gballoc_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#define ENABLE_MOCKS

#include "azure_c_shared_utility/gballoc.h"

#undef ENABLE_MOCKS

#include "azure_uamqp_c/async_operation.h"

MOCK_FUNCTION_WITH_CODE(, void, test_cancel_handler, ASYNC_OPERATION_HANDLE, async_operation)
MOCK_FUNCTION_END()

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(async_operation_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);

    REGISTER_UMOCK_ALIAS_TYPE(ASYNC_OPERATION_HANDLE, void*);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
}

TEST_FUNCTION_INITIALIZE(test_function_init)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* async_operation_create */

/* Tests_SRS_ASYNC_OPERATION_01_001: [ `async_operation_create` shall return a non-NULL handle to a newly created asynchronous operation instance.]*/
TEST_FUNCTION(async_operation_create_succeeds)
{
    // arrange
    ASYNC_OPERATION_HANDLE result;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    // act
    result = async_operation_create(test_cancel_handler, sizeof(uintptr_t));

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    async_operation_destroy(result);
}

/* Tests_SRS_ASYNC_OPERATION_01_002: [ If `async_operation_cancel_handler` is NULL, `async_operation_create` shall fail and return NULL.]*/
TEST_FUNCTION(async_operation_create_with_NULL_cancel_handler_fails)
{
    // arrange
    ASYNC_OPERATION_HANDLE result;

    // act
    result = async_operation_create(NULL, 64);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_ASYNC_OPERATION_01_003: [ If `context_size` is less than the size of the `async_operation_cancel_handler` argument, `async_operation_create` shall fail and return NULL.]*/
TEST_FUNCTION(async_operation_create_with_not_enough_context_size_fails)
{
    // arrange
    ASYNC_OPERATION_HANDLE result;

    // act
    result = async_operation_create(test_cancel_handler, sizeof(uintptr_t) - 1);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_ASYNC_OPERATION_01_004: [ If allocating memory for the new asynchronous operation instance fails, `async_operation_create` shall fail and return NULL.]*/
TEST_FUNCTION(when_allocating_memory_fails_async_operation_create_fails)
{
    // arrange
    ASYNC_OPERATION_HANDLE result;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    result = async_operation_create(test_cancel_handler, sizeof(uintptr_t));

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* async_operation_destroy */

/* Tests_SRS_ASYNC_OPERATION_01_005: [ `async_operation_destroy` shall free all recources associated with the asyncronous operation instance.]*/
TEST_FUNCTION(async_operation_destroy_frees_allocated_memory)
{
    // arrange
    ASYNC_OPERATION_HANDLE async_operation;
    async_operation = async_operation_create(test_cancel_handler, sizeof(uintptr_t));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    async_operation_destroy(async_operation);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_ASYNC_OPERATION_01_006: [ If `async_operation` is NULL, `async_operation_destroy` shall do nothing.]*/
TEST_FUNCTION(async_operation_destroy_with_NULL_async_operation_does_not_free_anything)
{
    // arrange

    // act
    async_operation_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* async_operation_cancel */

/* Tests_SRS_ASYNC_OPERATION_01_007: [ `async_operation_cancel` shall cancel the operation by calling the cancel handler function passed to `async_operation_create`.]*/
/* Tests_SRS_ASYNC_OPERATION_01_008: [ On success `async_operation_cancel` shall return 0.]*/
TEST_FUNCTION(async_operation_cancel_calls_the_cancel_handles)
{
    // arrange
    ASYNC_OPERATION_HANDLE async_operation;
    int result;
    async_operation = async_operation_create(test_cancel_handler, sizeof(uintptr_t));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_cancel_handler(async_operation));

    // act
    result = async_operation_cancel(async_operation);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    async_operation_destroy(async_operation);
}

/* Tests_SRS_ASYNC_OPERATION_01_009: [ If `async_operation` is NULL, `async_operation_cancel` shall fail and return a non-zero value.]*/
TEST_FUNCTION(async_operation_cancel_with_NULL_async_operation_fails)
{
    // arrange
    int result;

    // act
    result = async_operation_cancel(NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(async_operation_ut)
