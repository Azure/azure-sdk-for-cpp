// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstdint>
#include <cstddef>
#include <cstring>
#else
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_stdint.h"

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void* my_gballoc_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

static void* my_gballoc_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#define ENABLE_MOCKS

#include "azure_c_shared_utility/gballoc.h"
#include "azure_uamqp_c/amqpvalue.h"
#include "azure_uamqp_c/frame_codec.h"

#undef ENABLE_MOCKS

#include "azure_uamqp_c/amqp_frame_codec.h"

#define TEST_FRAME_CODEC_HANDLE            (FRAME_CODEC_HANDLE)0x4242
#define TEST_DESCRIPTOR_AMQP_VALUE        (AMQP_VALUE)0x4243
#define TEST_DECODER_HANDLE                (AMQPVALUE_DECODER_HANDLE)0x4244
#define TEST_ENCODER_HANDLE                (ENCODER_HANDLE)0x4245
#define TEST_AMQP_VALUE                    (AMQP_VALUE)0x4246
#define TEST_CONTEXT                    (void*)0x4247

static const unsigned char test_encoded_bytes[2] = { 0x42, 0x43 };

static ON_FRAME_RECEIVED saved_on_frame_received;
static void* saved_callback_context;

static ON_VALUE_DECODED saved_value_decoded_callback;
static void* saved_value_decoded_callback_context;
static size_t total_bytes;
static PAYLOAD* actual_payloads;
static size_t actual_payload_count;

static unsigned char test_performative[] = { 0x42, 0x43, 0x44 };
static unsigned char test_frame[] = { 0x42, 0x43, 0x44, 0x41, 0x43 };
static unsigned char test_frame_payload_bytes[] = { 0x41, 0x43 };
static unsigned char* performative_decoded_bytes;
static size_t performative_decoded_byte_count;

static const PAYLOAD test_user_payload = { test_frame_payload_bytes, sizeof(test_frame_payload_bytes) };

static char expected_stringified_io[8192];
static char actual_stringified_io[8192];
static uint64_t performative_ulong;

static void deallocate_actual_payload(void)
{
    if (actual_payloads != NULL)
    {
        size_t i;
        for (i = 0; i < actual_payload_count; i++)
        {
            my_gballoc_free((void*)actual_payloads[i].bytes);
        }
        my_gballoc_free(actual_payloads);
        actual_payloads = NULL;
        actual_payload_count = 0;
    }
}

static void stringify_bytes(const unsigned char* bytes, size_t byte_count, char* output_string)
{
    size_t i;
    size_t pos = 0;

    output_string[pos++] = '[';
    for (i = 0; i < byte_count; i++)
    {
        (void)sprintf(&output_string[pos], "0x%02X", bytes[i]);
        if (i < byte_count - 1)
        {
            strcat(output_string, ",");
        }
        pos = strlen(output_string);
    }
    output_string[pos++] = ']';
    output_string[pos++] = '\0';
}

static int umocktypes_copy_PAYLOAD_ptr(PAYLOAD** destination, const PAYLOAD** source)
{
    int result;

    if (*source == NULL)
    {
        *destination = NULL;
        result = 0;
    }
    else
    {
        *destination = (PAYLOAD*)my_gballoc_malloc(sizeof(PAYLOAD));
        if (*destination == NULL)
        {
            result = __LINE__;
        }
        else
        {
            (*destination)->length = (*source)->length;
            if ((*destination)->length == 0)
            {
                (*destination)->bytes = NULL;
                result = 0;
            }
            else
            {
                (*destination)->bytes = (const unsigned char*)my_gballoc_malloc((*destination)->length);
                if ((*destination)->bytes == NULL)
                {
                    my_gballoc_free(*destination);
                    result = __LINE__;
                }
                else
                {
                    (void)memcpy((void*)(*destination)->bytes, (*source)->bytes, (*destination)->length);
                    result = 0;
                }
            }
        }
    }

    return result;
}

static void umocktypes_free_PAYLOAD_ptr(PAYLOAD** value)
{
    if (*value != NULL)
    {
        my_gballoc_free((void*)(*value)->bytes);
        my_gballoc_free(*value);
    }
}

static char* umocktypes_stringify_PAYLOAD_ptr(const PAYLOAD** value)
{
    char* result;
    if (*value == NULL)
    {
        result = (char*)my_gballoc_malloc(5);
        if (result != NULL)
        {
            (void)memcpy(result, "NULL", 5);
        }
    }
    else
    {
        result = (char*)my_gballoc_malloc(3 + (5 * (*value)->length));
        if (result != NULL)
        {
            size_t pos = 0;
            size_t i;

            result[pos++] = '[';
            for (i = 0; i < (*value)->length; i++)
            {
                (void)sprintf(&result[pos], "0x%02X ", (*value)->bytes[i]);
                pos += 5;
            }
            result[pos++] = ']';
            result[pos++] = '\0';
        }
    }

    return result;
}

static int umocktypes_are_equal_PAYLOAD_ptr(PAYLOAD** left, PAYLOAD** right)
{
    int result;

    if (*left == *right)
    {
        result = 1;
    }
    else
    {
        if ((*left == NULL) ||
            (*right == NULL))
        {
            result = 0;
        }
        else
        {
            if ((*left)->length != (*right)->length)
            {
                result = 0;
            }
            else
            {
                if ((*left)->length == 0)
                {
                    result = 1;
                }
                else
                {
                    result = (memcmp((*left)->bytes, (*right)->bytes, (*left)->length) == 0) ? 1 : 0;
                }
            }
        }
    }

    return result;
}

static int my_amqpvalue_get_ulong(AMQP_VALUE value, uint64_t* ulong_value)
{
    (void)value;
    *ulong_value = performative_ulong;
    return 0;
}

static int my_frame_codec_subscribe(FRAME_CODEC_HANDLE frame_codec, uint8_t type, ON_FRAME_RECEIVED on_frame_received, void* callback_context)
{
    (void)type;
    (void)frame_codec;
    saved_on_frame_received = on_frame_received;
    saved_callback_context = callback_context;
    return 0;
}

static int my_frame_codec_encode_frame(FRAME_CODEC_HANDLE frame_codec, uint8_t type, const PAYLOAD* payloads, size_t payload_count, const unsigned char* type_specific_bytes, uint32_t type_specific_size, ON_BYTES_ENCODED on_bytes_encoded, void* callback_context)
{
    (void)frame_codec;
    (void)type;
    (void)type_specific_bytes;
    (void)type_specific_size;
    (void)on_bytes_encoded;
    (void)callback_context;

    deallocate_actual_payload();

    actual_payloads = (PAYLOAD*)my_gballoc_malloc(sizeof(PAYLOAD) * payload_count);
    if (actual_payloads != NULL)
    {
        size_t i;
        for (i = 0; i < payload_count; i++)
        {
            actual_payloads[i].bytes = (unsigned char*)my_gballoc_malloc(payloads[i].length);
            if (actual_payloads[i].bytes != NULL)
            {
                (void)memcpy((void*)(actual_payloads[i].bytes), payloads[i].bytes, payloads[i].length);
            }
            actual_payloads[i].length = payloads[i].length;
        }
        actual_payload_count = payload_count;
    }
    return 0;
}

static AMQPVALUE_DECODER_HANDLE my_amqpvalue_decoder_create(ON_VALUE_DECODED value_decoded_callback, void* value_decoded_callback_context)
{
    saved_value_decoded_callback = value_decoded_callback;
    saved_value_decoded_callback_context = value_decoded_callback_context;
    total_bytes = 0;
    return TEST_DECODER_HANDLE;
}

static int my_amqpvalue_decode_bytes(AMQPVALUE_DECODER_HANDLE handle, const unsigned char* buffer, size_t size)
{
    unsigned char* new_bytes = (unsigned char*)my_gballoc_realloc(performative_decoded_bytes, performative_decoded_byte_count + size);
    (void)handle;
    if (new_bytes != NULL)
    {
        performative_decoded_bytes = new_bytes;
        (void)memcpy(performative_decoded_bytes + performative_decoded_byte_count, buffer, size);
        performative_decoded_byte_count += size;
    }
    total_bytes += size;
    if (total_bytes == sizeof(test_performative))
    {
        saved_value_decoded_callback(saved_value_decoded_callback_context, TEST_AMQP_VALUE);
        total_bytes = 0;
    }

    return 0;
}

static int my_amqpvalue_encode(AMQP_VALUE value, AMQPVALUE_ENCODER_OUTPUT encoder_output, void* context)
{
    (void)value;
    encoder_output(context, test_encoded_bytes, sizeof(test_encoded_bytes));
    return 0;
}

MOCK_FUNCTION_WITH_CODE(, void, amqp_empty_frame_received_callback_1, void*, context, uint16_t, channel);
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, amqp_frame_received_callback_1, void*, context, uint16_t, channel, AMQP_VALUE, performative, const unsigned char*, payload_bytes, uint32_t, frame_payload_size);
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_amqp_frame_codec_error, void*, context);
MOCK_FUNCTION_END();

static void test_on_bytes_encoded(void* context, const unsigned char* bytes, size_t length, bool encode_complete)
{
    (void)context;
    (void)bytes;
    (void)length;
    (void)encode_complete;
}

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(amqp_frame_codec_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, my_gballoc_calloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_HOOK(amqpvalue_get_ulong, my_amqpvalue_get_ulong);
    REGISTER_GLOBAL_MOCK_HOOK(frame_codec_subscribe, my_frame_codec_subscribe);
    REGISTER_GLOBAL_MOCK_HOOK(frame_codec_encode_frame, my_frame_codec_encode_frame);
    REGISTER_GLOBAL_MOCK_HOOK(amqpvalue_decoder_create, my_amqpvalue_decoder_create);
    REGISTER_GLOBAL_MOCK_HOOK(amqpvalue_decode_bytes, my_amqpvalue_decode_bytes);
    REGISTER_GLOBAL_MOCK_HOOK(amqpvalue_encode, my_amqpvalue_encode);

    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_create_ulong, TEST_AMQP_VALUE);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_get_inplace_descriptor, TEST_DESCRIPTOR_AMQP_VALUE);
    REGISTER_GLOBAL_MOCK_RETURN(frame_codec_create, TEST_FRAME_CODEC_HANDLE);
    REGISTER_GLOBAL_MOCK_RETURN(frame_codec_unsubscribe, 0);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_get_encoded_size, 0);

    REGISTER_TYPE(PAYLOAD*, PAYLOAD_ptr);

    REGISTER_UMOCK_ALIAS_TYPE(ON_VALUE_DECODED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(FRAME_CODEC_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_FRAME_RECEIVED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(AMQPVALUE_DECODER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(AMQP_VALUE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_ENCODED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(AMQPVALUE_ENCODER_OUTPUT, void*);
    REGISTER_UMOCK_ALIAS_TYPE(const PAYLOAD*, PAYLOAD*);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }

    umock_c_reset_all_calls();

    actual_payloads = NULL;
    actual_payload_count = 0;
    performative_ulong = AMQP_OPEN;
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    deallocate_actual_payload();
    if (performative_decoded_bytes != NULL)
    {
        my_gballoc_free(performative_decoded_bytes);
        performative_decoded_bytes = NULL;
    }
    performative_decoded_byte_count = 0;

    TEST_MUTEX_RELEASE(g_testByTest);
}

/* amqp_frame_codec_create */

/* Tests_SRS_AMQP_FRAME_CODEC_01_011: [amqp_frame_codec_create shall create an instance of an amqp_frame_codec and return a non-NULL handle to it.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_013: [amqp_frame_codec_create shall subscribe for AMQP frames with the given frame_codec.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_018: [amqp_frame_codec_create shall create a decoder to be used for decoding AMQP values.] */
TEST_FUNCTION(amqp_frame_codec_create_with_valid_args_succeeds)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_decoder_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(frame_codec_subscribe(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_AMQP, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);

    // assert
    ASSERT_IS_NOT_NULL(amqp_frame_codec);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_011: [amqp_frame_codec_create shall create an instance of an amqp_frame_codec and return a non-NULL handle to it.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_013: [amqp_frame_codec_create shall subscribe for AMQP frames with the given frame_codec.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_018: [amqp_frame_codec_create shall create a decoder to be used for decoding AMQP values.] */
TEST_FUNCTION(amqp_frame_codec_create_with_valid_args_and_NULL_context_succeeds)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec;
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));

    STRICT_EXPECTED_CALL(amqpvalue_decoder_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(frame_codec_subscribe(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_AMQP, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, NULL);

    // assert
    ASSERT_IS_NOT_NULL(amqp_frame_codec);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_012: [If any of the arguments frame_codec, frame_received_callback, amqp_frame_codec_error_callback or empty_frame_received_callback is NULL, amqp_frame_codec_create shall return NULL.] */
TEST_FUNCTION(amqp_frame_codec_create_with_NULL_frame_codec_fails)
{
    // arrange

    // act
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(NULL, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);

    // assert
    ASSERT_IS_NULL(amqp_frame_codec);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_012: [If any of the arguments frame_codec, frame_received_callback, amqp_frame_codec_error_callback or empty_frame_received_callback is NULL, amqp_frame_codec_create shall return NULL.] */
TEST_FUNCTION(amqp_frame_codec_create_with_NULL_frame_received_callback_fails)
{
    // arrange

    // act
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, NULL, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);

    // assert
    ASSERT_IS_NULL(amqp_frame_codec);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_012: [If any of the arguments frame_codec, frame_received_callback, amqp_frame_codec_error_callback or empty_frame_received_callback is NULL, amqp_frame_codec_create shall return NULL.] */
TEST_FUNCTION(amqp_frame_codec_create_with_NULL_empty_frame_received_callback_fails)
{
    // arrange

    // act
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, NULL, test_amqp_frame_codec_error, TEST_CONTEXT);

    // assert
    ASSERT_IS_NULL(amqp_frame_codec);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_012: [If any of the arguments frame_codec, frame_received_callback, amqp_frame_codec_error_callback or empty_frame_received_callback is NULL, amqp_frame_codec_create shall return NULL.] */
TEST_FUNCTION(amqp_frame_codec_create_with_NULL_error_callback_fails)
{
    // arrange

    // act
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, NULL, TEST_CONTEXT);

    // assert
    ASSERT_IS_NULL(amqp_frame_codec);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_014: [If subscribing for AMQP frames fails, amqp_frame_codec_create shall fail and return NULL.] */
TEST_FUNCTION(when_frame_codec_subscribe_fails_then_amqp_frame_codec_create_fails)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec;
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));

    STRICT_EXPECTED_CALL(amqpvalue_decoder_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(frame_codec_subscribe(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_AMQP, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1);

    STRICT_EXPECTED_CALL(amqpvalue_decoder_destroy(TEST_DECODER_HANDLE));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);

    // assert
    ASSERT_IS_NULL(amqp_frame_codec);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_019: [If creating the decoder fails, amqp_frame_codec_create shall fail and return NULL.] */
TEST_FUNCTION(when_creating_the_decoder_fails_then_amqp_frame_codec_create_fails)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec;
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));

    STRICT_EXPECTED_CALL(amqpvalue_decoder_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn((AMQPVALUE_DECODER_HANDLE)NULL);

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_020: [If allocating memory for the new amqp_frame_codec fails, then amqp_frame_codec_create shall fail and return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_amqp_frame_codec_fails_then_amqp_frame_codec_create_fails)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(amqp_frame_codec);
}

/* amqp_frame_codec_destroy */

/* Tests_SRS_AMQP_FRAME_CODEC_01_015: [amqp_frame_codec_destroy shall free all resources associated with the amqp_frame_codec instance.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_021: [The decoder created in amqp_frame_codec_create shall be destroyed by amqp_frame_codec_destroy.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_017: [amqp_frame_codec_destroy shall unsubscribe from receiving AMQP frames from the frame_codec that was passed to amqp_frame_codec_create.] */
TEST_FUNCTION(amqp_frame_codec_destroy_frees_the_decoder_and_unsubscribes_from_AMQP_frames)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(frame_codec_unsubscribe(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_AMQP));
    STRICT_EXPECTED_CALL(amqpvalue_decoder_destroy(TEST_DECODER_HANDLE));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    amqp_frame_codec_destroy(amqp_frame_codec);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_015: [amqp_frame_codec_destroy shall free all resources associated with the amqp_frame_codec instance.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_021: [The decoder created in amqp_frame_codec_create shall be destroyed by amqp_frame_codec_destroy.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_017: [amqp_frame_codec_destroy shall unsubscribe from receiving AMQP frames from the frame_codec that was passed to amqp_frame_codec_create.] */
TEST_FUNCTION(when_unsubscribe_fails_amqp_frame_codec_destroy_still_frees_everything)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(frame_codec_unsubscribe(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_AMQP))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_decoder_destroy(TEST_DECODER_HANDLE));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    amqp_frame_codec_destroy(amqp_frame_codec);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_016: [If amqp_frame_codec is NULL, amqp_frame_codec_destroy shall do nothing.] */
TEST_FUNCTION(amqp_frame_codec_destroy_with_NULL_handle_does_nothing)
{
    // arrange

    // act
    amqp_frame_codec_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* amqp_frame_codec_encode_frame */

/* Tests_SRS_AMQP_FRAME_CODEC_01_022: [amqp_frame_codec_encode_frame shall encode the frame header and AMQP performative in an AMQP frame and on success it shall return 0.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_025: [amqp_frame_codec_encode_frame shall encode the frame header by using frame_codec_encode_frame.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_026: [The payload frame size shall be computed based on the encoded size of the performative and its fields plus the sum of the payload sizes passed via the payloads argument.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_027: [The encoded size of the performative and its fields shall be obtained by calling amqpvalue_get_encoded_size.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_030: [Encoding of the AMQP performative and its fields shall be done by calling amqpvalue_encode.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_028: [The encode result for the performative shall be placed in a PAYLOAD structure.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_070: [The payloads argument for frame_codec_encode_frame shall be made of the payload for the encoded performative and the payloads passed to amqp_frame_codec_encode_frame.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_005: [Bytes 6 and 7 of an AMQP frame contain the channel number ] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_006: [The frame body is defined as a performative followed by an opaque payload.] */
TEST_FUNCTION(encoding_a_frame_succeeds)
{
    // arrange
    int result;
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    size_t performative_size = 2;
    uint16_t channel = 0;
    unsigned char channel_bytes[] = { 0, 0 };
    PAYLOAD expected_payloads[] = { { test_encoded_bytes, sizeof(test_encoded_bytes) } };
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_encoded_size(TEST_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &performative_size, sizeof(performative_size));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_encode(TEST_AMQP_VALUE, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(frame_codec_encode_frame(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_AMQP, IGNORED_PTR_ARG, 2, channel_bytes, sizeof(channel_bytes), test_on_bytes_encoded, (void*)0x4242))
        .ValidateArgumentBuffer(5, &channel_bytes, sizeof(channel_bytes));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = amqp_frame_codec_encode_frame(amqp_frame_codec, channel, TEST_AMQP_VALUE, &test_user_payload, 1, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, expected_payloads[0].length, actual_payloads[0].length);
    ASSERT_ARE_EQUAL(int, 0, memcmp(expected_payloads[0].bytes, actual_payloads[0].bytes, actual_payloads[0].length));
    ASSERT_ARE_EQUAL(size_t, test_user_payload.length, actual_payloads[1].length);
    ASSERT_ARE_EQUAL(int, 0, memcmp(test_user_payload.bytes, actual_payloads[1].bytes, actual_payloads[1].length));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_005: [Bytes 6 and 7 of an AMQP frame contain the channel number ] */
TEST_FUNCTION(using_channel_no_0x4243_passes_the_channel_number_as_type_specific_bytes)
{
    // arrange
    int result;
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    size_t performative_size = 2;
    uint16_t channel = 0x4243;
    unsigned char channel_bytes[] = { 0x42, 0x43 };
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_encoded_size(TEST_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &performative_size, sizeof(performative_size));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_encode(TEST_AMQP_VALUE, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(frame_codec_encode_frame(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_AMQP, IGNORED_PTR_ARG, 2, channel_bytes, sizeof(channel_bytes), test_on_bytes_encoded, (void*)0x4242))
        .ValidateArgumentBuffer(5, &channel_bytes, sizeof(channel_bytes));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = amqp_frame_codec_encode_frame(amqp_frame_codec, channel, TEST_AMQP_VALUE, &test_user_payload, 1, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_026: [The payload frame size shall be computed based on the encoded size of the performative and its fields plus the sum of the payload sizes passed via the payloads argument.] */
TEST_FUNCTION(encoding_a_frame_with_no_payloads_send_down_to_frame_codec_just_the_paylod_for_the_encoded_performative)
{
    // arrange
    int result;
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    size_t performative_size = 2;
    uint16_t channel = 0x4243;
    unsigned char channel_bytes[] = { 0x42, 0x43 };
    PAYLOAD expected_payloads[] = { { test_encoded_bytes, sizeof(test_encoded_bytes) } };
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_encoded_size(TEST_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &performative_size, sizeof(performative_size));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_encode(TEST_AMQP_VALUE, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(frame_codec_encode_frame(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_AMQP, IGNORED_PTR_ARG, 1, channel_bytes, sizeof(channel_bytes), test_on_bytes_encoded, (void*)0x4242))
        .ValidateArgumentBuffer(5, &channel_bytes, sizeof(channel_bytes));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = amqp_frame_codec_encode_frame(amqp_frame_codec, channel, TEST_AMQP_VALUE, NULL, 0, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, expected_payloads[0].length, actual_payloads[0].length);
    ASSERT_ARE_EQUAL(int, 0, memcmp(expected_payloads[0].bytes, actual_payloads[0].bytes, actual_payloads[0].length));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_024: [If frame_codec, performative or on_bytes_encoded is NULL, amqp_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(amqp_frame_codec_encode_frame_with_NULL_amqp_frame_codec_fails)
{
    // arrange
    PAYLOAD payload = { test_encoded_bytes, (uint32_t)sizeof(test_encoded_bytes) };

    // act
    int result = amqp_frame_codec_encode_frame(NULL, 0, TEST_AMQP_VALUE, &payload, 1, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_024: [If frame_codec, performative or on_bytes_encoded is NULL, amqp_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(amqp_frame_codec_encode_frame_with_NULL_performative_value_fails)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    PAYLOAD payload = { test_encoded_bytes, (uint32_t)sizeof(test_encoded_bytes) };
    int result;

    umock_c_reset_all_calls();

    // act
    result = amqp_frame_codec_encode_frame(amqp_frame_codec, 0, NULL, &payload, 1, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_024: [If frame_codec, performative or on_bytes_encoded is NULL, amqp_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(amqp_frame_codec_encode_frame_with_NULL_on_bytes_received_fails)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    PAYLOAD payload = { test_encoded_bytes, (uint32_t)sizeof(test_encoded_bytes) };
    int result;

    umock_c_reset_all_calls();

    // act
    result = amqp_frame_codec_encode_frame(amqp_frame_codec, 0, TEST_AMQP_VALUE, &payload, 1, NULL, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_029: [If any error occurs during encoding, amqp_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(when_amqpvalue_get_encoded_size_fails_then_amqp_frame_codec_encode_frame_fails)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    size_t performative_size = 2;
    uint16_t channel = 0;
    int result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_encoded_size(TEST_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &performative_size, sizeof(performative_size))
        .SetReturn(1);

    // act
    result = amqp_frame_codec_encode_frame(amqp_frame_codec, channel, TEST_AMQP_VALUE, &test_user_payload, 1, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_029: [If any error occurs during encoding, amqp_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(when_allocating_memory_for_the_new_payloads_array_fails_then_amqp_frame_codec_encode_frame_fails)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    size_t performative_size = 2;
    uint16_t channel = 0;
    int result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_encoded_size(TEST_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &performative_size, sizeof(performative_size));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    result = amqp_frame_codec_encode_frame(amqp_frame_codec, channel, TEST_AMQP_VALUE, &test_user_payload, 1, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_029: [If any error occurs during encoding, amqp_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(when_allocating_memory_for_the_encoded_performative_fails_then_amqp_frame_codec_encode_frame_fails)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    size_t performative_size = 2;
    uint16_t channel = 0;
    int result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_encoded_size(TEST_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &performative_size, sizeof(performative_size));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = amqp_frame_codec_encode_frame(amqp_frame_codec, channel, TEST_AMQP_VALUE, &test_user_payload, 1, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_029: [If any error occurs during encoding, amqp_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(when_amqpvalue_encode_fails_then_amqp_frame_codec_encode_frame_fails)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    size_t performative_size = 2;
    uint16_t channel = 0;
    int result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_encoded_size(TEST_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &performative_size, sizeof(performative_size));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_encode(TEST_AMQP_VALUE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = amqp_frame_codec_encode_frame(amqp_frame_codec, channel, TEST_AMQP_VALUE, &test_user_payload, 1, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_029: [If any error occurs during encoding, amqp_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(when_frame_codec_encode_frame_fails_then_amqp_frame_codec_encode_frame_fails)
{
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    size_t performative_size = 2;
    uint16_t channel = 0;
    unsigned char channel_bytes[] = { 0, 0 };
    int result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_encoded_size(TEST_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &performative_size, sizeof(performative_size));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_encode(TEST_AMQP_VALUE, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(frame_codec_encode_frame(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_AMQP, IGNORED_PTR_ARG, 2, channel_bytes, sizeof(channel_bytes), test_on_bytes_encoded, (void*)0x4242))
        .ValidateArgumentBuffer(5, &channel_bytes, sizeof(channel_bytes))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = amqp_frame_codec_encode_frame(amqp_frame_codec, channel, TEST_AMQP_VALUE, &test_user_payload, 1, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_008: [The performative MUST be one of those defined in section 2.7 and is encoded as a described type in the AMQP type system.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_008: [The performative MUST be one of those defined in section 2.7 and is encoded as a described type in the AMQP type system.] */
TEST_FUNCTION(amqp_performatives_are_encoded_successfully)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    size_t i;
    uint64_t valid_performatives[] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18 };

    for (i = 0; i < sizeof(valid_performatives) / sizeof(valid_performatives[0]); i++)
    {
        size_t performative_size = 2;
        uint16_t channel = 0;
        unsigned char channel_bytes[] = { 0, 0 };
        int result;
        char test_string[128];

        umock_c_reset_all_calls();

        performative_ulong = valid_performatives[i];

        STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
        STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(amqpvalue_get_encoded_size(TEST_AMQP_VALUE, IGNORED_PTR_ARG))
            .CopyOutArgumentBuffer(2, &performative_size, sizeof(performative_size));
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(amqpvalue_encode(TEST_AMQP_VALUE, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(frame_codec_encode_frame(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_AMQP, IGNORED_PTR_ARG, 1, channel_bytes, sizeof(channel_bytes), test_on_bytes_encoded, (void*)0x4242))
            .ValidateArgumentBuffer(5, &channel_bytes, sizeof(channel_bytes));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        // act
        result = amqp_frame_codec_encode_frame(amqp_frame_codec, channel, TEST_AMQP_VALUE, NULL, 0, test_on_bytes_encoded, (void*)0x4242);

        // assert
        ASSERT_ARE_EQUAL(int, 0, result);
        (void)sprintf(test_string, "test %lu", (unsigned long)i);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls(), test_string);
    }

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_029: [If any error occurs during encoding, amqp_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(when_getting_the_descriptor_fails_then_amqp_frame_codec_encode_frame_fails)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    uint16_t channel = 0;
    int result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE))
        .SetReturn(NULL);

    // act
    result = amqp_frame_codec_encode_frame(amqp_frame_codec, channel, TEST_AMQP_VALUE, NULL, 0, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_029: [If any error occurs during encoding, amqp_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(when_getting_the_ulong_value_of_the_descriptor_fails_then_amqp_frame_codec_encode_frame_fails)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    uint16_t channel = 0;
    int result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG))
        .SetReturn(1);

    // act
    result = amqp_frame_codec_encode_frame(amqp_frame_codec, channel, TEST_AMQP_VALUE, NULL, 0, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_029: [If any error occurs during encoding, amqp_frame_codec_encode_frame shall fail and return a non-zero value.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_008: [The performative MUST be one of those defined in section 2.7 and is encoded as a described type in the AMQP type system.] */
TEST_FUNCTION(when_performative_ulong_is_0x09_amqp_frame_codec_encode_frame_fails)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    uint16_t channel = 0;
    int result;
    umock_c_reset_all_calls();

    performative_ulong = 0x09;
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG));

    // act
    result = amqp_frame_codec_encode_frame(amqp_frame_codec, channel, TEST_AMQP_VALUE, NULL, 0, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_029: [If any error occurs during encoding, amqp_frame_codec_encode_frame shall fail and return a non-zero value.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_008: [The performative MUST be one of those defined in section 2.7 and is encoded as a described type in the AMQP type system.] */
TEST_FUNCTION(when_performative_ulong_is_0x19_amqp_frame_codec_encode_frame_fails)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    uint16_t channel = 0;
    int result;
    umock_c_reset_all_calls();

    performative_ulong = 0x19;
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG));

    // act
    result = amqp_frame_codec_encode_frame(amqp_frame_codec, channel, TEST_AMQP_VALUE, NULL, 0, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* amqp_frame_codec_encode_empty_frame */

/* Tests_SRS_AMQP_FRAME_CODEC_01_042: [amqp_frame_codec_encode_empty_frame shall encode a frame with no payload.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_043: [On success, amqp_frame_codec_encode_empty_frame shall return 0.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_044: [amqp_frame_codec_encode_empty_frame shall use frame_codec_encode_frame to encode the frame.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_010: [An AMQP frame with no body MAY be used to generate artificial traffic as needed to satisfy any negotiated idle timeout interval ] */
TEST_FUNCTION(encoding_of_an_empty_frame_succeeds)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    int result;
    unsigned char channel_bytes[] = { 0, 0 };
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(frame_codec_encode_frame(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_AMQP, NULL, 0, channel_bytes, sizeof(channel_bytes), test_on_bytes_encoded, (void*)0x4242))
        .ValidateArgumentBuffer(5, &channel_bytes, sizeof(channel_bytes));

    // act
    result = amqp_frame_codec_encode_empty_frame(amqp_frame_codec, 0, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_045: [If amqp_frame_codec is NULL, amqp_frame_codec_encode_empty_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(amqp_frame_codec_encode_empty_frame_with_NULL_amqp_frame_codec_fails)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    int result;
    umock_c_reset_all_calls();

    // act
    result = amqp_frame_codec_encode_empty_frame(NULL, 0, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_046: [If encoding fails in any way, amqp_frame_codec_encode_empty_frame shall fail and return a non-zero value.]  */
TEST_FUNCTION(when_frame_codec_encode_frame_fails_then_amqp_frame_codec_encode_empty_frame_fails)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    int result;
    unsigned char channel_bytes[] = { 0, 0 };
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(frame_codec_encode_frame(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_AMQP, NULL, 0, channel_bytes, sizeof(channel_bytes), test_on_bytes_encoded, (void*)0x4242))
        .ValidateArgumentBuffer(5, &channel_bytes, sizeof(channel_bytes))
        .SetReturn(1);

    // act
    result = amqp_frame_codec_encode_empty_frame(amqp_frame_codec, 0, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Receive frames */

/* Tests_SRS_AMQP_FRAME_CODEC_01_048: [When a frame header is received from frame_codec and the frame payload size is 0, empty_frame_received_callback shall be invoked, while passing the channel number as argument.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_007: [An AMQP frame with no body MAY be used to generate artificial traffic as needed to satisfy any negotiated idle timeout interval ] */
TEST_FUNCTION(when_an_empty_frame_is_decoded_the_empty_frame_callback_is_called)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    unsigned char channel_bytes[] = { 0, 0 };
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_empty_frame_received_callback_1(TEST_CONTEXT, 0));

    // act
    saved_on_frame_received(saved_callback_context, channel_bytes, sizeof(channel_bytes), NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_048: [When a frame header is received from frame_codec and the frame payload size is 0, empty_frame_received_callback shall be invoked, while passing the channel number as argument.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_001: [Bytes 6 and 7 of an AMQP frame contain the channel number ] */
TEST_FUNCTION(when_an_empty_frame_is_decoded_the_empty_frame_callback_is_called_and_the_channel_number_is_passed_to_the_callback)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    unsigned char channel_bytes[] = { 0x42, 0x43 };
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_empty_frame_received_callback_1(TEST_CONTEXT, 0x4243));

    // act
    saved_on_frame_received(saved_callback_context, channel_bytes, sizeof(channel_bytes), NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_049: [If not enough type specific bytes are received to decode the channel number, the decoding shall stop with an error.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_069: [If any error occurs while decoding a frame, the decoder shall indicate the error by calling the amqp_frame_codec_error_callback  and passing to it the callback context argument that was given in amqp_frame_codec_create.] */
TEST_FUNCTION(when_an_empty_frame_with_only_1_byte_of_type_specific_data_is_received_decoding_fails)
{
    // arrange
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    unsigned char channel_bytes[] = { 0x42, 0x43 };
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_amqp_frame_codec_error(TEST_CONTEXT));

    // act
    saved_on_frame_received(saved_callback_context, channel_bytes, 1, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_050: [All subsequent decoding shall fail and no AMQP frames shall be indicated from that point on to the consumers of amqp_frame_codec.] */
TEST_FUNCTION(when_an_empty_frame_with_only_1_byte_of_type_specific_data_is_received_decoding_fails_and_subsequent_decodes_fail_too)
{
    // arrange
    unsigned char channel_bytes[] = { 0x42, 0x43 };
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    (void)saved_on_frame_received(saved_callback_context, channel_bytes, 1, NULL, 0);
    umock_c_reset_all_calls();

    // act
    saved_on_frame_received(saved_callback_context, channel_bytes, sizeof(channel_bytes), NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_052: [Decoding the performative shall be done by feeding the bytes to the decoder create in amqp_frame_codec_create.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_054: [Once the performative is decoded, the callback frame_received_callback shall be called.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_055: [The decoded channel and performative shall be passed to frame_received_callback.]  */
TEST_FUNCTION(when_all_performative_bytes_are_received_and_AMQP_frame_payload_is_0_callback_is_triggered)
{
    // arrange
    unsigned char channel_bytes[] = { 0x42, 0x43 };
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    uint64_t descriptor_ulong = AMQP_OPEN;
    size_t i;
    umock_c_reset_all_calls();

    for (i = 0; i < sizeof(test_performative); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &descriptor_ulong, sizeof(descriptor_ulong));
    STRICT_EXPECTED_CALL(amqp_frame_received_callback_1(TEST_CONTEXT, 0x4243, TEST_AMQP_VALUE, IGNORED_PTR_ARG, 0));

    // act
    saved_on_frame_received(saved_callback_context, channel_bytes, sizeof(channel_bytes), test_performative, sizeof(test_performative));

    // assert
    stringify_bytes(test_performative, sizeof(test_performative), expected_stringified_io);
    stringify_bytes(performative_decoded_bytes, performative_decoded_byte_count, actual_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, expected_stringified_io, actual_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_002: [The frame body is defined as a performative followed by an opaque payload.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_067: [When the performative is decoded, the rest of the frame_bytes shall not be given to the AMQP decoder, but they shall be buffered so that later they are given to the frame_received callback.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_054: [Once the performative is decoded and all frame payload bytes are received, the callback frame_received_callback shall be called.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_068: [A pointer to all the payload bytes shall also be passed to frame_received_callback.] */
TEST_FUNCTION(amqp_frame_with_1_payload_bytes_are_reported_via_the_amqp_frame_payload_bytes_received_callback)
{
    // arrange
    unsigned char channel_bytes[] = { 0x42, 0x43 };
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    uint64_t descriptor_ulong = AMQP_OPEN;
    size_t i;
    umock_c_reset_all_calls();

    for (i = 0; i < sizeof(test_performative); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &descriptor_ulong, sizeof(descriptor_ulong));

    STRICT_EXPECTED_CALL(amqp_frame_received_callback_1(TEST_CONTEXT, 0x4243, TEST_AMQP_VALUE, test_frame_payload_bytes, 1))
        .ValidateArgumentBuffer(4, test_frame_payload_bytes, 1);

    // act
    saved_on_frame_received(saved_callback_context, channel_bytes, sizeof(channel_bytes), test_frame, sizeof(test_performative) + 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_002: [The frame body is defined as a performative followed by an opaque payload.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_067: [When the performative is decoded, the rest of the frame_bytes shall not be given to the AMQP decoder, but they shall be buffered so that later they are given to the frame_received callback.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_054: [Once the performative is decoded and all frame payload bytes are received, the callback frame_received_callback shall be called.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_068: [A pointer to all the payload bytes shall also be passed to frame_received_callback.] */
TEST_FUNCTION(amqp_frame_with_2_payload_bytes_are_reported_via_the_amqp_frame_payload_bytes_received_callback)
{
    // arrange
    unsigned char channel_bytes[] = { 0x42, 0x43 };
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    uint64_t descriptor_ulong = AMQP_OPEN;
    size_t i;
    umock_c_reset_all_calls();

    for (i = 0; i < sizeof(test_performative); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &descriptor_ulong, sizeof(descriptor_ulong));

    STRICT_EXPECTED_CALL(amqp_frame_received_callback_1(TEST_CONTEXT, 0x4243, TEST_AMQP_VALUE, test_frame_payload_bytes, 2))
        .ValidateArgumentBuffer(4, test_frame_payload_bytes, 2);

    // act
    saved_on_frame_received(saved_callback_context, channel_bytes, sizeof(channel_bytes), test_frame, sizeof(test_performative) + 2);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_002: [The frame body is defined as a performative followed by an opaque payload.] */
TEST_FUNCTION(after_decoding_succesfully_a_second_frame_can_be_decoded)
{
    // arrange
    unsigned char channel_bytes[] = { 0x42, 0x43 };
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    uint64_t descriptor_ulong = AMQP_OPEN;
    size_t i;

    for (i = 0; i < sizeof(test_performative); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &descriptor_ulong, sizeof(descriptor_ulong));

    STRICT_EXPECTED_CALL(amqp_frame_received_callback_1(TEST_CONTEXT, 0x4243, TEST_AMQP_VALUE, test_frame_payload_bytes, 2))
        .ValidateArgumentBuffer(4, test_frame_payload_bytes, 2);

    (void)saved_on_frame_received(saved_callback_context, channel_bytes, sizeof(channel_bytes), test_frame, sizeof(test_performative) + 2);
    umock_c_reset_all_calls();

    for (i = 0; i < sizeof(test_performative); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &descriptor_ulong, sizeof(descriptor_ulong));

    STRICT_EXPECTED_CALL(amqp_frame_received_callback_1(TEST_CONTEXT, 0x4243, TEST_AMQP_VALUE, test_frame_payload_bytes, 2))
        .ValidateArgumentBuffer(4, test_frame_payload_bytes, 2);

    // act
    saved_on_frame_received(saved_callback_context, channel_bytes, sizeof(channel_bytes), test_frame, sizeof(test_performative) + 2);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_003: [The performative MUST be one of those defined in section 2.7 and is encoded as a described type in the AMQP type system.] */
TEST_FUNCTION(valid_performative_codes_trigger_callbacks)
{
    // arrange
    unsigned char channel_bytes[] = { 0x42, 0x43 };
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    size_t i;
    uint64_t valid_performatives[] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18 };

    for (i = 0; i < 2; i++)
    {
        size_t j;
        umock_c_reset_all_calls();

        performative_ulong = valid_performatives[i];

        for (j = 0; j < sizeof(test_performative); j++)
        {
            STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        }
        STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
        STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG))
            .CopyOutArgumentBuffer(2, &performative_ulong, sizeof(performative_ulong));

        STRICT_EXPECTED_CALL(amqp_frame_received_callback_1(TEST_CONTEXT, 0x4243, TEST_AMQP_VALUE, test_frame_payload_bytes, 2))
            .ValidateArgumentBuffer(4, test_frame_payload_bytes, 2);

        // act
        saved_on_frame_received(saved_callback_context, channel_bytes, sizeof(channel_bytes), test_frame, sizeof(test_performative) + 2);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_003: [The performative MUST be one of those defined in section 2.7 and is encoded as a described type in the AMQP type system.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_069: [If any error occurs while decoding a frame, the decoder shall indicate the error by calling the amqp_frame_codec_error_callback  and passing to it the callback context argument that was given in amqp_frame_codec_create.] */
TEST_FUNCTION(performative_0x09_can_not_be_decoded)
{
    // arrange
    unsigned char channel_bytes[] = { 0x42, 0x43 };
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    size_t i;
    umock_c_reset_all_calls();
    performative_ulong = 0x09;

    for (i = 0; i < sizeof(test_performative); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &performative_ulong, sizeof(performative_ulong));

    STRICT_EXPECTED_CALL(test_amqp_frame_codec_error(TEST_CONTEXT));

    // act
    saved_on_frame_received(saved_callback_context, channel_bytes, sizeof(channel_bytes), test_frame, sizeof(test_performative) + 2);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_003: [The performative MUST be one of those defined in section 2.7 and is encoded as a described type in the AMQP type system.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_069: [If any error occurs while decoding a frame, the decoder shall indicate the error by calling the amqp_frame_codec_error_callback  and passing to it the callback context argument that was given in amqp_frame_codec_create.] */
TEST_FUNCTION(performative_0x19_can_not_be_decoded)
{
    // arrange
    unsigned char channel_bytes[] = { 0x42, 0x43 };
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    size_t i;
    umock_c_reset_all_calls();
    performative_ulong = 0x19;

    for (i = 0; i < sizeof(test_performative); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &performative_ulong, sizeof(performative_ulong));

    STRICT_EXPECTED_CALL(test_amqp_frame_codec_error(TEST_CONTEXT));

    // act
    saved_on_frame_received(saved_callback_context, channel_bytes, sizeof(channel_bytes), test_frame, sizeof(test_performative) + 2);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_060: [If any error occurs while decoding a frame, the decoder shall switch to an error state where decoding shall not be possible anymore.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_069: [If any error occurs while decoding a frame, the decoder shall indicate the error by calling the amqp_frame_codec_error_callback  and passing to it the callback context argument that was given in amqp_frame_codec_create.] */
TEST_FUNCTION(when_amqp_value_decoding_for_the_performative_fails_decoder_fails)
{
    // arrange
    unsigned char channel_bytes[] = { 0x42, 0x43 };
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    umock_c_reset_all_calls();

    performative_ulong = AMQP_OPEN;
    STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(1);

    STRICT_EXPECTED_CALL(test_amqp_frame_codec_error(TEST_CONTEXT));

    // act
    saved_on_frame_received(saved_callback_context, channel_bytes, sizeof(channel_bytes), test_frame, sizeof(test_performative) + 2);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_060: [If any error occurs while decoding a frame, the decoder shall switch to an error state where decoding shall not be possible anymore.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_069: [If any error occurs while decoding a frame, the decoder shall indicate the error by calling the amqp_frame_codec_error_callback  and passing to it the callback context argument that was given in amqp_frame_codec_create.] */
TEST_FUNCTION(when_second_amqp_value_decoding_for_the_performative_fails_decoder_fails)
{
    // arrange
    unsigned char channel_bytes[] = { 0x42, 0x43 };
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    umock_c_reset_all_calls();

    performative_ulong = AMQP_OPEN;
    STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(1);

    STRICT_EXPECTED_CALL(test_amqp_frame_codec_error(TEST_CONTEXT));

    // act
    saved_on_frame_received(saved_callback_context, channel_bytes, sizeof(channel_bytes), test_frame, sizeof(test_performative) + 2);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_060: [If any error occurs while decoding a frame, the decoder shall switch to an error state where decoding shall not be possible anymore.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_069: [If any error occurs while decoding a frame, the decoder shall indicate the error by calling the amqp_frame_codec_error_callback  and passing to it the callback context argument that was given in amqp_frame_codec_create.] */
TEST_FUNCTION(when_getting_the_descriptor_fails_decoder_fails)
{
    // arrange
    unsigned char channel_bytes[] = { 0x42, 0x43 };
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    size_t i;
    umock_c_reset_all_calls();
    performative_ulong = AMQP_OPEN;

    for (i = 0; i < sizeof(test_performative); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE))
        .SetReturn(NULL);

    STRICT_EXPECTED_CALL(test_amqp_frame_codec_error(TEST_CONTEXT));

    // act
    saved_on_frame_received(saved_callback_context, channel_bytes, sizeof(channel_bytes), test_frame, sizeof(test_performative) + 2);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_060: [If any error occurs while decoding a frame, the decoder shall switch to an error state where decoding shall not be possible anymore.] */
/* Tests_SRS_AMQP_FRAME_CODEC_01_069: [If any error occurs while decoding a frame, the decoder shall indicate the error by calling the amqp_frame_codec_error_callback  and passing to it the callback context argument that was given in amqp_frame_codec_create.] */
TEST_FUNCTION(when_getting_the_ulong_value_of_the_descriptor_fails_decoder_fails)
{
    // arrange
    unsigned char channel_bytes[] = { 0x42, 0x43 };
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    size_t i;
    umock_c_reset_all_calls();
    performative_ulong = AMQP_OPEN;

    for (i = 0; i < sizeof(test_performative); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &performative_ulong, sizeof(performative_ulong))
        .SetReturn(1);

    STRICT_EXPECTED_CALL(test_amqp_frame_codec_error(TEST_CONTEXT));

    // act
    saved_on_frame_received(saved_callback_context, channel_bytes, sizeof(channel_bytes), test_frame, sizeof(test_performative) + 2);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

/* Tests_SRS_AMQP_FRAME_CODEC_01_060: [If any error occurs while decoding a frame, the decoder shall switch to an error state where decoding shall not be possible anymore.] */
TEST_FUNCTION(when_amqp_value_decoding_fails_subsequent_decoding_fails_even_if_the_args_are_correct)
{
    // arrange
    unsigned char channel_bytes[] = { 0x42, 0x43 };
    AMQP_FRAME_CODEC_HANDLE amqp_frame_codec = amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, amqp_frame_received_callback_1, amqp_empty_frame_received_callback_1, test_amqp_frame_codec_error, TEST_CONTEXT);
    umock_c_reset_all_calls();

    performative_ulong = AMQP_OPEN;
    STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(1);

    (void)saved_on_frame_received(saved_callback_context, channel_bytes, sizeof(channel_bytes), test_frame, sizeof(test_performative) + 2);
    umock_c_reset_all_calls();

    // act
    saved_on_frame_received(saved_callback_context, channel_bytes, sizeof(channel_bytes), test_frame, sizeof(test_performative) + 2);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_frame_codec_destroy(amqp_frame_codec);
}

END_TEST_SUITE(amqp_frame_codec_ut)
