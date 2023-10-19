// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#include "testrunnerswitcher.h"
#include "umock_c/umockcallpairs.h"
#include "umock_c/umockcall.h"
#include "umock_c/umock_log.h"

void UMOCK_LOG(const char* format, ...)
{
    (void)format;
}

typedef struct umocktypes_are_equal_CALL_TAG
{
    char* type;
    const void* left;
    const void* right;
} umocktypes_are_equal_CALL;

static umocktypes_are_equal_CALL* umocktypes_are_equal_calls;
static size_t umocktypes_are_equal_call_count;
static size_t when_shall_umocktypes_are_equal_call_fail;
static int umocktypes_are_equal_call_result;
static int umocktypes_are_equal_fail_call_result;

typedef struct umocktypes_copy_CALL_TAG
{
    char* type;
    void* destination;
    const void* source;
} umocktypes_copy_CALL;

static umocktypes_copy_CALL* umocktypes_copy_calls;
static size_t umocktypes_copy_call_count;
static size_t when_shall_umocktypes_copy_call_fail;
static int umocktypes_copy_call_result;
static int umocktypes_copy_fail_call_result;

typedef struct umocktypes_free_CALL_TAG
{
    char* type;
    const void* value;
} umocktypes_free_CALL;

static umocktypes_free_CALL* umocktypes_free_calls;
static size_t umocktypes_free_call_count;

#ifdef __cplusplus
extern "C" {
#endif

    int umocktypes_are_equal(const char* type, const void* left, const void* right)
    {
        int result;

        umocktypes_are_equal_CALL* new_calls = (umocktypes_are_equal_CALL*)realloc(umocktypes_are_equal_calls, sizeof(umocktypes_are_equal_CALL) * (umocktypes_are_equal_call_count + 1));
        if (new_calls != NULL)
        {
            size_t typename_length = strlen(type);
            umocktypes_are_equal_calls = new_calls;
            umocktypes_are_equal_calls[umocktypes_are_equal_call_count].type = (char*)malloc(typename_length + 1);
            (void)memcpy(umocktypes_are_equal_calls[umocktypes_are_equal_call_count].type, type, typename_length + 1);
            umocktypes_are_equal_calls[umocktypes_are_equal_call_count].left = left;
            umocktypes_are_equal_calls[umocktypes_are_equal_call_count].right = right;
            umocktypes_are_equal_call_count++;
        }

        if (when_shall_umocktypes_are_equal_call_fail == umocktypes_are_equal_call_count)
        {
            result = umocktypes_are_equal_fail_call_result;
        }
        else
        {
            result = umocktypes_are_equal_call_result;
        }

        return result;
    }

    int umocktypes_copy(const char* type, void* destination, const void* source)
    {
        int result;

        umocktypes_copy_CALL* new_calls = (umocktypes_copy_CALL*)realloc(umocktypes_copy_calls, sizeof(umocktypes_copy_CALL) * (umocktypes_copy_call_count + 1));
        if (new_calls != NULL)
        {
            size_t typename_length = strlen(type);
            umocktypes_copy_calls = new_calls;
            umocktypes_copy_calls[umocktypes_copy_call_count].type = (char*)malloc(typename_length + 1);
            (void)memcpy(umocktypes_copy_calls[umocktypes_copy_call_count].type, type, typename_length + 1);
            umocktypes_copy_calls[umocktypes_copy_call_count].destination = destination;
            umocktypes_copy_calls[umocktypes_copy_call_count].source = source;
            umocktypes_copy_call_count++;
        }

        if (when_shall_umocktypes_copy_call_fail == umocktypes_copy_call_count)
        {
            result = umocktypes_copy_fail_call_result;
        }
        else
        {
            result = umocktypes_copy_call_result;
        }

        return result;
    }

    void umocktypes_free(const char* type, const void* value)
    {
        umocktypes_free_CALL* new_calls = (umocktypes_free_CALL*)realloc(umocktypes_free_calls, sizeof(umocktypes_free_CALL) * (umocktypes_free_call_count + 1));
        if (new_calls != NULL)
        {
            size_t typename_length = strlen(type);
            umocktypes_free_calls = new_calls;
            umocktypes_free_calls[umocktypes_free_call_count].type = (char*)malloc(typename_length + 1);
            (void)memcpy(umocktypes_free_calls[umocktypes_free_call_count].type, type, typename_length + 1);
            umocktypes_free_calls[umocktypes_free_call_count].value = value;
            umocktypes_free_call_count++;
        }
    }

    void reset_umocktypes_are_equal_calls(void)
    {
        if (umocktypes_are_equal_calls != NULL)
        {
            size_t i;
            for (i = 0; i < umocktypes_are_equal_call_count; i++)
            {
                free(umocktypes_are_equal_calls[i].type);
            }

            free(umocktypes_are_equal_calls);
            umocktypes_are_equal_calls = NULL;
        }
        umocktypes_are_equal_call_count = 0;
        when_shall_umocktypes_are_equal_call_fail = 0;
    }

    void reset_umocktypes_copy_calls(void)
    {
        if (umocktypes_copy_calls != NULL)
        {
            size_t i;
            for (i = 0; i < umocktypes_copy_call_count; i++)
            {
                free(umocktypes_copy_calls[i].type);
            }

            free(umocktypes_copy_calls);
            umocktypes_copy_calls = NULL;
        }
        umocktypes_copy_call_count = 0;
        when_shall_umocktypes_copy_call_fail = 0;
    }

    void reset_umocktypes_free_calls(void)
    {
        if (umocktypes_free_calls != NULL)
        {
            size_t i;
            for (i = 0; i < umocktypes_free_call_count; i++)
            {
                free(umocktypes_free_calls[i].type);
            }

            free(umocktypes_free_calls);
            umocktypes_free_calls = NULL;
        }
        umocktypes_free_call_count = 0;
    }

    static size_t malloc_call_count;
    static size_t realloc_call_count;
    static size_t free_call_count;

    static size_t when_shall_malloc_fail;
    static size_t when_shall_realloc_fail;

    void* mock_malloc(size_t size)
    {
        void* result;
        malloc_call_count++;
        if (malloc_call_count == when_shall_malloc_fail)
        {
            result = NULL;
        }
        else
        {
            result = malloc(size);
        }
        return result;
    }

    void* mock_realloc(void* ptr, size_t size)
    {
        void* result;
        realloc_call_count++;
        if (realloc_call_count == when_shall_realloc_fail)
        {
            result = NULL;
        }
        else
        {
            result = realloc(ptr, size);
        }
        return result;
    }

    void mock_free(void* ptr)
    {
        free_call_count++;
        free(ptr);
    }

#ifdef __cplusplus
}
#endif

void reset_malloc_calls(void)
{
    malloc_call_count = 0;
    when_shall_malloc_fail = 0;
    realloc_call_count = 0;
    when_shall_realloc_fail = 0;
    free_call_count = 0;
}

static void reset_all_calls(void)
{
    reset_malloc_calls();
    reset_umocktypes_are_equal_calls();
    reset_umocktypes_copy_calls();
    reset_umocktypes_free_calls();
}

static TEST_MUTEX_HANDLE test_mutex;
static TEST_MUTEX_HANDLE global_mutex;

BEGIN_TEST_SUITE(umockcallpairs_unittests)

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

    reset_all_calls();
    umocktypes_copy_call_result = 0;
    umocktypes_are_equal_call_result = 1;
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
    TEST_MUTEX_RELEASE(test_mutex);

    reset_all_calls();
}

/* umockcallpairs_track_create_paired_call */

/* Tests_SRS_UMOCKCALLPAIRS_01_001: [ umockcallpairs_track_create_paired_call shall add a new entry to the PAIRED_HANDLES array and on success it shall return 0. ]*/
/* Tests_SRS_UMOCKCALLPAIRS_01_002: [ umockcallpairs_track_create_paired_call shall copy the handle_value to the handle_value member of the new entry. ] */
/* Tests_SRS_UMOCKCALLPAIRS_01_003: [ umockcallpairs_track_create_paired_call shall allocate a memory block and store a pointer to it in the memory field of the new entry. ]*/
/* Tests_SRS_UMOCKCALLPAIRS_01_006: [ The handle value shall be copied by using umocktypes_copy. ]*/
TEST_FUNCTION(umockcallpairs_track_create_paired_call_succeeds)
{
    // arrange
    PAIRED_HANDLES paired_handles = { NULL, 0 };
    void* handle = (void*)0x4242;

    // act
    int result = umockcallpairs_track_create_paired_call(&paired_handles, &handle, "void*", sizeof(handle));

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 2, malloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, realloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, umocktypes_copy_call_count);
    ASSERT_ARE_EQUAL(char_ptr, "void*", umocktypes_copy_calls[0].type);
    ASSERT_ARE_EQUAL(void_ptr, &handle, umocktypes_copy_calls[0].source);

    // cleanup
    free(paired_handles.paired_handles[0].handle_type);
    free(paired_handles.paired_handles[0].handle_value);
    free(paired_handles.paired_handles);
}

/* Tests_SRS_UMOCKCALLPAIRS_01_004: [ If any of the arguments paired_handles, handle or handle_type is NULL, umockcallpairs_track_create_paired_call shallfail and return a non-zero value. ]*/
TEST_FUNCTION(when_paired_handles_is_NULL_umockcallpairs_track_create_paired_call_fails)
{
    // arrange
    void* handle = (void*)0x4242;

    // act
    int result = umockcallpairs_track_create_paired_call(NULL, &handle, "void*", sizeof(handle));

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 0, malloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 0, realloc_call_count);
}

/* Tests_SRS_UMOCKCALLPAIRS_01_004: [ If any of the arguments paired_handles, handle or handle_type is NULL, umockcallpairs_track_create_paired_call shallfail and return a non-zero value. ]*/
TEST_FUNCTION(when_handle_is_NULL_umockcallpairs_track_create_paired_call_fails)
{
    // arrange
    PAIRED_HANDLES paired_handles = { NULL, 0 };

    // act
    int result = umockcallpairs_track_create_paired_call(&paired_handles, NULL, "void*", sizeof(void*));

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 0, malloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 0, realloc_call_count);
}

/* Tests_SRS_UMOCKCALLPAIRS_01_004: [ If any of the arguments paired_handles, handle or handle_type is NULL, umockcallpairs_track_create_paired_call shallfail and return a non-zero value. ]*/
TEST_FUNCTION(when_handle_type_is_NULL_umockcallpairs_track_create_paired_call_fails)
{
    // arrange
    PAIRED_HANDLES paired_handles = { NULL, 0 };
    void* handle = (void*)0x4242;

    // act
    int result = umockcallpairs_track_create_paired_call(&paired_handles, &handle, NULL, sizeof(handle));

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 0, malloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 0, realloc_call_count);
}

/* Tests_SRS_UMOCKCALLPAIRS_01_005: [ If allocating memory fails, umockcallpairs_track_create_paired_call shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_reallocating_the_entire_paired_handles_array_fails_umockcallpairs_track_create_paired_call_fails)
{
    // arrange
	int result;
    PAIRED_HANDLES paired_handles = { NULL, 0 };
    void* handle = (void*)0x4242;

    when_shall_realloc_fail = 1;

    // act
    result = umockcallpairs_track_create_paired_call(&paired_handles, &handle, "void*", sizeof(handle));

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 1, realloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 0, malloc_call_count);
}

/* Tests_SRS_UMOCKCALLPAIRS_01_005: [ If allocating memory fails, umockcallpairs_track_create_paired_call shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_allocating_the_handle_value_memory_block_fails_umockcallpairs_track_create_paired_call_fails)
{
    // arrange
	int result;
    PAIRED_HANDLES paired_handles = { NULL, 0 };
    void* handle = (void*)0x4242;

    when_shall_malloc_fail = 1;

    // act
    result = umockcallpairs_track_create_paired_call(&paired_handles, &handle, "void*", sizeof(handle));

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 1, malloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, realloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, free_call_count);
}

/* Tests_SRS_UMOCKCALLPAIRS_01_005: [ If allocating memory fails, umockcallpairs_track_create_paired_call shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_allocating_the_handle_type_block_fails_umockcallpairs_track_create_paired_call_fails)
{
    // arrange
	int result;
    PAIRED_HANDLES paired_handles = { NULL, 0 };
    void* handle = (void*)0x4242;

    when_shall_malloc_fail = 2;

    // act
    result = umockcallpairs_track_create_paired_call(&paired_handles, &handle, "void*", sizeof(handle));

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 2, malloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, realloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 2, free_call_count);
}

/* Tests_SRS_UMOCKCALLPAIRS_01_007: [ If umocktypes_copy fails, umockcallpairs_track_create_paired_call shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_copying_the_handle_fails_umockcallpairs_track_create_paired_call_fails)
{
    // arrange
	int result;
    PAIRED_HANDLES paired_handles = { NULL, 0 };
    void* handle = (void*)0x4242;

    when_shall_umocktypes_copy_call_fail = 1;
    umocktypes_copy_fail_call_result = -1;

    // act
    result = umockcallpairs_track_create_paired_call(&paired_handles, &handle, "void*", sizeof(handle));

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 1, realloc_call_count);
    ASSERT_ARE_EQUAL(size_t, 3, free_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, umocktypes_copy_call_count);
    ASSERT_ARE_EQUAL(size_t, 2, malloc_call_count);
    ASSERT_ARE_EQUAL(char_ptr, "void*", umocktypes_copy_calls[0].type);
    ASSERT_ARE_EQUAL(void_ptr, &handle, umocktypes_copy_calls[0].source);
}

TEST_FUNCTION(when_realloc_fails_a_subsequent_create_and_destroy_succeeds)
{
    // arrange
	int result;
    PAIRED_HANDLES paired_handles = { NULL, 0 };
    void* handle = (void*)0x4242;
	void* copied_handle;
    when_shall_realloc_fail = 1;
    (void)umockcallpairs_track_create_paired_call(&paired_handles, &handle, "void*", sizeof(handle));
    reset_all_calls();
    when_shall_realloc_fail = 0;

    (void)umockcallpairs_track_create_paired_call(&paired_handles, &handle, "void*", sizeof(handle));
    copied_handle = umocktypes_copy_calls[0].destination;
    reset_all_calls();

    // act
    result = umockcallpairs_track_destroy_paired_call(&paired_handles, &handle);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 3, free_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, umocktypes_are_equal_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, umocktypes_free_call_count);
    ASSERT_ARE_EQUAL(char_ptr, "void*", umocktypes_free_calls[0].type);
    ASSERT_ARE_EQUAL(void_ptr, copied_handle, umocktypes_free_calls[0].value);
}

/* umockcallpairs_track_destroy_paired_call */

/* Tests_SRS_UMOCKCALLPAIRS_01_008: [ umockcallpairs_track_destroy_paired_call shall remove from the paired handles array pointed by the paired_handles field the entry that is associated with the handle passed in the handle argument. ]*/
/* Tests_SRS_UMOCKCALLPAIRS_01_009: [ On success umockcallpairs_track_destroy_paired_call shall return 0. ]*/
/* Tests_SRS_UMOCKCALLPAIRS_01_011: [ umockcallpairs_track_destroy_paired_call shall free the memory pointed by the memory field in the PAIRED_HANDLES array entry associated with handle. ]*/
/* Tests_SRS_UMOCKCALLPAIRS_01_012: [ If the array paired handles array is empty after removing the entry, the paired_handles field shall be freed and set to NULL. ]*/
/* Tests_SRS_UMOCKCALLPAIRS_01_013: [ When looking up which entry to remove, the comparison of the handle values shall be done by calling umocktypes_are_equal. ]*/
TEST_FUNCTION(umockcallpairs_track_destroy_paired_call_removes_a_tracked_handle)
{
    // arrange
	int result;
    PAIRED_HANDLES paired_handles = { NULL, 0 };
    void* handle = (void*)0x4242;
	void* copied_handle;
    (void)umockcallpairs_track_create_paired_call(&paired_handles, &handle, "void*", sizeof(handle));
    copied_handle = umocktypes_copy_calls[0].destination;
    reset_all_calls();

    // act
    result = umockcallpairs_track_destroy_paired_call(&paired_handles, &handle);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 3, free_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, umocktypes_are_equal_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, umocktypes_free_call_count);
    ASSERT_ARE_EQUAL(char_ptr, "void*", umocktypes_free_calls[0].type);
    ASSERT_ARE_EQUAL(void_ptr, copied_handle, umocktypes_free_calls[0].value);
}

/* Tests_SRS_UMOCKCALLPAIRS_01_008: [ umockcallpairs_track_destroy_paired_call shall remove from the paired handles array pointed by the paired_handles field the entry that is associated with the handle passed in the handle argument. ]*/
/* Tests_SRS_UMOCKCALLPAIRS_01_009: [ On success umockcallpairs_track_destroy_paired_call shall return 0. ]*/
/* Tests_SRS_UMOCKCALLPAIRS_01_011: [ umockcallpairs_track_destroy_paired_call shall free the memory pointed by the memory field in the PAIRED_HANDLES array entry associated with handle. ]*/
/* Tests_SRS_UMOCKCALLPAIRS_01_012: [ If the array paired handles array is empty after removing the entry, the paired_handles field shall be freed and set to NULL. ]*/
/* Tests_SRS_UMOCKCALLPAIRS_01_013: [ When looking up which entry to remove, the comparison of the handle values shall be done by calling umocktypes_are_equal. ]*/
TEST_FUNCTION(umockcallpairs_track_destroy_paired_call_with_2_creates_removes_the_tracked_handle)
{
    // arrange
	int result;
    PAIRED_HANDLES paired_handles = { NULL, 0 };
    void* handle1 = (void*)0x4242;
    void* handle2 = (void*)0x4243;
	void* first_copied_handle;
    (void)umockcallpairs_track_create_paired_call(&paired_handles, &handle1, "void*", sizeof(handle1));
    (void)umockcallpairs_track_create_paired_call(&paired_handles, &handle2, "void*", sizeof(handle2));
    first_copied_handle = umocktypes_copy_calls[0].destination;
    reset_all_calls();

    // act
    result = umockcallpairs_track_destroy_paired_call(&paired_handles, &handle1);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 2, free_call_count);
    ASSERT_ARE_EQUAL(size_t, 1, umocktypes_are_equal_call_count);
    ASSERT_ARE_EQUAL(char_ptr, "void*", umocktypes_are_equal_calls[0].type);
    ASSERT_ARE_EQUAL(void_ptr, first_copied_handle, umocktypes_are_equal_calls[0].left);
    ASSERT_ARE_EQUAL(size_t, 1, umocktypes_free_call_count);
    ASSERT_ARE_EQUAL(char_ptr, "void*", umocktypes_free_calls[0].type);

    // cleanup
    free(paired_handles.paired_handles[0].handle_type);
    free(paired_handles.paired_handles[0].handle_value);
    free(paired_handles.paired_handles);
}

/* Tests_SRS_UMOCKCALLPAIRS_01_008: [ umockcallpairs_track_destroy_paired_call shall remove from the paired handles array pointed by the paired_handles field the entry that is associated with the handle passed in the handle argument. ]*/
/* Tests_SRS_UMOCKCALLPAIRS_01_009: [ On success umockcallpairs_track_destroy_paired_call shall return 0. ]*/
/* Tests_SRS_UMOCKCALLPAIRS_01_011: [ umockcallpairs_track_destroy_paired_call shall free the memory pointed by the memory field in the PAIRED_HANDLES array entry associated with handle. ]*/
/* Tests_SRS_UMOCKCALLPAIRS_01_013: [ When looking up which entry to remove, the comparison of the handle values shall be done by calling umocktypes_are_equal. ]*/
TEST_FUNCTION(when_the_handle_is_found_at_the_second_index_umockcallpairs_track_destroy_paired_call_succeeds)
{
    // arrange
	int result;
    PAIRED_HANDLES paired_handles = { NULL, 0 };
    void* handle1 = (void*)0x4242;
    void* handle2 = (void*)0x4243;
    (void)umockcallpairs_track_create_paired_call(&paired_handles, &handle1, "void*", sizeof(handle1));
    (void)umockcallpairs_track_create_paired_call(&paired_handles, &handle2, "void*", sizeof(handle2));
    reset_all_calls();

    when_shall_umocktypes_are_equal_call_fail = 1;
    umocktypes_are_equal_fail_call_result = 0;

    // act
    result = umockcallpairs_track_destroy_paired_call(&paired_handles, &handle2);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 1, umocktypes_free_call_count);
    ASSERT_ARE_EQUAL(char_ptr, "void*", umocktypes_free_calls[0].type);

    // cleanup
    free(paired_handles.paired_handles[0].handle_type);
    free(paired_handles.paired_handles[0].handle_value);
    free(paired_handles.paired_handles);
}

/* Tests_SRS_UMOCKCALLPAIRS_01_010: [ If any of the arguments is NULL, umockcallpairs_track_destroy_paired_call shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umockcallpairs_track_destroy_paired_call_with_NULL_paired_handles_fails)
{
    // arrange
    void* handle = (void*)0x4242;

    // act
    int result = umockcallpairs_track_destroy_paired_call(NULL, &handle);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 0, free_call_count);
    ASSERT_ARE_EQUAL(size_t, 0, umocktypes_are_equal_call_count);
    ASSERT_ARE_EQUAL(size_t, 0, umocktypes_free_call_count);
}

/* Tests_SRS_UMOCKCALLPAIRS_01_010: [ If any of the arguments is NULL, umockcallpairs_track_destroy_paired_call shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umockcallpairs_track_destroy_paired_call_with_NULL_handle_fails)
{
    // arrange
	int result;
    PAIRED_HANDLES paired_handles = { NULL, 0 };
    void* handle = (void*)0x4242;
    (void)umockcallpairs_track_create_paired_call(&paired_handles, &handle, "void*", sizeof(handle));
    reset_all_calls();

    // act
    result = umockcallpairs_track_destroy_paired_call(&paired_handles, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 0, free_call_count);
    ASSERT_ARE_EQUAL(size_t, 0, umocktypes_are_equal_call_count);
    ASSERT_ARE_EQUAL(size_t, 0, umocktypes_free_call_count);

    // cleanup
    free(paired_handles.paired_handles[0].handle_type);
    free(paired_handles.paired_handles[0].handle_value);
    free(paired_handles.paired_handles);
}

/* Tests_SRS_UMOCKCALLPAIRS_01_014: [ If umocktypes_are_equal fails, umockcallpairs_track_destroy_paired_call shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_are_equal_fails_umockcallpairs_track_destroy_paired_call_fails)
{
    // arrange
	int result;
    PAIRED_HANDLES paired_handles = { NULL, 0 };
    void* handle = (void*)0x4242;
    (void)umockcallpairs_track_create_paired_call(&paired_handles, &handle, "void*", sizeof(handle));
    reset_all_calls();

    when_shall_umocktypes_are_equal_call_fail = 1;
    umocktypes_are_equal_fail_call_result = -1;

    // act
    result = umockcallpairs_track_destroy_paired_call(&paired_handles, &handle);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    free(paired_handles.paired_handles[0].handle_type);
    free(paired_handles.paired_handles[0].handle_value);
    free(paired_handles.paired_handles);
}

/* Tests_SRS_UMOCKCALLPAIRS_01_015: [ If the handle is not found in the array then umockcallpairs_track_destroy_paired_call shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_the_handle_is_not_found_umockcallpairs_track_destroy_paired_call_fails)
{
    // arrange
	int result;
    PAIRED_HANDLES paired_handles = { NULL, 0 };
    void* handle1 = (void*)0x4242;
    void* handle2 = (void*)0x4243;
    (void)umockcallpairs_track_create_paired_call(&paired_handles, &handle1, "void*", sizeof(handle2));
    reset_all_calls();

    when_shall_umocktypes_are_equal_call_fail = 1;
    umocktypes_are_equal_fail_call_result = 0;

    // act
    result = umockcallpairs_track_destroy_paired_call(&paired_handles, &handle2);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    free(paired_handles.paired_handles[0].handle_type);
    free(paired_handles.paired_handles[0].handle_value);
    free(paired_handles.paired_handles);
}

END_TEST_SUITE(umockcallpairs_unittests)
