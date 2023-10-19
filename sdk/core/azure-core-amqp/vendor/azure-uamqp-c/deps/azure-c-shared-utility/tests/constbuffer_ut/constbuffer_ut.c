// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#else
#include <stdlib.h>
#include <stddef.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"

void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

void* my_gballoc_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#define ENABLE_MOCKS
#include "umock_c/umock_c.h"
#include "azure_c_shared_utility/buffer_.h"
#include "azure_c_shared_utility/gballoc.h"

#undef ENABLE_MOCKS
#include "azure_c_shared_utility/constbuffer.h"

static TEST_MUTEX_HANDLE g_testByTest;

static const char* buffer1 = "le buffer no 1";
static const char* buffer2 = NULL;
static const char* buffer3 = "three";

#define BUFFER1_HANDLE (BUFFER_HANDLE)1
#define BUFFER1_u_char ((unsigned char*)buffer1)
#define BUFFER1_length strlen(buffer1)

#define BUFFER2_HANDLE (BUFFER_HANDLE)2
#define BUFFER2_u_char ((unsigned char*)buffer2)
#define BUFFER2_length ((size_t)0)

#define BUFFER3_HANDLE (BUFFER_HANDLE)3
#define BUFFER3_u_char ((unsigned char*)buffer3)
#define BUFFER3_length ((size_t)0)

unsigned char* my_BUFFER_u_char(BUFFER_HANDLE handle)
{
    unsigned char* result;
    if (handle == BUFFER1_HANDLE)
    {
        result = BUFFER1_u_char;
    }
    else
    {
        result = NULL;
        ASSERT_FAIL("who am I?");
    }
    return result;
}

static size_t my_BUFFER_length(BUFFER_HANDLE handle)
{
    size_t result;
    if (handle == BUFFER1_HANDLE)
    {
        result = BUFFER1_length;
    }
    else
    {
        result = 0;
        ASSERT_FAIL("who am I?");
    }
    return result;
}

MOCK_FUNCTION_WITH_CODE(, void, test_free_func, void*, context)
MOCK_FUNCTION_END()

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(constbuffer_unittests)

    TEST_SUITE_INITIALIZE(setsBufferTempSize)
    {
        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        umock_c_init(on_umock_c_error);

        REGISTER_UMOCK_ALIAS_TYPE(BUFFER_HANDLE, void*);

        REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, my_gballoc_calloc);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
        REGISTER_GLOBAL_MOCK_HOOK(BUFFER_u_char, my_BUFFER_u_char);
        REGISTER_GLOBAL_MOCK_HOOK(BUFFER_length, my_BUFFER_length);
    }

    TEST_SUITE_CLEANUP(TestClassCleanup)
    {
        umock_c_deinit();

        TEST_MUTEX_DESTROY(g_testByTest);
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

    /* CONSTBUFFER_Create */

    /*Tests_SRS_CONSTBUFFER_02_001: [If source is NULL and size is different than 0 then CONSTBUFFER_Create shall fail and return NULL.]*/
    TEST_FUNCTION(CONSTBUFFER_Create_with_invalid_args_fails)
    {
        ///arrange

        ///act
        CONSTBUFFER_HANDLE handle = CONSTBUFFER_Create(NULL, 1);

        ///assert
        ASSERT_IS_NULL(handle);

        ///cleanup
    }

    /*Tests_SRS_CONSTBUFFER_02_002: [Otherwise, CONSTBUFFER_Create shall create a copy of the memory area pointed to by source having size bytes.]*/
    /*Tests_SRS_CONSTBUFFER_02_004: [Otherwise CONSTBUFFER_Create shall return a non-NULL handle.]*/
    TEST_FUNCTION(CONSTBUFFER_Create_succeeds)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle;
        const CONSTBUFFER* content;

        ///act
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        handle = CONSTBUFFER_Create(BUFFER1_u_char, BUFFER1_length);

        ///assert
        ASSERT_IS_NOT_NULL(handle);
        /*testing the "copy"*/
        content = CONSTBUFFER_GetContent(handle);
        ASSERT_ARE_EQUAL(size_t, BUFFER1_length, content->size);
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER1_u_char, content->buffer, BUFFER1_length));
        /*testing that it is a copy and not a pointer assignment*/
        ASSERT_ARE_NOT_EQUAL(void_ptr, BUFFER1_u_char, content->buffer);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        CONSTBUFFER_DecRef(handle);
    }

    /* CONSTBUFFER_CreateFromBuffer */

    /*Tests_SRS_CONSTBUFFER_02_009: [Otherwise, CONSTBUFFER_CreateFromBuffer shall return a non-NULL handle.]*/
    /*Tests_SRS_CONSTBUFFER_02_007: [Otherwise, CONSTBUFFER_CreateFromBuffer shall copy the content of buffer.]*/
    TEST_FUNCTION(CONSTBUFFER_CreateFromBuffer_succeeds)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle;
        const CONSTBUFFER* content;

        ///act
        STRICT_EXPECTED_CALL(BUFFER_length(BUFFER1_HANDLE));
        STRICT_EXPECTED_CALL(BUFFER_u_char(BUFFER1_HANDLE));
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        handle = CONSTBUFFER_CreateFromBuffer(BUFFER1_HANDLE);

        ///assert
        ASSERT_IS_NOT_NULL(handle);
        /*testing the "copy"*/
        content = CONSTBUFFER_GetContent(handle);
        ASSERT_ARE_EQUAL(size_t, BUFFER1_length, content->size);
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER1_u_char, content->buffer, BUFFER1_length));
        /*testing that it is a copy and not a pointer assignment*/
        ASSERT_ARE_NOT_EQUAL(void_ptr, BUFFER1_u_char, content->buffer);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        CONSTBUFFER_DecRef(handle);
    }

    /*Tests_SRS_CONSTBUFFER_02_008: [If copying the content fails, then CONSTBUFFER_CreateFromBuffer shall fail and return NULL.]*/
    TEST_FUNCTION(CONSTBUFFER_CreateFromBuffer_fails_when_malloc_fails)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle;

        ///act
        STRICT_EXPECTED_CALL(BUFFER_length(BUFFER1_HANDLE));
        STRICT_EXPECTED_CALL(BUFFER_u_char(BUFFER1_HANDLE));

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments()
            .SetReturn(NULL);


        handle = CONSTBUFFER_CreateFromBuffer(BUFFER1_HANDLE);

        ///assert
        ASSERT_IS_NULL(handle);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        CONSTBUFFER_DecRef(handle);
    }

    /*Tests_SRS_CONSTBUFFER_02_006: [If buffer is NULL then CONSTBUFFER_CreateFromBuffer shall fail and return NULL.]*/
    TEST_FUNCTION(CONSTBUFFER_CreateFromBuffer_with_NULL_fails)
    {
        ///arrange

        ///act
        CONSTBUFFER_HANDLE handle = CONSTBUFFER_CreateFromBuffer(NULL);

        ///assert
        ASSERT_IS_NULL(handle);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        CONSTBUFFER_DecRef(handle);
    }

    /*Tests_SRS_CONSTBUFFER_02_003: [If creating the copy fails then CONSTBUFFER_Create shall return NULL.]*/
    TEST_FUNCTION(CONSTBUFFER_Create_fails_when_malloc_fails)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle;

        ///act
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments()
            .SetReturn(NULL);

        handle = CONSTBUFFER_Create(BUFFER1_u_char, BUFFER1_length);

        ///assert
        ASSERT_IS_NULL(handle);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
    }

    /*Tests_SRS_CONSTBUFFER_02_005: [The non-NULL handle returned by CONSTBUFFER_Create shall have its ref count set to "1".]*/
    /*Tests_SRS_CONSTBUFFER_02_017: [If the refcount reaches zero, then CONSTBUFFER_DecRef shall deallocate all resources used by the CONSTBUFFER_HANDLE.]*/
    TEST_FUNCTION(CONSTBUFFER_Create_is_ref_counted_1)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle = CONSTBUFFER_Create(BUFFER1_u_char, BUFFER1_length);
        umock_c_reset_all_calls();
        ///act

        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        CONSTBUFFER_DecRef(handle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
    }

    /*Tests_SRS_CONSTBUFFER_02_002: [Otherwise, CONSTBUFFER_Create shall create a copy of the memory area pointed to by source having size bytes.]*/
    TEST_FUNCTION(CONSTBUFFER_Create_from_0_size_succeeds_1)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle;
        const CONSTBUFFER* content;

        ///act
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        handle = CONSTBUFFER_Create(BUFFER2_u_char, BUFFER2_length);

        ///assert
        ASSERT_IS_NOT_NULL(handle);
        /*testing the "copy"*/
        content = CONSTBUFFER_GetContent(handle);
        ASSERT_ARE_EQUAL(size_t, BUFFER2_length, content->size);
        /*testing that it is a copy and not a pointer assignment*/
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        CONSTBUFFER_DecRef(handle);
    }

    /*Tests_SRS_CONSTBUFFER_02_002: [Otherwise, CONSTBUFFER_Create shall create a copy of the memory area pointed to by source having size bytes.]*/
    /*Tests_SRS_CONSTBUFFER_02_009: [Otherwise, CONSTBUFFER_CreateFromBuffer shall return a non-NULL handle.]*/
    TEST_FUNCTION(CONSTBUFFER_Create_from_0_size_succeeds_2)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle;
        const CONSTBUFFER* content;

        ///act
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        handle = CONSTBUFFER_Create(BUFFER3_u_char, BUFFER3_length);

        ///assert
        ASSERT_IS_NOT_NULL(handle);
        /*testing the "copy"*/
        content = CONSTBUFFER_GetContent(handle);
        ASSERT_ARE_EQUAL(size_t, BUFFER3_length, content->size);
        /*testing that it is a copy and not a pointer assignment*/
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        CONSTBUFFER_DecRef(handle);
    }

    /* CONSTBUFFER_CreateWithMoveMemory */

    /* Tests_SRS_CONSTBUFFER_01_001: [ If source is NULL and size is different than 0 then CONSTBUFFER_CreateWithMoveMemory shall fail and return NULL. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateWithMoveMemory_with_invalid_args_fails)
    {
        ///arrange

        ///act
        CONSTBUFFER_HANDLE handle = CONSTBUFFER_CreateWithMoveMemory(NULL, 1);

        ///assert
        ASSERT_IS_NULL(handle);

        ///cleanup
    }

    /* Tests_SRS_CONSTBUFFER_01_002: [ CONSTBUFFER_CreateWithMoveMemory shall store the source and size and return a non-NULL handle to the newly created const buffer. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateWithMoveMemory_succeeds)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle;
        const CONSTBUFFER* content;
        unsigned char* test_buffer = (unsigned char* )my_gballoc_malloc(2);
        test_buffer[0] = 42;
        test_buffer[1] = 43;

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        ///act
        handle = CONSTBUFFER_CreateWithMoveMemory(test_buffer, 2);

        ///assert
        ASSERT_IS_NOT_NULL(handle);
        /*testing the "storage"*/
        content = CONSTBUFFER_GetContent(handle);
        ASSERT_ARE_EQUAL(size_t, 2, content->size);
        ASSERT_ARE_EQUAL(void_ptr, test_buffer, content->buffer, "same buffer should be returned");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        CONSTBUFFER_DecRef(handle);
    }

    /* Tests_SRS_CONSTBUFFER_01_002: [ CONSTBUFFER_CreateWithMoveMemory shall store the source and size and return a non-NULL handle to the newly created const buffer. ]*/
    /* Tests_SRS_CONSTBUFFER_01_004: [ If source is non-NULL and size is 0, the source pointer shall be owned (and freed) by the newly created instance of const buffer. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateWithMoveMemory_with_0_size_succeeds)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle;
        const CONSTBUFFER* content;
        unsigned char* test_buffer = (unsigned char* )my_gballoc_malloc(2);
        test_buffer[0] = 42;
        test_buffer[1] = 43;

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        ///act
        handle = CONSTBUFFER_CreateWithMoveMemory(test_buffer, 0);

        ///assert
        ASSERT_IS_NOT_NULL(handle);
        /*testing the "storage"*/
        content = CONSTBUFFER_GetContent(handle);
        ASSERT_ARE_EQUAL(size_t, 0, content->size);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        CONSTBUFFER_DecRef(handle);
    }

    /* Tests_SRS_CONSTBUFFER_01_002: [ CONSTBUFFER_CreateWithMoveMemory shall store the source and size and return a non-NULL handle to the newly created const buffer. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateWithMoveMemory_with_NULL_source_and_0_size_succeeds)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle;
        const CONSTBUFFER* content;

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        ///act
        handle = CONSTBUFFER_CreateWithMoveMemory(NULL, 0);

        ///assert
        ASSERT_IS_NOT_NULL(handle);
        /*testing the "storage"*/
        content = CONSTBUFFER_GetContent(handle);
        ASSERT_ARE_EQUAL(size_t, 0, content->size);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        CONSTBUFFER_DecRef(handle);
    }

    /* Tests_SRS_CONSTBUFFER_01_005: [ If any error occurs, CONSTBUFFER_CreateWithMoveMemory shall fail and return NULL. ]*/
    TEST_FUNCTION(when_malloc_fails_CONSTBUFFER_CreateWithMoveMemory_fails)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle;
        unsigned char* test_buffer = (unsigned char* )my_gballoc_malloc(2);
        test_buffer[0] = 42;
        test_buffer[1] = 43;

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments()
            .SetReturn(NULL);

        ///act
        handle = CONSTBUFFER_CreateWithMoveMemory(test_buffer, 2);

        ///assert
        ASSERT_IS_NULL(handle);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        free(test_buffer);
    }

    /* CONSTBUFFER_CreateWithCustomFree */

    /* Tests_SRS_CONSTBUFFER_01_006: [ If source is NULL and size is different than 0 then CONSTBUFFER_CreateWithCustomFree shall fail and return NULL. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateWithCustomFree_with_invalid_args_fails)
    {
        ///arrange

        ///act
        CONSTBUFFER_HANDLE handle = CONSTBUFFER_CreateWithCustomFree(NULL, 1, test_free_func, (void*)0x4242);

        ///assert
        ASSERT_IS_NULL(handle);
    }

    /* Tests_SRS_CONSTBUFFER_01_013: [ If customFreeFunc is NULL, CONSTBUFFER_CreateWithCustomFree shall fail and return NULL. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateWithCustomFree_with_NULL_customFreeFunc_fails)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle;
        unsigned char* test_buffer = (unsigned char*)my_gballoc_malloc(2);
        test_buffer[0] = 42;
        test_buffer[1] = 43;

        ///act
        handle = CONSTBUFFER_CreateWithCustomFree(test_buffer, 2, NULL, test_buffer);

        ///assert
        ASSERT_IS_NULL(handle);

        /// cleanup
        my_gballoc_free(test_buffer);
    }

    /* Tests_SRS_CONSTBUFFER_01_008: [ CONSTBUFFER_CreateWithCustomFree shall store the source and size and return a non-NULL handle to the newly created const buffer. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateWithCustomFree_succeeds)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle;
        const CONSTBUFFER* content;
        unsigned char* test_buffer = (unsigned char*)my_gballoc_malloc(2);
        test_buffer[0] = 42;
        test_buffer[1] = 43;

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        ///act
        handle = CONSTBUFFER_CreateWithCustomFree(test_buffer, 2, free, test_buffer);

        ///assert
        ASSERT_IS_NOT_NULL(handle);
        /*testing the "storage"*/
        content = CONSTBUFFER_GetContent(handle);
        ASSERT_ARE_EQUAL(size_t, 2, content->size);
        ASSERT_ARE_EQUAL(void_ptr, test_buffer, content->buffer, "same buffer should be returned");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        CONSTBUFFER_DecRef(handle);
    }

    /* Tests_SRS_CONSTBUFFER_01_014: [ customFreeFuncContext shall be allowed to be NULL. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateWithCustomFree_succeedswith_NULL_free_function_context)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle;
        const CONSTBUFFER* content;
        unsigned char* test_buffer = (unsigned char*)my_gballoc_malloc(2);
        test_buffer[0] = 42;
        test_buffer[1] = 43;

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        ///act
        handle = CONSTBUFFER_CreateWithCustomFree(test_buffer, 2, test_free_func, NULL);

        ///assert
        ASSERT_IS_NOT_NULL(handle);
        /*testing the "storage"*/
        content = CONSTBUFFER_GetContent(handle);
        ASSERT_ARE_EQUAL(size_t, 2, content->size);
        ASSERT_ARE_EQUAL(void_ptr, test_buffer, content->buffer, "same buffer should be returned");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        CONSTBUFFER_DecRef(handle);
        my_gballoc_free(test_buffer);
    }

    /* Tests_SRS_CONSTBUFFER_01_008: [ CONSTBUFFER_CreateWithCustomFree shall store the source and size and return a non-NULL handle to the newly created const buffer. ]*/
    /* Tests_SRS_CONSTBUFFER_01_007: [ If source is non-NULL and size is 0, the source pointer shall be owned (and freed) by the newly created instance of const buffer. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateWithCustomFree_with_0_size_succeeds)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle;
        const CONSTBUFFER* content;
        unsigned char* test_buffer = (unsigned char*)my_gballoc_malloc(2);
        test_buffer[0] = 42;
        test_buffer[1] = 43;

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        ///act
        handle = CONSTBUFFER_CreateWithCustomFree(test_buffer, 0, free, test_buffer);

        ///assert
        ASSERT_IS_NOT_NULL(handle);
        /*testing the "storage"*/
        content = CONSTBUFFER_GetContent(handle);
        ASSERT_ARE_EQUAL(size_t, 0, content->size);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        CONSTBUFFER_DecRef(handle);
    }

    /* Tests_SRS_CONSTBUFFER_01_008: [ CONSTBUFFER_CreateWithCustomFree shall store the source and size and return a non-NULL handle to the newly created const buffer. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateWithCustomFree_with_NULL_source_and_0_size_succeeds)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle;
        const CONSTBUFFER* content;

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        ///act
        handle = CONSTBUFFER_CreateWithCustomFree(NULL, 0, free, NULL);

        ///assert
        ASSERT_IS_NOT_NULL(handle);
        /*testing the "storage"*/
        content = CONSTBUFFER_GetContent(handle);
        ASSERT_ARE_EQUAL(size_t, 0, content->size);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        CONSTBUFFER_DecRef(handle);
    }

    /* Tests_SRS_CONSTBUFFER_01_011: [ If any error occurs, CONSTBUFFER_CreateWithMoveMemory shall fail and return NULL. ]*/
    TEST_FUNCTION(when_malloc_fails_CONSTBUFFER_CreateWithCustomFree_fails)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle;
        unsigned char* test_buffer = (unsigned char*)my_gballoc_malloc(2);
        test_buffer[0] = 42;
        test_buffer[1] = 43;

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments()
            .SetReturn(NULL);

        ///act
        handle = CONSTBUFFER_CreateWithCustomFree(test_buffer, 2, free, test_buffer);

        ///assert
        ASSERT_IS_NULL(handle);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        free(test_buffer);
    }

    /* CONSTBUFFER_GetContent */

    /*Tests_SRS_CONSTBUFFER_02_011: [If constbufferHandle is NULL then CONSTBUFFER_GetContent shall return NULL.]*/
    TEST_FUNCTION(CONSTBUFFER_GetContent_with_NULL_returns_NULL)
    {
        ///arrange

        ///act
        const CONSTBUFFER* content = CONSTBUFFER_GetContent(NULL);

        ///assert
        ASSERT_IS_NULL(content);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
    }

    /*Tests_SRS_CONSTBUFFER_02_012: [Otherwise, CONSTBUFFER_GetContent shall return a const CONSTBUFFER* that matches byte by byte the original bytes used to created the const buffer and has the same length.]*/
    TEST_FUNCTION(CONSTBUFFER_GetContent_succeeds_1)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle;
        const CONSTBUFFER* content;

        handle = CONSTBUFFER_Create(BUFFER1_u_char, BUFFER1_length);
        umock_c_reset_all_calls();

        ///act
        content = CONSTBUFFER_GetContent(handle);

        ///assert
        ASSERT_IS_NOT_NULL(content);
        /*testing the "copy"*/
        ASSERT_ARE_EQUAL(size_t, BUFFER1_length, content->size);
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER1_u_char, content->buffer, BUFFER1_length));
        /*testing that it is a copy and not a pointer assignment*/
        ASSERT_ARE_NOT_EQUAL(void_ptr, BUFFER1_u_char, content->buffer);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        CONSTBUFFER_DecRef(handle);
    }

    /*Tests_SRS_CONSTBUFFER_02_012: [Otherwise, CONSTBUFFER_GetContent shall return a const CONSTBUFFER* that matches byte by byte the original bytes used to created the const buffer and has the same length.]*/
    TEST_FUNCTION(CONSTBUFFER_GetContent_succeeds_2)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle;
        const CONSTBUFFER* content;
        handle = CONSTBUFFER_Create(NULL, 0);
        umock_c_reset_all_calls();

        ///act
        content = CONSTBUFFER_GetContent(handle);

        ///assert
        ASSERT_IS_NOT_NULL(content);
        /*testing the "copy"*/
        ASSERT_ARE_EQUAL(size_t, 0, content->size);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        CONSTBUFFER_DecRef(handle);
    }

    /*Tests_SRS_CONSTBUFFER_02_025: [ If handle is NULL then CONSTBUFFER_CreateFromOffsetAndSize shall fail and return NULL. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateFromOffsetAndSize_with_handle_NULL_fails)
    {
        ///arrange

        ///act
        CONSTBUFFER_HANDLE result = CONSTBUFFER_CreateFromOffsetAndSize(NULL, 0, 0);

        ///assert
        ASSERT_IS_NULL(result);
    }

    /*Tests_SRS_CONSTBUFFER_02_033: [ If offset is greater than handles's size then CONSTBUFFER_CreateFromOffsetAndSize shall fail and return NULL. ]*/
    /*Tests_SRS_CONSTBUFFER_02_027: [ If offset + size exceed handles's size then CONSTBUFFER_CreateFromOffsetAndSize shall fail and return NULL. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateFromOffsetAndSize_with_offset_greater_than_handle_s_size_fails)
    {
        ///arrange
        CONSTBUFFER_HANDLE origin;
        const char source[] = "source";
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        origin = CONSTBUFFER_Create((const unsigned char*)source, sizeof(source));
        ASSERT_IS_NOT_NULL(origin);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        umock_c_reset_all_calls();

        ///act
        CONSTBUFFER_HANDLE result;
        result = CONSTBUFFER_CreateFromOffsetAndSize(origin, sizeof(source) + 1, 0);
        ASSERT_IS_NULL(result);
        result = CONSTBUFFER_CreateFromOffsetAndSize(origin, 0, sizeof(source) + 1);
        ASSERT_IS_NULL(result);
        result = CONSTBUFFER_CreateFromOffsetAndSize(origin, sizeof(source), 1);
        ASSERT_IS_NULL(result);
        result = CONSTBUFFER_CreateFromOffsetAndSize(origin, 1, sizeof(source));
        ASSERT_IS_NULL(result);

        ///cleanup
        CONSTBUFFER_DecRef(origin);
    }

    /*Tests_SRS_CONSTBUFFER_02_032: [ If there are any failures then CONSTBUFFER_CreateFromOffsetAndSize shall fail and return NULL. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateFromOffsetAndSize_with_offset_plus_size_equal_to_SIZE_MAX_fail)
    {
        ///arrange
        CONSTBUFFER_HANDLE origin;
        const char source[] = "source";
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        origin = CONSTBUFFER_Create((const unsigned char*)source, sizeof(source));
        ASSERT_IS_NOT_NULL(origin);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        umock_c_reset_all_calls();

        ///act
        CONSTBUFFER_HANDLE result = CONSTBUFFER_CreateFromOffsetAndSize(origin, sizeof(source) - 1, SIZE_MAX- sizeof(source) + 2);

        ///assert
        ASSERT_IS_NULL(result);

        ///cleanup
        CONSTBUFFER_DecRef(origin);
    }

    /*Tests_SRS_CONSTBUFFER_02_027: [ If offset + size exceed handles's size then CONSTBUFFER_CreateFromOffsetAndSize shall fail and return NULL. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateFromOffsetAndSize_with_offset_plus_size_exceed_handle_size_fails_2)
    {
        ///arrange
        CONSTBUFFER_HANDLE origin;
        const char source[] = "source";
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        origin = CONSTBUFFER_Create((const unsigned char*)source, sizeof(source));
        ASSERT_IS_NOT_NULL(origin);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        umock_c_reset_all_calls();

        ///act
        CONSTBUFFER_HANDLE result = CONSTBUFFER_CreateFromOffsetAndSize(origin, sizeof(source)-1, 2);

        ///assert
        ASSERT_IS_NULL(result);

        ///cleanup
        CONSTBUFFER_DecRef(origin);
    }

    /*Tests_SRS_CONSTBUFFER_02_028: [ CONSTBUFFER_CreateFromOffsetAndSize shall allocate memory for a new CONSTBUFFER_HANDLE's content. ]*/
    /*Tests_SRS_CONSTBUFFER_02_029: [ CONSTBUFFER_CreateFromOffsetAndSize shall set the ref count of the newly created CONSTBUFFER_HANDLE to the initial value. ]*/
    /*Tests_SRS_CONSTBUFFER_02_030: [ CONSTBUFFER_CreateFromOffsetAndSize shall increment the reference count of handle. ]*/
    /*Tests_SRS_CONSTBUFFER_02_031: [ CONSTBUFFER_CreateFromOffsetAndSize shall succeed and return a non-NULL value. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateFromOffsetAndSize_succeeds_1)
    {
        ///arrange
        CONSTBUFFER_HANDLE origin;
        const char source[] = "source";
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        origin = CONSTBUFFER_Create((const unsigned char*)source, sizeof(source));
        ASSERT_IS_NOT_NULL(origin);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        ///act
        CONSTBUFFER_HANDLE result = CONSTBUFFER_CreateFromOffsetAndSize(origin, 0, sizeof(source));

        ///assert
        ASSERT_IS_NOT_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        const CONSTBUFFER* content = CONSTBUFFER_GetContent(result);
        ASSERT_ARE_EQUAL(size_t, sizeof(source), content->size);
        ASSERT_IS_TRUE(memcmp(content->buffer, source, sizeof(source)) == 0);

        ///cleanup
        CONSTBUFFER_DecRef(origin);
        CONSTBUFFER_DecRef(result);
    }

    /*Tests_SRS_CONSTBUFFER_02_028: [ CONSTBUFFER_CreateFromOffsetAndSize shall allocate memory for a new CONSTBUFFER_HANDLE's content. ]*/
    /*Tests_SRS_CONSTBUFFER_02_029: [ CONSTBUFFER_CreateFromOffsetAndSize shall set the ref count of the newly created CONSTBUFFER_HANDLE to the initial value. ]*/
    /*Tests_SRS_CONSTBUFFER_02_030: [ CONSTBUFFER_CreateFromOffsetAndSize shall increment the reference count of handle. ]*/
    /*Tests_SRS_CONSTBUFFER_02_031: [ CONSTBUFFER_CreateFromOffsetAndSize shall succeed and return a non-NULL value. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateFromOffsetAndSize_succeeds_2)
    {
        ///arrange
        CONSTBUFFER_HANDLE origin;
        const char source[] = "source";
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        origin = CONSTBUFFER_Create((const unsigned char*)source, sizeof(source));
        ASSERT_IS_NOT_NULL(origin);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        ///act
        CONSTBUFFER_HANDLE result = CONSTBUFFER_CreateFromOffsetAndSize(origin, 0, 0);

        ///assert
        ASSERT_IS_NOT_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        const CONSTBUFFER* content = CONSTBUFFER_GetContent(result);
        ASSERT_ARE_EQUAL(size_t, 0, content->size);

        ///cleanup
        CONSTBUFFER_DecRef(origin);
        CONSTBUFFER_DecRef(result);
    }

    /*Tests_SRS_CONSTBUFFER_02_028: [ CONSTBUFFER_CreateFromOffsetAndSize shall allocate memory for a new CONSTBUFFER_HANDLE's content. ]*/
    /*Tests_SRS_CONSTBUFFER_02_029: [ CONSTBUFFER_CreateFromOffsetAndSize shall set the ref count of the newly created CONSTBUFFER_HANDLE to the initial value. ]*/
    /*Tests_SRS_CONSTBUFFER_02_030: [ CONSTBUFFER_CreateFromOffsetAndSize shall increment the reference count of handle. ]*/
    /*Tests_SRS_CONSTBUFFER_02_031: [ CONSTBUFFER_CreateFromOffsetAndSize shall succeed and return a non-NULL value. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateFromOffsetAndSize_succeeds_3)
    {
        ///arrange
        CONSTBUFFER_HANDLE origin;
        const char source[] = "source";
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        origin = CONSTBUFFER_Create((const unsigned char*)source, sizeof(source));
        ASSERT_IS_NOT_NULL(origin);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        ///act
        CONSTBUFFER_HANDLE result = CONSTBUFFER_CreateFromOffsetAndSize(origin, sizeof(source) - 1, 1);

        ///assert
        ASSERT_IS_NOT_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        const CONSTBUFFER* content = CONSTBUFFER_GetContent(result);
        ASSERT_ARE_EQUAL(size_t, 1, content->size);

        ///cleanup
        CONSTBUFFER_DecRef(origin);
        CONSTBUFFER_DecRef(result);
    }

    /*Tests_SRS_CONSTBUFFER_02_028: [ CONSTBUFFER_CreateFromOffsetAndSize shall allocate memory for a new CONSTBUFFER_HANDLE's content. ]*/
    /*Tests_SRS_CONSTBUFFER_02_029: [ CONSTBUFFER_CreateFromOffsetAndSize shall set the ref count of the newly created CONSTBUFFER_HANDLE to the initial value. ]*/
    /*Tests_SRS_CONSTBUFFER_02_030: [ CONSTBUFFER_CreateFromOffsetAndSize shall increment the reference count of handle. ]*/
    /*Tests_SRS_CONSTBUFFER_02_031: [ CONSTBUFFER_CreateFromOffsetAndSize shall succeed and return a non-NULL value. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateFromOffsetAndSize_succeeds_4)
    {
        ///arrange
        CONSTBUFFER_HANDLE origin;
        const char source[] = "source";
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        origin = CONSTBUFFER_Create((const unsigned char*)source, sizeof(source));
        ASSERT_IS_NOT_NULL(origin);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        ///act
        CONSTBUFFER_HANDLE result = CONSTBUFFER_CreateFromOffsetAndSize(origin, 1, sizeof(source) - 1);

        ///assert
        ASSERT_IS_NOT_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        const CONSTBUFFER* content = CONSTBUFFER_GetContent(result);
        ASSERT_ARE_EQUAL(size_t, sizeof(source)-1, content->size);
        ASSERT_IS_TRUE(memcmp(content->buffer, source+1, sizeof(source)-1) == 0);

        ///cleanup
        CONSTBUFFER_DecRef(origin);
        CONSTBUFFER_DecRef(result);
    }

    /*Tests_SRS_CONSTBUFFER_02_028: [ CONSTBUFFER_CreateFromOffsetAndSize shall allocate memory for a new CONSTBUFFER_HANDLE's content. ]*/
    /*Tests_SRS_CONSTBUFFER_02_029: [ CONSTBUFFER_CreateFromOffsetAndSize shall set the ref count of the newly created CONSTBUFFER_HANDLE to the initial value. ]*/
    /*Tests_SRS_CONSTBUFFER_02_030: [ CONSTBUFFER_CreateFromOffsetAndSize shall increment the reference count of handle. ]*/
    /*Tests_SRS_CONSTBUFFER_02_031: [ CONSTBUFFER_CreateFromOffsetAndSize shall succeed and return a non-NULL value. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateFromOffsetAndSize_succeeds_5)
    {
        ///arrange
        CONSTBUFFER_HANDLE origin;
        const char source[] = "source";
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        origin = CONSTBUFFER_Create((const unsigned char*)source, sizeof(source));
        ASSERT_IS_NOT_NULL(origin);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        ///act
        CONSTBUFFER_HANDLE result = CONSTBUFFER_CreateFromOffsetAndSize(origin, sizeof(source), 0);

        ///assert
        ASSERT_IS_NOT_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        const CONSTBUFFER* content = CONSTBUFFER_GetContent(result);
        ASSERT_ARE_EQUAL(size_t, 0, content->size);

        ///cleanup
        CONSTBUFFER_DecRef(origin);
        CONSTBUFFER_DecRef(result);
    }

    /*Tests_SRS_CONSTBUFFER_02_032: [ If there are any failures then CONSTBUFFER_CreateFromOffsetAndSize shall fail and return NULL. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateFromOffsetAndSize_when_malloc_fails_it_fails)
    {
        ///arrange
        CONSTBUFFER_HANDLE origin;
        const char source[] = "source";
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        origin = CONSTBUFFER_Create((const unsigned char*)source, sizeof(source));
        ASSERT_IS_NOT_NULL(origin);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments()
            .SetReturn(NULL);

        ///act
        CONSTBUFFER_HANDLE result = CONSTBUFFER_CreateFromOffsetAndSize(origin, 1, sizeof(source) - 1);

        ///assert
        ASSERT_IS_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        CONSTBUFFER_DecRef(origin);
    }

    /*Tests_SRS_CONSTBUFFER_02_024: [ If the constbufferHandle was created by calling CONSTBUFFER_CreateFromOffsetAndSize then CONSTBUFFER_DecRef shall decrement the ref count of the original handle passed to CONSTBUFFER_CreateFromOffsetAndSize. ]*/
    /*Tests_SRS_CONSTBUFFER_02_032: [ If there are any failures then CONSTBUFFER_CreateFromOffsetAndSize shall fail and return NULL. ]*/
    TEST_FUNCTION(CONSTBUFFER_DecRef_for_CONSTBUFFER_CreateFromOffsetAndSize_succeeds)
    {
        ///arrange
        CONSTBUFFER_HANDLE origin;
        const char source[] = "source";
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        origin = CONSTBUFFER_Create((const unsigned char*)source, sizeof(source));
        ASSERT_IS_NOT_NULL(origin);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        CONSTBUFFER_HANDLE result = CONSTBUFFER_CreateFromOffsetAndSize(origin, 1, sizeof(source) - 1);
        ASSERT_IS_NOT_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_free(result));

        ///act
        CONSTBUFFER_DecRef(result);

        ///assert - origin should pretty much be still accesible
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        const CONSTBUFFER* content = CONSTBUFFER_GetContent(origin);
        ASSERT_ARE_EQUAL(size_t, sizeof(source), content->size);
        ASSERT_IS_TRUE(memcmp(content->buffer, source, sizeof(source)) == 0);
        
        ///cleanup
        CONSTBUFFER_DecRef(origin);
    }

    /*Tests_SRS_CONSTBUFFER_02_024: [ If the constbufferHandle was created by calling CONSTBUFFER_CreateFromOffsetAndSize then CONSTBUFFER_DecRef shall decrement the ref count of the original handle passed to CONSTBUFFER_CreateFromOffsetAndSize. ]*/
    /*Tests_SRS_CONSTBUFFER_02_032: [ If there are any failures then CONSTBUFFER_CreateFromOffsetAndSize shall fail and return NULL. ]*/
    TEST_FUNCTION(CONSTBUFFER_DecRef_for_CONSTBUFFER_CreateFromOffsetAndSize_succeeds_2)
    {
        ///arrange
        CONSTBUFFER_HANDLE origin;
        const char source[] = "source";
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        origin = CONSTBUFFER_Create((const unsigned char*)source, sizeof(source));
        ASSERT_IS_NOT_NULL(origin);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        CONSTBUFFER_HANDLE result = CONSTBUFFER_CreateFromOffsetAndSize(origin, 1, sizeof(source) - 1);
        ASSERT_IS_NOT_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        umock_c_reset_all_calls();

        /*note: no expected calls - after the below call both buffers are at ref count == 1*/

        ///act
        CONSTBUFFER_DecRef(origin);

        ///assert - origin should pretty much be still accesible
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        const CONSTBUFFER * content = CONSTBUFFER_GetContent(origin);
        ASSERT_ARE_EQUAL(size_t, sizeof(source), content->size);
        ASSERT_IS_TRUE(memcmp(content->buffer, source, sizeof(source)) == 0);

        const CONSTBUFFER* contentResult = CONSTBUFFER_GetContent(result);
        ASSERT_ARE_EQUAL(size_t, sizeof(source)-1, contentResult->size);
        ASSERT_IS_TRUE(memcmp(contentResult->buffer, source+ 1, sizeof(source)-1) == 0);

        ///cleanup
        CONSTBUFFER_DecRef(result);
    }

    /*Tests_SRS_CONSTBUFFER_02_024: [ If the constbufferHandle was created by calling CONSTBUFFER_CreateFromOffsetAndSize then CONSTBUFFER_DecRef shall decrement the ref count of the original handle passed to CONSTBUFFER_CreateFromOffsetAndSize. ]*/
    TEST_FUNCTION(CONSTBUFFER_DecRef_for_CONSTBUFFER_CreateFromOffsetAndSize_succeeds_3)
    {
        ///arrange
        CONSTBUFFER_HANDLE origin;
        const char source[] = "source";
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        origin = CONSTBUFFER_Create((const unsigned char*)source, sizeof(source));
        ASSERT_IS_NOT_NULL(origin);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        CONSTBUFFER_HANDLE result1 = CONSTBUFFER_CreateFromOffsetAndSize(origin, 0, 2);
        ASSERT_IS_NOT_NULL(result1);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        CONSTBUFFER_HANDLE result2 = CONSTBUFFER_CreateFromOffsetAndSize(result1, 1, 1);
        ASSERT_IS_NOT_NULL(result2);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        umock_c_reset_all_calls();

        CONSTBUFFER_DecRef(origin);
        umock_c_reset_all_calls();

        /*note: no expected calls - after the below call all buffers are at ref count == 1*/

        ///act
        CONSTBUFFER_DecRef(result1); /*at this time result 2 has a ref to result1, which has a ref to origin. Nothing is freed*/

        ///assert 
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        CONSTBUFFER_DecRef(result2); /*triggers the release of result1, which triggers the release of origin*/
    }

    /*Tests_SRS_CONSTBUFFER_02_024: [ If the constbufferHandle was created by calling CONSTBUFFER_CreateFromOffsetAndSize then CONSTBUFFER_DecRef shall decrement the ref count of the original handle passed to CONSTBUFFER_CreateFromOffsetAndSize. ]*/
    TEST_FUNCTION(CONSTBUFFER_DecRef_for_CONSTBUFFER_CreateFromOffsetAndSize_succeeds_4)
    {
        ///arrange
        CONSTBUFFER_HANDLE origin;
        const char source[] = "source";
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        origin = CONSTBUFFER_Create((const unsigned char*)source, sizeof(source));
        ASSERT_IS_NOT_NULL(origin);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        CONSTBUFFER_HANDLE result1 = CONSTBUFFER_CreateFromOffsetAndSize(origin, 0, 2);
        ASSERT_IS_NOT_NULL(result1);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        CONSTBUFFER_HANDLE result2 = CONSTBUFFER_CreateFromOffsetAndSize(result1, 1, 1);
        ASSERT_IS_NOT_NULL(result2);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        umock_c_reset_all_calls();

        CONSTBUFFER_DecRef(origin);
        CONSTBUFFER_DecRef(result1); /*at this time result 2 has a ref to result1, which has a ref to origin. Nothing is freed*/
        umock_c_reset_all_calls();

        /*note: no expected calls - after the below call all buffers are at ref count == 1*/

        STRICT_EXPECTED_CALL(gballoc_free(origin));
        STRICT_EXPECTED_CALL(gballoc_free(result1));
        STRICT_EXPECTED_CALL(gballoc_free(result2));

        ///act
        CONSTBUFFER_DecRef(result2); /*triggers the release of result1, which triggers the release of origin*/

        ///assert 
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup - nothing left
        
    }

    /* CONSTBUFFER_IncRef */

    /*Tests_SRS_CONSTBUFFER_02_013: [If constbufferHandle is NULL then CONSTBUFFER_IncRef shall return.]*/
    TEST_FUNCTION(CONSTBUFFER_IncRef_with_NULL_returns_NULL)
    {
        ///arrange

        ///act
        CONSTBUFFER_IncRef(NULL);

        ///assert
    }

    /*Tests_SRS_CONSTBUFFER_02_014: [Otherwise, CONSTBUFFER_IncRef shall increment the reference count.]*/
    TEST_FUNCTION(CONSTBUFFER_IncRef_increments_ref_count_1)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle = CONSTBUFFER_Create(BUFFER1_u_char, BUFFER1_length);
        umock_c_reset_all_calls();

        ///act
        CONSTBUFFER_IncRef(handle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        CONSTBUFFER_DecRef(handle);
        CONSTBUFFER_DecRef(handle);
    }

    /*Tests_SRS_CONSTBUFFER_02_014: [Otherwise, CONSTBUFFER_IncRef shall increment the reference count and return constbufferHandle.]*/
    /*Tests_SRS_CONSTBUFFER_02_016: [Otherwise, CONSTBUFFER_DecRef shall decrement the refcount on the constbufferHandle handle.]*/
    /*Tests_SRS_CONSTBUFFER_02_017: [If the refcount reaches zero, then CONSTBUFFER_DecRef shall deallocate all resources used by the CONSTBUFFER_HANDLE.]*/
    TEST_FUNCTION(CONSTBUFFER_IncRef_increments_ref_count_2)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle = CONSTBUFFER_Create(BUFFER1_u_char, BUFFER1_length);
        CONSTBUFFER_IncRef(handle);
        umock_c_reset_all_calls();

        ///act
        CONSTBUFFER_DecRef(handle); /*only a dec_Ref is expected here, so no effects*/

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        CONSTBUFFER_DecRef(handle);
    }

    /*Tests_SRS_CONSTBUFFER_02_014: [Otherwise, CONSTBUFFER_IncRef shall increment the reference count and return constbufferHandle.]*/
    /*Tests_SRS_CONSTBUFFER_02_016: [Otherwise, CONSTBUFFER_DecRef shall decrement the refcount on the constbufferHandle handle.]*/
    /*Tests_SRS_CONSTBUFFER_02_017: [If the refcount reaches zero, then CONSTBUFFER_DecRef shall deallocate all resources used by the CONSTBUFFER_HANDLE.]*/
    TEST_FUNCTION(CONSTBUFFER_IncRef_increments_ref_count_3)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle = CONSTBUFFER_Create(BUFFER1_u_char, BUFFER1_length);
        CONSTBUFFER_IncRef(handle);
        CONSTBUFFER_DecRef(handle); /*only a dec_Ref is expected here, so no effects*/
        umock_c_reset_all_calls();

        ///act
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
        CONSTBUFFER_DecRef(handle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
    }

    /* CONSTBUFFER_DecRef */

    /*Tests_SRS_CONSTBUFFER_02_015: [If constbufferHandle is NULL then CONSTBUFFER_DecRef shall do nothing.]*/
    TEST_FUNCTION(CONSTBUFFER_DecRef_with_NULL_argument_does_nothing)
    {
        ///arrange

        ///act
        CONSTBUFFER_DecRef(NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
    }

    /*Tests_SRS_CONSTBUFFER_02_010: [The non-NULL handle returned by CONSTBUFFER_CreateFromBuffer shall have its ref count set to "1".]*/
    /*Tests_SRS_CONSTBUFFER_02_005: [The non-NULL handle returned by CONSTBUFFER_Create shall have its ref count set to "1".]*/
    /*Tests_SRS_CONSTBUFFER_02_017: [If the refcount reaches zero, then CONSTBUFFER_DecRef shall deallocate all resources used by the CONSTBUFFER_HANDLE.]*/
    TEST_FUNCTION(CONSTBUFFER_CreateFromBuffer_is_ref_counted_1)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle = CONSTBUFFER_CreateFromBuffer(BUFFER1_HANDLE);
        umock_c_reset_all_calls();
        ///act

        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        CONSTBUFFER_DecRef(handle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
    }

    /* Tests_SRS_CONSTBUFFER_01_010: [ The non-NULL handle returned by CONSTBUFFER_CreateWithCustomFree shall have its ref count set to 1. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateWithCustomFree_is_ref_counted_1)
    {
        ///arrange
        unsigned char* test_buffer = (unsigned char*)my_gballoc_malloc(2);
        test_buffer[0] = 42;
        test_buffer[1] = 43;
        CONSTBUFFER_HANDLE handle = CONSTBUFFER_CreateWithCustomFree(test_buffer, 2, free, test_buffer);
        umock_c_reset_all_calls();
        ///act

        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        CONSTBUFFER_DecRef(handle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_CONSTBUFFER_01_009: [ CONSTBUFFER_CreateWithCustomFree shall store customFreeFunc and customFreeFuncContext in order to use them to free the memory when the CONST buffer resources are freed. ]*/
    /* Tests_SRS_CONSTBUFFER_01_012: [ If the buffer was created by calling CONSTBUFFER_CreateWithCustomFree, the customFreeFunc function shall be called to free the memory, while passed customFreeFuncContext as argument. ]*/
    TEST_FUNCTION(CONSTBUFFER_CreateWithCustomFree_with_custom_free_function_calls_the_custom_free_func)
    {
        ///arrange
        unsigned char* test_buffer = (unsigned char*)my_gballoc_malloc(2);
        test_buffer[0] = 42;
        test_buffer[1] = 43;
        CONSTBUFFER_HANDLE handle = CONSTBUFFER_CreateWithCustomFree(test_buffer, 2, test_free_func, (void*)0x4242);
        umock_c_reset_all_calls();
        ///act

        STRICT_EXPECTED_CALL(test_free_func((void*)0x4242));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        CONSTBUFFER_DecRef(handle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        // cleanup
        my_gballoc_free(test_buffer);
    }

    /* Tests_SRS_CONSTBUFFER_01_003: [ The non-NULL handle returned by CONSTBUFFER_CreateWithMoveMemory shall have its ref count set to "1". ]*/
    /*Tests_SRS_CONSTBUFFER_02_017: [If the refcount reaches zero, then CONSTBUFFER_DecRef shall deallocate all resources used by the CONSTBUFFER_HANDLE.]*/
    TEST_FUNCTION(CONSTBUFFER_CreateWithMoveMemory_is_ref_counted_1)
    {
        ///arrange
        CONSTBUFFER_HANDLE handle;
        unsigned char* test_buffer = (unsigned char* )my_gballoc_malloc(2);
        test_buffer[0] = 42;
        test_buffer[1] = 43;
        handle = CONSTBUFFER_CreateWithMoveMemory(test_buffer, 2);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        ///act
        CONSTBUFFER_DecRef(handle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /*Tests_SRS_CONSTBUFFER_02_018: [ If left is NULL and right is NULL then CONSTBUFFER_HANDLE_contain_same shall return true. ]*/
    TEST_FUNCTION(CONSTBUFFER_HANDLE_contain_same_with_left_NULL_and_right_NULL_returns_true)
    {
        ///arrange
        bool result;

        ///act
        result = CONSTBUFFER_HANDLE_contain_same(NULL, NULL);

        ///assert
        ASSERT_IS_TRUE(result);
    }

    /*Tests_SRS_CONSTBUFFER_02_019: [ If left is NULL and right is not NULL then CONSTBUFFER_HANDLE_contain_same shall return false. ]*/
    TEST_FUNCTION(CONSTBUFFER_HANDLE_contain_same_with_left_NULL_and_right_not_NULL_returns_false)
    {
        ///arrange
        bool result;
        unsigned char rightSource = 'r';
        CONSTBUFFER_HANDLE right = CONSTBUFFER_Create(&rightSource, sizeof(rightSource));
        ASSERT_IS_NOT_NULL(right);

        ///act
        result = CONSTBUFFER_HANDLE_contain_same(NULL, right);

        ///assert
        ASSERT_IS_FALSE(result);

        ///clean
        CONSTBUFFER_DecRef(right);
    }

    /*Tests_SRS_CONSTBUFFER_02_020: [ If left is not NULL and right is NULL then CONSTBUFFER_HANDLE_contain_same shall return false. ]*/
    TEST_FUNCTION(CONSTBUFFER_HANDLE_contain_same_with_left_not_NULL_and_right_NULL_returns_false)
    {
        ///arrange
        bool result;
        unsigned char leftSource = 'l';
        CONSTBUFFER_HANDLE left = CONSTBUFFER_Create(&leftSource, sizeof(leftSource));
        ASSERT_IS_NOT_NULL(left);

        ///act
        result = CONSTBUFFER_HANDLE_contain_same(left, NULL);

        ///assert
        ASSERT_IS_FALSE(result);

        ///clean
        CONSTBUFFER_DecRef(left);
    }

    /*Tests_SRS_CONSTBUFFER_02_021: [ If left's size is different than right's size then CONSTBUFFER_HANDLE_contain_same shall return false. ]*/
    TEST_FUNCTION(CONSTBUFFER_HANDLE_contain_same_with_left_and_right_sizes_not_equal_returns_false)
    {
        ///arrange
        bool result;
        unsigned char leftSource = 'l';
        CONSTBUFFER_HANDLE left = CONSTBUFFER_Create(&leftSource, sizeof(leftSource));
        ASSERT_IS_NOT_NULL(left);

        unsigned char rightSource[2] = { 'r', 'r' };
        CONSTBUFFER_HANDLE right = CONSTBUFFER_Create(rightSource, sizeof(rightSource));
        ASSERT_IS_NOT_NULL(right);

        ///act
        result = CONSTBUFFER_HANDLE_contain_same(left, right);

        ///assert
        ASSERT_IS_FALSE(result);

        ///clean
        CONSTBUFFER_DecRef(left);
        CONSTBUFFER_DecRef(right);
    }

    /*Tests_SRS_CONSTBUFFER_02_022: [ If left's buffer is contains different bytes than rights's buffer then CONSTBUFFER_HANDLE_contain_same shall return false. ]*/
    TEST_FUNCTION(CONSTBUFFER_HANDLE_contain_same_with_left_and_right_content_not_equal_returns_false)
    {
        ///arrange
        bool result;
        unsigned char leftSource[2] = { 'l', 'l' };
        CONSTBUFFER_HANDLE left = CONSTBUFFER_Create(leftSource, sizeof(leftSource));
        ASSERT_IS_NOT_NULL(left);

        unsigned char rightSource[2] = { 'r', 'r' };
        CONSTBUFFER_HANDLE right = CONSTBUFFER_Create(rightSource, sizeof(rightSource));
        ASSERT_IS_NOT_NULL(right);

        ///act
        result = CONSTBUFFER_HANDLE_contain_same(left, right);

        ///assert
        ASSERT_IS_FALSE(result);

        ///clean
        CONSTBUFFER_DecRef(left);
        CONSTBUFFER_DecRef(right);
    }

    /*Tests_SRS_CONSTBUFFER_02_023: [ CONSTBUFFER_HANDLE_contain_same shall return true. ]*/
    TEST_FUNCTION(CONSTBUFFER_HANDLE_contain_same_with_left_and_right_same_returns_true)
    {
        ///arrange
        bool result;
        unsigned char leftSource[2] = { '1', '2' };
        CONSTBUFFER_HANDLE left = CONSTBUFFER_Create(leftSource, sizeof(leftSource));
        ASSERT_IS_NOT_NULL(left);

        unsigned char rightSource[2] = { '1', '2' };
        CONSTBUFFER_HANDLE right = CONSTBUFFER_Create(rightSource, sizeof(rightSource));
        ASSERT_IS_NOT_NULL(right);

        ///act
        result = CONSTBUFFER_HANDLE_contain_same(left, right);

        ///assert
        ASSERT_IS_TRUE(result);

        ///clean
        CONSTBUFFER_DecRef(left);
        CONSTBUFFER_DecRef(right);
    }
END_TEST_SUITE(constbuffer_unittests)
