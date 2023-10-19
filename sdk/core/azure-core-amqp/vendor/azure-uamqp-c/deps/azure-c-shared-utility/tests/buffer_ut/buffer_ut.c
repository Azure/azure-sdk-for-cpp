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
#include "umock_c/umock_c.h"
#include "azure_c_shared_utility/buffer_.h"
#include "testrunnerswitcher.h"

static size_t currentmalloc_call = 0;
static size_t whenShallmalloc_fail = 0;

static size_t currentcalloc_call = 0;
static size_t whenShallcalloc_fail = 0;

static size_t currentrealloc_call = 0;
static size_t whenShallrealloc_fail = 0;

void* my_gballoc_malloc(size_t size)
{
    void* result;
    currentmalloc_call++;
    if (whenShallmalloc_fail > 0)
    {
        if (currentmalloc_call == whenShallmalloc_fail)
        {
            result = NULL;
        }
        else
        {
            result = malloc(size);
        }
    }
    else
    {
        result = malloc(size);
    }
    return result;
}

void* my_gballoc_calloc(size_t nmemb, size_t size)
{
    void* result;
    currentcalloc_call++;
    if (whenShallcalloc_fail > 0)
    {
        if (currentcalloc_call == whenShallcalloc_fail)
        {
            result = NULL;
        }
        else
        {
            result = calloc(nmemb, size);
        }
    }
    else
    {
        result = calloc(nmemb, size);
    }
    return result;
}

void* my_gballoc_realloc(void* ptr, size_t size)
{
    void* result;
    currentrealloc_call++;
    if (whenShallrealloc_fail > 0)
    {
        if (currentrealloc_call == whenShallrealloc_fail)
        {
            result = NULL;
        }
        else
        {
            result = realloc(ptr, size);
        }
    }
    else
    {
        result = realloc(ptr, size);
    }

    return result;
}

void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"

#define ALLOCATION_SIZE             16
#define TOTAL_ALLOCATION_SIZE       32

#define BUFFER_TEST1_SIZE             5
#define BUFFER_TEST2_SIZE             6

static const unsigned char BUFFER_Test1[] = {0x01,0x02,0x03,0x04,0x05};
static const unsigned char BUFFER_Test2[] = {0x06,0x07,0x08,0x09,0x10,0x11};
static const unsigned char BUFFER_TEST_VALUE[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10,0x11,0x12,0x13,0x14,0x15,0x16};
static const unsigned char ADDITIONAL_BUFFER[] = {0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26};
static const unsigned char TOTAL_BUFFER[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26};

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(Buffer_UnitTests)

    TEST_SUITE_INITIALIZE(setsBufferTempSize)
    {
        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        umock_c_init(on_umock_c_error);

        REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, my_gballoc_calloc);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
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

        currentmalloc_call = 0;
        whenShallmalloc_fail = 0;

        currentcalloc_call = 0;
        whenShallcalloc_fail = 0;

        currentrealloc_call = 0;
        whenShallrealloc_fail = 0;
    }

    TEST_FUNCTION_CLEANUP(cleans)
    {
        TEST_MUTEX_RELEASE(g_testByTest);
    }

    /* Tests_SRS_BUFFER_07_001: [BUFFER_new shall allocate a BUFFER_HANDLE that will contain a NULL unsigned char*.] */
    TEST_FUNCTION(BUFFER_new_Succeed)
    {
        ///arrange
        BUFFER_HANDLE g_hBuffer;
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        ///act
        g_hBuffer = BUFFER_new();

        ///assert
        ASSERT_IS_NOT_NULL(g_hBuffer);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* BUFFER_delete Tests BEGIN */
    /* Tests_SRS_BUFFER_07_003: [BUFFER_delete shall delete the data associated with the BUFFER_HANDLE.] */
    TEST_FUNCTION(BUFFER_delete_Succeed)
    {
        ///arrange
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument(1);

        ///act
        BUFFER_delete(g_hBuffer);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        //none
    }

    /* Tests_SRS_BUFFER_07_003: [BUFFER_delete shall delete the data associated with the BUFFER_HANDLE.] */
    TEST_FUNCTION(BUFFER_delete_Alloc_Succeed)
    {
        ///arrange
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        ///act
        BUFFER_delete(g_hBuffer);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        //none
    }

    /* Tests_SRS_BUFFER_07_004: [BUFFER_delete shall not delete any BUFFER_HANDLE that is NULL.] */
    TEST_FUNCTION(BUFFER_delete_NULL_HANDLE_Succeed)
    {
        ///arrange

        ///act
        BUFFER_delete(NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* BUFFER_pre_Build Tests BEGIN */
    /* Tests_SRS_BUFFER_07_005: [BUFFER_pre_build allocates size_t bytes of BUFFER_HANDLE and returns zero on success.] */
    TEST_FUNCTION(BUFFER_pre_build_Succeed)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(ALLOCATION_SIZE));

        ///act
        nResult = BUFFER_pre_build(g_hBuffer, ALLOCATION_SIZE);

        ///assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(size_t, BUFFER_length(g_hBuffer), ALLOCATION_SIZE);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* Tests_SRS_BUFFER_07_006: [If handle is NULL or size is 0 then BUFFER_pre_build shall return a nonzero value.] */
    /* Tests_SRS_BUFFER_07_013: [BUFFER_pre_build shall return nonzero if any error is encountered.] */
    TEST_FUNCTION(BUFFER_pre_build_HANDLE_NULL_Fail)
    {
        ///arrange

        ///act
        int nResult = BUFFER_pre_build(NULL, ALLOCATION_SIZE);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        //none
    }

    /* Tests_SRS_BUFFER_07_006: [If handle is NULL or size is 0 then BUFFER_pre_build shall return a nonzero value.] */
    TEST_FUNCTION(BUFFER_pre_Size_Zero_Fail)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        umock_c_reset_all_calls();

        ///act
        nResult = BUFFER_pre_build(g_hBuffer, 0);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* Tests_SRS_BUFFER_07_013: [BUFFER_pre_build shall return nonzero if any error is encountered.] */
    TEST_FUNCTION(BUFFER_pre_build_HANDLE_NULL_Size_Zero_Fail)
    {
        ///arrange

        ///act
        int nResult = BUFFER_pre_build(NULL, 0);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
    }

    /* Tests_SRS_BUFFER_07_007: [BUFFER_pre_build shall return nonzero if the buffer has been previously allocated and is not NULL.] */
    /* Tests_SRS_BUFFER_07_013: [BUFFER_pre_build shall return nonzero if any error is encountered.] */
    TEST_FUNCTION(BUFFER_pre_build_Multiple_Alloc_Fail)
    {
        ///arrange
        BUFFER_HANDLE g_hBuffer;
        int nResult;
        g_hBuffer = BUFFER_new();
        nResult = BUFFER_pre_build(g_hBuffer, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        ///act
        nResult = BUFFER_pre_build(g_hBuffer, ALLOCATION_SIZE);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* Tests_SRS_BUFFER_07_008: [BUFFER_build allocates size_t bytes, copies the unsigned char* into the buffer and returns zero on success.] */
    TEST_FUNCTION(BUFFER_build_Succeed)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE g_hBuffer;

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, ALLOCATION_SIZE))
            .IgnoreArgument(1);

        ///act
        g_hBuffer = BUFFER_new();

        nResult = BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);

        ///assert
        ASSERT_ARE_EQUAL(size_t, BUFFER_length(g_hBuffer), ALLOCATION_SIZE);
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER_u_char(g_hBuffer), BUFFER_TEST_VALUE, ALLOCATION_SIZE));
        ASSERT_ARE_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* Tests_SRS_BUFFER_07_009: [BUFFER_build shall return nonzero if handle is NULL ] */
    TEST_FUNCTION(BUFFER_build_NULL_HANDLE_Fail)
    {
        ///arrange

        ///act
        int nResult = BUFFER_build(NULL, BUFFER_TEST_VALUE, ALLOCATION_SIZE);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
    }

    /* Tests_SRS_BUFFER_01_001: [If size is positive and source is NULL, BUFFER_build shall return nonzero] */
    TEST_FUNCTION(BUFFER_build_Content_NULL_Fail)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        umock_c_reset_all_calls();

        ///act
        nResult = BUFFER_build(g_hBuffer, NULL, ALLOCATION_SIZE);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* Tests_SRS_BUFFER_01_002: [The size argument can be zero, in which case the underlying buffer held by the buffer instance shall be freed.] */
    TEST_FUNCTION(BUFFER_build_Size_Zero_non_NULL_buffer_Succeeds)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument(1);

        ///act
        nResult = BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, 0);

        ///assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* Tests_SRS_BUFFER_01_002: [The size argument can be zero, in which case the underlying buffer held by the buffer instance shall be freed.] */
    TEST_FUNCTION(BUFFER_build_Size_Zero_NULL_buffer_Succeeds)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument(1);

        ///act
        nResult = BUFFER_build(g_hBuffer, NULL, 0);

        ///assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* Tests_SRS_BUFFER_07_029: [ BUFFER_append_build shall return nonzero if handle or source are NULL or if size is 0. ] */
    TEST_FUNCTION(BUFFER_append_build_handle_NULL_fail)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE hBuffer;
        hBuffer = BUFFER_new();
        umock_c_reset_all_calls();

        ///act
        nResult = BUFFER_append_build(NULL, BUFFER_Test1, BUFFER_TEST1_SIZE);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(hBuffer);
    }

    /* Tests_SRS_BUFFER_07_029: [ BUFFER_append_build shall return nonzero if handle or source are NULL or if size is 0. ] */
    TEST_FUNCTION(BUFFER_append_build_buffer_NULL_buffer_fail)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE hBuffer;
        hBuffer = BUFFER_new();
        umock_c_reset_all_calls();

        ///act
        nResult = BUFFER_append_build(hBuffer, NULL, BUFFER_TEST1_SIZE);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(hBuffer);
    }

    /* Tests_SRS_BUFFER_07_029: [ BUFFER_append_build shall return nonzero if handle or source are NULL or if size is 0. ] */
    TEST_FUNCTION(BUFFER_append_build_Size_Zero_NULL_buffer_fail)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE hBuffer;
        hBuffer = BUFFER_new();
        umock_c_reset_all_calls();

        ///act
        nResult = BUFFER_append_build(hBuffer, BUFFER_Test1, 0);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(hBuffer);
    }

    /* Tests_SRS_BUFFER_07_030: [ if handle->buffer is NULL BUFFER_append_build shall allocate the a buffer of size bytes... ] */
    /* Tests_SRS_BUFFER_07_031: [ ... and copy the contents of source to handle->buffer. ] */
    /* Tests_SRS_BUFFER_07_034: [ On success BUFFER_append_build shall return 0 ] */
    TEST_FUNCTION(BUFFER_append_build_buffer_NULL_succeed)
    {
        //arrange
        int nResult;
        BUFFER_HANDLE hBuffer;
        hBuffer = BUFFER_new();
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

        //act
        nResult = BUFFER_append_build(hBuffer, BUFFER_Test1, BUFFER_TEST1_SIZE);

        //assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER_u_char(hBuffer), BUFFER_Test1, BUFFER_TEST1_SIZE));
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        BUFFER_delete(hBuffer);
    }

    /* Tests_SRS_BUFFER_07_035: [ If any error is encountered BUFFER_append_build shall return a non-null value. ] */
    TEST_FUNCTION(BUFFER_append_build_buffer_NULL_fail)
    {
        //arrange
        int nResult;
        BUFFER_HANDLE hBuffer;
        hBuffer = BUFFER_new();
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);

        //act
        nResult = BUFFER_append_build(hBuffer, BUFFER_Test1, BUFFER_TEST1_SIZE);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        BUFFER_delete(hBuffer);
    }

    /* Tests_SRS_BUFFER_07_032: [ if handle->buffer is not NULL BUFFER_append_build shall realloc the buffer to be the handle->size + size ] */
    /* Tests_SRS_BUFFER_07_033: [ ... and copy the contents of source to the end of the buffer. ] */
    /* Tests_SRS_BUFFER_07_034: [ On success BUFFER_append_build shall return 0 ] */
    TEST_FUNCTION(BUFFER_append_build_succeed)
    {
        //arrange
        int nResult;
        BUFFER_HANDLE hBuffer;
        hBuffer = BUFFER_create(BUFFER_TEST_VALUE, ALLOCATION_SIZE);

        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

        //act
        nResult = BUFFER_append_build(hBuffer, ADDITIONAL_BUFFER, ALLOCATION_SIZE);

        //assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(size_t, BUFFER_length(hBuffer), TOTAL_ALLOCATION_SIZE);

        ASSERT_ARE_EQUAL(int, memcmp(BUFFER_u_char(hBuffer), TOTAL_BUFFER, TOTAL_ALLOCATION_SIZE), 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        BUFFER_delete(hBuffer);
    }

    /* Tests_SRS_BUFFER_07_035: [ If any error is encountered BUFFER_append_build shall return a non-null value. ] */
    TEST_FUNCTION(BUFFER_append_build_fail)
    {
        //arrange
        int nResult;
        BUFFER_HANDLE hBuffer;
        hBuffer = BUFFER_create(BUFFER_TEST_VALUE, ALLOCATION_SIZE);

        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(NULL);

        //act
        nResult = BUFFER_append_build(hBuffer, ADDITIONAL_BUFFER, ALLOCATION_SIZE);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        BUFFER_delete(hBuffer);
    }

    /* Tests_SRS_BUFFER_07_011: [BUFFER_build shall overwrite previous contents if the buffer has been previously allocated.] */
    TEST_FUNCTION(BUFFER_build_when_the_buffer_is_already_allocated_and_the_same_amount_of_bytes_is_needed_succeeds)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        nResult = BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, ALLOCATION_SIZE))
            .IgnoreArgument(1);

        ///act
        nResult = BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);

        ///assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* Tests_SRS_BUFFER_07_011: [BUFFER_build shall overwrite previous contents if the buffer has been previously allocated.] */
    TEST_FUNCTION(BUFFER_build_when_the_buffer_is_already_allocated_and_more_bytes_are_needed_succeeds)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        nResult = BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE - 1);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, ALLOCATION_SIZE))
            .IgnoreArgument(1);

        ///act
        nResult = BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);

        ///assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* Tests_SRS_BUFFER_07_011: [BUFFER_build shall overwrite previous contents if the buffer has been previously allocated.] */
    TEST_FUNCTION(BUFFER_build_when_the_buffer_is_already_allocated_and_less_bytes_are_needed_succeeds)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        nResult = BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, ALLOCATION_SIZE - 1))
            .IgnoreArgument(1);

        ///act
        nResult = BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE - 1);

        ///assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* BUFFER_unbuild Tests BEGIN */
    /* Tests_SRS_BUFFER_07_012: [BUFFER_unbuild shall clear the underlying unsigned char* data associated with the BUFFER_HANDLE this will return zero on success.] */
    TEST_FUNCTION(BUFFER_unbuild_Succeed)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        nResult = BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument(1);

        ///act
        nResult = BUFFER_unbuild(g_hBuffer);

        ///assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* Tests_SRS_BUFFER_07_014: [BUFFER_unbuild shall return a nonzero value if BUFFER_HANDLE is NULL.] */
    TEST_FUNCTION(BUFFER_unbuild_HANDLE_NULL_Fail)
    {
        ///arrange

        ///act
        int nResult = BUFFER_unbuild(NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
    }

    /* Codes_SRS_BUFFER_07_015: [BUFFER_unbuild shall always return success if the unsigned char* referenced by BUFFER_HANDLE is NULL.] */
    TEST_FUNCTION(BUFFER_unbuild_Multiple_Alloc_Fail)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        nResult = BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);
        nResult = BUFFER_unbuild(g_hBuffer);
        umock_c_reset_all_calls();

        ///act
        nResult = BUFFER_unbuild(g_hBuffer);

        ///assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* BUFFER_enlarge Tests BEGIN */
    /* Tests_SRS_BUFFER_07_016: [BUFFER_enlarge shall increase the size of the unsigned char* referenced by BUFFER_HANDLE.] */
    TEST_FUNCTION(BUFFER_enlarge_Succeed)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        nResult = BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, 2 * ALLOCATION_SIZE))
            .IgnoreArgument(1);

        ///act
        nResult = BUFFER_enlarge(g_hBuffer, ALLOCATION_SIZE);

        ///assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(size_t, BUFFER_length(g_hBuffer), TOTAL_ALLOCATION_SIZE);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* Tests_SRS_BUFFER_07_017: [BUFFER_enlarge shall return a nonzero result if any parameters are NULL or zero.] */
    /* Tests_SRS_BUFFER_07_018: [BUFFER_enlarge shall return a nonzero result if any error is encountered.] */
    TEST_FUNCTION(BUFFER_enlarge_NULL_HANDLE_Fail)
    {
        ///arrange

        ///act
        int nResult = BUFFER_enlarge(NULL, ALLOCATION_SIZE);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
    }

    /* Tests_SRS_BUFFER_07_036: [ if handle is NULL, BUFFER_shrink shall return a non-null value ]*/
    TEST_FUNCTION(BUFFER_shrink_handle_NULL_fail)
    {
        //arrange
        int result;
        //act
        result = BUFFER_shrink(NULL, ALLOCATION_SIZE, true);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, result, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    /* Tests_SRS_BUFFER_07_037: [ If decreaseSize is equal zero, BUFFER_shrink shall return a non-null value ] */
    TEST_FUNCTION(BUFFER_shrink_decrease_size_0_fail)
    {
        //arrange
        int nResult;
        BUFFER_HANDLE hBuffer;
        hBuffer = BUFFER_new();
        nResult = BUFFER_build(hBuffer, TOTAL_BUFFER, TOTAL_ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        //act
        nResult = BUFFER_shrink(hBuffer, 0, true);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER_u_char(hBuffer), TOTAL_BUFFER, TOTAL_ALLOCATION_SIZE));
        ASSERT_ARE_EQUAL(size_t, BUFFER_length(hBuffer), TOTAL_ALLOCATION_SIZE);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        BUFFER_delete(hBuffer);
    }

    /* Tests_SRS_BUFFER_07_038: [ If decreaseSize is less than the size of the buffer, BUFFER_shrink shall return a non-null value ] */
    TEST_FUNCTION(BUFFER_shrink_decrease_size_less_than_len_succeed)
    {
        //arrange
        int nResult;
        BUFFER_HANDLE hBuffer;
        hBuffer = BUFFER_new();
        nResult = BUFFER_build(hBuffer, TOTAL_BUFFER, TOTAL_ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        //act
        nResult = BUFFER_shrink(hBuffer, TOTAL_ALLOCATION_SIZE+1, true);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER_u_char(hBuffer), TOTAL_BUFFER, TOTAL_ALLOCATION_SIZE));
        ASSERT_ARE_EQUAL(size_t, BUFFER_length(hBuffer), TOTAL_ALLOCATION_SIZE);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        BUFFER_delete(hBuffer);
    }

    /* Tests_SRS_BUFFER_07_042: [ If a failure is encountered, BUFFER_shrink shall return a non-null value ] */
    TEST_FUNCTION(BUFFER_shrink_malloc_fail)
    {
        //arrange
        int nResult;
        BUFFER_HANDLE hBuffer;
        hBuffer = BUFFER_new();
        nResult = BUFFER_build(hBuffer, TOTAL_BUFFER, TOTAL_ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(TOTAL_ALLOCATION_SIZE - ALLOCATION_SIZE)).SetReturn(NULL);

        //act
        nResult = BUFFER_shrink(hBuffer, ALLOCATION_SIZE, true);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER_u_char(hBuffer), TOTAL_BUFFER, TOTAL_ALLOCATION_SIZE));
        ASSERT_ARE_EQUAL(size_t, BUFFER_length(hBuffer), TOTAL_ALLOCATION_SIZE);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        BUFFER_delete(hBuffer);
    }

    /* Tests_SRS_BUFFER_07_039: [ BUFFER_shrink shall allocate a temporary buffer of existing buffer size minus decreaseSize. ] */
    /* Tests_SRS_BUFFER_07_040: [ if the fromEnd variable is true, BUFFER_shrink shall remove the end of the buffer of size decreaseSize. ] */
    TEST_FUNCTION(BUFFER_shrink_from_end_succeed)
    {
        //arrange
        int nResult;
        BUFFER_HANDLE hBuffer;
        hBuffer = BUFFER_new();
        nResult = BUFFER_build(hBuffer, TOTAL_BUFFER, TOTAL_ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(TOTAL_ALLOCATION_SIZE-ALLOCATION_SIZE));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        //act
        nResult = BUFFER_shrink(hBuffer, ALLOCATION_SIZE, true);

        //assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER_u_char(hBuffer), BUFFER_Test1, BUFFER_TEST1_SIZE));
        ASSERT_ARE_EQUAL(size_t, BUFFER_length(hBuffer), ALLOCATION_SIZE);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        BUFFER_delete(hBuffer);
    }

    /* Tests_SRS_BUFFER_07_039: [ BUFFER_shrink shall allocate a temporary buffer of existing buffer size minus decreaseSize. ] */
    /* Tests_SRS_BUFFER_07_040: [ if the fromEnd variable is true, BUFFER_shrink shall remove the end of the buffer of size decreaseSize. ] */
    /* Tests_SRS_BUFFER_07_043: [ If the decreaseSize is equal the buffer size , BUFFER_shrink shall deallocate the buffer and set the size to zero. ] */
    TEST_FUNCTION(BUFFER_shrink_all_buffer_succeed)
    {
        //arrange
        int nResult;
        BUFFER_HANDLE hBuffer;
        hBuffer = BUFFER_new();
        nResult = BUFFER_build(hBuffer, TOTAL_BUFFER, TOTAL_ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        //act
        nResult = BUFFER_shrink(hBuffer, TOTAL_ALLOCATION_SIZE, true);

        //assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        ASSERT_IS_NULL(BUFFER_u_char(hBuffer));
        ASSERT_ARE_EQUAL(size_t, BUFFER_length(hBuffer), 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        BUFFER_delete(hBuffer);
    }

    /* Tests_SRS_BUFFER_07_039: [ BUFFER_shrink shall allocate a temporary buffer of existing buffer size minus decreaseSize. ] */
    /* Tests_SRS_BUFFER_07_041: [ if the fromEnd variable is false, BUFFER_shrink shall remove the beginning of the buffer of size decreaseSize. ] */
    TEST_FUNCTION(BUFFER_shrink_from_beginning_succeed)
    {
        const unsigned char TEST_TOTAL_BUFFER[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26 };

        //arrange
        int nResult;
        BUFFER_HANDLE hBuffer;
        hBuffer = BUFFER_new();
        nResult = BUFFER_build(hBuffer, TEST_TOTAL_BUFFER, TOTAL_ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(TOTAL_ALLOCATION_SIZE-ALLOCATION_SIZE));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        //act
        nResult = BUFFER_shrink(hBuffer, ALLOCATION_SIZE, false);

        //assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER_u_char(hBuffer), ADDITIONAL_BUFFER, ALLOCATION_SIZE));
        ASSERT_ARE_EQUAL(size_t, BUFFER_length(hBuffer), ALLOCATION_SIZE);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        BUFFER_delete(hBuffer);
    }


    /* Tests_SRS_BUFFER_07_017: [BUFFER_enlarge shall return a nonzero result if any parameters are NULL or zero.] */
    /* Tests_SRS_BUFFER_07_018: [BUFFER_enlarge shall return a nonzero result if any error is encountered.] */
    TEST_FUNCTION(BUFFER_enlarge_Size_Zero_Fail)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        nResult = BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        ///act
        nResult = BUFFER_enlarge(g_hBuffer, 0);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* BUFFER_content Tests BEGIN */
    /* Tests_SRS_BUFFER_07_019: [BUFFER_content shall return the data contained within the BUFFER_HANDLE.] */
    TEST_FUNCTION(BUFFER_content_Succeed)
    {
        ///arrange
        int nResult;
        const unsigned char* content;
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        nResult = BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        ///act
        content = NULL;
        nResult = BUFFER_content(g_hBuffer, &content);

        ///assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(int, 0, memcmp(content, BUFFER_TEST_VALUE, ALLOCATION_SIZE));
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* Tests_SRS_BUFFER_07_020: [If the handle and/or content*is NULL BUFFER_content shall return nonzero.] */
    TEST_FUNCTION(BUFFER_content_HANDLE_NULL_Fail)
    {
        ///arrange

        ///act
        const unsigned char* content = NULL;
        int nResult = BUFFER_content(NULL, &content);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_IS_NULL(content);

        ///cleanup

    }

    /* Tests_SRS_BUFFER_07_020: [If the handle and/or content*is NULL BUFFER_content shall return nonzero.] */
    TEST_FUNCTION(BUFFER_content_Char_NULL_Fail)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        nResult = BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        ///act
        nResult = BUFFER_content(g_hBuffer, NULL);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* BUFFER_size Tests BEGIN */
    /* Tests_SRS_BUFFER_07_021: [BUFFER_size shall place the size of the associated buffer in the size variable and return zero on success.] */
    TEST_FUNCTION(BUFFER_size_Succeed)
    {
        ///arrange
        int nResult;
        size_t size;
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        nResult = BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        ///act
        size = 0;
        nResult = BUFFER_size(g_hBuffer, &size);

        ///assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(size_t, size, ALLOCATION_SIZE);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());


        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* Tests_SRS_BUFFER_07_022: [BUFFER_size shall return a nonzero value for any error that is encountered.] */
    TEST_FUNCTION(BUFFER_size_HANDLE_NULL_Fail)
    {
        ///arrange

        ///act
        size_t size = 0;
        int nResult = BUFFER_size(NULL, &size);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
    }

    /* Tests_SRS_BUFFER_07_022: [BUFFER_size shall return a nonzero value for any error that is encountered.] */
    TEST_FUNCTION(BUFFER_size_Size_t_NULL_Fail)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        nResult = BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        ///act
        nResult = BUFFER_size(g_hBuffer, NULL);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* BUFFER_append Tests BEGIN */
    /* Tests_SRS_BUFFER_07_024: [BUFFER_append concatenates b2 onto b1 without modifying b2 and shall return zero on success.] */
    TEST_FUNCTION(BUFFER_append_Succeed)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE hAppend;
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        nResult = BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);
        hAppend = BUFFER_new();
        nResult = BUFFER_build(hAppend, ADDITIONAL_BUFFER, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, ALLOCATION_SIZE + ALLOCATION_SIZE))
            .IgnoreArgument(1);

        ///act
        nResult = BUFFER_append(g_hBuffer, hAppend);

        ///assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER_u_char(g_hBuffer), TOTAL_BUFFER, TOTAL_ALLOCATION_SIZE));
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER_u_char(hAppend), ADDITIONAL_BUFFER, ALLOCATION_SIZE));
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(hAppend);
        BUFFER_delete(g_hBuffer);
    }

    /* Tests_SRS_BUFFER_07_023: [BUFFER_append shall return a nonzero upon any error that is encountered.] */
    TEST_FUNCTION(BUFFER_append_HANDLE_NULL_Fail)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE hAppend = BUFFER_new();
        nResult = BUFFER_build(hAppend, ADDITIONAL_BUFFER, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        ///act
        nResult = BUFFER_append(NULL, hAppend);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(hAppend);
    }

    /* Tests_SRS_BUFFER_07_023: [BUFFER_append shall return a nonzero upon any error that is encountered.] */
    TEST_FUNCTION(BUFFER_append_APPEND_HANDLE_NULL_Fail)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        nResult = BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        ///act
        nResult = BUFFER_append(g_hBuffer, NULL);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* Tests_SRS_BUFFER_07_024: [BUFFER_append concatenates b2 onto b1 without modifying b2 and shall return zero on success.] */
    TEST_FUNCTION(BUFFER_append_HANDLE2_SIZE_ZERO_SUCCEED)
    {
        ///arrange
        int nResult;
        size_t howBig;
        BUFFER_HANDLE handle1 = BUFFER_create(BUFFER_Test1, BUFFER_TEST1_SIZE);
        BUFFER_HANDLE handle2 = BUFFER_create(BUFFER_Test2, 0);
        // umock_c_reset_all_calls();

        ///act
        nResult = BUFFER_append(handle1, handle2);

        ///assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        howBig = BUFFER_length(handle1);
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER_u_char(handle1), BUFFER_Test1, BUFFER_TEST1_SIZE));
        ASSERT_ARE_EQUAL(size_t, BUFFER_TEST1_SIZE, howBig);

        //ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(handle1);
        BUFFER_delete(handle2);
    }

    /* Tests_SRS_BUFFER_07_024: [BUFFER_append concatenates b2 onto b1 without modifying b2 and shall return zero on success.] */
    TEST_FUNCTION(BUFFER_append_HANDLE1_SIZE_ZERO_SUCCEED)
    {
        ///arrange
        int nResult;
        size_t howBig;
        BUFFER_HANDLE handle1 = BUFFER_create(BUFFER_Test1, 0);
        BUFFER_HANDLE handle2 = BUFFER_create(BUFFER_Test2, BUFFER_TEST2_SIZE);
        // umock_c_reset_all_calls();

        ///act
        nResult = BUFFER_append(handle1, handle2);

        ///assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        howBig = BUFFER_length(handle1);
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER_u_char(handle1), BUFFER_Test2, BUFFER_TEST2_SIZE));
        ASSERT_ARE_EQUAL(size_t, BUFFER_TEST2_SIZE, howBig);

        //ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(handle1);
        BUFFER_delete(handle2);
    }

    /* Tests_SRS_BUFFER_07_024: [BUFFER_append concatenates b2 onto b1 without modifying b2 and shall return zero on success.] */
    TEST_FUNCTION(BUFFER_prepend_HANDLE1_SIZE_ZERO_SUCCEED)
    {
        ///arrange
        int nResult;
        size_t howBig;
        BUFFER_HANDLE handle1 = BUFFER_create(BUFFER_Test1, 0);
        BUFFER_HANDLE handle2 = BUFFER_create(BUFFER_Test2, BUFFER_TEST2_SIZE);
        // umock_c_reset_all_calls();

        ///act
        nResult = BUFFER_prepend(handle1, handle2);

        ///assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        howBig = BUFFER_length(handle1);
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER_u_char(handle1), BUFFER_Test2, BUFFER_TEST2_SIZE));
        ASSERT_ARE_EQUAL(size_t, BUFFER_TEST2_SIZE, howBig);

        //ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(handle1);
        BUFFER_delete(handle2);
    }

    /* Tests_SRS_BUFFER_07_024: [BUFFER_append concatenates b2 onto b1 without modifying b2 and shall return zero on success.] */
    TEST_FUNCTION(BUFFER_prepend_HANDLE2_SIZE_ZERO_SUCCEED)
    {
        ///arrange
        int nResult;
        size_t howBig;
        BUFFER_HANDLE handle1 = BUFFER_create(BUFFER_Test1, BUFFER_TEST1_SIZE);
        BUFFER_HANDLE handle2 = BUFFER_create(BUFFER_Test2, 0);
        // umock_c_reset_all_calls();

        ///act
        nResult = BUFFER_prepend(handle1, handle2);

        ///assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        howBig = BUFFER_length(handle1);
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER_u_char(handle1), BUFFER_Test1, BUFFER_TEST1_SIZE));
        ASSERT_ARE_EQUAL(size_t, BUFFER_TEST1_SIZE, howBig);

        //ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(handle1);
        BUFFER_delete(handle2);
    }


    /* Tests_SRS_BUFFER_01_005: [ BUFFER_prepend shall return a non-zero upon value any error that is encountered. ]*/
    TEST_FUNCTION(BUFFER_prepend_APPEND_HANDLE1_NULL_Fail)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE g_hBuffer;
        g_hBuffer = BUFFER_new();
        nResult = BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        ///act
        nResult = BUFFER_prepend(g_hBuffer, NULL);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* Tests_SRS_BUFFER_01_005: [ BUFFER_prepend shall return a non-zero upon value any error that is encountered. ]*/
    TEST_FUNCTION(BUFFER_prepend_APPEND_HANDLE2_NULL_Fail)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE hAppend = BUFFER_new();
        nResult = BUFFER_build(hAppend, ADDITIONAL_BUFFER, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        ///act
        nResult = BUFFER_prepend(NULL, hAppend);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(hAppend);
    }

    /* Tests_SRS_BUFFER_07_024: [BUFFER_append concatenates b2 onto b1 without modifying b2 and shall return zero on success.] */
    TEST_FUNCTION(BUFFER_prepend_Succeed)
    {
        ///arrange
        int nResult;
        BUFFER_HANDLE g_hBuffer;
        BUFFER_HANDLE hAppend;
        g_hBuffer = BUFFER_new();
        nResult = BUFFER_build(g_hBuffer, ADDITIONAL_BUFFER, ALLOCATION_SIZE);
        hAppend = BUFFER_new();
        nResult = BUFFER_build(hAppend, BUFFER_TEST_VALUE, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(ALLOCATION_SIZE + ALLOCATION_SIZE));
        EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        ///act
        nResult = BUFFER_prepend(g_hBuffer, hAppend);

        ///assert
        ASSERT_ARE_EQUAL(int, nResult, 0);
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER_u_char(g_hBuffer), TOTAL_BUFFER, TOTAL_ALLOCATION_SIZE));
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER_u_char(hAppend), BUFFER_TEST_VALUE, ALLOCATION_SIZE));

        //TOTAL_BUFFER
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(hAppend);
        BUFFER_delete(g_hBuffer);
    }

    /* BUFFER_u_char */

    /* Tests_SRS_BUFFER_07_025: [BUFFER_u_char shall return a pointer to the underlying unsigned char*.] */
    TEST_FUNCTION(BUFFER_U_CHAR_Succeed)
    {
        ///arrange
        BUFFER_HANDLE g_hBuffer;
        unsigned char* u;
        g_hBuffer = BUFFER_new();
        (void)BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        ///act
        u = BUFFER_u_char(g_hBuffer);

        ///assert
        ASSERT_ARE_EQUAL(int, 0, memcmp(u, BUFFER_TEST_VALUE, ALLOCATION_SIZE) );
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* Tests_SRS_BUFFER_07_026: [BUFFER_u_char shall return NULL for any error that is encountered.] */
    TEST_FUNCTION(BUFFER_U_CHAR_HANDLE_NULL_Fail)
    {
        ///arrange

        ///act
        ASSERT_IS_NULL(BUFFER_u_char(NULL));

        /// assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_BUFFER_07_029: [BUFFER_u_char shall return NULL if underlying buffer size is zero.] */
    TEST_FUNCTION(BUFFER_U_CHAR_HANDLE_SIZE_ZERO_Fail)
    {
        ///arrange
        unsigned char c = 'c';
        BUFFER_HANDLE g_hBuffer = BUFFER_create(&c, 0);
        umock_c_reset_all_calls();

        ///act
        ASSERT_IS_NULL(BUFFER_u_char(g_hBuffer));

        /// assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        /// cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* BUFFER_length */

    /* Tests_SRS_BUFFER_07_027: [BUFFER_length shall return the size of the underlying buffer.] */
    TEST_FUNCTION(BUFFER_length_Succeed)
    {
        ///arrange
        BUFFER_HANDLE g_hBuffer;
        size_t l;
        g_hBuffer = BUFFER_new();
        (void)BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        ///act
        l = BUFFER_length(g_hBuffer);

        ///assert
        ASSERT_ARE_EQUAL(size_t, l, ALLOCATION_SIZE);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
    }

    /* Tests_SRS_BUFFER_07_028: [BUFFER_length shall return zero for any error that is encountered.] */
    TEST_FUNCTION(BUFFER_length_HANDLE_NULL_Succeed)
    {
        ///arrange

        ///act
        size_t size = BUFFER_length(NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(size_t, size, 0);
    }

    TEST_FUNCTION(BUFFER_Clone_Succeed)
    {
        ///arrange
        BUFFER_HANDLE g_hBuffer;
        BUFFER_HANDLE hclone;
        g_hBuffer = BUFFER_new();
        (void)BUFFER_build(g_hBuffer, BUFFER_TEST_VALUE, ALLOCATION_SIZE);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).IgnoreAllArguments();
        STRICT_EXPECTED_CALL(gballoc_malloc(ALLOCATION_SIZE));

        ///act
        hclone = BUFFER_clone(g_hBuffer);

        ///assert
        ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER_u_char(hclone), BUFFER_TEST_VALUE, ALLOCATION_SIZE) );
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(g_hBuffer);
        BUFFER_delete(hclone);
    }

    TEST_FUNCTION(BUFFER_Clone_HANDLE_NULL_Fail)
    {
        ///arrange

        ///act
        BUFFER_HANDLE result = BUFFER_clone(NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(result);
    }

    /*Tests_SRS_BUFFER_02_001: [If source is NULL then BUFFER_create shall return NULL.] */
    TEST_FUNCTION(BUFFER_create_with_NULL_source_fails)
    {
        ///arrange

        ///act
        BUFFER_HANDLE res = BUFFER_create(NULL, 0);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(res);

        ///cleanup
    }

    /*Tests_SRS_BUFFER_02_002: [Otherwise, BUFFER_create shall allocate memory to hold size bytes and shall copy from source size bytes into the newly allocated memory.] */
    /*Tests_SRS_BUFFER_02_004: [Otherwise, BUFFER_create shall return a non-NULL handle*/
    TEST_FUNCTION(BUFFER_create_happy_path)
    {
        ///arrange
        BUFFER_HANDLE res;
        const unsigned char* data;
        size_t howBig;
        char c = '3';

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        STRICT_EXPECTED_CALL(gballoc_malloc(1));

        ///act
        res = BUFFER_create((const unsigned char*)&c, 1);

        ///assert
        ASSERT_IS_NOT_NULL(res);
        howBig = BUFFER_length(res);
        ASSERT_ARE_EQUAL(size_t, 1, howBig);
        data = BUFFER_u_char(res);
        ASSERT_ARE_EQUAL(uint8_t, '3', data[0]);

        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(res);
    }

    /*Tests_SRS_BUFFER_02_002: [Otherwise, BUFFER_create shall allocate memory to hold size bytes and shall copy from source size bytes into the newly allocated memory.] */
    /* Tests_SRS_BUFFER_02_005: [If size parameter is 0 then 1 byte of memory shall be allocated yet size of the buffer shall be set to 0.]*/
    TEST_FUNCTION(BUFFER_create_ZERO_SIZE_SUCCEED)
    {
        ///arrange
        size_t howBig;
        BUFFER_HANDLE res;
        const unsigned char* data;
        char c = '3';

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        STRICT_EXPECTED_CALL(gballoc_malloc(1));

        ///act
        res = BUFFER_create((const unsigned char*)&c, 0);
        ///assert
        ASSERT_IS_NOT_NULL(res);
        howBig = BUFFER_length(res);
        data = BUFFER_u_char(res);
        ASSERT_IS_NULL(data);
        ASSERT_ARE_EQUAL(size_t, 0, howBig);

        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(res);
    }

    /*Tests_SRS_BUFFER_02_003: [If allocating memory fails, then BUFFER_create shall return NULL.] */
    TEST_FUNCTION(BUFFER_create_fails_when_gballoc_fails_1)
    {
        ///arrange
        char c = '3';
        BUFFER_HANDLE res;

        whenShallmalloc_fail = 1;
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(gballoc_malloc(1));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        ///act
        res = BUFFER_create((const unsigned char*)&c, 1);

        ///assert
        ASSERT_IS_NULL(res);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(res);
    }

    /*Tests_SRS_BUFFER_02_003: [If allocating memory fails, then BUFFER_create shall return NULL.] */
    TEST_FUNCTION(BUFFER_create_fails_when_gballoc_fails_2)
    {
        ///arrange
        char c = '3';
        BUFFER_HANDLE res;

        whenShallcalloc_fail = 1;
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        ///act
        res = BUFFER_create((const unsigned char*)&c, 1);

        ///assert
        ASSERT_IS_NULL(res);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        BUFFER_delete(res);
    }

    // Tests_SRS_BUFFER_07_029: [ BUFFER_create_with_size shall create a BUFFER_HANDLE with a pre allocated underlying buffer size.]
    // Tests_SRS_BUFFER_07_031: [ BUFFER_create_with_size shall allocate a buffer of buff_size. ]
    // Tests_SRS_BUFFER_07_033: [ Otherwise, BUFFER_create_with_size shall return a non-NULL handle. ]
    TEST_FUNCTION(BUFFER_create_with_size_succeeds)
    {
        //arrange
        BUFFER_HANDLE res;
        size_t alloc_size = 32;

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        STRICT_EXPECTED_CALL(gballoc_malloc(alloc_size));

        //act
        res = BUFFER_create_with_size(alloc_size);

        //assert
        ASSERT_IS_NOT_NULL(res);
        ASSERT_ARE_EQUAL(size_t, alloc_size, BUFFER_length(res));
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        BUFFER_delete(res);
    }

    // Tests_SRS_BUFFER_07_030: [ If buff_size is 0 BUFFER_create_with_size shall create a valid non-NULL handle of zero size. ]
    TEST_FUNCTION(BUFFER_create_with_size_size_zero_succeeds)
    {
        //arrange
        BUFFER_HANDLE res;
        size_t alloc_size = 0;

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();

        //act
        res = BUFFER_create_with_size(alloc_size);

        //assert
        ASSERT_IS_NOT_NULL(res);
        ASSERT_ARE_EQUAL(size_t, alloc_size, BUFFER_length(res));
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        BUFFER_delete(res);
    }

    // Tests_SRS_BUFFER_07_032: [ If allocating memory fails, then BUFFER_create_with_size shall return NULL. ]
    TEST_FUNCTION(BUFFER_create_with_size_malloc_fails)
    {
        //arrange
        BUFFER_HANDLE res;
        size_t alloc_size = 32;

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments()
            .SetReturn(NULL);

        //act
        res = BUFFER_create_with_size(alloc_size);

        //assert
        ASSERT_IS_NULL(res);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    // Tests_SRS_BUFFER_07_031: [ BUFFER_create_with_size shall allocate a buffer of buff_size. ]
    TEST_FUNCTION(BUFFER_create_with_size_2nd_malloc_fails)
    {
        //arrange
        BUFFER_HANDLE res;
        size_t alloc_size = 32;

        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreAllArguments();
        STRICT_EXPECTED_CALL(gballoc_malloc(alloc_size)).SetReturn(NULL);
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        //act
        res = BUFFER_create_with_size(alloc_size);

        //assert
        ASSERT_IS_NULL(res);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    /* BUFFER_fill */

    /* Tests_SRS_BUFFER_07_001: [ BUFFER_fill shall fill the supplied BUFFER_HANDLE with the supplied fill character. ] */
    TEST_FUNCTION(BUFFER_fill_succeed)
    {
        int result;
        char* expected;
        char* actual;

        //arrange
        BUFFER_HANDLE buffer = BUFFER_new();
        const unsigned char RESULT_BUFFER[] = { '@', '@', '@', '@', '@' };
        (void)BUFFER_build(buffer, BUFFER_Test1, BUFFER_TEST1_SIZE);
        umock_c_reset_all_calls();

        //act
        result = BUFFER_fill(buffer, '@');

        expected = umockc_stringify_buffer(RESULT_BUFFER, BUFFER_TEST1_SIZE);
        actual = umockc_stringify_buffer(BUFFER_u_char(buffer), BUFFER_length(buffer));

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, expected, actual);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        BUFFER_delete(buffer);
        my_gballoc_free(expected);
        my_gballoc_free(actual);
    }

    /* Tests_SRS_BUFFER_07_002: [ If handle is NULL BUFFER_fill shall return a non-zero value. ] */
    TEST_FUNCTION(BUFFER_fill_handle_NULL_fail)
    {
        int result;

        //arrange
        umock_c_reset_all_calls();

        //act
        result = BUFFER_fill(NULL, '@');

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    /* Tests_SRS_BUFFER_07_001: [ BUFFER_fill shall fill the supplied BUFFER_HANDLE with the supplied fill character. ] */
    TEST_FUNCTION(BUFFER_fill_empty_buffer_succeed)
    {
        int result;

        //arrange
        BUFFER_HANDLE buffer = BUFFER_new();

        umock_c_reset_all_calls();

        //act
        result = BUFFER_fill(buffer, '@');

        ///assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        BUFFER_delete(buffer);
    }

END_TEST_SUITE(Buffer_UnitTests)
