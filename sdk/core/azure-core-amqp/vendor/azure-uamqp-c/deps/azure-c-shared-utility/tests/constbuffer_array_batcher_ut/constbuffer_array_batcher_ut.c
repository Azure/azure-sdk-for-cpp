// Copyright (c) Microsoft. All rights reserved.

#ifdef __cplusplus
#include <cstdlib>
#include <cinttypes>
#else
#include <stdlib.h>
#include <inttypes.h>
#endif

#include "azure_macro_utils/macro_utils.h"

void* real_malloc(size_t size)
{
    return malloc(size);
}

void real_free(void* ptr)
{
    free(ptr);
}

#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_stdint.h"
#include "umock_c/umock_c_negative_tests.h"

#define ENABLE_MOCKS

#include "azure_c_shared_utility/constbuffer.h"
#include "azure_c_shared_utility/constbuffer_array.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/memory_data.h"

#undef ENABLE_MOCKS

#include "azure_c_shared_utility/constbuffer_array_batcher.h"

#include "../real_test_files/real_constbuffer.h"
#include "../real_test_files/real_constbuffer_array.h"
#include "../real_test_files/real_memory_data.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(constbuffer_array_batcher_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    result = umock_c_init(on_umock_c_error);
	ASSERT_ARE_EQUAL(int, 0, result, "umock_c_init failed");

    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result, "umocktypes_stdint_register_types failed");

    REGISTER_GLOBAL_MOCK_HOOK(malloc, real_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(free, real_free);

    REGISTER_GLOBAL_MOCK_FAIL_RETURN(malloc, NULL);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(constbuffer_array_create_empty, NULL);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(constbuffer_array_create, NULL);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(CONSTBUFFER_CreateWithMoveMemory, NULL);

    REGISTER_CONSTBUFFER_GLOBAL_MOCK_HOOK();
    REGISTER_CONSTBUFFER_ARRAY_GLOBAL_MOCK_HOOK();
    REGISTER_MEMORY_DATA_GLOBAL_MOCK_HOOK();

    REGISTER_UMOCK_ALIAS_TYPE(CONSTBUFFER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(CONSTBUFFER_ARRAY_HANDLE, void*);
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
    umock_c_negative_tests_init();
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    umock_c_negative_tests_deinit();
    TEST_MUTEX_RELEASE(test_serialize_mutex);
}

/* constbuffer_array_batcher_batch */

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_001: [ If payloads is NULL, constbuffer_array_batcher_batch shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_batcher_batch_with_NULL_payloads_fails)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE result;

    // act
    result = constbuffer_array_batcher_batch(NULL, 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_002: [ If count is 0, constbuffer_array_batcher_batch shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_batcher_batch_with_0_count_fails)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    CONSTBUFFER_ARRAY_HANDLE test_array = real_constbuffer_array_create_empty();
    umock_c_reset_all_calls();

    // act
    result = constbuffer_array_batcher_batch(&test_array, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    real_constbuffer_array_dec_ref(test_array);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_003: [ Otherwise constbuffer_array_batcher_batch shall obtain the number of buffers used by each CONSTBUFFER_ARRAY. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_004: [ constbuffer_array_batcher_batch shall allocate memory for the header buffer (enough to hold the entire batch header namingly (count + 1) uint32_t values). ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_005: [ count shall be written as the first uint32_t in the header memory. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_006: [ The count of buffers for each array in payloads shall also be written in the header. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_007: [ constbuffer_array_batcher_batch shall allocate enough memory for all the buffer handles in all the arrays + one extra header buffer handle. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_008: [ constbuffer_array_batcher_batch shall populate the first handle in the newly allocated handles array with the header buffer handle. ]*/
TEST_FUNCTION(constbuffer_array_batcher_batch_succeeds)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    CONSTBUFFER_ARRAY_HANDLE test_array = real_constbuffer_array_create_empty();
    CONSTBUFFER_HANDLE actual_first_buffer;
    CONSTBUFFER_HANDLE header_buffer;
    uint8_t expected_header_memory[] = { 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 };
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(malloc(sizeof(uint32_t) * 2));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 1));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_array, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 0));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_HANDLE) * 1));
    STRICT_EXPECTED_CALL(CONSTBUFFER_CreateWithMoveMemory(IGNORED_PTR_ARG, sizeof(uint32_t) * 2))
        .ValidateArgumentBuffer(1, expected_header_memory, sizeof(expected_header_memory))
        .CaptureReturn(&header_buffer);
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_array, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_create(IGNORED_PTR_ARG, 1));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(IGNORED_PTR_ARG))
        .ValidateArgumentValue_constbufferHandle(&header_buffer);
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = constbuffer_array_batcher_batch(&test_array, 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);
    actual_first_buffer = real_constbuffer_array_get_buffer(result, 0);
    ASSERT_ARE_EQUAL(void_ptr, header_buffer, actual_first_buffer);

    // cleanup
    real_constbuffer_array_dec_ref(test_array);
    real_constbuffer_array_dec_ref(result);
    real_CONSTBUFFER_DecRef(actual_first_buffer);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_003: [ Otherwise constbuffer_array_batcher_batch shall obtain the number of buffers used by each CONSTBUFFER_ARRAY. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_004: [ constbuffer_array_batcher_batch shall allocate memory for the header buffer (enough to hold the entire batch header namingly (count + 1) uint32_t values). ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_005: [ count shall be written as the first uint32_t in the header memory. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_006: [ The count of buffers for each array in payloads shall also be written in the header. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_007: [ constbuffer_array_batcher_batch shall allocate enough memory for all the buffer handles in all the arrays + one extra header buffer handle. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_008: [ constbuffer_array_batcher_batch shall populate the first handle in the newly allocated handles array with the header buffer handle. ]*/
TEST_FUNCTION(constbuffer_array_batcher_batch_with_2_empty_arrays_succeeds)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    CONSTBUFFER_ARRAY_HANDLE test_arrays[2];
    CONSTBUFFER_HANDLE actual_first_buffer;
    CONSTBUFFER_HANDLE header_buffer;
    uint8_t expected_header_memory[] = { 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    test_arrays[0] = real_constbuffer_array_create_empty();
    test_arrays[1] = real_constbuffer_array_create_empty();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(malloc(sizeof(uint32_t) * 3));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 2));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[0], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 0));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[1], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 0));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_HANDLE) * 1));
    STRICT_EXPECTED_CALL(CONSTBUFFER_CreateWithMoveMemory(IGNORED_PTR_ARG, sizeof(uint32_t) * 3))
        .ValidateArgumentBuffer(1, expected_header_memory, sizeof(expected_header_memory))
        .CaptureReturn(&header_buffer);
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[0], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[1], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_create(IGNORED_PTR_ARG, 1));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(IGNORED_PTR_ARG))
        .ValidateArgumentValue_constbufferHandle(&header_buffer);
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = constbuffer_array_batcher_batch(test_arrays, 2);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);
    actual_first_buffer = real_constbuffer_array_get_buffer(result, 0);
    ASSERT_ARE_EQUAL(void_ptr, header_buffer, actual_first_buffer);

    // cleanup
    real_constbuffer_array_dec_ref(test_arrays[0]);
    real_constbuffer_array_dec_ref(result);
    real_constbuffer_array_dec_ref(test_arrays[1]);
    real_CONSTBUFFER_DecRef(actual_first_buffer);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_009: [ constbuffer_array_batcher_batch shall populate the rest of the handles in the newly allocated handles array with the const buffer handles obtained from the arrays in payloads. ]*/
TEST_FUNCTION(constbuffer_array_batcher_batch_with_an_array_with_1_buffer_succeeds)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    CONSTBUFFER_ARRAY_HANDLE test_arrays[1];
    CONSTBUFFER_HANDLE actual_buffers[2];
    CONSTBUFFER_HANDLE header_buffer;
    uint8_t test_buffer_payload[] = { 0x42 };
    CONSTBUFFER_HANDLE test_buffer = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    uint8_t expected_header_memory[] = { 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01 };
    test_arrays[0] = real_constbuffer_array_create(&test_buffer, 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(malloc(sizeof(uint32_t) * 2));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 1));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[0], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 1));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_HANDLE) * 2));
    STRICT_EXPECTED_CALL(CONSTBUFFER_CreateWithMoveMemory(IGNORED_PTR_ARG, sizeof(uint32_t) * 2))
        .ValidateArgumentBuffer(1, expected_header_memory, sizeof(expected_header_memory))
        .CaptureReturn(&header_buffer);
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[0], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(test_arrays[0], 0));
    STRICT_EXPECTED_CALL(constbuffer_array_create(IGNORED_PTR_ARG, 2));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(IGNORED_PTR_ARG))
        .ValidateArgumentValue_constbufferHandle(&header_buffer);
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffer));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = constbuffer_array_batcher_batch(test_arrays, 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);
    actual_buffers[0] = real_constbuffer_array_get_buffer(result, 0);
    ASSERT_ARE_EQUAL(void_ptr, header_buffer, actual_buffers[0]);
    actual_buffers[1] = real_constbuffer_array_get_buffer(result, 1);
    ASSERT_ARE_EQUAL(void_ptr, test_buffer, actual_buffers[1]);

    // cleanup
    real_constbuffer_array_dec_ref(test_arrays[0]);
    real_constbuffer_array_dec_ref(result);
    real_CONSTBUFFER_DecRef(test_buffer);
    real_CONSTBUFFER_DecRef(actual_buffers[0]);
    real_CONSTBUFFER_DecRef(actual_buffers[1]);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_009: [ constbuffer_array_batcher_batch shall populate the rest of the handles in the newly allocated handles array with the const buffer handles obtained from the arrays in payloads. ]*/
TEST_FUNCTION(constbuffer_array_batcher_batch_with_2_arrays_each_with_1_buffer_succeeds)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    CONSTBUFFER_ARRAY_HANDLE test_arrays[2];
    CONSTBUFFER_HANDLE actual_buffers[3];
    CONSTBUFFER_HANDLE header_buffer;
    uint8_t test_buffer_payload[] = { 0x42 };
    CONSTBUFFER_HANDLE test_buffer_1 = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    CONSTBUFFER_HANDLE test_buffer_2 = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    uint8_t expected_header_memory[] = { 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01 };
    test_arrays[0] = real_constbuffer_array_create(&test_buffer_1, 1);
    test_arrays[1] = real_constbuffer_array_create(&test_buffer_2, 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(malloc(sizeof(uint32_t) * 3));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 2));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[0], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 1));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[1], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 1));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_HANDLE) * 3));
    STRICT_EXPECTED_CALL(CONSTBUFFER_CreateWithMoveMemory(IGNORED_PTR_ARG, sizeof(uint32_t) * 3))
        .ValidateArgumentBuffer(1, expected_header_memory, sizeof(expected_header_memory))
        .CaptureReturn(&header_buffer);
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[0], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(test_arrays[0], 0));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[1], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(test_arrays[1], 0));
    STRICT_EXPECTED_CALL(constbuffer_array_create(IGNORED_PTR_ARG, 3));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(IGNORED_PTR_ARG))
        .ValidateArgumentValue_constbufferHandle(&header_buffer);
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffer_1));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffer_2));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = constbuffer_array_batcher_batch(test_arrays, 2);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);
    actual_buffers[0] = real_constbuffer_array_get_buffer(result, 0);
    ASSERT_ARE_EQUAL(void_ptr, header_buffer, actual_buffers[0]);
    actual_buffers[1] = real_constbuffer_array_get_buffer(result, 1);
    ASSERT_ARE_EQUAL(void_ptr, test_buffer_1, actual_buffers[1]);
    actual_buffers[2] = real_constbuffer_array_get_buffer(result, 2);
    ASSERT_ARE_EQUAL(void_ptr, test_buffer_2, actual_buffers[2]);

    // cleanup
    real_constbuffer_array_dec_ref(test_arrays[0]);
    real_constbuffer_array_dec_ref(test_arrays[1]);
    real_constbuffer_array_dec_ref(result);
    real_CONSTBUFFER_DecRef(test_buffer_1);
    real_CONSTBUFFER_DecRef(test_buffer_2);
    real_CONSTBUFFER_DecRef(actual_buffers[0]);
    real_CONSTBUFFER_DecRef(actual_buffers[1]);
    real_CONSTBUFFER_DecRef(actual_buffers[2]);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_009: [ constbuffer_array_batcher_batch shall populate the rest of the handles in the newly allocated handles array with the const buffer handles obtained from the arrays in payloads. ]*/
TEST_FUNCTION(constbuffer_array_batcher_batch_with_an_array_with_2_buffers_succeeds)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    CONSTBUFFER_ARRAY_HANDLE test_arrays[1];
    CONSTBUFFER_HANDLE actual_buffers[3];
    CONSTBUFFER_HANDLE header_buffer;
    uint8_t test_buffer_payload[] = { 0x42 };
    CONSTBUFFER_HANDLE test_buffers[2];
    test_buffers[0] = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    test_buffers[1] = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    uint8_t expected_header_memory[] = { 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02 };
    test_arrays[0] = real_constbuffer_array_create(test_buffers, 2);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(malloc(sizeof(uint32_t) * 2));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 1));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[0], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 2));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_HANDLE) * 3));
    STRICT_EXPECTED_CALL(CONSTBUFFER_CreateWithMoveMemory(IGNORED_PTR_ARG, sizeof(uint32_t) * 2))
        .ValidateArgumentBuffer(1, expected_header_memory, sizeof(expected_header_memory))
        .CaptureReturn(&header_buffer);
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[0], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(test_arrays[0], 0));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(test_arrays[0], 1));
    STRICT_EXPECTED_CALL(constbuffer_array_create(IGNORED_PTR_ARG, 3));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(IGNORED_PTR_ARG))
        .ValidateArgumentValue_constbufferHandle(&header_buffer);
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[0]));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[1]));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = constbuffer_array_batcher_batch(test_arrays, 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);
    actual_buffers[0] = real_constbuffer_array_get_buffer(result, 0);
    ASSERT_ARE_EQUAL(void_ptr, header_buffer, actual_buffers[0]);
    actual_buffers[1] = real_constbuffer_array_get_buffer(result, 1);
    ASSERT_ARE_EQUAL(void_ptr, test_buffers[0], actual_buffers[1]);
    actual_buffers[2] = real_constbuffer_array_get_buffer(result, 2);
    ASSERT_ARE_EQUAL(void_ptr, test_buffers[1], actual_buffers[2]);

    // cleanup
    real_constbuffer_array_dec_ref(test_arrays[0]);
    real_constbuffer_array_dec_ref(result);
    real_CONSTBUFFER_DecRef(test_buffers[0]);
    real_CONSTBUFFER_DecRef(test_buffers[1]);
    real_CONSTBUFFER_DecRef(actual_buffers[0]);
    real_CONSTBUFFER_DecRef(actual_buffers[1]);
    real_CONSTBUFFER_DecRef(actual_buffers[2]);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_009: [ constbuffer_array_batcher_batch shall populate the rest of the handles in the newly allocated handles array with the const buffer handles obtained from the arrays in payloads. ]*/
TEST_FUNCTION(constbuffer_array_batcher_batch_with_2_arrays_with_1_and_3_buffers_succeeds)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    CONSTBUFFER_ARRAY_HANDLE test_arrays[2];
    CONSTBUFFER_HANDLE actual_buffers[5];
    CONSTBUFFER_HANDLE header_buffer;
    uint8_t test_buffer_payload[] = { 0x42 };
    CONSTBUFFER_HANDLE test_buffers[4];
    size_t i;
    for (i = 0; i < 4; i++)
    {
        test_buffers[i] = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    }
    uint8_t expected_header_memory[] = { 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03 };
    test_arrays[0] = real_constbuffer_array_create(test_buffers, 1);
    test_arrays[1] = real_constbuffer_array_create(test_buffers + 1, 3);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(malloc(sizeof(uint32_t) * 3));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 2));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[0], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 1));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[1], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 3));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_HANDLE) * 5));
    STRICT_EXPECTED_CALL(CONSTBUFFER_CreateWithMoveMemory(IGNORED_PTR_ARG, sizeof(uint32_t) * 3))
        .ValidateArgumentBuffer(1, expected_header_memory, sizeof(expected_header_memory))
        .CaptureReturn(&header_buffer);
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[0], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(test_arrays[0], 0));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[1], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(test_arrays[1], 0));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(test_arrays[1], 1));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(test_arrays[1], 2));
    STRICT_EXPECTED_CALL(constbuffer_array_create(IGNORED_PTR_ARG, 5));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(IGNORED_PTR_ARG))
        .ValidateArgumentValue_constbufferHandle(&header_buffer);
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[0]));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[1]));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[2]));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[3]));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = constbuffer_array_batcher_batch(test_arrays, 2);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);
    actual_buffers[0] = real_constbuffer_array_get_buffer(result, 0);
    ASSERT_ARE_EQUAL(void_ptr, header_buffer, actual_buffers[0]);
    actual_buffers[1] = real_constbuffer_array_get_buffer(result, 1);
    ASSERT_ARE_EQUAL(void_ptr, test_buffers[0], actual_buffers[1]);
    actual_buffers[2] = real_constbuffer_array_get_buffer(result, 2);
    ASSERT_ARE_EQUAL(void_ptr, test_buffers[1], actual_buffers[2]);
    actual_buffers[3] = real_constbuffer_array_get_buffer(result, 3);
    ASSERT_ARE_EQUAL(void_ptr, test_buffers[2], actual_buffers[3]);
    actual_buffers[4] = real_constbuffer_array_get_buffer(result, 4);
    ASSERT_ARE_EQUAL(void_ptr, test_buffers[3], actual_buffers[4]);

    // cleanup
    real_constbuffer_array_dec_ref(test_arrays[0]);
    real_constbuffer_array_dec_ref(test_arrays[1]);
    real_constbuffer_array_dec_ref(result);
    real_CONSTBUFFER_DecRef(test_buffers[0]);
    real_CONSTBUFFER_DecRef(test_buffers[1]);
    real_CONSTBUFFER_DecRef(test_buffers[2]);
    real_CONSTBUFFER_DecRef(test_buffers[3]);
    real_CONSTBUFFER_DecRef(actual_buffers[0]);
    real_CONSTBUFFER_DecRef(actual_buffers[1]);
    real_CONSTBUFFER_DecRef(actual_buffers[2]);
    real_CONSTBUFFER_DecRef(actual_buffers[3]);
    real_CONSTBUFFER_DecRef(actual_buffers[4]);
}

TEST_FUNCTION(constbuffer_array_batcher_batch_with_3_arrays_with_1_and_0_and_3_buffers_succeeds)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    CONSTBUFFER_ARRAY_HANDLE test_arrays[3];
    CONSTBUFFER_HANDLE actual_buffers[5];
    CONSTBUFFER_HANDLE header_buffer;
    uint8_t test_buffer_payload[] = { 0x42 };
    CONSTBUFFER_HANDLE test_buffers[4];
    size_t i;
    for (i = 0; i < 4; i++)
    {
        test_buffers[i] = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    }
    uint8_t expected_header_memory[] = { 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03 };
    test_arrays[0] = real_constbuffer_array_create(test_buffers, 1);
    test_arrays[1] = real_constbuffer_array_create_empty();
    test_arrays[2] = real_constbuffer_array_create(test_buffers + 1, 3);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(malloc(sizeof(uint32_t) * 4));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 3));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[0], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 1));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[1], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 0));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[2], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 3));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_HANDLE) * 5));
    STRICT_EXPECTED_CALL(CONSTBUFFER_CreateWithMoveMemory(IGNORED_PTR_ARG, sizeof(uint32_t) * 4))
        .ValidateArgumentBuffer(1, expected_header_memory, sizeof(expected_header_memory))
        .CaptureReturn(&header_buffer);
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[0], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(test_arrays[0], 0));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[1], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[2], IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(test_arrays[2], 0));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(test_arrays[2], 1));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(test_arrays[2], 2));
    STRICT_EXPECTED_CALL(constbuffer_array_create(IGNORED_PTR_ARG, 5));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(IGNORED_PTR_ARG))
        .ValidateArgumentValue_constbufferHandle(&header_buffer);
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[0]));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[1]));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[2]));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[3]));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = constbuffer_array_batcher_batch(test_arrays, 3);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);
    actual_buffers[0] = real_constbuffer_array_get_buffer(result, 0);
    ASSERT_ARE_EQUAL(void_ptr, header_buffer, actual_buffers[0]);
    actual_buffers[1] = real_constbuffer_array_get_buffer(result, 1);
    ASSERT_ARE_EQUAL(void_ptr, test_buffers[0], actual_buffers[1]);
    actual_buffers[2] = real_constbuffer_array_get_buffer(result, 2);
    ASSERT_ARE_EQUAL(void_ptr, test_buffers[1], actual_buffers[2]);
    actual_buffers[3] = real_constbuffer_array_get_buffer(result, 3);
    ASSERT_ARE_EQUAL(void_ptr, test_buffers[2], actual_buffers[3]);
    actual_buffers[4] = real_constbuffer_array_get_buffer(result, 4);
    ASSERT_ARE_EQUAL(void_ptr, test_buffers[3], actual_buffers[4]);

    // cleanup
    real_constbuffer_array_dec_ref(test_arrays[0]);
    real_constbuffer_array_dec_ref(test_arrays[1]);
    real_constbuffer_array_dec_ref(test_arrays[2]);
    real_constbuffer_array_dec_ref(result);
    real_CONSTBUFFER_DecRef(test_buffers[0]);
    real_CONSTBUFFER_DecRef(test_buffers[1]);
    real_CONSTBUFFER_DecRef(test_buffers[2]);
    real_CONSTBUFFER_DecRef(test_buffers[3]);
    real_CONSTBUFFER_DecRef(actual_buffers[0]);
    real_CONSTBUFFER_DecRef(actual_buffers[1]);
    real_CONSTBUFFER_DecRef(actual_buffers[2]);
    real_CONSTBUFFER_DecRef(actual_buffers[3]);
    real_CONSTBUFFER_DecRef(actual_buffers[4]);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_010: [ If any error occurrs, constbuffer_array_batcher_batch shall fail and return NULL. ]*/
TEST_FUNCTION(when_underlying_calls_fail_constbuffer_array_batcher_batch_fails)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    CONSTBUFFER_ARRAY_HANDLE test_arrays[2];
    uint8_t test_buffer_payload[] = { 0x42 };
    CONSTBUFFER_HANDLE test_buffers[4];
    size_t i;
    for (i = 0; i < 4; i++)
    {
        test_buffers[i] = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    }
    uint8_t expected_header_memory[] = { 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03 };
    test_arrays[0] = real_constbuffer_array_create(test_buffers, 1);
    test_arrays[1] = real_constbuffer_array_create(test_buffers + 1, 3);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(malloc(sizeof(uint32_t) * 3));
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 2));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[0], IGNORED_PTR_ARG))
        .CallCannotFail();
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 1));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[1], IGNORED_PTR_ARG))
        .CallCannotFail();
    STRICT_EXPECTED_CALL(write_uint32_t(IGNORED_PTR_ARG, 3));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_HANDLE) * 5));
    STRICT_EXPECTED_CALL(CONSTBUFFER_CreateWithMoveMemory(IGNORED_PTR_ARG, sizeof(uint32_t) * 3))
        .ValidateArgumentBuffer(1, expected_header_memory, sizeof(expected_header_memory));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[0], IGNORED_PTR_ARG))
        .CallCannotFail();
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(test_arrays[0], 0))
        .CallCannotFail();
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(test_arrays[1], IGNORED_PTR_ARG))
        .CallCannotFail();
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(test_arrays[1], 0))
        .CallCannotFail();
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(test_arrays[1], 1))
        .CallCannotFail();
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(test_arrays[1], 2))
        .CallCannotFail();
    STRICT_EXPECTED_CALL(constbuffer_array_create(IGNORED_PTR_ARG, 5));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[0]));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[1]));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[2]));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[3]));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    umock_c_negative_tests_snapshot();

    for (i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        if (umock_c_negative_tests_can_call_fail(i))
        {
            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            // act
            result = constbuffer_array_batcher_batch(test_arrays, 2);

            // assert
            ASSERT_IS_NULL(result, "On failed call %zu", i);
        }
    }

    // cleanup
    real_constbuffer_array_dec_ref(test_arrays[0]);
    real_constbuffer_array_dec_ref(test_arrays[1]);
    real_CONSTBUFFER_DecRef(test_buffers[0]);
    real_CONSTBUFFER_DecRef(test_buffers[1]);
    real_CONSTBUFFER_DecRef(test_buffers[2]);
    real_CONSTBUFFER_DecRef(test_buffers[3]);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_023: [ If any of the payload const buffer arrays is NULL, constbuffer_array_batcher_batch shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_batcher_batch_with_first_array_NULL_fails)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    CONSTBUFFER_ARRAY_HANDLE test_arrays[2];
    uint8_t test_buffer_payload[] = { 0x42 };
    CONSTBUFFER_HANDLE test_buffer;
    test_buffer = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    test_arrays[0] = NULL;
    test_arrays[1] = real_constbuffer_array_create(&test_buffer, 1);
    umock_c_reset_all_calls();

    // act
    result = constbuffer_array_batcher_batch(test_arrays, 2);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    real_constbuffer_array_dec_ref(test_arrays[1]);
    real_CONSTBUFFER_DecRef(test_buffer);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_023: [ If any of the payload const buffer arrays is NULL, constbuffer_array_batcher_batch shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_batcher_batch_with_2nd_array_NULL_fails)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE result;
    CONSTBUFFER_ARRAY_HANDLE test_arrays[2];
    uint8_t test_buffer_payload[] = { 0x42 };
    CONSTBUFFER_HANDLE test_buffer;
    test_buffer = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    test_arrays[0] = real_constbuffer_array_create(&test_buffer, 1);
    test_arrays[1] = NULL;
    umock_c_reset_all_calls();

    // act
    result = constbuffer_array_batcher_batch(test_arrays, 2);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    real_constbuffer_array_dec_ref(test_arrays[0]);
    real_CONSTBUFFER_DecRef(test_buffer);
}

/* constbuffer_array_batcher_unbatch */

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_011: [ If batch is NULL, constbuffer_array_batcher_unbatch shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_batcher_unbatch_with_NULL_batch_fails)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE* result;
    uint32_t payload_count;

    // act
    result = constbuffer_array_batcher_unbatch(NULL, &payload_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_012: [ If payload_count is NULL, constbuffer_array_batcher_unbatch shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_batcher_unbatch_with_NULL_payload_count_fails)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE* result;
    uint8_t header_buffer_payload[] = { 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 };
    CONSTBUFFER_HANDLE test_buffers[1];
    CONSTBUFFER_ARRAY_HANDLE batch;
    test_buffers[0] = real_CONSTBUFFER_Create(header_buffer_payload, sizeof(header_buffer_payload));
    batch = real_constbuffer_array_create(test_buffers, 1);
    umock_c_reset_all_calls();

    // act
    result = constbuffer_array_batcher_unbatch(batch, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    real_constbuffer_array_dec_ref(batch);
    real_CONSTBUFFER_DecRef(test_buffers[0]);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_013: [ Otherwise, constbuffer_array_batcher_unbatch shall obtain the number of buffers in batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_014: [ constbuffer_array_batcher_unbatch shall obtain the content of first (header) buffer in batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_015: [ constbuffer_array_batcher_unbatch shall extract the number of buffer arrays batched by reading the first uint32_t. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_017: [ constbuffer_array_batcher_unbatch shall allocate enough memory to hold the handles for buffer arrays that will be unbatched. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_016: [ constbuffer_array_batcher_unbatch shall extract the number of buffers in each of the batched payloads reading the uint32_t values encoded in the rest of the first (header) buffer. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_018: [ constbuffer_array_batcher_unbatch shall create a const buffer array for each of the payloads in the batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_019: [ On success constbuffer_array_batcher_unbatch shall return the array of const buffer array handles that constitute the batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_020: [ On success constbuffer_array_batcher_unbatch shall write in payload_count the number of const buffer arrays that are in the batch. ]*/
TEST_FUNCTION(constbuffer_array_batcher_unbatch_with_1_payload_with_0_buffers_succeeds)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE* result;
    uint32_t payload_count;
    uint8_t header_buffer_payload[] = { 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 };
    CONSTBUFFER_HANDLE test_buffers[1];
    CONSTBUFFER_ARRAY_HANDLE batch;
    uint32_t buffer_count;
    test_buffers[0] = real_CONSTBUFFER_Create(header_buffer_payload, sizeof(header_buffer_payload));
    batch = real_constbuffer_array_create(test_buffers, 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(batch, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_content(batch, 0));
    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_ARRAY_HANDLE) * 1));
    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_create_empty());

    // act
    result = constbuffer_array_batcher_unbatch(batch, &payload_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(uint32_t, 1, payload_count);
    (void)real_constbuffer_array_get_buffer_count(result[0], &buffer_count);
    ASSERT_ARE_EQUAL(uint32_t, 0, buffer_count);

    // cleanup
    real_constbuffer_array_dec_ref(batch);
    real_CONSTBUFFER_DecRef(test_buffers[0]);
    real_constbuffer_array_dec_ref(result[0]);
    real_free(result);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_013: [ Otherwise, constbuffer_array_batcher_unbatch shall obtain the number of buffers in batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_014: [ constbuffer_array_batcher_unbatch shall obtain the content of first (header) buffer in batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_015: [ constbuffer_array_batcher_unbatch shall extract the number of buffer arrays batched by reading the first uint32_t. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_017: [ constbuffer_array_batcher_unbatch shall allocate enough memory to hold the handles for buffer arrays that will be unbatched. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_016: [ constbuffer_array_batcher_unbatch shall extract the number of buffers in each of the batched payloads reading the uint32_t values encoded in the rest of the first (header) buffer. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_018: [ constbuffer_array_batcher_unbatch shall create a const buffer array for each of the payloads in the batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_019: [ On success constbuffer_array_batcher_unbatch shall return the array of const buffer array handles that constitute the batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_020: [ On success constbuffer_array_batcher_unbatch shall write in payload_count the number of const buffer arrays that are in the batch. ]*/
TEST_FUNCTION(constbuffer_array_batcher_unbatch_with_2_payload_with_0_buffers_succeeds)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE* result;
    uint32_t payload_count;
    uint8_t header_buffer_payload[] = { 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    CONSTBUFFER_HANDLE test_buffers[1];
    CONSTBUFFER_ARRAY_HANDLE batch;
    uint32_t buffer_count;
    test_buffers[0] = real_CONSTBUFFER_Create(header_buffer_payload, sizeof(header_buffer_payload));
    batch = real_constbuffer_array_create(test_buffers, 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(batch, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_content(batch, 0));
    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_ARRAY_HANDLE) * 2));
    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_create_empty());
    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_create_empty());

    // act
    result = constbuffer_array_batcher_unbatch(batch, &payload_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(uint32_t, 2, payload_count);
    (void)real_constbuffer_array_get_buffer_count(result[0], &buffer_count);
    ASSERT_ARE_EQUAL(uint32_t, 0, buffer_count);
    (void)real_constbuffer_array_get_buffer_count(result[1], &buffer_count);
    ASSERT_ARE_EQUAL(uint32_t, 0, buffer_count);

    // cleanup
    real_constbuffer_array_dec_ref(batch);
    real_CONSTBUFFER_DecRef(test_buffers[0]);
    real_constbuffer_array_dec_ref(result[0]);
    real_constbuffer_array_dec_ref(result[1]);
    real_free(result);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_013: [ Otherwise, constbuffer_array_batcher_unbatch shall obtain the number of buffers in batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_014: [ constbuffer_array_batcher_unbatch shall obtain the content of first (header) buffer in batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_015: [ constbuffer_array_batcher_unbatch shall extract the number of buffer arrays batched by reading the first uint32_t. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_017: [ constbuffer_array_batcher_unbatch shall allocate enough memory to hold the handles for buffer arrays that will be unbatched. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_016: [ constbuffer_array_batcher_unbatch shall extract the number of buffers in each of the batched payloads reading the uint32_t values encoded in the rest of the first (header) buffer. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_018: [ constbuffer_array_batcher_unbatch shall create a const buffer array for each of the payloads in the batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_019: [ On success constbuffer_array_batcher_unbatch shall return the array of const buffer array handles that constitute the batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_020: [ On success constbuffer_array_batcher_unbatch shall write in payload_count the number of const buffer arrays that are in the batch. ]*/
TEST_FUNCTION(constbuffer_array_batcher_unbatch_with_1_payload_with_1_buffers_succeeds)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE* result;
    uint32_t payload_count;
    uint8_t header_buffer_payload[] = { 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01 };
    uint8_t test_buffer_payload[] = { 0x42 };
    CONSTBUFFER_HANDLE test_buffers[2];
    CONSTBUFFER_HANDLE actual_buffers[1];
    CONSTBUFFER_ARRAY_HANDLE batch;
    uint32_t buffer_count;
    test_buffers[0] = real_CONSTBUFFER_Create(header_buffer_payload, sizeof(header_buffer_payload));
    test_buffers[1] = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    batch = real_constbuffer_array_create(test_buffers, 2);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(batch, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_content(batch, 0));
    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_ARRAY_HANDLE) * 1));
    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_HANDLE) * 1));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(batch, 1));
    STRICT_EXPECTED_CALL(constbuffer_array_create(IGNORED_PTR_ARG, 1));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[1]));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = constbuffer_array_batcher_unbatch(batch, &payload_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(uint32_t, 1, payload_count);
    (void)real_constbuffer_array_get_buffer_count(result[0], &buffer_count);
    ASSERT_ARE_EQUAL(uint32_t, 1, buffer_count);
    actual_buffers[0] = real_constbuffer_array_get_buffer(result[0], 0);
    ASSERT_ARE_EQUAL(void_ptr, test_buffers[1], actual_buffers[0]);

    // cleanup
    real_constbuffer_array_dec_ref(batch);
    real_CONSTBUFFER_DecRef(test_buffers[0]);
    real_CONSTBUFFER_DecRef(test_buffers[1]);
    real_constbuffer_array_dec_ref(result[0]);
    real_CONSTBUFFER_DecRef(actual_buffers[0]);
    real_free(result);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_013: [ Otherwise, constbuffer_array_batcher_unbatch shall obtain the number of buffers in batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_014: [ constbuffer_array_batcher_unbatch shall obtain the content of first (header) buffer in batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_015: [ constbuffer_array_batcher_unbatch shall extract the number of buffer arrays batched by reading the first uint32_t. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_017: [ constbuffer_array_batcher_unbatch shall allocate enough memory to hold the handles for buffer arrays that will be unbatched. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_016: [ constbuffer_array_batcher_unbatch shall extract the number of buffers in each of the batched payloads reading the uint32_t values encoded in the rest of the first (header) buffer. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_018: [ constbuffer_array_batcher_unbatch shall create a const buffer array for each of the payloads in the batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_019: [ On success constbuffer_array_batcher_unbatch shall return the array of const buffer array handles that constitute the batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_020: [ On success constbuffer_array_batcher_unbatch shall write in payload_count the number of const buffer arrays that are in the batch. ]*/
TEST_FUNCTION(constbuffer_array_batcher_unbatch_with_1_payload_with_2_buffers_succeeds)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE* result;
    uint32_t payload_count;
    uint8_t header_buffer_payload[] = { 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02 };
    uint8_t test_buffer_payload[] = { 0x42 };
    CONSTBUFFER_HANDLE test_buffers[3];
    CONSTBUFFER_HANDLE actual_buffers[2];
    CONSTBUFFER_ARRAY_HANDLE batch;
    uint32_t buffer_count;
    test_buffers[0] = real_CONSTBUFFER_Create(header_buffer_payload, sizeof(header_buffer_payload));
    test_buffers[1] = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    test_buffers[2] = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    batch = real_constbuffer_array_create(test_buffers, 3);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(batch, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_content(batch, 0));
    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_ARRAY_HANDLE) * 1));
    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_HANDLE) * 2));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(batch, 1));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(batch, 2));
    STRICT_EXPECTED_CALL(constbuffer_array_create(IGNORED_PTR_ARG, 2));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[1]));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[2]));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = constbuffer_array_batcher_unbatch(batch, &payload_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(uint32_t, 1, payload_count);
    (void)real_constbuffer_array_get_buffer_count(result[0], &buffer_count);
    ASSERT_ARE_EQUAL(uint32_t, 2, buffer_count);
    actual_buffers[0] = real_constbuffer_array_get_buffer(result[0], 0);
    ASSERT_ARE_EQUAL(void_ptr, test_buffers[1], actual_buffers[0]);
    actual_buffers[1] = real_constbuffer_array_get_buffer(result[0], 1);
    ASSERT_ARE_EQUAL(void_ptr, test_buffers[2], actual_buffers[1]);

    // cleanup
    real_constbuffer_array_dec_ref(batch);
    real_CONSTBUFFER_DecRef(test_buffers[0]);
    real_CONSTBUFFER_DecRef(test_buffers[1]);
    real_CONSTBUFFER_DecRef(test_buffers[2]);
    real_constbuffer_array_dec_ref(result[0]);
    real_CONSTBUFFER_DecRef(actual_buffers[0]);
    real_CONSTBUFFER_DecRef(actual_buffers[1]);
    real_free(result);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_013: [ Otherwise, constbuffer_array_batcher_unbatch shall obtain the number of buffers in batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_014: [ constbuffer_array_batcher_unbatch shall obtain the content of first (header) buffer in batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_015: [ constbuffer_array_batcher_unbatch shall extract the number of buffer arrays batched by reading the first uint32_t. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_017: [ constbuffer_array_batcher_unbatch shall allocate enough memory to hold the handles for buffer arrays that will be unbatched. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_016: [ constbuffer_array_batcher_unbatch shall extract the number of buffers in each of the batched payloads reading the uint32_t values encoded in the rest of the first (header) buffer. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_018: [ constbuffer_array_batcher_unbatch shall create a const buffer array for each of the payloads in the batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_019: [ On success constbuffer_array_batcher_unbatch shall return the array of const buffer array handles that constitute the batch. ]*/
/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_020: [ On success constbuffer_array_batcher_unbatch shall write in payload_count the number of const buffer arrays that are in the batch. ]*/
TEST_FUNCTION(constbuffer_array_batcher_unbatch_with_2_payloads_each_with_different_number_of_buffers_succeeds)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE* result;
    uint32_t payload_count;
    uint8_t header_buffer_payload[] = { 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03 };
    uint8_t test_buffer_payload[] = { 0x42 };
    CONSTBUFFER_HANDLE test_buffers[5];
    CONSTBUFFER_HANDLE actual_buffers[4];
    CONSTBUFFER_ARRAY_HANDLE batch;
    uint32_t buffer_count;
    test_buffers[0] = real_CONSTBUFFER_Create(header_buffer_payload, sizeof(header_buffer_payload));
    test_buffers[1] = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    test_buffers[2] = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    test_buffers[3] = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    test_buffers[4] = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    batch = real_constbuffer_array_create(test_buffers, 5);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(batch, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_content(batch, 0));
    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_ARRAY_HANDLE) * 2));

    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_HANDLE) * 1)); // 1st payload with 1 buffer
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(batch, 1));
    STRICT_EXPECTED_CALL(constbuffer_array_create(IGNORED_PTR_ARG, 1));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[1]));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_HANDLE) * 3)); // 2nd payload with 3 buffer
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(batch, 2));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(batch, 3));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(batch, 4));
    STRICT_EXPECTED_CALL(constbuffer_array_create(IGNORED_PTR_ARG, 3));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[2]));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[3]));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[4]));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = constbuffer_array_batcher_unbatch(batch, &payload_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(uint32_t, 2, payload_count);

    // 1st payload
    (void)real_constbuffer_array_get_buffer_count(result[0], &buffer_count);
    ASSERT_ARE_EQUAL(uint32_t, 1, buffer_count);
    actual_buffers[0] = real_constbuffer_array_get_buffer(result[0], 0);
    ASSERT_ARE_EQUAL(void_ptr, test_buffers[1], actual_buffers[0]);

    // 2nd payload
    (void)real_constbuffer_array_get_buffer_count(result[1], &buffer_count);
    ASSERT_ARE_EQUAL(uint32_t, 3, buffer_count);
    actual_buffers[1] = real_constbuffer_array_get_buffer(result[1], 0);
    ASSERT_ARE_EQUAL(void_ptr, test_buffers[2], actual_buffers[1]);
    actual_buffers[2] = real_constbuffer_array_get_buffer(result[1], 1);
    ASSERT_ARE_EQUAL(void_ptr, test_buffers[3], actual_buffers[2]);
    actual_buffers[3] = real_constbuffer_array_get_buffer(result[1], 2);
    ASSERT_ARE_EQUAL(void_ptr, test_buffers[4], actual_buffers[3]);

    // cleanup
    real_constbuffer_array_dec_ref(batch);
    real_CONSTBUFFER_DecRef(test_buffers[0]);
    real_CONSTBUFFER_DecRef(test_buffers[1]);
    real_CONSTBUFFER_DecRef(test_buffers[2]);
    real_CONSTBUFFER_DecRef(test_buffers[3]);
    real_CONSTBUFFER_DecRef(test_buffers[4]);
    real_constbuffer_array_dec_ref(result[0]);
    real_constbuffer_array_dec_ref(result[1]);
    real_CONSTBUFFER_DecRef(actual_buffers[0]);
    real_CONSTBUFFER_DecRef(actual_buffers[1]);
    real_CONSTBUFFER_DecRef(actual_buffers[2]);
    real_CONSTBUFFER_DecRef(actual_buffers[3]);
    real_free(result);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_022: [ If any error occurs, constbuffer_array_batcher_unbatch shall fail and return NULL. ]*/
TEST_FUNCTION(when_underlying_calls_fail_constbuffer_array_batcher_unbatch_fails)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE* result;
    uint32_t payload_count;
    uint8_t header_buffer_payload[] = { 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03 };
    uint8_t test_buffer_payload[] = { 0x42 };
    CONSTBUFFER_HANDLE test_buffers[5];
    CONSTBUFFER_ARRAY_HANDLE batch;
    size_t i;
    test_buffers[0] = real_CONSTBUFFER_Create(header_buffer_payload, sizeof(header_buffer_payload));
    test_buffers[1] = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    test_buffers[2] = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    test_buffers[3] = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    test_buffers[4] = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    batch = real_constbuffer_array_create(test_buffers, 5);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(batch, IGNORED_PTR_ARG))
        .CallCannotFail();
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_content(batch, 0))
        .CallCannotFail();
    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_ARRAY_HANDLE) * 2));

    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_HANDLE) * 1)); // 1st payload with 1 buffer
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(batch, 1))
        .CallCannotFail();
    STRICT_EXPECTED_CALL(constbuffer_array_create(IGNORED_PTR_ARG, 1));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[1]));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_HANDLE) * 3)); // 2nd payload with 3 buffer
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(batch, 2))
        .CallCannotFail();
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(batch, 3))
        .CallCannotFail();
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(batch, 4))
        .CallCannotFail();
    STRICT_EXPECTED_CALL(constbuffer_array_create(IGNORED_PTR_ARG, 3));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[2]));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[3]));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[4]));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    umock_c_negative_tests_snapshot();

    for (i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        if (umock_c_negative_tests_can_call_fail(i))
        {
            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            // act
            result = constbuffer_array_batcher_unbatch(batch, &payload_count);

            // assert
            ASSERT_IS_NULL(result, "On failed call %zu", i);
        }
    }

    // cleanup
    real_constbuffer_array_dec_ref(batch);
    real_CONSTBUFFER_DecRef(test_buffers[0]);
    real_CONSTBUFFER_DecRef(test_buffers[1]);
    real_CONSTBUFFER_DecRef(test_buffers[2]);
    real_CONSTBUFFER_DecRef(test_buffers[3]);
    real_CONSTBUFFER_DecRef(test_buffers[4]);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_022: [ If any error occurs, constbuffer_array_batcher_unbatch shall fail and return NULL. ]*/
TEST_FUNCTION(when_underlying_calls_fail_constbuffer_array_batcher_unbatch_of_2_payload_that_have_0_buffers_fails)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE* result;
    uint32_t payload_count;
    uint8_t header_buffer_payload[] = { 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    CONSTBUFFER_HANDLE test_buffers[1];
    CONSTBUFFER_ARRAY_HANDLE batch;
    size_t i;
    test_buffers[0] = real_CONSTBUFFER_Create(header_buffer_payload, sizeof(header_buffer_payload));
    batch = real_constbuffer_array_create(test_buffers, 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(batch, IGNORED_PTR_ARG))
        .CallCannotFail();
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_content(batch, 0))
        .CallCannotFail();
    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_ARRAY_HANDLE) * 2));

    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_create_empty());

    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_create_empty());

    umock_c_negative_tests_snapshot();

    for (i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        if (umock_c_negative_tests_can_call_fail(i))
        {
            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            // act
            result = constbuffer_array_batcher_unbatch(batch, &payload_count);

            // assert
            ASSERT_IS_NULL(result, "On failed call %zu", i);
        }
    }

    // cleanup
    real_constbuffer_array_dec_ref(batch);
    real_CONSTBUFFER_DecRef(test_buffers[0]);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_021: [ If there are not enough buffers in batch to properly create all the payloads, constbuffer_array_batcher_unbatch shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_batcher_unbatch_with_0_buffers_fails)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE* result;
    uint32_t payload_count;
    CONSTBUFFER_ARRAY_HANDLE batch;
    batch = real_constbuffer_array_create_empty();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(batch, IGNORED_PTR_ARG));

    // act
    result = constbuffer_array_batcher_unbatch(batch, &payload_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    real_constbuffer_array_dec_ref(batch);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_024: [ If the size of the first buffer is less than uint32_t or not a multiple of uint32_t, constbuffer_array_batcher_unbatch shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_batcher_unbatch_with_header_buffer_size_3_fails)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE* result;
    uint32_t payload_count;
    uint8_t header_buffer_payload[] = { 0x00, 0x00, 0x00 };
    CONSTBUFFER_HANDLE test_buffers[1];
    CONSTBUFFER_ARRAY_HANDLE batch;
    test_buffers[0] = real_CONSTBUFFER_Create(header_buffer_payload, sizeof(header_buffer_payload));
    batch = real_constbuffer_array_create(test_buffers, 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(batch, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_content(batch, 0));

    // act
    result = constbuffer_array_batcher_unbatch(batch, &payload_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    real_constbuffer_array_dec_ref(batch);
    real_CONSTBUFFER_DecRef(test_buffers[0]);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_024: [ If the size of the first buffer is less than uint32_t or not a multiple of uint32_t, constbuffer_array_batcher_unbatch shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_batcher_unbatch_with_header_buffer_size_5_fails)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE* result;
    uint32_t payload_count;
    uint8_t header_buffer_payload[] = { 0x00, 0x00, 0x00, 0x01, 0x00 };
    CONSTBUFFER_HANDLE test_buffers[1];
    CONSTBUFFER_ARRAY_HANDLE batch;
    test_buffers[0] = real_CONSTBUFFER_Create(header_buffer_payload, sizeof(header_buffer_payload));
    batch = real_constbuffer_array_create(test_buffers, 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(batch, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_content(batch, 0));

    // act
    result = constbuffer_array_batcher_unbatch(batch, &payload_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    real_constbuffer_array_dec_ref(batch);
    real_CONSTBUFFER_DecRef(test_buffers[0]);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_021: [ If there are not enough buffers in batch to properly create all the payloads, constbuffer_array_batcher_unbatch shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_batcher_unbatch_with_1_payload_with_1_buffer_but_only_one_buffer_in_batch_fails)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE* result;
    uint32_t payload_count;
    uint8_t header_buffer_payload[] = { 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01 };
    CONSTBUFFER_HANDLE test_buffers[1];
    CONSTBUFFER_ARRAY_HANDLE batch;
    test_buffers[0] = real_CONSTBUFFER_Create(header_buffer_payload, sizeof(header_buffer_payload));
    batch = real_constbuffer_array_create(test_buffers, 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(batch, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_content(batch, 0));
    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_ARRAY_HANDLE) * 1));
    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = constbuffer_array_batcher_unbatch(batch, &payload_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    real_constbuffer_array_dec_ref(batch);
    real_CONSTBUFFER_DecRef(test_buffers[0]);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_021: [ If there are not enough buffers in batch to properly create all the payloads, constbuffer_array_batcher_unbatch shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_batcher_unbatch_with_2_payloads_with_1_buffer_but_not_enough_buffers_for_first_payload_fails)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE* result;
    uint32_t payload_count;
    uint8_t header_buffer_payload[] = { 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01 };
    CONSTBUFFER_HANDLE test_buffers[1];
    CONSTBUFFER_ARRAY_HANDLE batch;
    test_buffers[0] = real_CONSTBUFFER_Create(header_buffer_payload, sizeof(header_buffer_payload));
    batch = real_constbuffer_array_create(test_buffers, 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(batch, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_content(batch, 0));
    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_ARRAY_HANDLE) * 2));
    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = constbuffer_array_batcher_unbatch(batch, &payload_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    real_constbuffer_array_dec_ref(batch);
    real_CONSTBUFFER_DecRef(test_buffers[0]);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_021: [ If there are not enough buffers in batch to properly create all the payloads, constbuffer_array_batcher_unbatch shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_batcher_unbatch_with_2_payloads_with_1_buffer_but_not_enough_buffers_for_second_payload_fails)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE* result;
    uint32_t payload_count;
    uint8_t header_buffer_payload[] = { 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01 };
    uint8_t test_buffer_payload[] = { 0x43 };
    CONSTBUFFER_HANDLE test_buffers[2];
    CONSTBUFFER_ARRAY_HANDLE batch;
    test_buffers[0] = real_CONSTBUFFER_Create(header_buffer_payload, sizeof(header_buffer_payload));
    test_buffers[1] = real_CONSTBUFFER_Create(test_buffer_payload, sizeof(test_buffer_payload));
    batch = real_constbuffer_array_create(test_buffers, 2);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(batch, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_content(batch, 0));
    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_ARRAY_HANDLE) * 2));

    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(sizeof(CONSTBUFFER_HANDLE) * 1)); // 1st payload with 1 buffer
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer(batch, 1));
    STRICT_EXPECTED_CALL(constbuffer_array_create(IGNORED_PTR_ARG, 1));
    STRICT_EXPECTED_CALL(CONSTBUFFER_DecRef(test_buffers[1]));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(constbuffer_array_dec_ref(IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = constbuffer_array_batcher_unbatch(batch, &payload_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    real_constbuffer_array_dec_ref(batch);
    real_CONSTBUFFER_DecRef(test_buffers[0]);
    real_CONSTBUFFER_DecRef(test_buffers[1]);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_025: [ If the number of buffer arrays does not match the size of the first buffer, constbuffer_array_batcher_unbatch shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_batcher_unbatch_with_size_of_header_buffer_not_matching_the_nbumber_of_payloads_fails)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE* result;
    uint32_t payload_count;
    uint8_t header_buffer_payload[] = { 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    CONSTBUFFER_HANDLE test_buffers[1];
    CONSTBUFFER_ARRAY_HANDLE batch;
    test_buffers[0] = real_CONSTBUFFER_Create(header_buffer_payload, sizeof(header_buffer_payload));
    batch = real_constbuffer_array_create(test_buffers, 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(batch, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_content(batch, 0));
    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = constbuffer_array_batcher_unbatch(batch, &payload_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    real_constbuffer_array_dec_ref(batch);
    real_CONSTBUFFER_DecRef(test_buffers[0]);
}

/* Tests_SRS_CONSTBUFFER_ARRAY_BATCHER_01_026: [ If the number of buffer arrays in the batch is 0, constbuffer_array_batcher_unbatch shall fail and return NULL. ]*/
TEST_FUNCTION(constbuffer_array_batcher_unbatch_with_payload_count_0_fails)
{
    // arrange
    CONSTBUFFER_ARRAY_HANDLE* result;
    uint32_t payload_count;
    uint8_t header_buffer_payload[] = { 0x00, 0x00, 0x00, 0x00 };
    CONSTBUFFER_HANDLE test_buffers[1];
    CONSTBUFFER_ARRAY_HANDLE batch;
    test_buffers[0] = real_CONSTBUFFER_Create(header_buffer_payload, sizeof(header_buffer_payload));
    batch = real_constbuffer_array_create(test_buffers, 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_count(batch, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(constbuffer_array_get_buffer_content(batch, 0));
    STRICT_EXPECTED_CALL(read_uint32_t(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = constbuffer_array_batcher_unbatch(batch, &payload_count);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    real_constbuffer_array_dec_ref(batch);
    real_CONSTBUFFER_DecRef(test_buffers[0]);
}

END_TEST_SUITE(constbuffer_array_batcher_unittests)
