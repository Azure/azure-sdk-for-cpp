// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#else
#include <stdlib.h>
#include <stddef.h>
#endif

void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "some_refcount_impl.h"
#include "umock_c/umock_c.h"

static TEST_MUTEX_HANDLE g_testByTest;

#define ENABLE_MOCKS

#include "azure_c_shared_utility/gballoc.h"

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(refcount_unittests)

    TEST_SUITE_INITIALIZE(TestClassInitialize)
    {
        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        umock_c_init(on_umock_c_error);

        REGISTER_UMOCK_ALIAS_TYPE(POS_HANDLE, void*);

        REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
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

    /* REFCOUNT_TYPE_CREATE */

    /* Tests_SRS_REFCOUNT_01_002: [ REFCOUNT_TYPE_CREATE shall allocate memory for the type that is ref counted. ]*/
    /* Tests_SRS_REFCOUNT_01_003: [ On success it shall return a non-NULL handle to the allocated ref counted type type. ]*/
    TEST_FUNCTION(refcount_create_returns_non_NULL)
    {
        ///arrange
        POS_HANDLE p;
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

        ///act
        p = Pos_Create(4);

        ///assert
        ASSERT_IS_NOT_NULL(p);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        Pos_Destroy(p);
    }

    /* Tests_SRS_REFCOUNT_01_004: [ If any error occurrs, REFCOUNT_TYPE_CREATE shall return NULL. ]*/
    TEST_FUNCTION(when_malloc_fails_refcount_create_fails)
    {
        ///arrange
        POS_HANDLE p;
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
            .SetReturn(NULL);

        ///act
        p = Pos_Create(4);

        ///assert
        ASSERT_IS_NULL(p);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* REFCOUNT_TYPE_CREATE_WITH_EXTRA_SIZE */

    /* Tests_SRS_REFCOUNT_01_005: [ REFCOUNT_TYPE_CREATE_WITH_EXTRA_SIZE shall allocate memory for the type that is ref counted (type) plus extra memory enough to hold size bytes. ]*/
    /* Tests_SRS_REFCOUNT_01_006: [ On success it shall return a non-NULL handle to the allocated ref counted type type. ]*/
    TEST_FUNCTION(refcount_create_with_extra_size_returns_non_NULL)
    {
        ///arrange
        POS_HANDLE p;
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

        ///act
        p = Pos_Create_With_Extra_Size(4, 42);

        ///assert
        ASSERT_IS_NOT_NULL(p);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        Pos_Destroy(p);
    }

    /* Tests_SRS_REFCOUNT_01_007: [ If any error occurrs, REFCOUNT_TYPE_CREATE_WITH_EXTRA_SIZE shall return NULL. ]*/
    TEST_FUNCTION(when_malloc_fails_refcount_create_with_extra_size_also_fails)
    {
        ///arrange
        POS_HANDLE p;
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
            .SetReturn(NULL);

        ///act
        p = Pos_Create_With_Extra_Size(4, 42);

        ///assert
        ASSERT_IS_NULL(p);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* REFCOUNT_TYPE_DESTROY */

    /* Tests_SRS_REFCOUNT_01_008: [ REFCOUNT_TYPE_DESTROY shall free the memory allocated by REFCOUNT_TYPE_CREATE or REFCOUNT_TYPE_CREATE_WITH_EXTRA_SIZE. ]*/
    TEST_FUNCTION(refcount_DEC_REF_after_create_says_we_should_free)
    {
        ///arrange
        POS_HANDLE p;
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
        p = Pos_Create(4);
        umock_c_reset_all_calls();

        ///act
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
        Pos_Destroy(p);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    /* Tests_SRS_REFCOUNT_01_009: [ If counted_type is NULL, REFCOUNT_TYPE_DESTROY shall return. ]*/
    TEST_FUNCTION(refcount_DESTROY_with_NULL_returns)
    {
        ///arrange

        ///act
        Pos_Destroy(NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    TEST_FUNCTION(refcount_INC_REF_and_DEC_REF_after_create_says_we_should_not_free)
    {
        ///arrange
        POS_HANDLE p, clone_of_p;
        p = Pos_Create(2);
        clone_of_p = Pos_Clone(p);
        umock_c_reset_all_calls();

        ///act
        Pos_Destroy(p);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        Pos_Destroy(p);
    }

    TEST_FUNCTION(refcount_after_clone_it_takes_2_destroys_to_free)
    {
        ///arrange
        POS_HANDLE p, clone_of_p;
        p = Pos_Create(2);
        clone_of_p = Pos_Clone(p);
        Pos_Destroy(p);
        umock_c_reset_all_calls();

        ///act
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
        Pos_Destroy(clone_of_p);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

END_TEST_SUITE(refcount_unittests)
