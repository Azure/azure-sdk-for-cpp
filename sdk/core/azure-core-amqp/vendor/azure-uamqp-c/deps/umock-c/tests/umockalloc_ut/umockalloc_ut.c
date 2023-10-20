// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#include "testrunnerswitcher.h"
#include "umock_c/umockalloc.h"
#include "umock_c/umock_log.h"

void UMOCK_LOG(const char* format, ...)
{
    (void)format;
}

typedef struct test_malloc_CALL_TAG
{
    size_t size;
} test_malloc_CALL;

static test_malloc_CALL* test_malloc_calls;
static size_t test_malloc_call_count;
static void* test_malloc_expected_result;

typedef struct test_calloc_CALL_TAG
{
    size_t size;
    size_t nmemb;
} test_calloc_CALL;

static test_calloc_CALL* test_calloc_calls;
static size_t test_calloc_call_count;
static void* test_calloc_expected_result;

typedef struct test_realloc_CALL_TAG
{
    size_t size;
    void* ptr;
} test_realloc_CALL;

static test_realloc_CALL* test_realloc_calls;
static size_t test_realloc_call_count;
static void* test_realloc_expected_result;

typedef struct test_free_CALL_TAG
{
    void* ptr;
} test_free_CALL;

static test_free_CALL* test_free_calls;
static size_t test_free_call_count;

#ifdef __cplusplus
extern "C" {
#endif

    void* mock_malloc(size_t size)
    {
        test_malloc_CALL* new_calls = (test_malloc_CALL*)realloc(test_malloc_calls, sizeof(test_malloc_CALL) * (test_malloc_call_count + 1));
        if (new_calls != NULL)
        {
            test_malloc_calls = new_calls;
            test_malloc_calls[test_malloc_call_count].size = size;
            test_malloc_call_count++;
        }

        return test_malloc_expected_result;
    }

    void* mock_calloc(size_t nmemb, size_t size)
    {
        test_calloc_CALL* new_calls = (test_calloc_CALL*)realloc(test_calloc_calls, sizeof(test_calloc_CALL) * (test_calloc_call_count + 1));
        if (new_calls != NULL)
        {
            test_calloc_calls = new_calls;
            test_calloc_calls[test_calloc_call_count].nmemb = nmemb;
            test_calloc_calls[test_calloc_call_count].size = size;
            test_calloc_call_count++;
        }

        return test_calloc_expected_result;
    }

    void* mock_realloc(void* ptr, size_t size)
    {
        test_realloc_CALL* new_calls = (test_realloc_CALL*)realloc(test_realloc_calls, sizeof(test_realloc_CALL) * (test_realloc_call_count + 1));
        if (new_calls != NULL)
        {
            test_realloc_calls = new_calls;
            test_realloc_calls[test_realloc_call_count].ptr = ptr;
            test_realloc_calls[test_realloc_call_count].size = size;
            test_realloc_call_count++;
        }

        return test_realloc_expected_result;
    }

    void mock_free(void* ptr)
    {
        test_free_CALL* new_calls = (test_free_CALL*)realloc(test_free_calls, sizeof(test_free_CALL) * (test_free_call_count + 1));
        if (new_calls != NULL)
        {
            test_free_calls = new_calls;
            test_free_calls[test_free_call_count].ptr = ptr;
            test_free_call_count++;
        }
    }

#ifdef __cplusplus
}
#endif

static TEST_MUTEX_HANDLE test_mutex;
static TEST_MUTEX_HANDLE global_mutex;

BEGIN_TEST_SUITE(umockalloc_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    test_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_mutex);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    TEST_MUTEX_DESTROY(test_mutex);
}

TEST_FUNCTION_INITIALIZE(test_function_init)
{
    int mutex_acquire_result = TEST_MUTEX_ACQUIRE(test_mutex);
    ASSERT_ARE_EQUAL(int, 0, mutex_acquire_result);

    test_free_calls = NULL;
    test_free_call_count = 0;

    test_malloc_calls = NULL;
    test_malloc_call_count = 0;
    test_malloc_expected_result = (void*)0x4242;

    test_calloc_calls = NULL;
    test_calloc_call_count = 0;
    test_calloc_expected_result = (void*)0x4242;

    test_realloc_calls = NULL;
    test_realloc_call_count = 0;
    test_realloc_expected_result = (void*)0x4243;
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
    free(test_malloc_calls);
    test_malloc_calls = NULL;
    test_malloc_call_count = 0;

    free(test_calloc_calls);
    test_calloc_calls = NULL;
    test_calloc_call_count = 0;

    free(test_realloc_calls);
    test_realloc_calls = NULL;
    test_realloc_call_count = 0;

    free(test_free_calls);
    test_free_calls = NULL;
    test_free_call_count = 0;

    TEST_MUTEX_RELEASE(test_mutex);
}

/* umockalloc_malloc */

/* Tests_SRS_UMOCKALLOC_01_001: [ umockalloc_malloc shall call malloc, while passing the size argument to malloc. ] */
/* Tests_SRS_UMOCKALLOC_01_002: [ umockalloc_malloc shall return the result of malloc. ]*/
TEST_FUNCTION(umockalloc_malloc_calls_malloc)
{
    // arrange
	void* result;
    test_malloc_expected_result = (void*)0x4242;

    // act
    result = umockalloc_malloc(42);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, (void*)0x4242, result);
    ASSERT_ARE_EQUAL(size_t, 1, test_malloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 42, test_malloc_calls[0].size);
}

/* Tests_SRS_UMOCKALLOC_01_001: [ umockalloc_malloc shall call malloc, while passing the size argument to malloc. ] */
/* Tests_SRS_UMOCKALLOC_01_002: [ umockalloc_malloc shall return the result of malloc. ]*/
TEST_FUNCTION(umockalloc_malloc_calls_malloc_other_value)
{
    // arrange
	void* result;
    test_malloc_expected_result = (void*)0x5252;

    // act
    result = umockalloc_malloc(43);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, (void*)0x5252, result);
    ASSERT_ARE_EQUAL(size_t, 1, test_malloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 43, test_malloc_calls[0].size);
}

/* Tests_SRS_UMOCKALLOC_01_001: [ umockalloc_malloc shall call malloc, while passing the size argument to malloc. ] */
TEST_FUNCTION(when_malloc_returns_NULL_umockalloc_malloc_returns_NULL)
{
    // arrange
	void* result;
    test_malloc_expected_result = NULL;

    // act
    result = umockalloc_malloc(43);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(size_t, 1, test_malloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 43, test_malloc_calls[0].size);
}

/* umockalloc_calloc */

/* Tests_SRS_UMOCKALLOC_09_001: [ umockalloc_calloc shall call calloc, while passing the number of members and size arguments to calloc. ] */
/* Tests_SRS_UMOCKALLOC_09_002: [ umockalloc_calloc shall return the result of calloc. ]*/
TEST_FUNCTION(umockalloc_calloc_calls_calloc)
{
    // arrange
	void* result;
    test_calloc_expected_result = (void*)0x4242;

    // act
    result = umockalloc_calloc(1, 42);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, (void*)0x4242, result);
    ASSERT_ARE_EQUAL(size_t, 1, test_calloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 42, test_calloc_calls[0].size);
    ASSERT_ARE_EQUAL(size_t, 1, test_calloc_calls[0].nmemb);
}

/* Tests_SRS_UMOCKALLOC_09_001: [ umockalloc_calloc shall call calloc, while passing the number of members and size arguments to calloc. ] */
/* Tests_SRS_UMOCKALLOC_09_002: [ umockalloc_calloc shall return the result of calloc. ]*/
TEST_FUNCTION(umockalloc_calloc_calls_calloc_other_value)
{
    // arrange
	void* result;
    test_calloc_expected_result = (void*)0x5252;

    // act
    result = umockalloc_calloc(1, 43);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, (void*)0x5252, result);
    ASSERT_ARE_EQUAL(size_t, 1, test_calloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 43, test_calloc_calls[0].size);
    ASSERT_ARE_EQUAL(size_t, 1, test_calloc_calls[0].nmemb);
}

/* Tests_SRS_UMOCKALLOC_09_001: [ umockalloc_calloc shall call calloc, while passing the number of members and size arguments to calloc. ] */
/* Tests_SRS_UMOCKALLOC_09_002: [ umockalloc_calloc shall return the result of calloc. ]*/
TEST_FUNCTION(umockalloc_calloc_calls_calloc_2_members)
{
    // arrange
	void* result;
    test_calloc_expected_result = (void*)0x4242;

    // act
    result = umockalloc_calloc(2, 40);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, (void*)0x4242, result);
    ASSERT_ARE_EQUAL(size_t, 1, test_calloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 40, test_calloc_calls[0].size);
    ASSERT_ARE_EQUAL(size_t, 2, test_calloc_calls[0].nmemb);
}

/* Tests_SRS_UMOCKALLOC_09_001: [ umockalloc_calloc shall call calloc, while passing the number of members and size arguments to calloc. ] */
/* Tests_SRS_UMOCKALLOC_09_002: [ umockalloc_calloc shall return the result of calloc. ]*/
TEST_FUNCTION(umockalloc_calloc_calls_calloc_0_members)
{
    // arrange
    void* result;
    test_calloc_expected_result = NULL;

    // act
    result = umockalloc_calloc(0, 40);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(size_t, 1, test_calloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 40, test_calloc_calls[0].size);
    ASSERT_ARE_EQUAL(size_t, 0, test_calloc_calls[0].nmemb);
}

/* Tests_SRS_UMOCKALLOC_09_001: [ umockalloc_malloc shall call malloc, while passing the size argument to malloc. ] */
TEST_FUNCTION(when_calloc_returns_NULL_umockalloc_calloc_returns_NULL)
{
    // arrange
	void* result;
    test_calloc_expected_result = NULL;

    // act
    result = umockalloc_calloc(1, 43);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(size_t, 1, test_calloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 43, test_calloc_calls[0].size);
    ASSERT_ARE_EQUAL(size_t, 1, test_calloc_calls[0].nmemb);
}

/* umockalloc_realloc */

/* Tests_SRS_UMOCKALLOC_01_005: [ umockalloc_realloc shall call realloc, while passing the ptr and size arguments to realloc. ] */
/* Tests_SRS_UMOCKALLOC_01_006: [ umockalloc_realloc shall return the result of realloc. ]*/
TEST_FUNCTION(umockalloc_realloc_calls_realloc)
{
    // arrange
	void* result;
    test_realloc_expected_result = (void*)0x4242;

    // act
    result = umockalloc_realloc((void*)0x2222, 42);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, (void*)0x4242, result);
    ASSERT_ARE_EQUAL(size_t, 1, test_realloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 42, test_realloc_calls[0].size);
    ASSERT_ARE_EQUAL(void_ptr, (void*)0x2222, test_realloc_calls[0].ptr);
}

/* Tests_SRS_UMOCKALLOC_01_005: [ umockalloc_realloc shall call realloc, while passing the ptr and size arguments to realloc. ] */
/* Tests_SRS_UMOCKALLOC_01_006: [ umockalloc_realloc shall return the result of realloc. ]*/
TEST_FUNCTION(umockalloc_realloc_calls_realloc_other_value)
{
    // arrange
	void* result;
    test_realloc_expected_result = (void*)0x5252;

    // act
    result = umockalloc_realloc((void*)0x3232, 43);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, (void*)0x5252, result);
    ASSERT_ARE_EQUAL(size_t, 1, test_realloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 43, test_realloc_calls[0].size);
    ASSERT_ARE_EQUAL(void_ptr, (void*)0x3232, test_realloc_calls[0].ptr);
}

/* Tests_SRS_UMOCKALLOC_01_005: [ umockalloc_realloc shall call realloc, while passing the ptr and size arguments to realloc. ] */
/* Tests_SRS_UMOCKALLOC_01_006: [ umockalloc_realloc shall return the result of realloc. ]*/
TEST_FUNCTION(when_realloc_returns_NULL_umockalloc_realloc_returns_NULL)
{
    // arrange
	void* result;
    test_realloc_expected_result = NULL;

    // act
    result = umockalloc_realloc((void*)0x3232, 43);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(size_t, 1, test_realloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 43, test_realloc_calls[0].size);
    ASSERT_ARE_EQUAL(void_ptr, (void*)0x3232, test_realloc_calls[0].ptr);
}

/* Tests_SRS_UMOCKALLOC_01_005: [ umockalloc_realloc shall call realloc, while passing the ptr and size arguments to realloc. ] */
/* Tests_SRS_UMOCKALLOC_01_006: [ umockalloc_realloc shall return the result of realloc. ]*/
TEST_FUNCTION(umockalloc_realloc_with_NULL_and_0_size_calls_the_underlying_realloc)
{
    // arrange
	void* result;
    test_realloc_expected_result = (void*)0x4242;

    // act
    result = umockalloc_realloc(NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, (void*)0x4242, result);
    ASSERT_ARE_EQUAL(size_t, 1, test_realloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 0, test_realloc_calls[0].size);
    ASSERT_ARE_EQUAL(void_ptr, NULL, test_realloc_calls[0].ptr);
}

/* umockalloc_free */

/* Tests_SRS_UMOCKALLOC_01_007: [ umockalloc_free shall call free, while passing the ptr argument to free. ]*/
TEST_FUNCTION(umockalloc_free_calls_free)
{
    // arrange

    // act
    umockalloc_free((void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(size_t, 1, test_free_call_count);
    ASSERT_ARE_EQUAL(void_ptr, (void*)0x4242, test_free_calls[0].ptr);
}

/* Tests_SRS_UMOCKALLOC_01_007: [ umockalloc_free shall call free, while passing the ptr argument to free. ]*/
TEST_FUNCTION(umockalloc_free_with_NULL_calls_free_with_NULL)
{
    // arrange

    // act
    umockalloc_free(NULL);

    // assert
    ASSERT_ARE_EQUAL(size_t, 1, test_free_call_count);
    ASSERT_ARE_EQUAL(void_ptr, NULL, test_free_calls[0].ptr);
}

END_TEST_SUITE(umockalloc_unittests)
