// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdbool.h>
#include <stdlib.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"

static size_t currentmalloc_call = 0;
static size_t whenShallmalloc_fail = 0;

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

void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_bool.h"
#include "azure_c_shared_utility/singlylinkedlist.h"

#define ENABLE_MOCKS

/* test match function mock */
MOCK_FUNCTION_WITH_CODE(, bool, test_match_function, LIST_ITEM_HANDLE, list_item, const void*, match_context)
MOCK_FUNCTION_END(true);

#include "azure_c_shared_utility/gballoc.h"

#undef ENABLE_MOCKS

static TEST_MUTEX_HANDLE test_serialize_mutex;

#define TEST_CONTEXT ((const void*)0x4242)

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(singlylinkedlist_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    result = umocktypes_bool_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_UMOCK_ALIAS_TYPE(LIST_ITEM_HANDLE, void*);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(test_serialize_mutex);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    if (TEST_MUTEX_ACQUIRE(test_serialize_mutex))
    {
        ASSERT_FAIL("Could not acquire test serialization mutex.");
    }

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    TEST_MUTEX_RELEASE(test_serialize_mutex);
}

/* singlylinkedlist_create */

/* Tests_SRS_LIST_01_001: [singlylinkedlist_create shall create a new list and return a non-NULL handle on success.] */
TEST_FUNCTION(when_underlying_calls_succeed_singlylinkedlist_create_succeeds)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE result;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    // act
    result = singlylinkedlist_create();

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(result);
}

/* Tests_SRS_LIST_01_002: [If any error occurs during the list creation, singlylinkedlist_create shall return NULL.] */
TEST_FUNCTION(when_underlying_malloc_fails_singlylinkedlist_create_fails)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE result;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);

    // act
    result = singlylinkedlist_create();

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* singlylinkedlist_destroy */

/* Tests_SRS_LIST_01_003: [singlylinkedlist_destroy shall free all resources associated with the list identified by the handle argument.] */
TEST_FUNCTION(singlylinkedlist_destroy_on_a_non_null_handle_frees_resources)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE handle = singlylinkedlist_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    singlylinkedlist_destroy(handle);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_LIST_01_004: [If the list argument is NULL, no freeing of resources shall occur.] */
TEST_FUNCTION(singlylinkedlist_destroy_on_a_null_list_frees_nothing)
{
    // arrange

    // act
    singlylinkedlist_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* singlylinkedlist_add */

/* Tests_SRS_LIST_01_006: [If any of the arguments is NULL, singlylinkedlist_add shall not add the item to the list and return NULL.] */
TEST_FUNCTION(singlylinkedlist_add_with_NULL_handle_fails)
{
    // arrange
    int x = 42;

    // act
    LIST_ITEM_HANDLE result = singlylinkedlist_add(NULL, &x);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_LIST_01_006: [If any of the arguments is NULL, singlylinkedlist_add shall not add the item to the list and return NULL.] */
TEST_FUNCTION(singlylinkedlist_add_with_NULL_item_fails)
{
    // arrange
    LIST_ITEM_HANDLE result;
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    umock_c_reset_all_calls();

    // act
    result = singlylinkedlist_add(list, NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_01_005: [singlylinkedlist_add shall add one item to the tail of the list and on success it shall return a handle to the added item.] */
/* Tests_SRS_LIST_01_008: [singlylinkedlist_get_head_item shall return the head of the list.] */
TEST_FUNCTION(singlylinkedlist_add_adds_the_item_and_returns_a_non_NULL_handle)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    int x = 42;
    LIST_ITEM_HANDLE result;
    LIST_ITEM_HANDLE head;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    // act
    result = singlylinkedlist_add(list, &x);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    head = singlylinkedlist_get_head_item(list);
    ASSERT_IS_NOT_NULL(head);
    ASSERT_ARE_EQUAL(int, x, *(const int*)singlylinkedlist_item_get_value(head));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_01_005: [singlylinkedlist_add shall add one item to the tail of the list and on success it shall return a handle to the added item.] */
/* Tests_SRS_LIST_01_008: [singlylinkedlist_get_head_item shall return the head of the list.] */
TEST_FUNCTION(singlylinkedlist_add_when_an_item_is_in_the_singlylinkedlist_adds_at_the_end)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    int x1 = 42;
    int x2 = 43;
    LIST_ITEM_HANDLE result;
    LIST_ITEM_HANDLE list_item;

    (void)singlylinkedlist_add(list, &x1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    // act
    result = singlylinkedlist_add(list, &x2);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    list_item = singlylinkedlist_get_head_item(list);
    ASSERT_IS_NOT_NULL(list_item);
    ASSERT_ARE_EQUAL(int, x1, *(const int*)singlylinkedlist_item_get_value(list_item));
    list_item = singlylinkedlist_get_next_item(list_item);
    ASSERT_IS_NOT_NULL(list_item);
    ASSERT_ARE_EQUAL(int, x2, *(const int*)singlylinkedlist_item_get_value(list_item));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_01_007: [If allocating the new list node fails, singlylinkedlist_add shall return NULL.] */
TEST_FUNCTION(when_the_underlying_malloc_fails_singlylinkedlist_add_fails)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    int x = 42;
    LIST_ITEM_HANDLE result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);

    // act
    result = singlylinkedlist_add(list, &x);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* singlylinkedlist_get_head_item */

/* Tests_SRS_LIST_01_010: [If the list is empty, singlylinkedlist_get_head_item_shall_return NULL.] */
TEST_FUNCTION(when_the_list_is_empty_singlylinkedlist_get_head_item_yields_NULL)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    LIST_ITEM_HANDLE result;
    umock_c_reset_all_calls();

    // act
    result = singlylinkedlist_get_head_item(list);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_01_009: [If the list argument is NULL, singlylinkedlist_get_head_item shall return NULL.] */
TEST_FUNCTION(singlylinkedlist_get_head_item_with_NULL_list_yields_NULL)
{
    // arrange

    // act
    LIST_ITEM_HANDLE result = singlylinkedlist_get_head_item(NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_LIST_01_008: [singlylinkedlist_get_head_item shall return the head of the list.] */
TEST_FUNCTION(singlylinkedlist_get_head_item_removes_the_item)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    int x = 42;
    LIST_ITEM_HANDLE head;
    (void)singlylinkedlist_add(list, &x);
    umock_c_reset_all_calls();

    // act
    head = singlylinkedlist_get_head_item(list);

    // assert
    ASSERT_IS_NOT_NULL(head);
    ASSERT_ARE_EQUAL(int, x, *(const int*)singlylinkedlist_item_get_value(head));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* singlylinkedlist_get_next_item */

/* Tests_SRS_LIST_01_018: [singlylinkedlist_get_next_item shall return the next item in the list following the item item_handle.] */
TEST_FUNCTION(singlylinkedlist_get_next_item_gets_the_next_item)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    int x1 = 42;
    int x2 = 43;
    LIST_ITEM_HANDLE item;
    (void)singlylinkedlist_add(list, &x1);
    (void)singlylinkedlist_add(list, &x2);
    umock_c_reset_all_calls();
    item = singlylinkedlist_get_head_item(list);

    // act
    item = singlylinkedlist_get_next_item(item);

    // assert
    ASSERT_IS_NOT_NULL(item);
    ASSERT_ARE_EQUAL(int, x2, *(const int*)singlylinkedlist_item_get_value(item));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_01_019: [If item_handle is NULL then singlylinkedlist_get_next_item shall return NULL.] */
TEST_FUNCTION(singlylinkedlist_get_next_item_with_NULL_item_handle_returns_NULL)
{
    // arrange

    // act
    LIST_ITEM_HANDLE item = singlylinkedlist_get_next_item(NULL);

    // assert
    ASSERT_IS_NULL(item);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_LIST_01_022: [If no more items exist in the list after the item_handle item, singlylinkedlist_get_next_item shall return NULL.] */
TEST_FUNCTION(singlylinkedlist_get_next_item_when_no_more_items_in_list_returns_NULL)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    int x1 = 42;
    int x2 = 43;
    LIST_ITEM_HANDLE item;
    (void)singlylinkedlist_add(list, &x1);
    (void)singlylinkedlist_add(list, &x2);
    umock_c_reset_all_calls();
    item = singlylinkedlist_get_head_item(list);
    item = singlylinkedlist_get_next_item(item);

    // act
    item = singlylinkedlist_get_next_item(item);

    // assert
    ASSERT_IS_NULL(item);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* singlylinkedlist_item_get_value */

/* Tests_SRS_LIST_01_020: [singlylinkedlist_item_get_value shall return the value associated with the list item identified by the item_handle argument.] */
TEST_FUNCTION(singlylinkedlist_item_get_value_returns_the_item_value)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    int x = 42;
    LIST_ITEM_HANDLE item;
    int result;
    (void)singlylinkedlist_add(list, &x);
    umock_c_reset_all_calls();
    item = singlylinkedlist_get_head_item(list);

    // act
    result = *(const int*)singlylinkedlist_item_get_value(item);

    // assert
    ASSERT_ARE_EQUAL(int, x, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_01_021: [If item_handle is NULL, singlylinkedlist_item_get_value shall return NULL.] */
TEST_FUNCTION(singlylinkedlist_item_get_value_with_NULL_item_returns_NULL)
{
    // arrange

    // act
    const void* result = singlylinkedlist_item_get_value(NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* singlylinkedlist_find */

/* Tests_SRS_LIST_01_012: [If the list or the match_function argument is NULL, singlylinkedlist_find shall return NULL.] */
TEST_FUNCTION(singlylinkedlist_find_with_NULL_list_fails_with_NULL)
{
    // arrange

    // act
    LIST_ITEM_HANDLE result = singlylinkedlist_find(NULL, test_match_function, TEST_CONTEXT);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_LIST_01_012: [If the list or the match_function argument is NULL, singlylinkedlist_find shall return NULL.] */
TEST_FUNCTION(singlylinkedlist_find_with_NULL_match_function_fails_with_NULL)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    int x = 42;
    LIST_ITEM_HANDLE result;
    (void)singlylinkedlist_add(list, &x);
    umock_c_reset_all_calls();

    // act
    result = singlylinkedlist_find(list, NULL, TEST_CONTEXT);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_01_011: [singlylinkedlist_find shall iterate through all items in a list and return the first one that satisfies a certain match function.] */
/* Tests_SRS_LIST_01_014: [list find shall determine whether an item satisfies the match criteria by invoking the match function for each item in the list until a matching item is found.] */
/* Tests_SRS_LIST_01_013: [The match_function shall get as arguments the list item being attempted to be matched and the match_context as is.] */
/* Tests_SRS_LIST_01_017: [If the match function returns true, singlylinkedlist_find shall consider that item as matching.] */
TEST_FUNCTION(singlylinkedlist_find_on_a_list_with_1_matching_item_yields_that_item)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    int x = 42;
    LIST_ITEM_HANDLE result;
    (void)singlylinkedlist_add(list, &x);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_match_function(IGNORED_PTR_ARG, TEST_CONTEXT))
        .IgnoreArgument(1);

    // act
    result = singlylinkedlist_find(list, test_match_function, TEST_CONTEXT);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(int, x, *(const int*)singlylinkedlist_item_get_value(result));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_01_016: [If the match function returns false, singlylinkedlist_find shall consider that item as not matching.] */
TEST_FUNCTION(singlylinkedlist_find_on_a_list_with_1_items_that_does_not_match_returns_NULL)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    int x = 42;
    LIST_ITEM_HANDLE result;
    (void)singlylinkedlist_add(list, &x);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_match_function(IGNORED_PTR_ARG, TEST_CONTEXT))
        .IgnoreArgument(1).SetReturn(false);

    // act
    result = singlylinkedlist_find(list, test_match_function, TEST_CONTEXT);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_01_011: [singlylinkedlist_find shall iterate through all items in a list and return the first one that satisfies a certain match function.] */
/* Tests_SRS_LIST_01_014: [list find shall determine whether an item satisfies the match criteria by invoking the match function for each item in the list until a matching item is found.] */
/* Tests_SRS_LIST_01_013: [The match_function shall get as arguments the list item being attempted to be matched and the match_context as is.] */
/* Tests_SRS_LIST_01_017: [If the match function returns true, singlylinkedlist_find shall consider that item as matching.] */
TEST_FUNCTION(singlylinkedlist_find_on_a_list_with_2_items_where_the_first_matches_yields_the_first_item)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    int x1 = 42;
    int x2 = 43;
    LIST_ITEM_HANDLE result;
    (void)singlylinkedlist_add(list, &x1);
    (void)singlylinkedlist_add(list, &x2);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_match_function(IGNORED_PTR_ARG, TEST_CONTEXT))
        .IgnoreArgument(1);

    // act
    result = singlylinkedlist_find(list, test_match_function, TEST_CONTEXT);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(int, x1, *(int*)singlylinkedlist_item_get_value(result));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_01_011: [singlylinkedlist_find shall iterate through all items in a list and return the first one that satisfies a certain match function.] */
/* Tests_SRS_LIST_01_014: [list find shall determine whether an item satisfies the match criteria by invoking the match function for each item in the list until a matching item is found.] */
/* Tests_SRS_LIST_01_013: [The match_function shall get as arguments the list item being attempted to be matched and the match_context as is.] */
/* Tests_SRS_LIST_01_017: [If the match function returns true, singlylinkedlist_find shall consider that item as matching.] */
/* Tests_SRS_LIST_01_016: [If the match function returns false, singlylinkedlist_find shall consider that item as not matching.] */
TEST_FUNCTION(singlylinkedlist_find_on_a_list_with_2_items_where_the_second_matches_yields_the_second_item)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    int x1 = 42;
    int x2 = 43;
    LIST_ITEM_HANDLE result;
    (void)singlylinkedlist_add(list, &x1);
    (void)singlylinkedlist_add(list, &x2);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_match_function(IGNORED_PTR_ARG, TEST_CONTEXT))
        .IgnoreArgument(1).SetReturn(false);
    STRICT_EXPECTED_CALL(test_match_function(IGNORED_PTR_ARG, TEST_CONTEXT))
        .IgnoreArgument(1);

    // act
    result = singlylinkedlist_find(list, test_match_function, TEST_CONTEXT);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(int, x2, *(int*)singlylinkedlist_item_get_value(result));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_01_011: [singlylinkedlist_find shall iterate through all items in a list and return the first one that satisfies a certain match function.] */
TEST_FUNCTION(singlylinkedlist_find_on_a_list_with_2_items_both_matching_yields_the_first_item)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    int x1 = 42;
    int x2 = 42;
    LIST_ITEM_HANDLE result;
    (void)singlylinkedlist_add(list, &x1);
    (void)singlylinkedlist_add(list, &x2);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_match_function(IGNORED_PTR_ARG, TEST_CONTEXT))
        .IgnoreArgument(1);

    // act
    result = singlylinkedlist_find(list, test_match_function, TEST_CONTEXT);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(int, x1, *(int*)singlylinkedlist_item_get_value(result));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_01_016: [If the match function returns false, singlylinkedlist_find shall consider that item as not matching.] */
TEST_FUNCTION(singlylinkedlist_find_on_a_list_with_2_items_where_none_matches_returns_NULL)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    int x1 = 42;
    int x2 = 43;
    LIST_ITEM_HANDLE result;
    (void)singlylinkedlist_add(list, &x1);
    (void)singlylinkedlist_add(list, &x2);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_match_function(IGNORED_PTR_ARG, TEST_CONTEXT))
        .IgnoreArgument(1).SetReturn(false);
    STRICT_EXPECTED_CALL(test_match_function(IGNORED_PTR_ARG, TEST_CONTEXT))
        .IgnoreArgument(1).SetReturn(false);

    // act
    result = singlylinkedlist_find(list, test_match_function, TEST_CONTEXT);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_01_015: [If the list is empty, singlylinkedlist_find shall return NULL.] */
TEST_FUNCTION(singlylinkedlist_find_on_a_list_with_no_items_yields_NULL)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    LIST_ITEM_HANDLE result;
    umock_c_reset_all_calls();

    // act
    result = singlylinkedlist_find(list, test_match_function, TEST_CONTEXT);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* singlylinkedlist_remove */

/* Tests_SRS_LIST_01_023: [singlylinkedlist_remove shall remove a list item from the list and on success it shall return 0.] */
TEST_FUNCTION(singlylinkedlist_remove_when_one_item_is_in_the_list_succeeds)
{
    // arrange
    int x1 = 0x42;
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    LIST_ITEM_HANDLE item;
    int result;
    singlylinkedlist_add(list, &x1);
    item = singlylinkedlist_find(list, test_match_function, TEST_CONTEXT);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = singlylinkedlist_remove(list, item);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_01_024: [If any of the arguments list or item_handle is NULL, singlylinkedlist_remove shall fail and return a non-zero value.] */
TEST_FUNCTION(singlylinkedlist_remove_with_NULL_list_fails)
{
    // arrange
    int x1 = 0x42;
    int result;
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    LIST_ITEM_HANDLE item = singlylinkedlist_add(list, &x1);
    umock_c_reset_all_calls();

    // act
    result = singlylinkedlist_remove(NULL, item);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_01_024: [If any of the arguments list or item_handle is NULL, singlylinkedlist_remove shall fail and return a non-zero value.] */
TEST_FUNCTION(singlylinkedlist_remove_with_NULL_item_fails)
{
    // arrange
    int result;
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    umock_c_reset_all_calls();

    // act
    result = singlylinkedlist_remove(list, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_01_025: [If the item item_handle is not found in the list, then singlylinkedlist_remove shall fail and return a non-zero value.] */
TEST_FUNCTION(singlylinkedlist_remove_with_an_item_that_has_already_been_removed_fails)
{
    // arrange
    int x1 = 0x42;
    int result;
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    LIST_ITEM_HANDLE item = singlylinkedlist_add(list, &x1);
    singlylinkedlist_remove(list, item);
    umock_c_reset_all_calls();

    // act
    result = singlylinkedlist_remove(list, item);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_01_023: [singlylinkedlist_remove shall remove a list item from the list and on success it shall return 0.] */
TEST_FUNCTION(singlylinkedlist_remove_first_of_2_items_succeeds)
{
    // arrange
    int x1 = 0x42;
    int result;
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    LIST_ITEM_HANDLE item1 = singlylinkedlist_add(list, &x1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = singlylinkedlist_remove(list, item1);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_01_023: [singlylinkedlist_remove shall remove a list item from the list and on success it shall return 0.] */
TEST_FUNCTION(singlylinkedlist_remove_second_of_2_items_succeeds)
{
    // arrange
    int x1 = 0x42;
    int x2 = 0x43;
    int result;
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    LIST_ITEM_HANDLE item2;
    (void)singlylinkedlist_add(list, &x1);
    item2 = singlylinkedlist_add(list, &x2);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = singlylinkedlist_remove(list, item2);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* singlylinkedlist_foreach */
typedef struct FOREACH_PROFILE_TAG
{
    int count;
    int stop;
    int sum;
} FOREACH_PROFILE;

static void foreach_action_function(const void* item, const void* action_context, bool* continue_processing)
{
    int* item_value = (int*)item;
    FOREACH_PROFILE* profile = (FOREACH_PROFILE*)action_context;

    profile->count++;

    profile->sum += *item_value;
    *continue_processing = profile->count < profile->stop;
}

/* Tests_SRS_LIST_09_008: [ If the list or the action_function argument is NULL, singlylinkedlist_foreach shall return non-zero value. ] */
TEST_FUNCTION(singlylinkedlist_foreach_NULL_list_argument)
{
    // arrange
    FOREACH_PROFILE profile;
    int result;

    profile.sum = 0;
    profile.count = 0;
    profile.stop = 1000000;

    umock_c_reset_all_calls();

    // act
    result = singlylinkedlist_foreach(NULL, foreach_action_function, &profile);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 0, profile.count);
    ASSERT_ARE_EQUAL(int, 0, profile.sum);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
}

/* Tests_SRS_LIST_09_008: [ If the list or the action_function argument is NULL, singlylinkedlist_foreach shall return non-zero value. ] */
TEST_FUNCTION(singlylinkedlist_foreach_NULL_action_function)
{
    // arrange
    FOREACH_PROFILE profile;
    int v1 = 115;
    int result;
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    (void)singlylinkedlist_add(list, &v1);

    profile.sum = 0;
    profile.count = 0;
    profile.stop = 1000000;

    umock_c_reset_all_calls();

    // act
    result = singlylinkedlist_foreach(list, NULL, &profile);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 0, profile.count);
    ASSERT_ARE_EQUAL(int, 0, profile.sum);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_09_009: [ singlylinkedlist_foreach shall iterate through all items in a list and invoke action_function for each one of them. ] */
/* Tests_SRS_LIST_09_011: [ If no errors occur, singlylinkedlist_foreach shall return zero. ] */
TEST_FUNCTION(singlylinkedlist_foreach_all_items_succeeds)
{
    // arrange
    FOREACH_PROFILE profile;
    int v1 = 115;
    int v2 = 10;
    int v3 = 88;
    int result;
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    (void)singlylinkedlist_add(list, &v1);
    (void)singlylinkedlist_add(list, &v2);
    (void)singlylinkedlist_add(list, &v3);

    profile.sum = 0;
    profile.count = 0;
    profile.stop = 1000000;

    umock_c_reset_all_calls();

    // act
    result = singlylinkedlist_foreach(list, foreach_action_function, &profile);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 3, profile.count);
    ASSERT_ARE_EQUAL(int, (v1 + v2 + v3), profile.sum);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_09_010: [ If the condition function returns continue_processing as false, singlylinkedlist_foreach shall stop iterating through the list and return. ] */
TEST_FUNCTION(singlylinkedlist_foreach_break_succeeds)
{
    // arrange
    FOREACH_PROFILE profile;
    int v1 = 115;
    int v2 = 10;
    int v3 = 88;
    int result;
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    (void)singlylinkedlist_add(list, &v1);
    (void)singlylinkedlist_add(list, &v2);
    (void)singlylinkedlist_add(list, &v3);

    profile.sum = 0;
    profile.count = 0;
    profile.stop = 2;

    umock_c_reset_all_calls();

    // act
    result = singlylinkedlist_foreach(list, foreach_action_function, &profile);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 2, profile.count);
    ASSERT_ARE_EQUAL(int, (v1 + v2), profile.sum);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/* singlylinkedlist_remove_if */

typedef struct REMOVE_IF_PROFILE_TAG
{
    int count;
    int items_to_remove[10];
    int stop_at_item_value;
} REMOVE_IF_PROFILE;

static bool removeif_condition_function(const void* item, const void* condition_context, bool* continue_processing)
{
    int* item_value = (int*)item;
    REMOVE_IF_PROFILE* profile = (REMOVE_IF_PROFILE*)condition_context;
    int i;
    bool remove_item = false;


    for (i = 0; i < profile->count; i++)
    {
        if (*item_value == profile->items_to_remove[i])
        {
            remove_item = true;
            break;
        }
    }

    *continue_processing = (*item_value != profile->stop_at_item_value);

    return remove_item;
}


/* Tests_SRS_LIST_09_001: [ If the list or the condition_function argument is NULL, singlylinkedlist_remove_if shall return non-zero value. ] */
TEST_FUNCTION(singlylinkedlist_remove_if_NULL_condition_argument)
{
    // arrange
    int result;
    int values[5] = { 3, 5, 7, 11, 17 };
    SINGLYLINKEDLIST_HANDLE list;
    REMOVE_IF_PROFILE profile;

    profile.count = 2;
    profile.items_to_remove[0] = values[1];
    profile.items_to_remove[1] = values[4];
    profile.stop_at_item_value = values[3];

    list = singlylinkedlist_create();
    (void)singlylinkedlist_add(list, &values[0]);
    (void)singlylinkedlist_add(list, &values[1]);
    (void)singlylinkedlist_add(list, &values[2]);
    (void)singlylinkedlist_add(list, &values[3]);
    (void)singlylinkedlist_add(list, &values[4]);

    umock_c_reset_all_calls();

    // act
    result = singlylinkedlist_remove_if(list, NULL, &profile);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    {
        FOREACH_PROFILE profile2;
        profile2.sum = 0;
        profile2.count = 0;
        profile2.stop = 10000;
        (void)singlylinkedlist_foreach(list, foreach_action_function, &profile2);
        ASSERT_ARE_EQUAL(int, (values[0] + values[1] + values[2] + values[3] + values[4]), profile2.sum);
    }

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_09_001: [ If the list or the condition_function argument is NULL, singlylinkedlist_remove_if shall return non-zero value. ] */
TEST_FUNCTION(singlylinkedlist_remove_if_NULL_list_argument)
{
    // arrange
    int result;

    REMOVE_IF_PROFILE profile;
    profile.count = 0;

    umock_c_reset_all_calls();

    // act
    result = singlylinkedlist_remove_if(NULL, removeif_condition_function, &profile);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
}

/* Tests_SRS_LIST_09_002: [ singlylinkedlist_remove_if shall iterate through all items in a list and remove all that satisfies a certain condition function. ] */
/* Tests_SRS_LIST_09_003: [ singlylinkedlist_remove_if shall determine whether an item satisfies the condition criteria by invoking the condition function for that item. ] */
/* Tests_SRS_LIST_09_004: [ If the condition function  remove_item as true, singlylinkedlist_find shall consider that item as to be removed. ] */
/* Tests_SRS_LIST_09_005: [ If the condition function returns remove_item as false or unchanged, singlylinkedlist_find shall consider that item as not to be removed. ] */
/* Tests_SRS_LIST_09_007: [ If no errors occur, singlylinkedlist_remove_if shall return zero. ] */
TEST_FUNCTION(singlylinkedlist_remove_if_all_items_succeeds)
{
    // arrange
    int result;
    int values[5] = { 3, 5, 7, 11, 17 };
    REMOVE_IF_PROFILE profile;
    SINGLYLINKEDLIST_HANDLE list;

    profile.count = 2;
    profile.items_to_remove[0] = values[1];
    profile.items_to_remove[1] = values[3];
    profile.stop_at_item_value = 1000000;

    list = singlylinkedlist_create();
    (void)singlylinkedlist_add(list, &values[0]);
    (void)singlylinkedlist_add(list, &values[1]);
    (void)singlylinkedlist_add(list, &values[2]);
    (void)singlylinkedlist_add(list, &values[3]);
    (void)singlylinkedlist_add(list, &values[4]);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = singlylinkedlist_remove_if(list, removeif_condition_function, &profile);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    {
        FOREACH_PROFILE profile2;
        profile2.sum = 0;
        profile2.count = 0;
        profile2.stop = 10000;
        (void)singlylinkedlist_foreach(list, foreach_action_function, &profile2);
        ASSERT_ARE_EQUAL(int, (values[0] + values[2] + values[4]), profile2.sum);
    }

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_09_006: [ If the condition function returns continue_processing as false, singlylinkedlist_remove_if shall stop iterating through the list and return. ] */
TEST_FUNCTION(singlylinkedlist_remove_if_break_succeeds)
{
    // arrange
    int result;
    int values[5] = { 3, 5, 7, 11, 17 };
    REMOVE_IF_PROFILE profile;
    SINGLYLINKEDLIST_HANDLE list;

    profile.count = 2;
    profile.items_to_remove[0] = values[1];
    profile.items_to_remove[1] = values[4];
    profile.stop_at_item_value = values[3];

    list = singlylinkedlist_create();
    (void)singlylinkedlist_add(list, &values[0]);
    (void)singlylinkedlist_add(list, &values[1]);
    (void)singlylinkedlist_add(list, &values[2]);
    (void)singlylinkedlist_add(list, &values[3]);
    (void)singlylinkedlist_add(list, &values[4]);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = singlylinkedlist_remove_if(list, removeif_condition_function, &profile);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    {
        FOREACH_PROFILE profile2;
        profile2.sum = 0;
        profile2.count = 0;
        profile2.stop = 10000;
        (void)singlylinkedlist_foreach(list, foreach_action_function, &profile2);
        ASSERT_ARE_EQUAL(int, (values[0] + values[2] + values[3] + values[4]), profile2.sum);
    }

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_09_006: [ If the condition function returns continue_processing as false, singlylinkedlist_remove_if shall stop iterating through the list and return. ] */
TEST_FUNCTION(singlylinkedlist_remove_if_remove_and_break_succeeds)
{
    // arrange
    int result;
    int values[5] = { 3, 5, 7, 11, 17 };
    REMOVE_IF_PROFILE profile;
    SINGLYLINKEDLIST_HANDLE list;

    profile.count = 2;
    profile.items_to_remove[0] = values[0];
    profile.items_to_remove[1] = values[3];
    profile.stop_at_item_value = values[3];

    list = singlylinkedlist_create();
    (void)singlylinkedlist_add(list, &values[0]);
    (void)singlylinkedlist_add(list, &values[1]);
    (void)singlylinkedlist_add(list, &values[2]);
    (void)singlylinkedlist_add(list, &values[3]);
    (void)singlylinkedlist_add(list, &values[4]);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = singlylinkedlist_remove_if(list, removeif_condition_function, &profile);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    {
        FOREACH_PROFILE profile2;
        profile2.sum = 0;
        profile2.count = 0;
        profile2.stop = 10000;
        (void)singlylinkedlist_foreach(list, foreach_action_function, &profile2);
        ASSERT_ARE_EQUAL(int, (values[1] + values[2] + values[4]), profile2.sum);
    }

    // cleanup
    singlylinkedlist_destroy(list);
}

/* Tests_SRS_LIST_09_006: [ If the condition function returns continue_processing as false, singlylinkedlist_remove_if shall stop iterating through the list and return. ] */
TEST_FUNCTION(singlylinkedlist_remove_if_removes_the_only_item_in_the_list)
{
    // arrange
    int result;
    int values[] = { 42 };
    REMOVE_IF_PROFILE profile;
    SINGLYLINKEDLIST_HANDLE list;

    profile.count = 1;
    profile.items_to_remove[0] = values[0];
    profile.stop_at_item_value = 0;

    list = singlylinkedlist_create();
    (void)singlylinkedlist_add(list, &values[0]);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = singlylinkedlist_remove_if(list, removeif_condition_function, &profile);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    singlylinkedlist_destroy(list);
}

/*Tests_SRS_LIST_02_001: [ If list is NULL then singlylinkedlist_add_head shall fail and return NULL. ]*/
TEST_FUNCTION(singlylinkedlist_add_head_with_list_NULL_fails)
{
    ///arrange

    ///act
    LIST_ITEM_HANDLE listItemHandle = singlylinkedlist_add_head(NULL, (void*)0x42);

    ///assert
    ASSERT_IS_NULL(listItemHandle);
}

/*Tests_SRS_LIST_02_002: [ singlylinkedlist_add_head shall insert item at head, succeed and return a non-NULL value. ]*/
/*Tests_SRS_LIST_02_003: [ If there are any failures then singlylinkedlist_add_head shall fail and return NULL. ]*/
TEST_FUNCTION(singlylinkedlist_add_head_succeeds)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    int x = 42;
    LIST_ITEM_HANDLE result;
    LIST_ITEM_HANDLE head;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    // act
    result = singlylinkedlist_add_head(list, &x);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    head = singlylinkedlist_get_head_item(list);
    ASSERT_IS_NOT_NULL(head);
    ASSERT_ARE_EQUAL(int, x, *(const int*)singlylinkedlist_item_get_value(head));

    // cleanup
    singlylinkedlist_destroy(list);
}

/*Tests_SRS_LIST_02_002: [ singlylinkedlist_add_head shall insert item at head, succeed and return a non-NULL value. ]*/
TEST_FUNCTION(singlylinkedlist_add_head_succeeds_two_times)
{
    // arrange
    LIST_ITEM_HANDLE result1;
    LIST_ITEM_HANDLE result2;
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    int x1 = 42;
    int x2 = 43;
    LIST_ITEM_HANDLE head;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    // act
    result1 = singlylinkedlist_add_head(list, &x1);
    result2 = singlylinkedlist_add_head(list, &x2);

    // assert
    ASSERT_ARE_NOT_EQUAL(void_ptr, result1, result2);

    ASSERT_IS_NOT_NULL(result1);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    head = singlylinkedlist_get_head_item(list);
    ASSERT_IS_NOT_NULL(head);
    ASSERT_ARE_EQUAL(int, x2, *(const int*)singlylinkedlist_item_get_value(head));

    // cleanup
    singlylinkedlist_destroy(list);
}

/*Tests_SRS_LIST_02_002: [ singlylinkedlist_add_head shall insert item at head, succeed and return a non-NULL value. ]*/
TEST_FUNCTION(singlylinkedlist_add_2_heads_and_remove_front_produces_first_item_succeds)
{
    // arrange
    LIST_ITEM_HANDLE result2;
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    int x1 = 42;
    int x2 = 43;
    LIST_ITEM_HANDLE head;
    (void)singlylinkedlist_add_head(list, &x1);
    result2 = singlylinkedlist_add_head(list, &x2);
    (void)singlylinkedlist_remove(list, result2);
    umock_c_reset_all_calls();

    // act
    head = singlylinkedlist_get_head_item(list);

    ///assert
    ASSERT_IS_NOT_NULL(head);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, x1, *(const int*)singlylinkedlist_item_get_value(head));

    // cleanup
    singlylinkedlist_destroy(list);
}

/*Tests_SRS_LIST_02_003: [ If there are any failures then singlylinkedlist_add_head shall fail and return NULL. ]*/
TEST_FUNCTION(singlylinkedlist_add_head_fails_when_malloc_fails)
{
    // arrange
    SINGLYLINKEDLIST_HANDLE list = singlylinkedlist_create();
    int x = 42;
    LIST_ITEM_HANDLE result;
    LIST_ITEM_HANDLE head;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    result = singlylinkedlist_add_head(list, &x);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    head = singlylinkedlist_get_head_item(list);
    ASSERT_IS_NULL(head);

    // cleanup
    singlylinkedlist_destroy(list);
}

END_TEST_SUITE(singlylinkedlist_unittests)
