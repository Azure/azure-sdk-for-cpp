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

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void* my_gballoc_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

static void my_gballoc_free(void* s)
{
    free(s);
}


#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#undef ENABLE_MOCKS

#include "azure_c_shared_utility/vector.h"

#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}


typedef struct VECTOR_UNITTEST_TAG
{
    int  nValue1;
    long lValue2;
} VECTOR_UNITTEST;

static bool VECTOR_UNITTEST_isEqual(const void* left_hand_side, const void* right_hand_side)
{
    VECTOR_UNITTEST* rhs = (VECTOR_UNITTEST*)left_hand_side;
    VECTOR_UNITTEST* lhs = (VECTOR_UNITTEST*)right_hand_side;

    return (rhs->nValue1 == lhs->nValue1 && rhs->lValue2 == lhs->lValue2);
}

#define NUM_ITEM_PUSH_BACK      128

BEGIN_TEST_SUITE(Vector_UnitTests)

    TEST_SUITE_INITIALIZE(a)
    {
        int result;

        result = umock_c_init(on_umock_c_error);
        ASSERT_ARE_EQUAL(int, 0, result);

        REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_realloc, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    }

    TEST_SUITE_CLEANUP(TestClassCleanup)
    {
        umock_c_deinit();
    }

    TEST_FUNCTION_INITIALIZE(initialize)
    {
        umock_c_reset_all_calls();
    }

    TEST_FUNCTION_CLEANUP(cleans)
    {
    }

    /* Vector_Tests BEGIN */

    /* Tests_SRS_VECTOR_10_001: [VECTOR_create shall allocate a VECTOR_HANDLE that will contain an empty vector.The size of each element is given with the parameter elementSize.] */
    TEST_FUNCTION(VECTOR_create_succeeds)
    {
        ///arrange
        VECTOR_HANDLE handle;
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument_size();

        ///act
        handle = VECTOR_create(sizeof(VECTOR_UNITTEST));

        ///assert
        ASSERT_IS_NOT_NULL(handle);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_002: [VECTOR_create shall fail and return NULL if elementsize is equal to 0.] */
    TEST_FUNCTION(VECTOR_create_fails_if_element_size_is_zero)
    {
        ///arrange

        ///act
        VECTOR_HANDLE handle = VECTOR_create(0);

        ///assert
        ASSERT_IS_NULL(handle);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_VECTOR_10_033: [VECTOR_create shall fail and return NULL if malloc fails.] */
    TEST_FUNCTION(VECTOR_create_returns_NULL_if_malloc_fails)
    {
        ///arrange
        VECTOR_HANDLE handle;

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument_size()
            .SetReturn(NULL);

        ///act
        handle = VECTOR_create(sizeof(VECTOR_UNITTEST));

        ///assert
        ASSERT_IS_NULL(handle);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_VECTOR_10_004: [VECTOR_move shall allocate a VECTOR_HANDLE and move the data to it from the given handle.] */
    TEST_FUNCTION(VECTOR_move_succeeds)
    {
        ///arrange
        VECTOR_UNITTEST* current;
        VECTOR_HANDLE test;
        VECTOR_UNITTEST sItem1 = {1, 2};
        VECTOR_UNITTEST sItem2 = {5, 6};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        (void)VECTOR_push_back(handle, &sItem1, 1);
        (void)VECTOR_push_back(handle, &sItem2, 1);
        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument_size();

        ///act
        test = VECTOR_move(handle);

        ///assert
        ASSERT_IS_NOT_NULL(test);
        ASSERT_ARE_EQUAL(size_t, VECTOR_size(test), 2);
        ASSERT_ARE_EQUAL(size_t, VECTOR_size(handle), 0);
        current = (VECTOR_UNITTEST *)VECTOR_element(test, 0);
        ASSERT_ARE_EQUAL(int, sItem1.nValue1, current->nValue1);
        ASSERT_ARE_EQUAL(long, sItem1.lValue2, current->lValue2);
        current = (VECTOR_UNITTEST *)VECTOR_element(test, 1);
        ASSERT_ARE_EQUAL(int, sItem2.nValue1, current->nValue1);
        ASSERT_ARE_EQUAL(long, sItem2.lValue2, current->lValue2);

        ///cleanup
        VECTOR_destroy(handle);
        VECTOR_destroy(test);
    }

    /* Tests_SRS_VECTOR_10_005: [VECTOR_move shall fail and return NULL if the given handle is NULL.] */
    TEST_FUNCTION(VECTOR_move_returns_NULL_if_handle_is_NULL)
    {
        ///arrange

        ///act
        VECTOR_HANDLE test = VECTOR_move(NULL);

        ///assert
        ASSERT_IS_NULL(test);
    }

    /* Tests_SRS_VECTOR_10_006: [VECTOR_move shall fail and return NULL if malloc fails.] */
    TEST_FUNCTION(VECTOR_move_returns_NULL_if_malloc_fails)
    {
        ///arrange
        VECTOR_HANDLE test;
        VECTOR_UNITTEST* current;
        VECTOR_UNITTEST sItem1 = {1, 2};
        VECTOR_UNITTEST sItem2 = {5, 6};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        (void)VECTOR_push_back(handle, &sItem1, 1);
        (void)VECTOR_push_back(handle, &sItem2, 1);
        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument_size()
            .SetReturn(NULL);

        ///act
        test = VECTOR_move(handle);

        ///assert
        ASSERT_IS_NULL(test);
        ASSERT_ARE_EQUAL(size_t, VECTOR_size(handle), 2);
        current = (VECTOR_UNITTEST *)VECTOR_element(handle, 0);
        ASSERT_ARE_EQUAL(int, sItem1.nValue1, current->nValue1);
        ASSERT_ARE_EQUAL(long, sItem1.lValue2, current->lValue2);
        current = (VECTOR_UNITTEST *)VECTOR_element(handle, 1);
        ASSERT_ARE_EQUAL(int, sItem2.nValue1, current->nValue1);
        ASSERT_ARE_EQUAL(long, sItem2.lValue2, current->lValue2);

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_008: [VECTOR_destroy shall free the given handle and its internal storage.] */
    TEST_FUNCTION(VECTOR_destroy_succeeds)
    {
        ///arrange
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument_ptr();
        STRICT_EXPECTED_CALL(gballoc_free(handle));

        ///act
        VECTOR_destroy(handle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_VECTOR_10_009: [VECTOR_destroy shall return if the given handle is NULL.] */
    TEST_FUNCTION(VECTOR_destroy_return_if_handle_is_NULL)
    {
        ///arrange

        ///act
        VECTOR_destroy(NULL);

        ///assert
        ///VECTOR_destroy doesn't crash
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_VECTOR_10_011: [VECTOR_push_back shall fail and return non - zero if handle is NULL.] */
    TEST_FUNCTION(VECTOR_push_back_fails_if_handle_is_NULL)
    {
        ///arrange
        VECTOR_UNITTEST vItem = { 0 };

        ///act
        int result = VECTOR_push_back(NULL, &vItem, 1);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_VECTOR_10_034: [VECTOR_push_back shall fail and return non - zero if elements is NULL.] */
    TEST_FUNCTION(VECTOR_push_back_fails_if_elements_is_NULL)
    {
        ///arrange
        int result;
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        umock_c_reset_all_calls();

        ///act
        result = VECTOR_push_back(handle, NULL, 1);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_035: [VECTOR_push_back shall fail and return non - zero if numElements is 0.] */
    TEST_FUNCTION(VECTOR_push_back_fails_if_numElements_is_zero)
    {
        ///arrange
        int result;
        VECTOR_UNITTEST sItem = {1, 2};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        umock_c_reset_all_calls();

        ///act
        result = VECTOR_push_back(handle, &sItem, 0);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_013: [VECTOR_push_back shall append the given elements and return 0 indicating success.] */
    TEST_FUNCTION(VECTOR_push_back_succeeds)
    {
        ///arrange
        int result;
        VECTOR_UNITTEST sItem = {1, 2};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        umock_c_reset_all_calls();

        ///act
        STRICT_EXPECTED_CALL(gballoc_realloc(NULL, sizeof(VECTOR_UNITTEST)));
        result = VECTOR_push_back(handle, &sItem, 1);

        ///assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_012: [VECTOR_push_back shall fail and return non - zero if memory allocation fails.] */
    TEST_FUNCTION(VECTOR_push_back_fails_if_realloc_fails)
    {
        ///arrange
        int result;
        VECTOR_UNITTEST sItem = {1, 2};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        umock_c_reset_all_calls();

        ///act
        STRICT_EXPECTED_CALL(gballoc_realloc(NULL, sizeof(VECTOR_UNITTEST)))
            .SetReturn(NULL);
        result = VECTOR_push_back(handle, &sItem, 1);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_026: [VECTOR_size shall return 0 if the given handle is NULL.] */
    TEST_FUNCTION(VECTOR_size_fails_if_handle_is_NULL)
    {
        ///arrange

        ///act
        size_t num = VECTOR_size(NULL);

        ///assert
        ASSERT_ARE_EQUAL(size_t, 0, num);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_VECTOR_10_025: [VECTOR_size shall return the number of elements stored with the given handle.] */
    TEST_FUNCTION(VECTOR_size_succeeds_if_vector_is_empty)
    {
        ///arrange
        size_t num;
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        umock_c_reset_all_calls();

        ///act
        num = VECTOR_size(handle);

        ///assert
        ASSERT_ARE_EQUAL(size_t, 0, num);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_025: [VECTOR_size shall return the number of elements stored with the given handle.] */
    TEST_FUNCTION(VECTOR_size_succeeds)
    {
        ///arrange
        size_t num;
        VECTOR_UNITTEST sItem = {1, 2};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        (void)VECTOR_push_back(handle, &sItem, 1);
        umock_c_reset_all_calls();

        ///act
        num = VECTOR_size(handle);

        ///assert
        ASSERT_ARE_EQUAL(size_t, 1, num);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_030: [VECTOR_find_if shall fail and return NULL if handle is NULL.] */
    TEST_FUNCTION(VECTOR_find_fails_if_handle_is_NULL)
    {
        ///arrange
        VECTOR_UNITTEST sItem = {1, 2};

        ///act
        VECTOR_UNITTEST* pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(NULL, VECTOR_UNITTEST_isEqual, &sItem);

        ///assert
        ASSERT_IS_NULL(pfindItem);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_VECTOR_10_036: [VECTOR_find_if shall fail and return NULL if pred is NULL.] */
    TEST_FUNCTION(VECTOR_find_fails_if_pred_is_NULL)
    {
        ///arrange
        VECTOR_UNITTEST* pfindItem;
        VECTOR_UNITTEST sItem = {1, 2};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        (void)VECTOR_push_back(handle, &sItem, 1);
        umock_c_reset_all_calls();

        ///act
        pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(handle, NULL, &sItem);

        ///assert
        ASSERT_IS_NULL(pfindItem);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_031: [VECTOR_find_if shall return the first element in the vector that matches pred.] */
    TEST_FUNCTION(VECTOR_find_if_succeeds)
    {
        ///arrange
        VECTOR_UNITTEST* pfindItem;
        VECTOR_UNITTEST sItem = {1, 2};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        (void)VECTOR_push_back(handle, &sItem, 1);
        umock_c_reset_all_calls();

        ///act
        pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem);

        ///assert
        ASSERT_IS_NOT_NULL(pfindItem);
        ASSERT_ARE_EQUAL(size_t, sItem.nValue1, pfindItem->nValue1);
        ASSERT_ARE_EQUAL(long, sItem.lValue2, pfindItem->lValue2);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_032: [VECTOR_find_if shall return NULL if no element is found that matches pred.] */
    TEST_FUNCTION(VECTOR_find_if_return_null_if_no_match)
    {
        ///arrange
        VECTOR_UNITTEST* pfindItem;
        VECTOR_UNITTEST sItem1 = {1, 2};
        VECTOR_UNITTEST sItem2 = {5, 8};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        (void)VECTOR_push_back(handle, &sItem1, 1);
        umock_c_reset_all_calls();

        ///act
        pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem2);

        ///assert
        ASSERT_IS_NULL(pfindItem);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_017: [VECTOR_clear shall if the object is NULL or empty.] */
    TEST_FUNCTION(VECTOR_clear_fails_if_handle_is_NULL)
    {
        ///arrange

        ///act
        VECTOR_clear(NULL);

        ///assert
        // Make sure this clear doesn't crash
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_VECTOR_10_017: [VECTOR_clear shall if the object is NULL or empty.] */
    TEST_FUNCTION(VECTOR_clear_fails_if_vector_is_empty)
    {
        ///arrange
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        umock_c_reset_all_calls();

        ///act
        STRICT_EXPECTED_CALL(gballoc_free(NULL));
        VECTOR_clear(handle);

        ///assert
        // Make sure this clear doesn't crash
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_016: [VECTOR_clear shall remove all elements from the object and release internal storage.] */
    TEST_FUNCTION(VECTOR_clear_succeeds)
    {
        ///arrange
        size_t num;
        VECTOR_UNITTEST sItem = {1, 2};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        (void)VECTOR_push_back(handle, &sItem, 1);
        (void)VECTOR_push_back(handle, &sItem, 1);
        umock_c_reset_all_calls();

        ///act
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument_ptr();
        VECTOR_clear(handle);

        ///assert
        num = VECTOR_size(handle);
        ASSERT_ARE_EQUAL(size_t, 0, num);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_018: [VECTOR_element shall return a pointer to the element at the given index.] */
    TEST_FUNCTION(VECTOR_element_succeeds)
    {
        ///arrange
        VECTOR_UNITTEST* pResult;
        VECTOR_UNITTEST sItem1 = {1, 2};
        VECTOR_UNITTEST sItem2 = {3, 4};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        (void)VECTOR_push_back(handle, &sItem1, 1);
        (void)VECTOR_push_back(handle, &sItem2, 1);
        umock_c_reset_all_calls();

        ///act
        pResult = (VECTOR_UNITTEST*)VECTOR_element(handle, 1);

        ///assert
        ASSERT_IS_NOT_NULL(pResult);
        ASSERT_ARE_EQUAL(size_t, sItem2.nValue1, pResult->nValue1);
        ASSERT_ARE_EQUAL(long, sItem2.lValue2, pResult->lValue2);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_019: [VECTOR_element shall fail and return NULL if handle is NULL.] */
    TEST_FUNCTION(VECTOR_element_fails_if_handle_is_NULL)
    {
        ///arrange

        ///act
        void* pResult = VECTOR_element(NULL, 0);

        ///assert
        ASSERT_IS_NULL(pResult);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_VECTOR_10_020: [VECTOR_element shall fail and return NULL if the given index is out of range.] */
    TEST_FUNCTION(VECTOR_element_fails_if_index_is_out_of_range)
    {
        ///arrange
        VECTOR_UNITTEST* pResult;
        VECTOR_UNITTEST sItem = {1, 2};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        (void)VECTOR_push_back(handle, &sItem, 1);
        (void)VECTOR_push_back(handle, &sItem, 1);
        umock_c_reset_all_calls();

        ///act
        pResult = (VECTOR_UNITTEST*)VECTOR_element(handle, 2);

        ///assert
        ASSERT_IS_NULL(pResult);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_022: [VECTOR_front shall fail and return NULL if handle is NULL.] */
    TEST_FUNCTION(VECTOR_front_fails_if_handle_is_NULL)
    {
        ///arrange

        ///act
        void* pResult = VECTOR_front(NULL);

        ///assert
        ASSERT_IS_NULL(pResult);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_VECTOR_10_021: [VECTOR_front shall return the element at index 0.] */
    TEST_FUNCTION(VECTOR_front_succeeds)
    {
        ///arrange
        VECTOR_UNITTEST* pResult;
        VECTOR_UNITTEST sItem1 = {1, 2};
        VECTOR_UNITTEST sItem2 = {3, 4};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        (void)VECTOR_push_back(handle, &sItem1, 1);
        (void)VECTOR_push_back(handle, &sItem2, 1);
        umock_c_reset_all_calls();

        ///act
        pResult = (VECTOR_UNITTEST*)VECTOR_front(handle);

        ///assert
        ASSERT_IS_NOT_NULL(pResult);
        ASSERT_ARE_EQUAL(size_t, sItem1.nValue1, pResult->nValue1);
        ASSERT_ARE_EQUAL(long, sItem1.lValue2, pResult->lValue2);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_028: [VECTOR_front shall fail and return NULL if the vector is empty.] */
    TEST_FUNCTION(VECTOR_front_return_null_if_vector_is_empty)
    {
        ///arrange
        VECTOR_UNITTEST* pResult;
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        umock_c_reset_all_calls();

        ///act
        pResult = (VECTOR_UNITTEST*)VECTOR_front(handle);

        ///assert
        ASSERT_IS_NULL(pResult);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_023: [VECTOR_back shall return the last element of the vector.] */
    TEST_FUNCTION(VECTOR_back_succeeds)
    {
        ///arrange
        VECTOR_UNITTEST* pResult;
        VECTOR_UNITTEST sItem1 = {1, 2};
        VECTOR_UNITTEST sItem2 = {3, 4};
        VECTOR_UNITTEST sItem3 = {5, 6};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        (void)VECTOR_push_back(handle, &sItem1, 1);
        (void)VECTOR_push_back(handle, &sItem2, 1);
        (void)VECTOR_push_back(handle, &sItem3, 1);
        umock_c_reset_all_calls();

        ///act
        pResult = (VECTOR_UNITTEST*)VECTOR_back(handle);

        ///assert
        ASSERT_IS_NOT_NULL(pResult);
        ASSERT_ARE_EQUAL(size_t, sItem3.nValue1, pResult->nValue1);
        ASSERT_ARE_EQUAL(long, sItem3.lValue2, pResult->lValue2);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_024: [VECTOR_back shall fail and return NULL if handle is NULL.] */
    TEST_FUNCTION(VECTOR_back_fails_if_handle_is_NULL)
    {
        ///arrange

        ///act
        void* pResult = VECTOR_back(NULL);

        ///assert
        ASSERT_IS_NULL(pResult);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_VECTOR_10_029: [VECTOR_back shall fail and return NULL if the vector is empty.] */
    TEST_FUNCTION(VECTOR_back_return_null_if_vector_is_empty)
    {
        ///arrange
        VECTOR_UNITTEST* pResult;
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        umock_c_reset_all_calls();

        ///act
        pResult = (VECTOR_UNITTEST*)VECTOR_back(handle);

        ///assert
        ASSERT_IS_NULL(pResult);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_015: [VECTOR_erase shall return if handle.] */
    TEST_FUNCTION(VECTOR_erase_if_handle_is_NULL)
    {
        ///arrange
        VECTOR_UNITTEST sItem = {1, 2};

        ///act
        VECTOR_erase(NULL, &sItem, 1);

        ///assert
        // Make sure this erase doesn't crash
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_VECTOR_10_038: [VECTOR_erase shall return if elements is NULL.] */
    TEST_FUNCTION(VECTOR_erase_if_elements_is_NULL)
    {
        ///arrange
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        umock_c_reset_all_calls();

        ///act
        VECTOR_erase(handle, NULL, 1);

        ///assert
        // Make sure this erase doesn't crash
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_039: [VECTOR_erase shall return if numElements is 0.] */
    TEST_FUNCTION(VECTOR_erase_if_numElements_is_zero)
    {
        ///arrange
        VECTOR_UNITTEST* pfindItem;
        size_t num;
        VECTOR_UNITTEST sItem1 = {1, 2};
        VECTOR_UNITTEST sItem2 = {3, 4};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        (void)VECTOR_push_back(handle, &sItem1, 1);
        (void)VECTOR_push_back(handle, &sItem2, 1);
        pfindItem = (VECTOR_UNITTEST*)VECTOR_back(handle);
        umock_c_reset_all_calls();

        ///act
        VECTOR_erase(handle, pfindItem, 0);

        ///assert
        // Make sure this erase doesn't crash
        num = VECTOR_size(handle);
        ASSERT_ARE_EQUAL(size_t, 2, num);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_014: [VECTOR_erase shall remove the numElements starting at elements and reduce its internal storage.] */
    TEST_FUNCTION(VECTOR_erase_succeeds_case_1)
    {
        ///arrange
        VECTOR_UNITTEST* pfindItem;
        size_t num;
        VECTOR_UNITTEST sItem1 = {1, 2};
        VECTOR_UNITTEST sItem2 = {3, 4};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        (void)VECTOR_push_back(handle, &sItem1, 1);
        (void)VECTOR_push_back(handle, &sItem2, 1);
        pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem1);
        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, sizeof(VECTOR_UNITTEST)))
            .IgnoreArgument_ptr();

        ///act
        VECTOR_erase(handle, pfindItem, 1);

        ///assert
        num = VECTOR_size(handle);
        ASSERT_ARE_EQUAL(size_t, 1, num);
        pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem1);
        ASSERT_IS_NULL(pfindItem);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_014: [VECTOR_erase shall remove the numElements starting at elements and reduce its internal storage.] */
    TEST_FUNCTION(VECTOR_erase_succeeds_case_2)
    {
        ///arrange
        VECTOR_UNITTEST* pfindItem;
        size_t num;
        VECTOR_UNITTEST sItem1 = {1, 2};
        VECTOR_UNITTEST sItem2 = {3, 4};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        (void)VECTOR_push_back(handle, &sItem1, 1);
        (void)VECTOR_push_back(handle, &sItem2, 1);
        pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem1);
        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument_ptr();

        ///act
        VECTOR_erase(handle, pfindItem, 2);

        ///assert
        num = VECTOR_size(handle);
        ASSERT_ARE_EQUAL(size_t, 0, num);
        pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem1);
        ASSERT_IS_NULL(pfindItem);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_014: [VECTOR_erase shall remove the numElements starting at elements and reduce its internal storage.] */
    TEST_FUNCTION(VECTOR_erase_succeeds_case_3)
    {
        ///arrange
        size_t num;
        VECTOR_UNITTEST* pfindItem;
        VECTOR_UNITTEST sItem1 = {1, 2};
        VECTOR_UNITTEST sItem2 = {3, 4};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        (void)VECTOR_push_back(handle, &sItem1, 1);
        (void)VECTOR_push_back(handle, &sItem2, 1);
        pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem1);
        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
            .IgnoreArgument_ptr()
            .IgnoreArgument_size()
            .SetReturn(NULL);

        ///act
        VECTOR_erase(handle, pfindItem, 1);

        ///assert
        num = VECTOR_size(handle);
        ASSERT_ARE_EQUAL(size_t, 1, num);
        pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem1);
        ASSERT_IS_NULL(pfindItem);
        pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem2);
        ASSERT_IS_NOT_NULL(pfindItem);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_027: [VECTOR_erase shall return if numElements is out of bound.] */
    TEST_FUNCTION(VECTOR_erase_numElements_out_of_bound)
    {
        ///arrange
        size_t num;
        VECTOR_UNITTEST* pfindItem;
        VECTOR_UNITTEST sItem1 = {1, 2};
        VECTOR_UNITTEST sItem2 = {3, 4};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        (void)VECTOR_push_back(handle, &sItem1, 1);
        (void)VECTOR_push_back(handle, &sItem2, 1);
        pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem2);
        umock_c_reset_all_calls();

        ///act
        VECTOR_erase(handle, pfindItem, 2);

        ///assert
        num = VECTOR_size(handle);
        ASSERT_ARE_EQUAL(size_t, 2, num);
        pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem1);
        ASSERT_IS_NOT_NULL(pfindItem);
        pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem2);
        ASSERT_IS_NOT_NULL(pfindItem);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_040: [VECTOR_erase shall return if elements is out of bound.] */
    TEST_FUNCTION(VECTOR_erase_elements_out_of_bound_case_1)
    {
        ///arrange
        VECTOR_UNITTEST* pfindItem;
        size_t num;
        VECTOR_UNITTEST sItem1 = {1, 2};
        VECTOR_UNITTEST sItem2 = {3, 4};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        (void)VECTOR_push_back(handle, &sItem1, 1);
        (void)VECTOR_push_back(handle, &sItem2, 1);
        pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem1);
        pfindItem -= 1;
        umock_c_reset_all_calls();

        ///act
        VECTOR_erase(handle, pfindItem, 1);

        ///assert
        num = VECTOR_size(handle);
        ASSERT_ARE_EQUAL(size_t, 2, num);
        pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem1);
        ASSERT_IS_NOT_NULL(pfindItem);
        pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem2);
        ASSERT_IS_NOT_NULL(pfindItem);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_040: [VECTOR_erase shall return if elements is out of bound.] */
    TEST_FUNCTION(VECTOR_erase_elements_out_of_bound_case_2)
    {
        ///arrange
        size_t num;
        VECTOR_UNITTEST* pfindItem;
        VECTOR_UNITTEST sItem1 = {1, 2};
        VECTOR_UNITTEST sItem2 = {3, 4};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        (void)VECTOR_push_back(handle, &sItem1, 1);
        (void)VECTOR_push_back(handle, &sItem2, 1);
        pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem2);
        pfindItem += 2;
        umock_c_reset_all_calls();

        ///act
        VECTOR_erase(handle, pfindItem, 1);

        ///assert
        num = VECTOR_size(handle);
        ASSERT_ARE_EQUAL(size_t, 2, num);
        pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem1);
        ASSERT_IS_NOT_NULL(pfindItem);
        pfindItem = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem2);
        ASSERT_IS_NOT_NULL(pfindItem);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Tests_SRS_VECTOR_10_041: [VECTOR_erase shall return if elements is misaligned.] */
    TEST_FUNCTION(VECTOR_erase_elements_misaligned)
    {
        ///arrange
        VECTOR_UNITTEST* pResult;
        void* pfindItem;
        size_t num;
        VECTOR_UNITTEST sItem1 = {1, 2};
        VECTOR_UNITTEST sItem2 = {3, 4};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        (void)VECTOR_push_back(handle, &sItem1, 1);
        (void)VECTOR_push_back(handle, &sItem2, 1);
        pfindItem = VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem1);
        pfindItem = (void*)(((unsigned char*)pfindItem) + 0x1);
        umock_c_reset_all_calls();

        ///act
        VECTOR_erase(handle, pfindItem, 1);

        ///assert
        num = VECTOR_size(handle);
        ASSERT_ARE_EQUAL(size_t, 2, num);
        pResult = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem1);
        ASSERT_IS_NOT_NULL(pfindItem);
        ASSERT_ARE_EQUAL(size_t, sItem1.nValue1, pResult->nValue1);
        ASSERT_ARE_EQUAL(long, sItem1.lValue2, pResult->lValue2);
        pResult = (VECTOR_UNITTEST*)VECTOR_find_if(handle, VECTOR_UNITTEST_isEqual, &sItem2);
        ASSERT_IS_NOT_NULL(pResult);
        ASSERT_ARE_EQUAL(size_t, sItem2.nValue1, pResult->nValue1);
        ASSERT_ARE_EQUAL(long, sItem2.lValue2, pResult->lValue2);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    TEST_FUNCTION(VECTOR_push_back_multiple_elements_succeeds)
    {
        ///arrange
        size_t nIndex;
        VECTOR_UNITTEST* pResult;
        int result = 0;
        VECTOR_UNITTEST sItem1 = {1, 2};
        VECTOR_HANDLE handle = VECTOR_create(sizeof(VECTOR_UNITTEST));
        umock_c_reset_all_calls();
        for (nIndex = 0; nIndex < NUM_ITEM_PUSH_BACK; nIndex++)
        {
            STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, (nIndex + 1) * sizeof(VECTOR_UNITTEST)))
                .IgnoreArgument_ptr();
        }

        ///act
        for (nIndex = 0; (nIndex < NUM_ITEM_PUSH_BACK) && (result == 0); nIndex++)
        {
            sItem1.nValue1++;
            sItem1.lValue2++;
            result = VECTOR_push_back(handle, &sItem1, 1);
        }

        ///assert
        ASSERT_ARE_EQUAL(int, 0, result);
        pResult = (VECTOR_UNITTEST*)VECTOR_back(handle);
        ASSERT_IS_NOT_NULL(pResult);
        ASSERT_ARE_EQUAL(size_t, sItem1.nValue1, pResult->nValue1);
        ASSERT_ARE_EQUAL(long, sItem1.lValue2, pResult->lValue2);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        VECTOR_destroy(handle);
    }

    /* Vector_Tests END */

END_TEST_SUITE(Vector_UnitTests)
