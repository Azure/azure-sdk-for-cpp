// Copyright (c) Microsoft. All rights reserved.

#ifdef __cplusplus
#include <cinttypes>
#include <cstdlib>
#else
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#endif

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void* my_gballoc_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

static void my_gballoc_free(void* s)
{
    free(s);
}

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_stdint.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_bool.h"
#include "umock_c/umock_c_negative_tests.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/constbuffer.h"
#include "../src/constbuffer.c"
#undef ENABLE_MOCKS

#include "azure_c_shared_utility/constbuffer_array.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

static const unsigned char one = '1';
static const unsigned char two[] = { '2', '2' };
static const unsigned char three[] = { '3', '3', '3' };
static const unsigned char four[] = { '4', '4', '4', '4' };
static const unsigned char five[] = { '5', '5', '5', '5', '5' };
static const unsigned char six[] = { '6', '6', '6', '6', '6', '6' };

static CONSTBUFFER_HANDLE TEST_CONSTBUFFER_HANDLE_1;
static CONSTBUFFER_HANDLE TEST_CONSTBUFFER_HANDLE_2;
static CONSTBUFFER_HANDLE TEST_CONSTBUFFER_HANDLE_3;
static CONSTBUFFER_HANDLE TEST_CONSTBUFFER_HANDLE_4;
static CONSTBUFFER_HANDLE TEST_CONSTBUFFER_HANDLE_5;
static CONSTBUFFER_HANDLE TEST_CONSTBUFFER_HANDLE_6;

BEGIN_TEST_SUITE(constbuffer_array_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    result = umock_c_init(on_umock_c_error);
    ASSERT_ARE_EQUAL(int, 0, result, "umock_c_init");

    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result, "umocktypes_stdint_register_types");

    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result, "umocktypes_charptr_register_types");

    result = umocktypes_bool_register_types();
    ASSERT_ARE_EQUAL(int, 0, result, "umocktypes_bool_register_types");

    REGISTER_GLOBAL_INTERFACE_HOOKS(constbuffer);

    REGISTER_UMOCK_ALIAS_TYPE(CONSTBUFFER_HANDLE, void*);

    REGISTER_GLOBAL_MOCK_FAIL_RETURN(CONSTBUFFER_GetContent, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, my_gballoc_calloc);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_calloc, NULL);

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

    TEST_CONSTBUFFER_HANDLE_1 = UMOCK_REAL(CONSTBUFFER_Create)(&one, sizeof(char));
    ASSERT_IS_NOT_NULL(TEST_CONSTBUFFER_HANDLE_1);

    TEST_CONSTBUFFER_HANDLE_2 = UMOCK_REAL(CONSTBUFFER_Create)(two, sizeof(two));
    ASSERT_IS_NOT_NULL(TEST_CONSTBUFFER_HANDLE_2);

    TEST_CONSTBUFFER_HANDLE_3 = UMOCK_REAL(CONSTBUFFER_Create)(three, sizeof(three));
    ASSERT_IS_NOT_NULL(TEST_CONSTBUFFER_HANDLE_3);

    TEST_CONSTBUFFER_HANDLE_4 = UMOCK_REAL(CONSTBUFFER_Create)(four, sizeof(four));
    ASSERT_IS_NOT_NULL(TEST_CONSTBUFFER_HANDLE_4);

    TEST_CONSTBUFFER_HANDLE_5 = UMOCK_REAL(CONSTBUFFER_Create)(five, sizeof(five));
    ASSERT_IS_NOT_NULL(TEST_CONSTBUFFER_HANDLE_5);

    TEST_CONSTBUFFER_HANDLE_6 = UMOCK_REAL(CONSTBUFFER_Create)(six, sizeof(six));
    ASSERT_IS_NOT_NULL(TEST_CONSTBUFFER_HANDLE_6);

    umock_c_reset_all_calls();
    umock_c_negative_tests_init();
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    UMOCK_REAL(CONSTBUFFER_DecRef)(TEST_CONSTBUFFER_HANDLE_6);
    UMOCK_REAL(CONSTBUFFER_DecRef)(TEST_CONSTBUFFER_HANDLE_5);
    UMOCK_REAL(CONSTBUFFER_DecRef)(TEST_CONSTBUFFER_HANDLE_4);
    UMOCK_REAL(CONSTBUFFER_DecRef)(TEST_CONSTBUFFER_HANDLE_3);
    UMOCK_REAL(CONSTBUFFER_DecRef)(TEST_CONSTBUFFER_HANDLE_2);
    UMOCK_REAL(CONSTBUFFER_DecRef)(TEST_CONSTBUFFER_HANDLE_1);
    umock_c_negative_tests_deinit();
    TEST_MUTEX_RELEASE(test_serialize_mutex);
}

static void constbuffer_array_create_empty_inert_path(void)
{
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
}

/* constbuffer_array_create */

/* Tests_SRS_CONSTBUFFER_ARRAY_01_009: [ constbuffer_array_create shall allocate memory for a new CONSTBUFFER_ARRAY_HANDLE that can hold buffer_count buffers. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_01_010: [ constbuffer_array_create shall clone the buffers in buffers and store them. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_01_011: [ On success constbuffer_array_create shall return a non-NULL handle. ]*/
TEST_FUNCTION(constbuffer_array_create_succeeds)
{
    ///arrange
    CONSTBUFFER_HANDLE test_buffers[2];
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array;

    test_buffers[0] = TEST_CONSTBUFFER_HANDLE_1;
    test_buffers[1] = TEST_CONSTBUFFER_HANDLE_2;

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(CONSTBUFFER_IncRef(TEST_CONSTBUFFER_HANDLE_1));
    STRICT_EXPECTED_CALL(CONSTBUFFER_IncRef(TEST_CONSTBUFFER_HANDLE_2));

    ///act
    constbuffer_array = constbuffer_array_create(test_buffers, sizeof(test_buffers) / sizeof(test_buffers[0]));

    ///assert
    ASSERT_IS_NOT_NULL(constbuffer_array);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///clean
    constbuffer_array_dec_ref(constbuffer_array);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_012: [ If buffers is NULL and buffer_count is not 0, constbuffer_array_create shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_create_with_NULL_buffers_fails)
{
    ///arrange
    CONSTBUFFER_HANDLE test_buffers[2];
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array;

    test_buffers[0] = TEST_CONSTBUFFER_HANDLE_1;
    test_buffers[1] = TEST_CONSTBUFFER_HANDLE_2;

    ///act
    constbuffer_array = constbuffer_array_create(NULL, sizeof(test_buffers) / sizeof(test_buffers[0]));

    ///assert
    ASSERT_IS_NULL(constbuffer_array);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_009: [ constbuffer_array_create shall allocate memory for a new CONSTBUFFER_ARRAY_HANDLE that can hold buffer_count buffers. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_01_010: [ constbuffer_array_create shall clone the buffers in buffers and store them. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_01_011: [ On success constbuffer_array_create shall return a non-NULL handle. ]*/
TEST_FUNCTION(constbuffer_array_create_with_0_buffer_count_succeeds)
{
    ///arrange
    CONSTBUFFER_HANDLE test_buffers[1];
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array;

    test_buffers[0] = TEST_CONSTBUFFER_HANDLE_1;

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    ///act
    constbuffer_array = constbuffer_array_create(test_buffers, 0);

    ///assert
    ASSERT_IS_NOT_NULL(constbuffer_array);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///clean
    constbuffer_array_dec_ref(constbuffer_array);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_014: [ If any error occurs, constbuffer_array_create shall fail and return NULL. ]*/
TEST_FUNCTION(when_underlying_calls_fail_constbuffer_array_create_fails)
{
    ///arrange
    CONSTBUFFER_HANDLE test_buffers[2];
    size_t i;

    test_buffers[0] = TEST_CONSTBUFFER_HANDLE_1;
    test_buffers[1] = TEST_CONSTBUFFER_HANDLE_2;

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(CONSTBUFFER_IncRef(TEST_CONSTBUFFER_HANDLE_1));
    STRICT_EXPECTED_CALL(CONSTBUFFER_IncRef(TEST_CONSTBUFFER_HANDLE_2));

    umock_c_negative_tests_snapshot();
    for (i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        if (umock_c_negative_tests_can_call_fail(i))
        {
            CONSTBUFFER_ARRAY_HANDLE constbuffer_array;

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            ///act
            constbuffer_array = constbuffer_array_create(test_buffers, sizeof(test_buffers) / sizeof(test_buffers[0]));

            ///assert
            ASSERT_IS_NULL(constbuffer_array);
        }
    }
}

/* constbuffer_array_create_with_move_buffers */

/* Tests_SRS_CONSTBUFFER_ARRAY_01_028: [ If buffers is NULL and buffer_count is not 0, constbuffer_array_create_with_move_buffers shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_create_with_move_buffers_with_NULL_buffers_fails)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array;

    ///act
    constbuffer_array = constbuffer_array_create_with_move_buffers(NULL, 1);

    ///assert
    ASSERT_IS_NULL(constbuffer_array);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_029: [ Otherwise, constbuffer_array_create_with_move_buffers shall allocate memory for a new CONSTBUFFER_ARRAY_HANDLE that holds the const buffers in buffers. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_01_031: [ On success constbuffer_array_create_with_move_buffers shall return a non-NULL handle. ]*/
TEST_FUNCTION(constbuffer_array_create_with_move_buffers_succeeds)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array;
    CONSTBUFFER_HANDLE* test_buffers = (CONSTBUFFER_HANDLE*)my_gballoc_malloc(sizeof(CONSTBUFFER_HANDLE) * 2);

    CONSTBUFFER_IncRef(TEST_CONSTBUFFER_HANDLE_1);
    CONSTBUFFER_IncRef(TEST_CONSTBUFFER_HANDLE_2);

    test_buffers[0] = TEST_CONSTBUFFER_HANDLE_1;
    test_buffers[1] = TEST_CONSTBUFFER_HANDLE_2;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));

    ///act
    constbuffer_array = constbuffer_array_create_with_move_buffers(test_buffers, 2);

    ///assert
    ASSERT_IS_NOT_NULL(constbuffer_array);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    constbuffer_array_dec_ref(constbuffer_array);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_030: [ If any error occurs, constbuffer_array_create_with_move_buffers shall fail and return NULL. ]*/
TEST_FUNCTION(when_underlying_calls_fail_constbuffer_array_create_with_move_buffers_also_fails)
{
    ///arrange
    CONSTBUFFER_HANDLE* test_buffers = (CONSTBUFFER_HANDLE*)my_gballoc_malloc(sizeof(CONSTBUFFER_HANDLE) * 2);
    size_t i;

    CONSTBUFFER_IncRef(TEST_CONSTBUFFER_HANDLE_1);
    CONSTBUFFER_IncRef(TEST_CONSTBUFFER_HANDLE_2);

    test_buffers[0] = TEST_CONSTBUFFER_HANDLE_1;
    test_buffers[1] = TEST_CONSTBUFFER_HANDLE_2;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));

    umock_c_negative_tests_snapshot();
    for (i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        if (umock_c_negative_tests_can_call_fail(i))
        {
            CONSTBUFFER_ARRAY_HANDLE constbuffer_array;

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            ///act
            constbuffer_array = constbuffer_array_create_with_move_buffers(test_buffers, 2);

            ///assert
            ASSERT_IS_NULL(constbuffer_array, "On failed call %lu", (unsigned long)i);
        }
    }

    CONSTBUFFER_DecRef(TEST_CONSTBUFFER_HANDLE_1);
    CONSTBUFFER_DecRef(TEST_CONSTBUFFER_HANDLE_2);
    free(test_buffers);
}

/* constbuffer_array_create_empty */

/*Tests_SRS_CONSTBUFFER_ARRAY_02_004: [ constbuffer_array_create_empty shall allocate memory for a new CONSTBUFFER_ARRAY_HANDLE. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_02_041: [ constbuffer_array_create_empty shall succeed and return a non-NULL value. ]*/
TEST_FUNCTION(constbuffer_array_create_empty_succeeds)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array;
    constbuffer_array_create_empty_inert_path();

    ///act
    constbuffer_array = constbuffer_array_create_empty();

    ///assert
    ASSERT_IS_NOT_NULL(constbuffer_array);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///clean
    constbuffer_array_dec_ref(constbuffer_array);
}

/*Tests_SRS_CONSTBUFFER_ARRAY_02_001: [ If are any failure is encountered, constbuffer_array_create_empty shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_create_empty_unhappy_paths)
{
    ///arrange
    size_t i;
    constbuffer_array_create_empty_inert_path();

    umock_c_negative_tests_snapshot();
    for (i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        CONSTBUFFER_ARRAY_HANDLE constbuffer_array;

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        ///act
        constbuffer_array = constbuffer_array_create_empty();

        ///assert
        ASSERT_IS_NULL(constbuffer_array);
    }
}

static CONSTBUFFER_ARRAY_HANDLE TEST_constbuffer_array_create_empty(void)
{
    CONSTBUFFER_ARRAY_HANDLE result;
    constbuffer_array_create_empty_inert_path();
    result = constbuffer_array_create_empty();
    ASSERT_IS_NOT_NULL(result);
    umock_c_reset_all_calls();
    return result;
}

static CONSTBUFFER_ARRAY_HANDLE TEST_constbuffer_array_create(uint32_t size, uint32_t start_buffer)
{
    static CONSTBUFFER_HANDLE all_buffers[6];
    CONSTBUFFER_ARRAY_HANDLE result;

    all_buffers[0] = TEST_CONSTBUFFER_HANDLE_1;
    all_buffers[1] = TEST_CONSTBUFFER_HANDLE_2;
    all_buffers[2] = TEST_CONSTBUFFER_HANDLE_3;
    all_buffers[3] = TEST_CONSTBUFFER_HANDLE_4;
    all_buffers[4] = TEST_CONSTBUFFER_HANDLE_5;
    all_buffers[5] = TEST_CONSTBUFFER_HANDLE_6;

    result = constbuffer_array_create(&all_buffers[start_buffer], size);
    ASSERT_IS_NOT_NULL(result);

    umock_c_reset_all_calls();
    return result;
}

static CONSTBUFFER_ARRAY_HANDLE TEST_constbuffer_array_add_front(CONSTBUFFER_ARRAY_HANDLE constbuffer_array, uint32_t nExistingBuffers, CONSTBUFFER_HANDLE constbuffer_handle)
{
    uint32_t i;
    CONSTBUFFER_ARRAY_HANDLE result;

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    for (i = 0; i < nExistingBuffers; i++)
    {
        STRICT_EXPECTED_CALL(CONSTBUFFER_IncRef(constbuffer_handle));
    }

    result = constbuffer_array_add_front(constbuffer_array, constbuffer_handle);
    ASSERT_IS_NOT_NULL(result);
    umock_c_reset_all_calls();
    return result;
}

static CONSTBUFFER_ARRAY_HANDLE TEST_constbuffer_array_remove_front(CONSTBUFFER_ARRAY_HANDLE constbuffer_array, uint32_t nExistingBuffers, CONSTBUFFER_HANDLE* constbuffer_handle)
{
    uint32_t i;
    CONSTBUFFER_ARRAY_HANDLE result;

    ASSERT_IS_TRUE(nExistingBuffers > 0);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    for (i = 0; i < nExistingBuffers-1; i++)
    {
        STRICT_EXPECTED_CALL(CONSTBUFFER_IncRef(IGNORED_PTR_ARG));
    }

    result = constbuffer_array_remove_front(constbuffer_array, constbuffer_handle);
    ASSERT_IS_NOT_NULL(result);
    umock_c_reset_all_calls();
    return result;
}

static void TEST_constbuffer_array_dec_ref(CONSTBUFFER_ARRAY_HANDLE constbuffer_array, uint32_t nExistingBuffers)
{
    uint32_t i;
    for (i = 0; i < nExistingBuffers; i++)
    {
        STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(IGNORED_PTR_ARG));
    }

    STRICT_EXPECTED_CALL(gballoc_free(constbuffer_array));
    constbuffer_array_dec_ref(constbuffer_array);
    umock_c_reset_all_calls();
}

/* constbuffer_array_create_from_array_array */

static void constbuffer_array_create_from_array_array_inert_path(uint32_t existing_item_count)
{
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    for (uint32_t i = 0; i < existing_item_count; i++)
    {
        STRICT_EXPECTED_CALL(CONSTBUFFER_IncRef(IGNORED_PTR_ARG));
    }
}

static void validate_sorted_constbuffer_array(CONSTBUFFER_ARRAY_HANDLE constbuffer_array, uint32_t size)
{
    static CONSTBUFFER_HANDLE all_buffers[6];
    uint32_t count;
    int count_result;

    all_buffers[0] = TEST_CONSTBUFFER_HANDLE_1;
    all_buffers[1] = TEST_CONSTBUFFER_HANDLE_2;
    all_buffers[2] = TEST_CONSTBUFFER_HANDLE_3;
    all_buffers[3] = TEST_CONSTBUFFER_HANDLE_4;
    all_buffers[4] = TEST_CONSTBUFFER_HANDLE_5;
    all_buffers[5] = TEST_CONSTBUFFER_HANDLE_6;

    ASSERT_IS_TRUE(size <= 6, "Invalid test, not enough test buffers defined");

    count_result = constbuffer_array_get_buffer_count(constbuffer_array, &count);
    ASSERT_ARE_EQUAL(int, 0, count_result);
    ASSERT_ARE_EQUAL(uint32_t, size, count);

    for (uint32_t i = 0; i < size; i++)
    {
        CONSTBUFFER_HANDLE temp = constbuffer_array_get_buffer(constbuffer_array, i);
        ASSERT_ARE_EQUAL(void_ptr, all_buffers[i], temp, "Validate result[%" PRIu32 "]", i);
        CONSTBUFFER_DecRef(temp);
    }
}

/*Tests_SRS_CONSTBUFFER_ARRAY_42_009: [ If buffer_arrays is NULL and buffer_array_count is not 0 then constbuffer_array_create_from_array_array shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_create_from_array_array_returns_null_when_buffer_arrays_is_null_and_count_non_zero)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE result;

    ///act
    result = constbuffer_array_create_from_array_array(NULL, 1);

    ///assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_CONSTBUFFER_ARRAY_42_001: [ If buffer_arrays is NULL or buffer_array_count is 0 then constbuffer_array_create_from_array_array shall create a new, empty CONSTBUFFER_ARRAY_HANDLE. ]*/
TEST_FUNCTION(constbuffer_array_create_from_array_array_returns_empty_array_when_buffer_arrays_is_null_and_count_zero)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE result;

    constbuffer_array_create_empty_inert_path();

    ///act
    result = constbuffer_array_create_from_array_array(NULL, 0);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    validate_sorted_constbuffer_array(result, 0);

    ///clean
    constbuffer_array_dec_ref(result);
}

/*Tests_SRS_CONSTBUFFER_ARRAY_42_001: [ If buffer_arrays is NULL or buffer_array_count is 0 then constbuffer_array_create_from_array_array shall create a new, empty CONSTBUFFER_ARRAY_HANDLE. ]*/
TEST_FUNCTION(constbuffer_array_create_from_array_array_returns_empty_array_when_count_zero)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    CONSTBUFFER_ARRAY_HANDLE empty_array = TEST_constbuffer_array_create_empty();

    constbuffer_array_create_empty_inert_path();

    ///act
    result = constbuffer_array_create_from_array_array(&empty_array, 0);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    validate_sorted_constbuffer_array(result, 0);

    ///clean
    constbuffer_array_dec_ref(result);
    constbuffer_array_dec_ref(empty_array);
}

/*Tests_SRS_CONSTBUFFER_ARRAY_42_002: [ If any const buffer array in buffer_arrays is NULL then constbuffer_array_create_from_array_array shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_create_from_array_array_returns_null_when_buffer_array_contains_only_null)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    CONSTBUFFER_ARRAY_HANDLE buffer_array_null[1] = { NULL };

    ///act
    result = constbuffer_array_create_from_array_array(buffer_array_null, 1);

    ///assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_CONSTBUFFER_ARRAY_42_002: [ If any const buffer array in buffer_arrays is NULL then constbuffer_array_create_from_array_array shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_create_from_array_array_returns_null_when_buffer_array_contains_valid_and_null_arrays)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    CONSTBUFFER_ARRAY_HANDLE empty_array = TEST_constbuffer_array_create_empty();
    CONSTBUFFER_ARRAY_HANDLE buffer_array[] = { NULL, NULL };
    buffer_array[0] = empty_array;

    ///act
    result = constbuffer_array_create_from_array_array(buffer_array, 2);

    ///assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///clean
    constbuffer_array_dec_ref(empty_array);
}

/*Tests_SRS_CONSTBUFFER_ARRAY_42_007: [ constbuffer_array_create_from_array_array shall succeed and return a non-NULL value. ]*/
TEST_FUNCTION(constbuffer_array_create_from_array_array_with_two_empty_arrays_succeeds)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    const uint32_t array_count = 2;
    CONSTBUFFER_ARRAY_HANDLE buffer_array[2];
    buffer_array[0] = TEST_constbuffer_array_create_empty();
    buffer_array[1] = TEST_constbuffer_array_create_empty();

    constbuffer_array_create_from_array_array_inert_path(0);

    ///act
    result = constbuffer_array_create_from_array_array(buffer_array, array_count);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    validate_sorted_constbuffer_array(result, 0);

    ///clean
    constbuffer_array_dec_ref(result);
    for (uint32_t i = 0; i < array_count; ++i)
    {
        constbuffer_array_dec_ref(buffer_array[i]);
    }
}

/*Tests_SRS_CONSTBUFFER_ARRAY_42_007: [ constbuffer_array_create_from_array_array shall succeed and return a non-NULL value. ]*/
TEST_FUNCTION(constbuffer_array_create_from_array_array_with_three_empty_arrays_succeeds)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    const uint32_t array_count = 3;
    CONSTBUFFER_ARRAY_HANDLE buffer_array[3];
    buffer_array[0] = TEST_constbuffer_array_create_empty();
    buffer_array[1] = TEST_constbuffer_array_create_empty();
    buffer_array[2] = TEST_constbuffer_array_create_empty();

    constbuffer_array_create_from_array_array_inert_path(0);

    ///act
    result = constbuffer_array_create_from_array_array(buffer_array, array_count);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    validate_sorted_constbuffer_array(result, 0);

    ///clean
    constbuffer_array_dec_ref(result);
    for (uint32_t i = 0; i < array_count; ++i)
    {
        constbuffer_array_dec_ref(buffer_array[i]);
    }
}

/*Tests_SRS_CONSTBUFFER_ARRAY_42_003: [ constbuffer_array_create_from_array_array shall allocate memory to hold all of the CONSTBUFFER_HANDLES from buffer_arrays. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_42_004: [ constbuffer_array_create_from_array_array shall copy all of the CONSTBUFFER_HANDLES from each const buffer array in buffer_arrays to the newly constructed array by calling CONSTBUFFER_IncRef. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_42_007: [ constbuffer_array_create_from_array_array shall succeed and return a non-NULL value. ]*/
TEST_FUNCTION(constbuffer_array_create_from_array_array_with_empty_array_and_1_element_array_succeeds)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    const uint32_t array_count = 2;
    CONSTBUFFER_ARRAY_HANDLE buffer_array[2];
    buffer_array[0] = TEST_constbuffer_array_create_empty();
    buffer_array[1] = TEST_constbuffer_array_create(1, 0);

    constbuffer_array_create_from_array_array_inert_path(1);

    ///act
    result = constbuffer_array_create_from_array_array(buffer_array, array_count);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    validate_sorted_constbuffer_array(result, 1);

    ///clean
    constbuffer_array_dec_ref(result);
    for (uint32_t i = 0; i < array_count; ++i)
    {
        constbuffer_array_dec_ref(buffer_array[i]);
    }
}

/*Tests_SRS_CONSTBUFFER_ARRAY_42_003: [ constbuffer_array_create_from_array_array shall allocate memory to hold all of the CONSTBUFFER_HANDLES from buffer_arrays. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_42_004: [ constbuffer_array_create_from_array_array shall copy all of the CONSTBUFFER_HANDLES from each const buffer array in buffer_arrays to the newly constructed array by calling CONSTBUFFER_IncRef. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_42_007: [ constbuffer_array_create_from_array_array shall succeed and return a non-NULL value. ]*/
TEST_FUNCTION(constbuffer_array_create_from_array_array_with_1_element_array_and_empty_array_succeeds)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    const uint32_t array_count = 2;
    CONSTBUFFER_ARRAY_HANDLE buffer_array[2];
    buffer_array[0] = TEST_constbuffer_array_create(1, 0);
    buffer_array[1] = TEST_constbuffer_array_create_empty();

    constbuffer_array_create_from_array_array_inert_path(1);

    ///act
    result = constbuffer_array_create_from_array_array(buffer_array, array_count);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    validate_sorted_constbuffer_array(result, 1);

    ///clean
    constbuffer_array_dec_ref(result);
    for (uint32_t i = 0; i < array_count; ++i)
    {
        constbuffer_array_dec_ref(buffer_array[i]);
    }
}

/*Tests_SRS_CONSTBUFFER_ARRAY_42_003: [ constbuffer_array_create_from_array_array shall allocate memory to hold all of the CONSTBUFFER_HANDLES from buffer_arrays. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_42_004: [ constbuffer_array_create_from_array_array shall copy all of the CONSTBUFFER_HANDLES from each const buffer array in buffer_arrays to the newly constructed array by calling CONSTBUFFER_IncRef. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_42_007: [ constbuffer_array_create_from_array_array shall succeed and return a non-NULL value. ]*/
TEST_FUNCTION(constbuffer_array_create_from_array_array_with_2_1_element_arrays_succeeds)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    const uint32_t array_count = 2;
    CONSTBUFFER_ARRAY_HANDLE buffer_array[2];
    buffer_array[0] = TEST_constbuffer_array_create(1, 0);
    buffer_array[1] = TEST_constbuffer_array_create(1, 1);

    constbuffer_array_create_from_array_array_inert_path(2);

    ///act
    result = constbuffer_array_create_from_array_array(buffer_array, array_count);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    validate_sorted_constbuffer_array(result, 2);

    ///clean
    constbuffer_array_dec_ref(result);
    for (uint32_t i = 0; i < array_count; ++i)
    {
        constbuffer_array_dec_ref(buffer_array[i]);
    }
}

/*Tests_SRS_CONSTBUFFER_ARRAY_42_003: [ constbuffer_array_create_from_array_array shall allocate memory to hold all of the CONSTBUFFER_HANDLES from buffer_arrays. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_42_004: [ constbuffer_array_create_from_array_array shall copy all of the CONSTBUFFER_HANDLES from each const buffer array in buffer_arrays to the newly constructed array by calling CONSTBUFFER_IncRef. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_42_007: [ constbuffer_array_create_from_array_array shall succeed and return a non-NULL value. ]*/
TEST_FUNCTION(constbuffer_array_create_from_array_array_with_3_1_element_arrays_succeeds)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    const uint32_t array_count = 3;
    CONSTBUFFER_ARRAY_HANDLE buffer_array[3];
    buffer_array[0] = TEST_constbuffer_array_create(1, 0);
    buffer_array[1] = TEST_constbuffer_array_create(1, 1);
    buffer_array[2] = TEST_constbuffer_array_create(1, 2);

    constbuffer_array_create_from_array_array_inert_path(3);

    ///act
    result = constbuffer_array_create_from_array_array(buffer_array, array_count);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    validate_sorted_constbuffer_array(result, 3);

    ///clean
    constbuffer_array_dec_ref(result);
    for (uint32_t i = 0; i < array_count; ++i)
    {
        constbuffer_array_dec_ref(buffer_array[i]);
    }
}

/*Tests_SRS_CONSTBUFFER_ARRAY_42_003: [ constbuffer_array_create_from_array_array shall allocate memory to hold all of the CONSTBUFFER_HANDLES from buffer_arrays. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_42_004: [ constbuffer_array_create_from_array_array shall copy all of the CONSTBUFFER_HANDLES from each const buffer array in buffer_arrays to the newly constructed array by calling CONSTBUFFER_IncRef. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_42_007: [ constbuffer_array_create_from_array_array shall succeed and return a non-NULL value. ]*/
TEST_FUNCTION(constbuffer_array_create_from_array_array_with_2_2_element_arrays_succeeds)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    const uint32_t array_count = 2;
    CONSTBUFFER_ARRAY_HANDLE buffer_array[2];
    buffer_array[0] = TEST_constbuffer_array_create(2, 0);
    buffer_array[1] = TEST_constbuffer_array_create(2, 2);

    constbuffer_array_create_from_array_array_inert_path(4);

    ///act
    result = constbuffer_array_create_from_array_array(buffer_array, array_count);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    validate_sorted_constbuffer_array(result, 4);

    ///clean
    constbuffer_array_dec_ref(result);
    for (uint32_t i = 0; i < array_count; ++i)
    {
        constbuffer_array_dec_ref(buffer_array[i]);
    }
}

/*Tests_SRS_CONSTBUFFER_ARRAY_42_003: [ constbuffer_array_create_from_array_array shall allocate memory to hold all of the CONSTBUFFER_HANDLES from buffer_arrays. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_42_004: [ constbuffer_array_create_from_array_array shall copy all of the CONSTBUFFER_HANDLES from each const buffer array in buffer_arrays to the newly constructed array by calling CONSTBUFFER_IncRef. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_42_007: [ constbuffer_array_create_from_array_array shall succeed and return a non-NULL value. ]*/
TEST_FUNCTION(constbuffer_array_create_from_array_array_with_3_2_element_arrays_succeeds)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    const uint32_t array_count = 3;
    CONSTBUFFER_ARRAY_HANDLE buffer_array[3];
    buffer_array[0] = TEST_constbuffer_array_create(2, 0);
    buffer_array[1] = TEST_constbuffer_array_create(2, 2);
    buffer_array[2] = TEST_constbuffer_array_create(2, 4);

    constbuffer_array_create_from_array_array_inert_path(6);

    ///act
    result = constbuffer_array_create_from_array_array(buffer_array, array_count);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    validate_sorted_constbuffer_array(result, 6);

    ///clean
    constbuffer_array_dec_ref(result);
    for (uint32_t i = 0; i < array_count; ++i)
    {
        constbuffer_array_dec_ref(buffer_array[i]);
    }
}

/*Tests_SRS_CONSTBUFFER_ARRAY_42_003: [ constbuffer_array_create_from_array_array shall allocate memory to hold all of the CONSTBUFFER_HANDLES from buffer_arrays. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_42_004: [ constbuffer_array_create_from_array_array shall copy all of the CONSTBUFFER_HANDLES from each const buffer array in buffer_arrays to the newly constructed array by calling CONSTBUFFER_IncRef. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_42_007: [ constbuffer_array_create_from_array_array shall succeed and return a non-NULL value. ]*/
TEST_FUNCTION(constbuffer_array_create_from_array_array_with_3_arrays_of_size_1_2_3_succeeds)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    const uint32_t array_count = 3;
    CONSTBUFFER_ARRAY_HANDLE buffer_array[3];
    buffer_array[0] = TEST_constbuffer_array_create(1, 0);
    buffer_array[1] = TEST_constbuffer_array_create(2, 1);
    buffer_array[2] = TEST_constbuffer_array_create(3, 3);

    constbuffer_array_create_from_array_array_inert_path(6);

    ///act
    result = constbuffer_array_create_from_array_array(buffer_array, array_count);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    validate_sorted_constbuffer_array(result, 6);

    ///clean
    constbuffer_array_dec_ref(result);
    for (uint32_t i = 0; i < array_count; ++i)
    {
        constbuffer_array_dec_ref(buffer_array[i]);
    }
}

/*Tests_SRS_CONSTBUFFER_ARRAY_42_003: [ constbuffer_array_create_from_array_array shall allocate memory to hold all of the CONSTBUFFER_HANDLES from buffer_arrays. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_42_004: [ constbuffer_array_create_from_array_array shall copy all of the CONSTBUFFER_HANDLES from each const buffer array in buffer_arrays to the newly constructed array by calling CONSTBUFFER_IncRef. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_42_007: [ constbuffer_array_create_from_array_array shall succeed and return a non-NULL value. ]*/
TEST_FUNCTION(constbuffer_array_create_from_array_array_with_2_2_element_arrays_same_pointer_succeeds)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    const uint32_t array_count = 2;
    CONSTBUFFER_ARRAY_HANDLE test_array = TEST_constbuffer_array_create(2, 0);
    CONSTBUFFER_ARRAY_HANDLE buffer_array[2];
    CONSTBUFFER_HANDLE temp_buffer;
    uint32_t count;
    int count_result;
    buffer_array[0] = test_array;
    buffer_array[1] = test_array;

    constbuffer_array_create_from_array_array_inert_path(4);

    ///act
    result = constbuffer_array_create_from_array_array(buffer_array, array_count);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    count_result = constbuffer_array_get_buffer_count(result, &count);
    ASSERT_ARE_EQUAL(int, 0, count_result);
    ASSERT_ARE_EQUAL(uint32_t, 4, count);

    temp_buffer = constbuffer_array_get_buffer(result, 0);
    ASSERT_ARE_EQUAL(void_ptr, TEST_CONSTBUFFER_HANDLE_1, temp_buffer, "Validate result[0]");
    CONSTBUFFER_DecRef(temp_buffer);
    temp_buffer = constbuffer_array_get_buffer(result, 1);
    ASSERT_ARE_EQUAL(void_ptr, TEST_CONSTBUFFER_HANDLE_2, temp_buffer, "Validate result[1]");
    CONSTBUFFER_DecRef(temp_buffer);
    temp_buffer = constbuffer_array_get_buffer(result, 2);
    ASSERT_ARE_EQUAL(void_ptr, TEST_CONSTBUFFER_HANDLE_1, temp_buffer, "Validate result[2]");
    CONSTBUFFER_DecRef(temp_buffer);
    temp_buffer = constbuffer_array_get_buffer(result, 3);
    ASSERT_ARE_EQUAL(void_ptr, TEST_CONSTBUFFER_HANDLE_2, temp_buffer, "Validate result[3]");
    CONSTBUFFER_DecRef(temp_buffer);

    ///clean
    constbuffer_array_dec_ref(result);
    constbuffer_array_dec_ref(test_array);
}

/*Tests_SRS_CONSTBUFFER_ARRAY_42_008: [ If there are any failures then constbuffer_array_create_from_array_array shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_create_from_array_array_fails_if_malloc_fails)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    const uint32_t array_count = 2;
    CONSTBUFFER_ARRAY_HANDLE buffer_array[2];
    buffer_array[0] = TEST_constbuffer_array_create(2, 0);
    buffer_array[1] = TEST_constbuffer_array_create(2, 2);

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    ///act
    result = constbuffer_array_create_from_array_array(buffer_array, array_count);

    ///assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///clean
    for (uint32_t i = 0; i < array_count; ++i)
    {
        constbuffer_array_dec_ref(buffer_array[i]);
    }
}

/*constbuffer_array_add_front*/

/*Tests_SRS_CONSTBUFFER_ARRAY_02_006: [ If constbuffer_array_handle is NULL then constbuffer_array_add_front shall fail and return NULL ]*/
TEST_FUNCTION(constbuffer_array_add_front_with_constbuffer_array_handle_NULL_fails)
{
    ///arrange

    ///act
    CONSTBUFFER_ARRAY_HANDLE result = constbuffer_array_add_front(NULL, TEST_CONSTBUFFER_HANDLE_1);

    ///assert
    ASSERT_IS_NULL(result);
}

/*Tests_SRS_CONSTBUFFER_ARRAY_02_007: [ If constbuffer_handle is NULL then constbuffer_array_add_front shall fail and return NULL ]*/
TEST_FUNCTION(constbuffer_array_add_front_with_constbuffer_handle_NULL_fails)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE TEST_CONSTBUFFER_ARRAY_HANDLE = TEST_constbuffer_array_create_empty();

    ///act
    CONSTBUFFER_ARRAY_HANDLE result = constbuffer_array_add_front(TEST_CONSTBUFFER_ARRAY_HANDLE, NULL);

    ///assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///clean
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
}

static void constbuffer_array_add_front_inert_path(void)
{
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(CONSTBUFFER_IncRef(TEST_CONSTBUFFER_HANDLE_1));
}

/*Tests_SRS_CONSTBUFFER_ARRAY_02_042: [ constbuffer_array_add_front shall allocate enough memory to hold all of constbuffer_array_handle existing CONSTBUFFER_HANDLE and constbuffer_handle. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_02_043: [ constbuffer_array_add_front shall copy constbuffer_handle and all of constbuffer_array_handle existing CONSTBUFFER_HANDLE. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_02_044: [ constbuffer_array_add_front shall inc_ref all the CONSTBUFFER_HANDLE it had copied. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_02_010: [ constbuffer_array_add_front shall succeed and return a non-NULL value. ]*/
TEST_FUNCTION(constbuffer_array_add_front_succeeds)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE TEST_CONSTBUFFER_ARRAY_HANDLE = TEST_constbuffer_array_create_empty();
    CONSTBUFFER_ARRAY_HANDLE result;

    constbuffer_array_add_front_inert_path();

    ///act
    result = constbuffer_array_add_front(TEST_CONSTBUFFER_ARRAY_HANDLE, TEST_CONSTBUFFER_HANDLE_1);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///clean
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
    constbuffer_array_dec_ref(result);
}

/*Tests_SRS_CONSTBUFFER_ARRAY_02_011: [ If there any failures constbuffer_array_add_front shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_add_front_unhappy_paths)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE TEST_CONSTBUFFER_ARRAY_HANDLE = TEST_constbuffer_array_create_empty();
    size_t i;

    constbuffer_array_add_front_inert_path();

    umock_c_negative_tests_snapshot();
    for (i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        if (umock_c_negative_tests_can_call_fail(i))
        {
            CONSTBUFFER_ARRAY_HANDLE result;

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            ///act
            result = constbuffer_array_add_front(TEST_CONSTBUFFER_ARRAY_HANDLE, TEST_CONSTBUFFER_HANDLE_1);

            ///assert
            ASSERT_IS_NULL(result);
        }
    }

    ///clean
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
}

/*Tests_SRS_CONSTBUFFER_ARRAY_02_012: [ If constbuffer_array_handle is NULL then constbuffer_array_remove_front shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_remove_front_with_constbuffer_array_handle_NULL_fails)
{
    ///arrange
    CONSTBUFFER_HANDLE constbuffer_handle;

    ///act
    CONSTBUFFER_ARRAY_HANDLE result = constbuffer_array_remove_front(NULL, &constbuffer_handle);

    ///assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_CONSTBUFFER_ARRAY_02_045: [ If constbuffer_handle is NULL then constbuffer_array_remove_front shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_remove_front_with_constbuffer_handle_NULL_fails)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE TEST_CONSTBUFFER_ARRAY_HANDLE = TEST_constbuffer_array_create_empty();

    ///act
    CONSTBUFFER_ARRAY_HANDLE result = constbuffer_array_remove_front(TEST_CONSTBUFFER_ARRAY_HANDLE, NULL);

    ///assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///clean
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
}

/*Tests_SRS_CONSTBUFFER_ARRAY_02_002: [ constbuffer_array_remove_front shall fail when called on a newly constructed CONSTBUFFER_ARRAY_HANDLE. ]*/
TEST_FUNCTION(constbuffer_array_remove_front_with_constbuffer_array_handle_empty_fails)
{
    ///arrange
    CONSTBUFFER_HANDLE constbuffer_handle;
    CONSTBUFFER_ARRAY_HANDLE TEST_CONSTBUFFER_ARRAY_HANDLE = TEST_constbuffer_array_create_empty();

    ///act
    CONSTBUFFER_ARRAY_HANDLE result = constbuffer_array_remove_front(TEST_CONSTBUFFER_ARRAY_HANDLE, &constbuffer_handle);

    ///assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
}

/*Tests_SRS_CONSTBUFFER_ARRAY_02_013: [ If there is no front CONSTBUFFER_HANDLE then constbuffer_array_remove_front shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_remove_front_with_constbuffer_array_handle_empty_fails_2)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE TEST_CONSTBUFFER_ARRAY_HANDLE = TEST_constbuffer_array_create_empty();
    CONSTBUFFER_ARRAY_HANDLE afterAdd = TEST_constbuffer_array_add_front(TEST_CONSTBUFFER_ARRAY_HANDLE, 0, TEST_CONSTBUFFER_HANDLE_1);
    CONSTBUFFER_HANDLE removed;
    CONSTBUFFER_ARRAY_HANDLE afterRemove = TEST_constbuffer_array_remove_front(afterAdd, 1, &removed); /*maybe this is a different kind of empty*/ /*shrugs*/
    CONSTBUFFER_HANDLE removed2;
    CONSTBUFFER_ARRAY_HANDLE result;
    CONSTBUFFER_DecRef(removed);
    TEST_constbuffer_array_dec_ref(afterAdd, 1);
    umock_c_reset_all_calls();

    ///act
    result = constbuffer_array_remove_front(afterRemove, &removed2);

    ///assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
    constbuffer_array_dec_ref(afterRemove);
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
}

static void constbuffer_array_remove_front_inert_path(uint32_t nExistingItems)
{
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    // clone front buffer
    STRICT_EXPECTED_CALL(CONSTBUFFER_IncRef(IGNORED_PTR_ARG));
    if (nExistingItems > 0)
    {
        uint32_t i;
        for (i = 0; i < nExistingItems-1; i++)
        {
            STRICT_EXPECTED_CALL(CONSTBUFFER_IncRef(IGNORED_PTR_ARG));
        }
    }
}

/*Tests_SRS_CONSTBUFFER_ARRAY_02_046: [ constbuffer_array_remove_front shall allocate memory to hold all of constbuffer_array_handle CONSTBUFFER_HANDLEs except the front one. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_02_047: [ constbuffer_array_remove_front shall copy all of constbuffer_array_handle CONSTBUFFER_HANDLEs except the front one. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_02_048: [ constbuffer_array_remove_front shall inc_ref all the copied CONSTBUFFER_HANDLEs. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_01_001: [ constbuffer_array_remove_front shall inc_ref the removed buffer. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_02_049: [ constbuffer_array_remove_front shall succeed, write in constbuffer_handle the front handle and return a non-NULL value. ]*/
TEST_FUNCTION(constbuffer_array_remove_front_with_1_item_succeeds)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE TEST_CONSTBUFFER_ARRAY_HANDLE = TEST_constbuffer_array_create_empty();
    CONSTBUFFER_ARRAY_HANDLE afterAdd =TEST_constbuffer_array_add_front(TEST_CONSTBUFFER_ARRAY_HANDLE, 0, TEST_CONSTBUFFER_HANDLE_1);
    CONSTBUFFER_HANDLE removed;
    CONSTBUFFER_ARRAY_HANDLE afterRemove;

    umock_c_reset_all_calls();

    constbuffer_array_remove_front_inert_path(1);

    ///act
    afterRemove = constbuffer_array_remove_front(afterAdd, &removed);

    ///assert
    ASSERT_IS_NOT_NULL(removed);
    ASSERT_IS_NOT_NULL(afterRemove);
    ASSERT_ARE_EQUAL(void_ptr, TEST_CONSTBUFFER_HANDLE_1, removed);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
    constbuffer_array_dec_ref(afterRemove);
    constbuffer_array_dec_ref(afterAdd);
    CONSTBUFFER_DecRef(removed);
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
}

/*Tests_SRS_CONSTBUFFER_ARRAY_02_046: [ constbuffer_array_remove_front shall allocate memory to hold all of constbuffer_array_handle CONSTBUFFER_HANDLEs except the front one. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_02_047: [ constbuffer_array_remove_front shall copy all of constbuffer_array_handle CONSTBUFFER_HANDLEs except the front one. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_02_048: [ constbuffer_array_remove_front shall inc_ref all the copied CONSTBUFFER_HANDLEs. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_01_001: [ constbuffer_array_remove_front shall inc_ref the removed buffer. ]*/
/*Tests_SRS_CONSTBUFFER_ARRAY_02_049: [ constbuffer_array_remove_front shall succeed, write in constbuffer_handle the front handle and return a non-NULL value. ]*/
TEST_FUNCTION(constbuffer_array_remove_front_with_2_items_succeeds)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE TEST_CONSTBUFFER_ARRAY_HANDLE = TEST_constbuffer_array_create_empty();
    CONSTBUFFER_ARRAY_HANDLE afterAdd1 = TEST_constbuffer_array_add_front(TEST_CONSTBUFFER_ARRAY_HANDLE, 0, TEST_CONSTBUFFER_HANDLE_1);
    CONSTBUFFER_ARRAY_HANDLE afterAdd2 = TEST_constbuffer_array_add_front(afterAdd1, 1, TEST_CONSTBUFFER_HANDLE_2);
    CONSTBUFFER_HANDLE removed = NULL;
    CONSTBUFFER_ARRAY_HANDLE afterRemove1;
    umock_c_reset_all_calls();

    constbuffer_array_remove_front_inert_path(2);

    ///act
    afterRemove1 = constbuffer_array_remove_front(afterAdd2, &removed);

    ///assert
    ASSERT_IS_NOT_NULL(afterRemove1);
    ASSERT_IS_NOT_NULL(removed);
    ASSERT_ARE_EQUAL(void_ptr, TEST_CONSTBUFFER_HANDLE_2, removed);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
    constbuffer_array_dec_ref(afterRemove1);
    CONSTBUFFER_DecRef(removed);
    constbuffer_array_dec_ref(afterAdd2);
    constbuffer_array_dec_ref(afterAdd1);
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
}

/*Tests_SRS_CONSTBUFFER_ARRAY_02_036: [ If there are any failures then constbuffer_array_remove_front shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_remove_front_unhappy_paths)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE TEST_CONSTBUFFER_ARRAY_HANDLE = TEST_constbuffer_array_create_empty();
    CONSTBUFFER_ARRAY_HANDLE afterAdd = TEST_constbuffer_array_add_front(TEST_CONSTBUFFER_ARRAY_HANDLE, 0, TEST_CONSTBUFFER_HANDLE_1);
    size_t i;
    umock_c_reset_all_calls();

    constbuffer_array_remove_front_inert_path(1);

    umock_c_negative_tests_snapshot();
    for (i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        if (umock_c_negative_tests_can_call_fail(i))
        {
            CONSTBUFFER_HANDLE removed;
            CONSTBUFFER_ARRAY_HANDLE afterRemove;

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            ///act
            afterRemove = constbuffer_array_remove_front(afterAdd, &removed);

            ///assert
            ASSERT_IS_NULL(afterRemove);
        }
    }

    ///clean
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
    constbuffer_array_dec_ref(afterAdd);
}

/* constbuffer_array_get_buffer_count */

/* Tests_SRS_CONSTBUFFER_ARRAY_01_002: [ On success, constbuffer_array_get_buffer_count shall return 0 and write the buffer count in buffer_count. ]*/
TEST_FUNCTION(constbuffer_array_get_buffer_count_returns_0_for_an_empty_array)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array = TEST_constbuffer_array_create_empty();
    uint32_t buffer_count;

    // act
    int result = constbuffer_array_get_buffer_count(constbuffer_array, &buffer_count);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(uint32_t, 0, buffer_count);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    constbuffer_array_dec_ref(constbuffer_array);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_002: [ On success, constbuffer_array_get_buffer_count shall return 0 and write the buffer count in buffer_count. ]*/
TEST_FUNCTION(constbuffer_array_get_buffer_count_after_add_on_empty_array_yields_1)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array = TEST_constbuffer_array_create_empty();
    CONSTBUFFER_ARRAY_HANDLE afterAdd1 = TEST_constbuffer_array_add_front(constbuffer_array, 0, TEST_CONSTBUFFER_HANDLE_1);
    uint32_t buffer_count;

    // act
    int result = constbuffer_array_get_buffer_count(afterAdd1, &buffer_count);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(uint32_t, 1, buffer_count);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    constbuffer_array_dec_ref(afterAdd1);
    constbuffer_array_dec_ref(constbuffer_array);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_002: [ On success, constbuffer_array_get_buffer_count shall return 0 and write the buffer count in buffer_count. ]*/
TEST_FUNCTION(constbuffer_array_get_buffer_count_on_a_1_buffer_array_yields_1)
{
    // arrange
    CONSTBUFFER_HANDLE test_buffers[1];
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array;
    uint32_t buffer_count;
    int result;

    test_buffers[0] = TEST_CONSTBUFFER_HANDLE_1;

    constbuffer_array = constbuffer_array_create(test_buffers, sizeof(test_buffers) / sizeof(test_buffers[0]));
    umock_c_reset_all_calls();

    // act
    result = constbuffer_array_get_buffer_count(constbuffer_array, &buffer_count);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(uint32_t, 1, buffer_count);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    constbuffer_array_dec_ref(constbuffer_array);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_002: [ On success, constbuffer_array_get_buffer_count shall return 0 and write the buffer count in buffer_count. ]*/
TEST_FUNCTION(constbuffer_array_get_buffer_count_on_a_2_buffer_array_yields_2)
{
    // arrange
    CONSTBUFFER_HANDLE test_buffers[2];
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array;
    uint32_t buffer_count;
    int result;

    test_buffers[0] = TEST_CONSTBUFFER_HANDLE_1;
    test_buffers[1] = TEST_CONSTBUFFER_HANDLE_2;

    constbuffer_array = constbuffer_array_create(test_buffers, sizeof(test_buffers) / sizeof(test_buffers[0]));
    umock_c_reset_all_calls();

    // act
    result = constbuffer_array_get_buffer_count(constbuffer_array, &buffer_count);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(uint32_t, 2, buffer_count);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    constbuffer_array_dec_ref(constbuffer_array);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_003: [ If constbuffer_array_handle is NULL, constbuffer_array_get_buffer_count shall fail and return a non-zero value. ]*/
TEST_FUNCTION(constbuffer_array_get_buffer_count_with_NULL_constbuffer_array_handle_fails)
{
    // arrange
    uint32_t buffer_count;

    // act
    int result = constbuffer_array_get_buffer_count(NULL, &buffer_count);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_004: [ If buffer_count is NULL, constbuffer_array_get_buffer_count shall fail and return a non-zero value. ]*/
TEST_FUNCTION(constbuffer_array_get_buffer_count_with_NULL_buffer_count_fails)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array = TEST_constbuffer_array_create_empty();

    // act
    int result = constbuffer_array_get_buffer_count(constbuffer_array, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    constbuffer_array_dec_ref(constbuffer_array);
}

/* constbuffer_array_get_buffer */

/* Tests_SRS_CONSTBUFFER_ARRAY_01_005: [ On success, constbuffer_array_get_buffer shall return a non-NULL handle to the buffer_index-th const buffer in the array. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_01_006: [ The returned handle shall have its reference count incremented. ]*/
TEST_FUNCTION(constbuffer_array_get_buffer_succeeds)
{
    // arrange
    CONSTBUFFER_HANDLE test_buffers[2];
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array;
    CONSTBUFFER_HANDLE result;

    test_buffers[0] = TEST_CONSTBUFFER_HANDLE_1;
    test_buffers[1] = TEST_CONSTBUFFER_HANDLE_2;

    constbuffer_array = constbuffer_array_create(test_buffers, sizeof(test_buffers) / sizeof(test_buffers[0]));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(CONSTBUFFER_IncRef(TEST_CONSTBUFFER_HANDLE_1));

    // act
    result = constbuffer_array_get_buffer(constbuffer_array, 0);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, TEST_CONSTBUFFER_HANDLE_1, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    constbuffer_array_dec_ref(constbuffer_array);
    CONSTBUFFER_DecRef(result);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_005: [ On success, constbuffer_array_get_buffer shall return a non-NULL handle to the buffer_index-th const buffer in the array. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_01_006: [ The returned handle shall have its reference count incremented. ]*/
TEST_FUNCTION(constbuffer_array_get_buffer_for_2nd_buffer_succeeds)
{
    // arrange
    CONSTBUFFER_HANDLE test_buffers[2];
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array;
    CONSTBUFFER_HANDLE result;

    test_buffers[0] = TEST_CONSTBUFFER_HANDLE_1;
    test_buffers[1] = TEST_CONSTBUFFER_HANDLE_2;

    constbuffer_array = constbuffer_array_create(test_buffers, sizeof(test_buffers) / sizeof(test_buffers[0]));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(CONSTBUFFER_IncRef(TEST_CONSTBUFFER_HANDLE_2));

    // act
    result = constbuffer_array_get_buffer(constbuffer_array, 1);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, TEST_CONSTBUFFER_HANDLE_2, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    constbuffer_array_dec_ref(constbuffer_array);
    CONSTBUFFER_DecRef(result);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_007: [ If constbuffer_array_handle is NULL, constbuffer_array_get_buffer shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_get_buffer_with_NULL_constbuffer_array_handle_fails)
{
    // arrange
    CONSTBUFFER_HANDLE result;

    // act
    result = constbuffer_array_get_buffer(NULL, 0);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_008: [ If buffer_index is greater or equal to the number of buffers in the array, constbuffer_array_get_buffer shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_get_buffer_with_index_equal_to_number_of_buffers_fails)
{
    // arrange
    CONSTBUFFER_HANDLE test_buffers[2];
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array;
    CONSTBUFFER_HANDLE result;

    test_buffers[0] = TEST_CONSTBUFFER_HANDLE_1;
    test_buffers[1] = TEST_CONSTBUFFER_HANDLE_2;

    constbuffer_array = constbuffer_array_create(test_buffers, sizeof(test_buffers) / sizeof(test_buffers[0]));
    umock_c_reset_all_calls();

    // act
    result = constbuffer_array_get_buffer(constbuffer_array, 2);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    constbuffer_array_dec_ref(constbuffer_array);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_008: [ If buffer_index is greater or equal to the number of buffers in the array, constbuffer_array_get_buffer shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_get_buffer_with_index_grater_than_number_of_buffers_fails)
{
    // arrange
    CONSTBUFFER_HANDLE test_buffers[2];
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array;
    CONSTBUFFER_HANDLE result;

    test_buffers[0] = TEST_CONSTBUFFER_HANDLE_1;
    test_buffers[1] = TEST_CONSTBUFFER_HANDLE_2;

    constbuffer_array = constbuffer_array_create(test_buffers, sizeof(test_buffers) / sizeof(test_buffers[0]));
    umock_c_reset_all_calls();

    // act
    result = constbuffer_array_get_buffer(constbuffer_array, 3);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    constbuffer_array_dec_ref(constbuffer_array);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_008: [ If buffer_index is greater or equal to the number of buffers in the array, constbuffer_array_get_buffer shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_get_buffer_with_index_0_on_empty_array_fails)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array;
    CONSTBUFFER_HANDLE result;

    constbuffer_array = TEST_constbuffer_array_create_empty();

    // act
    result = constbuffer_array_get_buffer(constbuffer_array, 0);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    constbuffer_array_dec_ref(constbuffer_array);
}

/* constbuffer_array_get_buffer_content */

/* Tests_SRS_CONSTBUFFER_ARRAY_01_023: [ If constbuffer_array_handle is NULL, constbuffer_array_get_buffer_content shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_get_buffer_content_with_NULL_constbuffer_array_handle_fails)
{
    // arrange
    const CONSTBUFFER* result;

    // act
    result = constbuffer_array_get_buffer_content(NULL, 0);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_025: [ Otherwise constbuffer_array_get_buffer_content shall call CONSTBUFFER_GetContent for the buffer_index-th buffer and return its result. ]*/
TEST_FUNCTION(constbuffer_array_get_buffer_content_succeeds)
{
    // arrange
    CONSTBUFFER_HANDLE test_buffers[2];
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array;
    const CONSTBUFFER* result;

    test_buffers[0] = TEST_CONSTBUFFER_HANDLE_1;
    test_buffers[1] = TEST_CONSTBUFFER_HANDLE_2;

    constbuffer_array = constbuffer_array_create(test_buffers, sizeof(test_buffers) / sizeof(test_buffers[0]));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(CONSTBUFFER_GetContent(TEST_CONSTBUFFER_HANDLE_1));

    // act
    result = constbuffer_array_get_buffer_content(constbuffer_array, 0);

    // assert
    ASSERT_ARE_EQUAL(size_t, 1, result->size);
    ASSERT_ARE_EQUAL(int, 0, memcmp(&one, result->buffer, result->size));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    constbuffer_array_dec_ref(constbuffer_array);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_025: [ Otherwise constbuffer_array_get_buffer_content shall call CONSTBUFFER_GetContent for the buffer_index-th buffer and return its result. ]*/
TEST_FUNCTION(constbuffer_array_get_buffer_content_for_the_2nd_buffer_succeeds)
{
    // arrange
    CONSTBUFFER_HANDLE test_buffers[2];
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array;
    const CONSTBUFFER* result;

    test_buffers[0] = TEST_CONSTBUFFER_HANDLE_1;
    test_buffers[1] = TEST_CONSTBUFFER_HANDLE_2;

    constbuffer_array = constbuffer_array_create(test_buffers, sizeof(test_buffers) / sizeof(test_buffers[0]));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(CONSTBUFFER_GetContent(TEST_CONSTBUFFER_HANDLE_2));

    // act
    result = constbuffer_array_get_buffer_content(constbuffer_array, 1);

    // assert
    ASSERT_ARE_EQUAL(size_t, 2, result->size);
    ASSERT_ARE_EQUAL(int, 0, memcmp(two, result->buffer, result->size));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    constbuffer_array_dec_ref(constbuffer_array);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_024: [ If buffer_index is greater or equal to the number of buffers in the array, constbuffer_array_get_buffer_content shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_get_buffer_content_with_index_out_of_range_fails)
{
    // arrange
    CONSTBUFFER_HANDLE test_buffers[2];
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array;
    const CONSTBUFFER* result;

    test_buffers[0] = TEST_CONSTBUFFER_HANDLE_1;
    test_buffers[1] = TEST_CONSTBUFFER_HANDLE_2;

    constbuffer_array = constbuffer_array_create(test_buffers, sizeof(test_buffers) / sizeof(test_buffers[0]));
    umock_c_reset_all_calls();

    // act
    result = constbuffer_array_get_buffer_content(constbuffer_array, 2);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    constbuffer_array_dec_ref(constbuffer_array);
}

/* constbuffer_array_inc_ref */

/* Tests_SRS_CONSTBUFFER_ARRAY_01_018: [ Otherwise constbuffer_array_inc_ref shall increment the reference count for constbuffer_array_handle. ]*/
TEST_FUNCTION(constbuffer_array_inc_ref_increments_the_ref_count_for_empty_buffer_array)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE TEST_CONSTBUFFER_ARRAY_HANDLE = TEST_constbuffer_array_create_empty();

    ///act
    constbuffer_array_inc_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_018: [ Otherwise constbuffer_array_inc_ref shall increment the reference count for constbuffer_array_handle. ]*/
TEST_FUNCTION(constbuffer_array_inc_ref_increments_the_ref_count)
{
    ///arrange
    CONSTBUFFER_HANDLE test_buffers[2];
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array;

    test_buffers[0] = TEST_CONSTBUFFER_HANDLE_1;
    test_buffers[1] = TEST_CONSTBUFFER_HANDLE_2;

    constbuffer_array = constbuffer_array_create(test_buffers, sizeof(test_buffers) / sizeof(test_buffers[0]));
    umock_c_reset_all_calls();

    ///act
    constbuffer_array_inc_ref(constbuffer_array);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
    constbuffer_array_dec_ref(constbuffer_array);
    constbuffer_array_dec_ref(constbuffer_array);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_017: [ If constbuffer_array_handle is NULL then constbuffer_array_inc_ref shall return. ]*/
TEST_FUNCTION(constbuffer_array_inc_ref_with_NULL_constbuffer_array_handle_returns)
{
    ///arrange

    ///act
    constbuffer_array_inc_ref(NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* constbuffer_array_dec_ref */

/*Tests_SRS_CONSTBUFFER_ARRAY_02_039: [ If constbuffer_array_handle is NULL then constbuffer_array_dec_ref shall return. ]*/
TEST_FUNCTION(constbuffer_array_dec_ref_with_constbuffer_array_handle_NULL_returns)
{
    ///arrange

    ///act
    constbuffer_array_dec_ref(NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_016: [ Otherwise constbuffer_array_dec_ref shall decrement the reference count for constbuffer_array_handle. ]*/
TEST_FUNCTION(constbuffer_array_dec_ref_does_not_free_when_references_are_still_held)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE TEST_CONSTBUFFER_ARRAY_HANDLE = TEST_constbuffer_array_create_empty();
    CONSTBUFFER_ARRAY_HANDLE afterAdd1 = TEST_constbuffer_array_add_front(TEST_CONSTBUFFER_ARRAY_HANDLE, 0, TEST_CONSTBUFFER_HANDLE_1);
    CONSTBUFFER_ARRAY_HANDLE afterAdd2 = TEST_constbuffer_array_add_front(afterAdd1, 1, TEST_CONSTBUFFER_HANDLE_2);
    constbuffer_array_inc_ref(afterAdd2);
    umock_c_reset_all_calls();

    ///act
    constbuffer_array_dec_ref(afterAdd2);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
    constbuffer_array_dec_ref(afterAdd2);
    constbuffer_array_dec_ref(afterAdd1);
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_016: [ Otherwise constbuffer_array_dec_ref shall decrement the reference count for constbuffer_array_handle. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_02_038: [ If the reference count reaches 0, constbuffer_array_dec_ref shall free all used resources. ]*/
TEST_FUNCTION(constbuffer_array_dec_ref_frees)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE TEST_CONSTBUFFER_ARRAY_HANDLE = TEST_constbuffer_array_create_empty();
    CONSTBUFFER_ARRAY_HANDLE afterAdd1 = TEST_constbuffer_array_add_front(TEST_CONSTBUFFER_ARRAY_HANDLE, 0, TEST_CONSTBUFFER_HANDLE_1);
    CONSTBUFFER_ARRAY_HANDLE afterAdd2 = TEST_constbuffer_array_add_front(afterAdd1, 1, TEST_CONSTBUFFER_HANDLE_2);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    ///act
    constbuffer_array_dec_ref(afterAdd2);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
    constbuffer_array_dec_ref(afterAdd1);
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
}

/* constbuffer_array_get_all_buffers_size */

/* Tests_SRS_CONSTBUFFER_ARRAY_01_019: [ If constbuffer_array_handle is NULL, constbuffer_array_get_all_buffers_size shall fail and return a non-zero value. ]*/
TEST_FUNCTION(constbuffer_array_get_all_buffers_size_with_NULL_constbuffer_array_handle_fails)
{
    ///arrange
    uint32_t all_buffers_size;
    int result;

    ///act
    result = constbuffer_array_get_all_buffers_size(NULL, &all_buffers_size);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_020: [ If all_buffers_size is NULL, constbuffer_array_get_all_buffers_size shall fail and return a non-zero value. ]*/
TEST_FUNCTION(constbuffer_array_get_all_buffers_size_with_NULL_all_buffers_size_fails)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE TEST_CONSTBUFFER_ARRAY_HANDLE = TEST_constbuffer_array_create_empty();
    int result;

    ///act
    result = constbuffer_array_get_all_buffers_size(TEST_CONSTBUFFER_ARRAY_HANDLE, NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_021: [ If summing up the sizes results in an uint32_t overflow, shall fail and return a non-zero value. ]*/
TEST_FUNCTION(constbuffer_array_get_all_buffers_size_when_overflow_happens_fails)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE TEST_CONSTBUFFER_ARRAY_HANDLE = TEST_constbuffer_array_create_empty();
    CONSTBUFFER_ARRAY_HANDLE afterAdd1 = TEST_constbuffer_array_add_front(TEST_CONSTBUFFER_ARRAY_HANDLE, 0, TEST_CONSTBUFFER_HANDLE_1);
    CONSTBUFFER_ARRAY_HANDLE afterAdd2 = TEST_constbuffer_array_add_front(afterAdd1, 1, TEST_CONSTBUFFER_HANDLE_2);
    uint32_t all_buffers_size;
    int result;
    const CONSTBUFFER fake_const_buffer_1 = { (const unsigned char*)0x4242, UINT32_MAX };
    const CONSTBUFFER fake_const_buffer_2 = { (const unsigned char*)0x4242, 1 };

    STRICT_EXPECTED_CALL(CONSTBUFFER_GetContent(TEST_CONSTBUFFER_HANDLE_2))
        .SetReturn(&fake_const_buffer_2);
    STRICT_EXPECTED_CALL(CONSTBUFFER_GetContent(TEST_CONSTBUFFER_HANDLE_1))
        .SetReturn(&fake_const_buffer_1);

    ///act
    result = constbuffer_array_get_all_buffers_size(afterAdd2, &all_buffers_size);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
    constbuffer_array_dec_ref(afterAdd1);
    constbuffer_array_dec_ref(afterAdd2);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_021: [ If summing up the sizes results in an uint32_t overflow, shall fail and return a non-zero value. ]*/
TEST_FUNCTION(constbuffer_array_get_all_buffers_size_max_all_size_succeeds)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE TEST_CONSTBUFFER_ARRAY_HANDLE = TEST_constbuffer_array_create_empty();
    CONSTBUFFER_ARRAY_HANDLE afterAdd1 = TEST_constbuffer_array_add_front(TEST_CONSTBUFFER_ARRAY_HANDLE, 0, TEST_CONSTBUFFER_HANDLE_1);
    CONSTBUFFER_ARRAY_HANDLE afterAdd2 = TEST_constbuffer_array_add_front(afterAdd1, 1, TEST_CONSTBUFFER_HANDLE_2);
    uint32_t all_buffers_size;
    int result;
    const CONSTBUFFER fake_const_buffer_1 = { (const unsigned char*)0x4242, UINT32_MAX - 1 };
    const CONSTBUFFER fake_const_buffer_2 = { (const unsigned char*)0x4242, 1 };

    STRICT_EXPECTED_CALL(CONSTBUFFER_GetContent(TEST_CONSTBUFFER_HANDLE_2))
        .SetReturn(&fake_const_buffer_2);
    STRICT_EXPECTED_CALL(CONSTBUFFER_GetContent(TEST_CONSTBUFFER_HANDLE_1))
        .SetReturn(&fake_const_buffer_1);

    ///act
    result = constbuffer_array_get_all_buffers_size(afterAdd2, &all_buffers_size);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, UINT32_MAX, all_buffers_size);

    // cleanup
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
    constbuffer_array_dec_ref(afterAdd1);
    constbuffer_array_dec_ref(afterAdd2);
}

#if SIZE_MAX > UINT32_MAX
/* Tests_SRS_CONSTBUFFER_ARRAY_01_021: [ If summing up the sizes results in an uint32_t overflow, shall fail and return a non-zero value. ]*/
TEST_FUNCTION(constbuffer_array_get_all_buffers_size_when_buffer_size_bigger_than_UINT32_MAX_fails)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE TEST_CONSTBUFFER_ARRAY_HANDLE = TEST_constbuffer_array_create_empty();
    CONSTBUFFER_ARRAY_HANDLE afterAdd1 = TEST_constbuffer_array_add_front(TEST_CONSTBUFFER_ARRAY_HANDLE, 0, TEST_CONSTBUFFER_HANDLE_1);
    uint32_t all_buffers_size;
    int result;
    const CONSTBUFFER fake_const_buffer_1 = { (const unsigned char*)0x4242, (size_t)UINT32_MAX + 1 };

    STRICT_EXPECTED_CALL(CONSTBUFFER_GetContent(TEST_CONSTBUFFER_HANDLE_1))
        .SetReturn(&fake_const_buffer_1);

    ///act
    result = constbuffer_array_get_all_buffers_size(afterAdd1, &all_buffers_size);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
    constbuffer_array_dec_ref(afterAdd1);
}
#endif

/* Tests_SRS_CONSTBUFFER_ARRAY_01_022: [ Otherwise constbuffer_array_get_all_buffers_size shall write in all_buffers_size the total size of all buffers in the array and return 0. ]*/
TEST_FUNCTION(constbuffer_array_get_all_buffers_on_empty_const_buffer_array_succeeds)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE TEST_CONSTBUFFER_ARRAY_HANDLE = TEST_constbuffer_array_create_empty();
    uint32_t all_buffers_size;
    int result;

    ///act
    result = constbuffer_array_get_all_buffers_size(TEST_CONSTBUFFER_ARRAY_HANDLE, &all_buffers_size);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(uint32_t, 0, all_buffers_size);

    // cleanup
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_022: [ Otherwise constbuffer_array_get_all_buffers_size shall write in all_buffers_size the total size of all buffers in the array and return 0. ]*/
TEST_FUNCTION(constbuffer_array_get_all_buffers_size_with_1_buffer_succeeds)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE TEST_CONSTBUFFER_ARRAY_HANDLE = TEST_constbuffer_array_create_empty();
    CONSTBUFFER_ARRAY_HANDLE afterAdd1 = TEST_constbuffer_array_add_front(TEST_CONSTBUFFER_ARRAY_HANDLE, 0, TEST_CONSTBUFFER_HANDLE_1);
    uint32_t all_buffers_size;
    int result;

    STRICT_EXPECTED_CALL(CONSTBUFFER_GetContent(TEST_CONSTBUFFER_HANDLE_1));

    ///act
    result = constbuffer_array_get_all_buffers_size(afterAdd1, &all_buffers_size);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(uint32_t, 1, all_buffers_size);

    // cleanup
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
    constbuffer_array_dec_ref(afterAdd1);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_022: [ Otherwise constbuffer_array_get_all_buffers_size shall write in all_buffers_size the total size of all buffers in the array and return 0. ]*/
TEST_FUNCTION(constbuffer_array_get_all_buffers_size_with_2_buffers_succeeds)
{
    ///arrange
    CONSTBUFFER_ARRAY_HANDLE TEST_CONSTBUFFER_ARRAY_HANDLE = TEST_constbuffer_array_create_empty();
    CONSTBUFFER_ARRAY_HANDLE afterAdd1 = TEST_constbuffer_array_add_front(TEST_CONSTBUFFER_ARRAY_HANDLE, 0, TEST_CONSTBUFFER_HANDLE_1);
    CONSTBUFFER_ARRAY_HANDLE afterAdd2 = TEST_constbuffer_array_add_front(afterAdd1, 1, TEST_CONSTBUFFER_HANDLE_2);
    uint32_t all_buffers_size;
    int result;

    STRICT_EXPECTED_CALL(CONSTBUFFER_GetContent(TEST_CONSTBUFFER_HANDLE_2));
    STRICT_EXPECTED_CALL(CONSTBUFFER_GetContent(TEST_CONSTBUFFER_HANDLE_1));

    ///act
    result = constbuffer_array_get_all_buffers_size(afterAdd2, &all_buffers_size);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(uint32_t, 3, all_buffers_size);

    // cleanup
    constbuffer_array_dec_ref(TEST_CONSTBUFFER_ARRAY_HANDLE);
    constbuffer_array_dec_ref(afterAdd1);
    constbuffer_array_dec_ref(afterAdd2);
}

/* constbuffer_array_get_const_buffer_handle_array */

/* Tests_SRS_CONSTBUFFER_ARRAY_01_026: [ If constbuffer_array_handle is NULL, constbuffer_array_get_const_buffer_handle_array shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_get_const_buffer_handle_array_with_NULL_constbuffer_array_handle_fails)
{
    ///arrange
    const CONSTBUFFER_HANDLE* result;

    ///act
    result = constbuffer_array_get_const_buffer_handle_array(NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_027: [ Otherwise constbuffer_array_get_const_buffer_handle_array shall return the array of const buffer handles backing the const buffer array. ]*/
TEST_FUNCTION(constbuffer_array_get_const_buffer_handle_array_with_empty_array_succeeds)
{
    ///arrange
    const CONSTBUFFER_HANDLE* result;
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array = constbuffer_array_create_empty();
    umock_c_reset_all_calls();

    ///act
    result = constbuffer_array_get_const_buffer_handle_array(constbuffer_array);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);

    /// cleanup
    constbuffer_array_dec_ref(constbuffer_array);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_027: [ Otherwise constbuffer_array_get_const_buffer_handle_array shall return the array of const buffer handles backing the const buffer array. ]*/
TEST_FUNCTION(constbuffer_array_get_const_buffer_handle_array_with_array_with_1_buffer_succeeds)
{
    ///arrange
    const CONSTBUFFER_HANDLE* result;
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array = constbuffer_array_create_empty();
    CONSTBUFFER_ARRAY_HANDLE afterAdd1 = TEST_constbuffer_array_add_front(constbuffer_array, 0, TEST_CONSTBUFFER_HANDLE_1);
    umock_c_reset_all_calls();

    ///act
    result = constbuffer_array_get_const_buffer_handle_array(afterAdd1);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(void_ptr, TEST_CONSTBUFFER_HANDLE_1, result[0]);

    /// cleanup
    constbuffer_array_dec_ref(constbuffer_array);
    constbuffer_array_dec_ref(afterAdd1);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_01_027: [ Otherwise constbuffer_array_get_const_buffer_handle_array shall return the array of const buffer handles backing the const buffer array. ]*/
TEST_FUNCTION(constbuffer_array_get_const_buffer_handle_array_with_array_with_2_buffers_succeeds)
{
    ///arrange
    const CONSTBUFFER_HANDLE* result;
    CONSTBUFFER_ARRAY_HANDLE constbuffer_array = constbuffer_array_create_empty();
    CONSTBUFFER_ARRAY_HANDLE afterAdd1 = TEST_constbuffer_array_add_front(constbuffer_array, 0, TEST_CONSTBUFFER_HANDLE_1);
    CONSTBUFFER_ARRAY_HANDLE afterAdd2 = TEST_constbuffer_array_add_front(afterAdd1, 0, TEST_CONSTBUFFER_HANDLE_2);
    umock_c_reset_all_calls();

    ///act
    result = constbuffer_array_get_const_buffer_handle_array(afterAdd2);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(void_ptr, TEST_CONSTBUFFER_HANDLE_2, result[0]);
    ASSERT_ARE_EQUAL(void_ptr, TEST_CONSTBUFFER_HANDLE_1, result[1]);

    /// cleanup
    constbuffer_array_dec_ref(constbuffer_array);
    constbuffer_array_dec_ref(afterAdd1);
    constbuffer_array_dec_ref(afterAdd2);
}

/*Tests_SRS_CONSTBUFFER_ARRAY_02_050: [ If left is NULL and right is NULL then CONSTBUFFER_ARRAY_HANDLE_contain_same shall return true. ]*/
TEST_FUNCTION(CONSTBUFFER_ARRAY_HANDLE_contain_same_with_left_NULL_and_right_NULL_returns_true)
{
    ///arrange
    bool result;

    ///act
    result = CONSTBUFFER_ARRAY_HANDLE_contain_same(NULL, NULL);

    ///assert
    ASSERT_IS_TRUE(result);
}

/*Tests_SRS_CONSTBUFFER_ARRAY_02_051: [ If left is NULL and right is not NULL then CONSTBUFFER_ARRAY_HANDLE_contain_same shall return false. ]*/
TEST_FUNCTION(CONSTBUFFER_ARRAY_HANDLE_contain_same_with_left_NULL_and_right_non_NULL_returns_false)
{
    ///arrange
    bool result;
    CONSTBUFFER_ARRAY_HANDLE right = constbuffer_array_create(&TEST_CONSTBUFFER_HANDLE_1, 1);
    ASSERT_IS_NOT_NULL(right);
    umock_c_reset_all_calls();

    ///act
    result = CONSTBUFFER_ARRAY_HANDLE_contain_same(NULL, right);

    ///assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///clean
    constbuffer_array_dec_ref(right);
}

/*Tests_SRS_CONSTBUFFER_ARRAY_02_052: [ If left is not NULL and right is NULL then CONSTBUFFER_ARRAY_HANDLE_contain_same shall return false. ]*/
TEST_FUNCTION(CONSTBUFFER_ARRAY_HANDLE_contain_same_with_left_non_NULL_and_right_NULL_returns_false)
{
    ///arrange
    bool result;
    CONSTBUFFER_ARRAY_HANDLE left = constbuffer_array_create(&TEST_CONSTBUFFER_HANDLE_1, 1);
    ASSERT_IS_NOT_NULL(left);
    umock_c_reset_all_calls();

    ///act
    result = CONSTBUFFER_ARRAY_HANDLE_contain_same(left, NULL);

    ///assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///clean
    constbuffer_array_dec_ref(left);
}

/*Tests_SRS_CONSTBUFFER_ARRAY_02_053: [ If the number of CONSTBUFFER_HANDLEs in left is different then the number of CONSTBUFFER_HANDLEs in right then CONSTBUFFER_ARRAY_HANDLE_contain_same shall return false. ]*/
TEST_FUNCTION(CONSTBUFFER_ARRAY_HANDLE_contain_same_with_different_number_of_buffers_return_false)
{
    ///arrange
    bool result;
    CONSTBUFFER_ARRAY_HANDLE left = constbuffer_array_create(&TEST_CONSTBUFFER_HANDLE_1, 1);
    ASSERT_IS_NOT_NULL(left);

    CONSTBUFFER_HANDLE twoAndThree[2];
    twoAndThree[0] = TEST_CONSTBUFFER_HANDLE_2;
    twoAndThree[1] = TEST_CONSTBUFFER_HANDLE_3;
    CONSTBUFFER_ARRAY_HANDLE right = constbuffer_array_create(twoAndThree, 2);
    ASSERT_IS_NOT_NULL(right);
    umock_c_reset_all_calls();

    ///act
    result = CONSTBUFFER_ARRAY_HANDLE_contain_same(left, right);

    ///assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///clean
    constbuffer_array_dec_ref(left);
    constbuffer_array_dec_ref(right);
}

/*Tests_SRS_CONSTBUFFER_ARRAY_02_054: [ If left and right CONSTBUFFER_HANDLEs at same index are different (as indicated by CONSTBUFFER_HANDLE_contain_same call) then CONSTBUFFER_ARRAY_HANDLE_contain_same shall return false. ]*/
TEST_FUNCTION(CONSTBUFFER_ARRAY_HANDLE_contain_same_with_content_of_buffers_different_return_false)
{
    ///arrange
    bool result;
    CONSTBUFFER_HANDLE twoAndOne[2];
    twoAndOne[0] = TEST_CONSTBUFFER_HANDLE_2;
    twoAndOne[1] = TEST_CONSTBUFFER_HANDLE_1;
    CONSTBUFFER_ARRAY_HANDLE left = constbuffer_array_create(twoAndOne, 2);
    ASSERT_IS_NOT_NULL(left);

    CONSTBUFFER_HANDLE twoAndThree[2];
    twoAndThree[0] = TEST_CONSTBUFFER_HANDLE_2;
    twoAndThree[1] = TEST_CONSTBUFFER_HANDLE_3;
    CONSTBUFFER_ARRAY_HANDLE right = constbuffer_array_create(twoAndThree, 2);
    ASSERT_IS_NOT_NULL(right);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(CONSTBUFFER_HANDLE_contain_same(TEST_CONSTBUFFER_HANDLE_2, TEST_CONSTBUFFER_HANDLE_2));
    STRICT_EXPECTED_CALL(CONSTBUFFER_HANDLE_contain_same(TEST_CONSTBUFFER_HANDLE_1, TEST_CONSTBUFFER_HANDLE_3));

    ///act
    result = CONSTBUFFER_ARRAY_HANDLE_contain_same(left, right);

    ///assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///clean
    constbuffer_array_dec_ref(left);
    constbuffer_array_dec_ref(right);
}

/*Tests_SRS_CONSTBUFFER_ARRAY_02_055: [ CONSTBUFFER_ARRAY_HANDLE_contain_same shall return true. ]*/
TEST_FUNCTION(CONSTBUFFER_ARRAY_HANDLE_contain_same_with_content_of_buffers_same_return_true)
{
    ///arrange
    bool result;
    CONSTBUFFER_HANDLE twoAndOne[2];
    twoAndOne[0] = TEST_CONSTBUFFER_HANDLE_2;
    twoAndOne[1] = TEST_CONSTBUFFER_HANDLE_1;
    CONSTBUFFER_ARRAY_HANDLE left = constbuffer_array_create(twoAndOne, 2);
    ASSERT_IS_NOT_NULL(left);

    CONSTBUFFER_HANDLE alsoTwoAndOne[2];
    alsoTwoAndOne[0] = TEST_CONSTBUFFER_HANDLE_2;
    alsoTwoAndOne[1] = TEST_CONSTBUFFER_HANDLE_1;
    CONSTBUFFER_ARRAY_HANDLE right = constbuffer_array_create(alsoTwoAndOne, 2);
    ASSERT_IS_NOT_NULL(right);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(CONSTBUFFER_HANDLE_contain_same(TEST_CONSTBUFFER_HANDLE_2, TEST_CONSTBUFFER_HANDLE_2));
    STRICT_EXPECTED_CALL(CONSTBUFFER_HANDLE_contain_same(TEST_CONSTBUFFER_HANDLE_1, TEST_CONSTBUFFER_HANDLE_1));

    ///act
    result = CONSTBUFFER_ARRAY_HANDLE_contain_same(left, right);

    ///assert
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///clean
    constbuffer_array_dec_ref(left);
    constbuffer_array_dec_ref(right);
}


END_TEST_SUITE(constbuffer_array_unittests)
