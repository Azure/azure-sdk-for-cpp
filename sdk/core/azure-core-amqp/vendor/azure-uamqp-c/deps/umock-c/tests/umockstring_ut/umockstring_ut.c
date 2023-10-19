// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#include "testrunnerswitcher.h"
#include "umock_c/umockstring.h"
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
static size_t when_shall_malloc_fail;

#ifdef __cplusplus
extern "C" {
#endif

    void* mock_malloc(size_t size)
    {
        void* result;

        test_malloc_CALL* new_calls = (test_malloc_CALL*)realloc(test_malloc_calls, sizeof(test_malloc_CALL) * (test_malloc_call_count + 1));
        if (new_calls != NULL)
        {
            test_malloc_calls = new_calls;
            test_malloc_calls[test_malloc_call_count].size = size;
            test_malloc_call_count++;
        }

        if (when_shall_malloc_fail == test_malloc_call_count)
        {
            result = NULL;
        }
        else
        {
            result = malloc(size);
        }

        return result;
    }

#ifdef __cplusplus
}
#endif

static TEST_MUTEX_HANDLE test_mutex;
static TEST_MUTEX_HANDLE global_mutex;

BEGIN_TEST_SUITE(umockstring_unittests)

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

    when_shall_malloc_fail = 0;

    test_malloc_calls = NULL;
    test_malloc_call_count = 0;
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
    free(test_malloc_calls);
    test_malloc_calls = NULL;
    test_malloc_call_count = 0;

    TEST_MUTEX_RELEASE(test_mutex);
}

/* umockstring_clone */

/* Tests_UMOCK_STRING_01_001: [ umockstring_clone shall allocate memory for the cloned string (including the NULL terminator). ]*/
/* Tests_UMOCK_STRING_01_002: [ umockstring_clone shall copy the string to the newly allocated memory (including the NULL terminator). ]*/
/* Tests_UMOCK_STRING_01_003: [ On success umockstring_clone shall return a pointer to the newly allocated memory containing the copy of the string. ]*/
TEST_FUNCTION(umockstring_clone_with_an_empty_string_succeeds)
{
    // arrange
    char* result;

    // act
    result = umockstring_clone("");

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", result);
    ASSERT_ARE_EQUAL(size_t, 1, test_malloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, test_malloc_calls[0].size);

    // cleanup
    free(result);
}

/* Tests_UMOCK_STRING_01_001: [ umockstring_clone shall allocate memory for the cloned string (including the NULL terminator). ]*/
/* Tests_UMOCK_STRING_01_002: [ umockstring_clone shall copy the string to the newly allocated memory (including the NULL terminator). ]*/
/* Tests_UMOCK_STRING_01_003: [ On success umockstring_clone shall return a pointer to the newly allocated memory containing the copy of the string. ]*/
TEST_FUNCTION(umockstring_clone_with_a_one_char_string_succeeds)
{
    // arrange
    char* result;

    // act
    result = umockstring_clone("a");

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "a", result);
    ASSERT_ARE_EQUAL(size_t, 1, test_malloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 2, test_malloc_calls[0].size);

    // cleanup
    free(result);
}

/* Tests_UMOCK_STRING_01_001: [ umockstring_clone shall allocate memory for the cloned string (including the NULL terminator). ]*/
/* Tests_UMOCK_STRING_01_002: [ umockstring_clone shall copy the string to the newly allocated memory (including the NULL terminator). ]*/
/* Tests_UMOCK_STRING_01_003: [ On success umockstring_clone shall return a pointer to the newly allocated memory containing the copy of the string. ]*/
TEST_FUNCTION(umockstring_clone_with_a_longer_string_succeeds)
{
    // arrange
    char* result;

    // act
    result = umockstring_clone("Management takes the code out of you");

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "Management takes the code out of you", result);
    ASSERT_ARE_EQUAL(size_t, 1, test_malloc_call_count);
    ASSERT_ARE_EQUAL(size_t, strlen("Management takes the code out of you") + 1, test_malloc_calls[0].size);

    // cleanup
    free(result);
}

/* Tests_UMOCK_STRING_01_004: [ If allocating the memory fails, umockstring_clone shall return NULL. ]*/
TEST_FUNCTION(when_allocating_memory_fails_umockstring_clone_fails)
{
    // arrange
    char* result;

    when_shall_malloc_fail = 1;

    // act
    result = umockstring_clone("Management takes the code out of you");

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(size_t, 1, test_malloc_call_count);
    ASSERT_ARE_EQUAL(size_t, strlen("Management takes the code out of you") + 1, test_malloc_calls[0].size);
}

/* Tests_UMOCK_STRING_01_005: [ If umockstring_clone is called with a NULL source, it shall return NULL. ]*/
TEST_FUNCTION(umockstring_clone_called_with_NULL_fails)
{
    // arrange
    char* result;

    // act
    result = umockstring_clone(NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(size_t, 0, test_malloc_call_count);
}

END_TEST_SUITE(umockstring_unittests)
