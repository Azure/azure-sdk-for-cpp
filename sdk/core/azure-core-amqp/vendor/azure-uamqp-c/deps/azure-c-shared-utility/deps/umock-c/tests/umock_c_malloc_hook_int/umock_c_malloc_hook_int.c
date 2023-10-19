// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#else
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#endif


#include "testrunnerswitcher.h"

static size_t my_malloc_count;

void* my_malloc(size_t size)
{
    my_malloc_count++;
    return malloc(size);
}

static size_t my_calloc_count;

void* my_calloc(size_t nmemb, size_t size)
{
    my_calloc_count++;
    return calloc(nmemb, size);
}

static size_t my_realloc_count;

void* my_realloc(void* ptr, size_t size)
{
    my_realloc_count++;
    return realloc(ptr, size);
}

static size_t my_free_count;

void my_free(void* ptr)
{
    my_free_count++;
    free(ptr);
}

#define malloc my_malloc
#define calloc my_calloc
#define realloc my_realloc
#define free my_free

#define ENABLE_MOCKS

#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "azure_macro_utils/macro_utils.h"

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void test_on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static TEST_MUTEX_HANDLE test_mutex;

MOCK_FUNCTION_WITH_CODE(, int, function1, int, a)
MOCK_FUNCTION_END(42)

BEGIN_TEST_SUITE(umock_c_malloc_hook_integrationtests)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    test_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_mutex);

    result = umock_c_init(test_on_umock_c_error);
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_charptr_register_types();
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

    umock_c_reset_all_calls();
    my_malloc_count = 0;
    my_calloc_count = 0;
    my_realloc_count = 0;
    my_free_count = 0;
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
    TEST_MUTEX_RELEASE(test_mutex);
}

TEST_FUNCTION(when_malloc_is_hooked_no_calls_are_made_to_it)
{
    // arrange
    STRICT_EXPECTED_CALL(function1(42));

    // act
    function1(42);

    // assert
    ASSERT_ARE_EQUAL(size_t, 0, my_malloc_count);
    ASSERT_ARE_EQUAL(size_t, 0, my_calloc_count);
    ASSERT_ARE_EQUAL(size_t, 0, my_realloc_count);
    ASSERT_ARE_EQUAL(size_t, 0, my_free_count);
}

END_TEST_SUITE(umock_c_malloc_hook_integrationtests)
