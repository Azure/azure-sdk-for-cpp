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
#include <stddef.h>
#include <string.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_stdint.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_bool.h"

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
#include "azure_uamqp_c/frame_codec.h"
#include "azure_uamqp_c/amqpvalue.h"
#include "azure_uamqp_c/amqp_definitions.h"

#undef ENABLE_MOCKS

#include "azure_uamqp_c/sasl_frame_codec.h"

#define TEST_FRAME_CODEC_HANDLE            (FRAME_CODEC_HANDLE)0x4242
#define TEST_DESCRIPTOR_AMQP_VALUE        (AMQP_VALUE)0x4243
#define TEST_DECODER_HANDLE                (AMQPVALUE_DECODER_HANDLE)0x4244
#define TEST_ENCODER_HANDLE                (ENCODER_HANDLE)0x4245
#define TEST_AMQP_VALUE                    (AMQP_VALUE)0x4246
#define TEST_CONTEXT                    (void*)0x4247

#define TEST_MIX_MAX_FRAME_SIZE            512

static const unsigned char default_test_encoded_bytes[2] = { 0x42, 0x43 };
static const unsigned char* test_encoded_bytes;
static size_t test_encoded_bytes_size;

static ON_FRAME_RECEIVED saved_on_frame_received;
static void* saved_callback_context;

static ON_VALUE_DECODED saved_value_decoded_callback;
static void* saved_value_decoded_callback_context;
static size_t total_bytes;

static unsigned char test_sasl_frame_value[] = { 0x42, 0x43, 0x44 };
static size_t test_sasl_frame_value_size;
static unsigned char* sasl_frame_value_decoded_bytes;
static size_t sasl_frame_value_decoded_byte_count;

static char expected_stringified_io[8192];
static char actual_stringified_io[8192];
static uint64_t sasl_frame_descriptor_ulong;

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
    *ulong_value = sasl_frame_descriptor_ulong;
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

static AMQPVALUE_DECODER_HANDLE my_amqpvalue_decoder_create(ON_VALUE_DECODED value_decoded_callback, void* value_decoded_callback_context)
{
    saved_value_decoded_callback = value_decoded_callback;
    saved_value_decoded_callback_context = value_decoded_callback_context;
    total_bytes = 0;
    return TEST_DECODER_HANDLE;
}

static int my_amqpvalue_decode_bytes(AMQPVALUE_DECODER_HANDLE handle, const unsigned char* buffer, size_t size)
{
    unsigned char* new_bytes = (unsigned char*)my_gballoc_realloc(sasl_frame_value_decoded_bytes, sasl_frame_value_decoded_byte_count + size);
    (void)handle;
    if (new_bytes != NULL)
    {
        sasl_frame_value_decoded_bytes = new_bytes;
        (void)memcpy(sasl_frame_value_decoded_bytes + sasl_frame_value_decoded_byte_count, buffer, size);
        sasl_frame_value_decoded_byte_count += size;
    }
    total_bytes += size;
    if (total_bytes == test_sasl_frame_value_size)
    {
        saved_value_decoded_callback(saved_value_decoded_callback_context, TEST_AMQP_VALUE);
        total_bytes = 0;
    }

    return 0;
}

static int my_amqpvalue_encode(AMQP_VALUE value, AMQPVALUE_ENCODER_OUTPUT encoder_output, void* context)
{
    (void)value;
    encoder_output(context, test_encoded_bytes, test_encoded_bytes_size);
    return 0;
}

MOCK_FUNCTION_WITH_CODE(, void, test_on_sasl_frame_received, void*, context, AMQP_VALUE, sasl_frame_value)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_on_sasl_frame_codec_error, void*, context)
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

BEGIN_TEST_SUITE(sasl_frame_codec_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    result = umocktypes_bool_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, my_gballoc_calloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_HOOK(amqpvalue_get_ulong, my_amqpvalue_get_ulong);
    REGISTER_GLOBAL_MOCK_HOOK(frame_codec_subscribe, my_frame_codec_subscribe);
    REGISTER_GLOBAL_MOCK_HOOK(amqpvalue_decoder_create, my_amqpvalue_decoder_create);
    REGISTER_GLOBAL_MOCK_HOOK(amqpvalue_decode_bytes, my_amqpvalue_decode_bytes);
    REGISTER_GLOBAL_MOCK_HOOK(amqpvalue_encode, my_amqpvalue_encode);

    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_get_inplace_descriptor, TEST_DESCRIPTOR_AMQP_VALUE);
    REGISTER_GLOBAL_MOCK_RETURN(frame_codec_unsubscribe, 0);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_get_encoded_size, 0);
    REGISTER_GLOBAL_MOCK_RETURN(frame_codec_encode_frame, 0);
    REGISTER_GLOBAL_MOCK_RETURN(is_sasl_mechanisms_type_by_descriptor, true);
    REGISTER_GLOBAL_MOCK_RETURN(is_sasl_init_type_by_descriptor, true);
    REGISTER_GLOBAL_MOCK_RETURN(is_sasl_challenge_type_by_descriptor, true);
    REGISTER_GLOBAL_MOCK_RETURN(is_sasl_response_type_by_descriptor, true);
    REGISTER_GLOBAL_MOCK_RETURN(is_sasl_outcome_type_by_descriptor, true);

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

    test_encoded_bytes = default_test_encoded_bytes;
    test_encoded_bytes_size = sizeof(default_test_encoded_bytes);
    test_sasl_frame_value_size = sizeof(test_sasl_frame_value);
    sasl_frame_descriptor_ulong = SASL_MECHANISMS;
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    if (sasl_frame_value_decoded_bytes != NULL)
    {
        free(sasl_frame_value_decoded_bytes);
        sasl_frame_value_decoded_bytes = NULL;
    }
    sasl_frame_value_decoded_byte_count = 0;

    TEST_MUTEX_RELEASE(g_testByTest);
}

/* sasl_frame_codec_create */

/* Tests_SRS_SASL_FRAME_CODEC_01_018: [sasl_frame_codec_create shall create an instance of an sasl_frame_codec and return a non-NULL handle to it.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_020: [sasl_frame_codec_create shall subscribe for SASL frames with the given frame_codec.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_022: [sasl_frame_codec_create shall create a decoder to be used for decoding SASL values.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_001: [A SASL frame has a type code of 0x01.] */
TEST_FUNCTION(sasl_frame_codec_create_with_valid_args_succeeds)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec;
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_decoder_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(frame_codec_subscribe(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_SASL, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);

    // assert
    ASSERT_IS_NOT_NULL(sasl_frame_codec);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_018: [sasl_frame_codec_create shall create an instance of an sasl_frame_codec and return a non-NULL handle to it.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_020: [sasl_frame_codec_create shall subscribe for SASL frames with the given frame_codec.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_022: [sasl_frame_codec_create shall create a decoder to be used for decoding SASL values.] */
TEST_FUNCTION(sasl_frame_codec_create_with_valid_args_and_NULL_context_succeeds)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec;
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_decoder_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(frame_codec_subscribe(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_SASL, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, NULL);

    // assert
    ASSERT_IS_NOT_NULL(sasl_frame_codec);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_019: [If any of the arguments frame_codec, frame_received_callback or error_callback is NULL, sasl_frame_codec_create shall return NULL.] */
TEST_FUNCTION(sasl_frame_codec_create_with_NULL_frame_codec_fails)
{
    // arrange

    // act
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(NULL, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_019: [If any of the arguments frame_codec, frame_received_callback or error_callback is NULL, sasl_frame_codec_create shall return NULL.] */
TEST_FUNCTION(sasl_frame_codec_create_with_NULL_frame_received_callback_fails)
{
    // arrange

    // act
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, NULL, test_on_sasl_frame_codec_error, TEST_CONTEXT);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_019: [If any of the arguments frame_codec, frame_received_callback or error_callback is NULL, sasl_frame_codec_create shall return NULL.] */
TEST_FUNCTION(sasl_frame_codec_create_with_NULL_error_callback_fails)
{
    // arrange

    // act
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, NULL, TEST_CONTEXT);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_021: [If subscribing for SASL frames fails, sasl_frame_codec_create shall fail and return NULL.] */
TEST_FUNCTION(when_frame_codec_subscribe_fails_then_sasl_frame_codec_create_fails)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec;
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_decoder_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(frame_codec_subscribe(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_SASL, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1);

    STRICT_EXPECTED_CALL(amqpvalue_decoder_destroy(TEST_DECODER_HANDLE));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_023: [If creating the decoder fails, sasl_frame_codec_create shall fail and return NULL.] */
TEST_FUNCTION(when_creating_the_decoder_fails_then_sasl_frame_codec_create_fails)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_decoder_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn((AMQPVALUE_DECODER_HANDLE)NULL);

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);

    // assert
    ASSERT_IS_NULL(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_024: [If allocating memory for the new sasl_frame_codec fails, then sasl_frame_codec_create shall fail and return NULL.]  */
TEST_FUNCTION(when_allocating_memory_for_sasl_frame_codec_fails_then_sasl_frame_codec_create_fails)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec;
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(sasl_frame_codec);
}

/* sasl_frame_codec_destroy */

/* Tests_SRS_SASL_FRAME_CODEC_01_025: [sasl_frame_codec_destroy shall free all resources associated with the sasl_frame_codec instance.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_027: [sasl_frame_codec_destroy shall unsubscribe from receiving SASL frames from the frame_codec that was passed to sasl_frame_codec_create.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_028: [The decoder created in sasl_frame_codec_create shall be destroyed by sasl_frame_codec_destroy.] */
TEST_FUNCTION(sasl_frame_codec_destroy_frees_the_decoder_and_unsubscribes_from_AMQP_frames)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(frame_codec_unsubscribe(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_SASL));
    STRICT_EXPECTED_CALL(amqpvalue_decoder_destroy(TEST_DECODER_HANDLE));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    sasl_frame_codec_destroy(sasl_frame_codec);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_FRAME_CODEC_01_025: [sasl_frame_codec_destroy shall free all resources associated with the sasl_frame_codec instance.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_027: [sasl_frame_codec_destroy shall unsubscribe from receiving SASL frames from the frame_codec that was passed to sasl_frame_codec_create.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_028: [The decoder created in sasl_frame_codec_create shall be destroyed by sasl_frame_codec_destroy.] */
TEST_FUNCTION(when_unsubscribe_fails_sasl_frame_codec_destroy_still_frees_everything)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(frame_codec_unsubscribe(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_SASL))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_decoder_destroy(TEST_DECODER_HANDLE));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    sasl_frame_codec_destroy(sasl_frame_codec);

    // assert
    // uMock checks the calls
}

/* Tests_SRS_SASL_FRAME_CODEC_01_026: [If sasl_frame_codec is NULL, sasl_frame_codec_destroy shall do nothing.] */
TEST_FUNCTION(sasl_frame_codec_destroy_with_NULL_handle_does_nothing)
{
    // arrange

    // act
    sasl_frame_codec_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* sasl_frame_codec_encode_frame */

/* Tests_SRS_SASL_FRAME_CODEC_01_029: [sasl_frame_codec_encode_frame shall encode the frame header and sasl_frame_value AMQP value in a SASL frame and on success it shall return 0.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_031: [sasl_frame_codec_encode_frame shall encode the frame header and its contents by using frame_codec_encode_frame.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_032: [The payload frame size shall be computed based on the encoded size of the sasl_frame_value and its fields.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_033: [The encoded size of the sasl_frame_value and its fields shall be obtained by calling amqpvalue_get_encoded_size.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_035: [Encoding of the sasl_frame_value and its fields shall be done by calling amqpvalue_encode.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_012: [Bytes 6 and 7 of the header are ignored.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_013: [Implementations SHOULD set these to 0x00.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_014: [The extended header is ignored.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_015: [Implementations SHOULD therefore set DOFF to 0x02.] */
TEST_FUNCTION(encoding_a_sasl_frame_succeeds)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    PAYLOAD payload;
    size_t sasl_frame_value_size = 2;
    int result;
    umock_c_reset_all_calls();

    payload.bytes = test_encoded_bytes;
    payload.length = test_encoded_bytes_size;

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_encoded_size(TEST_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &sasl_frame_value_size, sizeof(sasl_frame_value_size));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_encode(TEST_AMQP_VALUE, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(frame_codec_encode_frame(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_SASL, &payload, 1, NULL, 0, test_on_bytes_encoded, (void*)0x4242));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = sasl_frame_codec_encode_frame(sasl_frame_codec, TEST_AMQP_VALUE, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_030: [If sasl_frame_codec or sasl_frame_value is NULL, sasl_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(sasl_frame_codec_encode_frame_with_NULL_sasl_frame_codec_fails)
{
    // arrange

    // act
    int result = sasl_frame_codec_encode_frame(NULL, TEST_AMQP_VALUE, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_030: [If sasl_frame_codec or sasl_frame_value is NULL, sasl_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(sasl_frame_codec_encode_frame_with_NULL_performative_value_fails)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    int result;
    umock_c_reset_all_calls();

    // act
    result = sasl_frame_codec_encode_frame(sasl_frame_codec, NULL, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_034: [If any error occurs during encoding, sasl_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(when_amqpvalue_get_inplace_descriptor_fails_then_sasl_frame_codec_encode_frame_fails)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    int result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE))
        .SetReturn((AMQP_VALUE)NULL);

    // act
    result = sasl_frame_codec_encode_frame(sasl_frame_codec, TEST_AMQP_VALUE, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_034: [If any error occurs during encoding, sasl_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(when_amqpvalue_get_ulong_fails_then_sasl_frame_codec_encode_frame_fails)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    int result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG))
        .SetReturn(1);

    // act
    result = sasl_frame_codec_encode_frame(sasl_frame_codec, TEST_AMQP_VALUE, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_034: [If any error occurs during encoding, sasl_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(when_amqpvalue_get_encoded_size_fails_then_sasl_frame_codec_encode_frame_fails)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    int result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_encoded_size(TEST_AMQP_VALUE, IGNORED_PTR_ARG))
        .SetReturn(1);

    // act
    result = sasl_frame_codec_encode_frame(sasl_frame_codec, TEST_AMQP_VALUE, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_034: [If any error occurs during encoding, sasl_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(when_amqpvalue_encode_fails_then_sasl_frame_codec_encode_frame_fails)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    size_t sasl_frame_value_size = 2;
    int result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_encoded_size(TEST_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &sasl_frame_value_size, sizeof(sasl_frame_value_size));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_encode(TEST_AMQP_VALUE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = sasl_frame_codec_encode_frame(sasl_frame_codec, TEST_AMQP_VALUE, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_034: [If any error occurs during encoding, sasl_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(when_frame_codec_encode_frame_fails_then_sasl_frame_codec_encode_frame_fails)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    size_t sasl_frame_value_size = 2;
    PAYLOAD payload;
    int result;
    umock_c_reset_all_calls();

    payload.bytes = test_encoded_bytes;
    payload.length = test_encoded_bytes_size;
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_encoded_size(TEST_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &sasl_frame_value_size, sizeof(sasl_frame_value_size));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_encode(TEST_AMQP_VALUE, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(frame_codec_encode_frame(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_SASL, &payload, 1, NULL, 0, test_on_bytes_encoded, (void*)0x4242))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = sasl_frame_codec_encode_frame(sasl_frame_codec, TEST_AMQP_VALUE, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_034: [If any error occurs during encoding, sasl_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(when_allocating_memory_for_the_encoded_sasl_value_fails_then_sasl_frame_codec_encode_frame_fails)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    PAYLOAD payload;
    size_t sasl_frame_value_size = 2;
    int result;
    umock_c_reset_all_calls();

    payload.bytes = test_encoded_bytes;
    payload.length = test_encoded_bytes_size;

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_encoded_size(TEST_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &sasl_frame_value_size, sizeof(sasl_frame_value_size));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    result = sasl_frame_codec_encode_frame(sasl_frame_codec, TEST_AMQP_VALUE, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_011: [A SASL frame has a type code of 0x01.] */
TEST_FUNCTION(the_SASL_frame_type_is_according_to_ISO)
{
    // arrange
    // act

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 1, FRAME_TYPE_SASL);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_016: [The maximum size of a SASL frame is defined by MIN-MAX-FRAME-SIZE.] */
TEST_FUNCTION(when_encoding_a_sasl_frame_value_that_makes_the_frame_be_the_max_size_sasl_frame_codec_encode_frame_succeeds)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    PAYLOAD payload;
    size_t sasl_frame_value_size;
    unsigned char encoded_bytes[TEST_MIX_MAX_FRAME_SIZE - 8] = { 0 };
    int result;
    umock_c_reset_all_calls();

    test_encoded_bytes = encoded_bytes;
    test_encoded_bytes_size = sizeof(encoded_bytes);
    payload.bytes = test_encoded_bytes;
    payload.length = test_encoded_bytes_size;

    sasl_frame_value_size = test_encoded_bytes_size;
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_encoded_size(TEST_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &sasl_frame_value_size, sizeof(sasl_frame_value_size));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_encode(TEST_AMQP_VALUE, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(frame_codec_encode_frame(TEST_FRAME_CODEC_HANDLE, FRAME_TYPE_SASL, &payload, 1, NULL, 0, test_on_bytes_encoded, (void*)0x4242));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = sasl_frame_codec_encode_frame(sasl_frame_codec, TEST_AMQP_VALUE, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_016: [The maximum size of a SASL frame is defined by MIN-MAX-FRAME-SIZE.] */
TEST_FUNCTION(when_encoding_a_sasl_frame_value_that_makes_the_frame_exceed_the_allowed_size_sasl_frame_codec_encode_frame_fails)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    PAYLOAD payload;
    unsigned char encoded_bytes[TEST_MIX_MAX_FRAME_SIZE - 8 + 1] = { 0 };
    size_t sasl_frame_value_size;
    int result;
    umock_c_reset_all_calls();

    payload.bytes = test_encoded_bytes;
    payload.length = test_encoded_bytes_size;

    test_encoded_bytes = encoded_bytes;
    test_encoded_bytes_size = sizeof(encoded_bytes);
    sasl_frame_value_size = test_encoded_bytes_size;
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_encoded_size(TEST_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &sasl_frame_value_size, sizeof(sasl_frame_value_size));

    // act
    result = sasl_frame_codec_encode_frame(sasl_frame_codec, TEST_AMQP_VALUE, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_034: [If any error occurs during encoding, sasl_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(when_the_sasl_frame_value_has_a_descriptor_ulong_lower_than_MECHANISMS_frame_codec_encode_frame_fails)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    int result;
    umock_c_reset_all_calls();

    sasl_frame_descriptor_ulong = SASL_MECHANISMS - 1;
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &sasl_frame_descriptor_ulong, sizeof(sasl_frame_descriptor_ulong));

    // act
    result = sasl_frame_codec_encode_frame(sasl_frame_codec, TEST_AMQP_VALUE, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_034: [If any error occurs during encoding, sasl_frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(when_the_sasl_frame_value_has_a_descriptor_ulong_higher_than_OUTCOME_frame_codec_encode_frame_fails)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    int result;
    umock_c_reset_all_calls();

    sasl_frame_descriptor_ulong = SASL_OUTCOME + 1;
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(TEST_DESCRIPTOR_AMQP_VALUE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &sasl_frame_descriptor_ulong, sizeof(sasl_frame_descriptor_ulong));

    // act
    result = sasl_frame_codec_encode_frame(sasl_frame_codec, TEST_AMQP_VALUE, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Receive frames */

/* Tests_SRS_SASL_FRAME_CODEC_01_039: [sasl_frame_codec shall decode the sasl-frame value as a described type.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_040: [Decoding the sasl-frame type shall be done by feeding the bytes to the decoder create in sasl_frame_codec_create.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_041: [Once the sasl frame is decoded, the callback frame_received_callback shall be called.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_042: [The decoded sasl-frame value and the context passed in sasl_frame_codec_create shall be passed to frame_received_callback.] */
TEST_FUNCTION(when_sasl_frame_bytes_are_received_it_is_decoded_and_indicated_as_a_received_sasl_frame)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    size_t i;
    umock_c_reset_all_calls();

    for (i = 0; i < sizeof(test_sasl_frame_value); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    STRICT_EXPECTED_CALL(test_on_sasl_frame_received(TEST_CONTEXT, TEST_AMQP_VALUE));

    // act
    saved_on_frame_received(saved_callback_context, NULL, 0, test_sasl_frame_value,  sizeof(test_sasl_frame_value));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_039: [sasl_frame_codec shall decode the sasl-frame value as a described type.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_040: [Decoding the sasl-frame type shall be done by feeding the bytes to the decoder create in sasl_frame_codec_create.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_041: [Once the sasl frame is decoded, the callback frame_received_callback shall be called.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_042: [The decoded sasl-frame value and the context passed in sasl_frame_codec_create shall be passed to frame_received_callback.] */
TEST_FUNCTION(when_context_is_NULL_decoding_a_sasl_frame_still_succeeds)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, NULL);
    size_t i;
    umock_c_reset_all_calls();

    for (i = 0; i < sizeof(test_sasl_frame_value); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    STRICT_EXPECTED_CALL(test_on_sasl_frame_received(NULL, TEST_AMQP_VALUE));

    // act
    saved_on_frame_received(saved_callback_context, NULL, 0, test_sasl_frame_value, sizeof(test_sasl_frame_value));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_046: [If any error occurs while decoding a frame, the decoder shall switch to an error state where decoding shall not be possible anymore.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_049: [If any error occurs while decoding a frame, the decoder shall call the error_callback and pass to it the callback_context, both of those being the ones given to sasl_frame_codec_create.] */
TEST_FUNCTION(when_amqpvalue_decode_bytes_fails_then_the_decoder_switches_to_an_error_state)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(1);

    STRICT_EXPECTED_CALL(test_on_sasl_frame_codec_error(TEST_CONTEXT));

    // act
    saved_on_frame_received(saved_callback_context, NULL, 0, test_sasl_frame_value, sizeof(test_sasl_frame_value));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_046: [If any error occurs while decoding a frame, the decoder shall switch to an error state where decoding shall not be possible anymore.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_049: [If any error occurs while decoding a frame, the decoder shall call the error_callback and pass to it the callback_context, both of those being the ones given to sasl_frame_codec_create.] */
TEST_FUNCTION(when_the_second_call_for_amqpvalue_decode_bytes_fails_then_the_decoder_switches_to_an_error_state)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(1);

    STRICT_EXPECTED_CALL(test_on_sasl_frame_codec_error(TEST_CONTEXT));

    // act
    saved_on_frame_received(saved_callback_context, NULL, 0, test_sasl_frame_value, sizeof(test_sasl_frame_value));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_046: [If any error occurs while decoding a frame, the decoder shall switch to an error state where decoding shall not be possible anymore.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_049: [If any error occurs while decoding a frame, the decoder shall call the error_callback and pass to it the callback_context, both of those being the ones given to sasl_frame_codec_create.] */
TEST_FUNCTION(when_amqpvalue_get_inplace_descriptor_fails_then_the_decoder_switches_to_an_error_state)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    size_t i;
    umock_c_reset_all_calls();

    for (i = 0; i < sizeof(test_sasl_frame_value); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE))
        .SetReturn((AMQP_VALUE)NULL);

    STRICT_EXPECTED_CALL(test_on_sasl_frame_codec_error(TEST_CONTEXT));

    // act
    saved_on_frame_received(saved_callback_context, NULL, 0, test_sasl_frame_value, sizeof(test_sasl_frame_value));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_006: [Bytes 6 and 7 of the header are ignored.] */
TEST_FUNCTION(when_some_extra_type_specific_bytes_are_passed_to_the_sasl_codec_they_are_ignored)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, NULL);
    size_t i;
    unsigned char test_extra_bytes[2] = { 0x42, 0x43 };
    umock_c_reset_all_calls();

    for (i = 0; i < sizeof(test_sasl_frame_value); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    STRICT_EXPECTED_CALL(test_on_sasl_frame_received(NULL, TEST_AMQP_VALUE));

    // act
    saved_on_frame_received(saved_callback_context, test_extra_bytes, sizeof(test_extra_bytes), test_sasl_frame_value, sizeof(test_sasl_frame_value));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_007: [The extended header is ignored.] */
TEST_FUNCTION(when_type_specific_byte_count_is_more_than_2_the_sasl_frame_codec_ignores_them_and_still_succeeds)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, NULL);
    size_t i;
    unsigned char test_extra_bytes[4] = { 0x42, 0x43 };
    umock_c_reset_all_calls();

    for (i = 0; i < sizeof(test_sasl_frame_value); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    STRICT_EXPECTED_CALL(test_on_sasl_frame_received(NULL, TEST_AMQP_VALUE));

    // act
    saved_on_frame_received(saved_callback_context, test_extra_bytes, sizeof(test_extra_bytes), test_sasl_frame_value, sizeof(test_sasl_frame_value));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_008: [The maximum size of a SASL frame is defined by MIN-MAX-FRAME-SIZE.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_049: [If any error occurs while decoding a frame, the decoder shall call the error_callback and pass to it the callback_context, both of those being the ones given to sasl_frame_codec_create.] */
TEST_FUNCTION(when_a_sasl_frame_of_513_bytes_is_received_decoding_fails)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    unsigned char test_extra_bytes[2] = { 0x42, 0x43 };
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_sasl_frame_codec_error(TEST_CONTEXT));

    // act
    saved_on_frame_received(saved_callback_context, test_extra_bytes, sizeof(test_extra_bytes), test_sasl_frame_value, TEST_MIX_MAX_FRAME_SIZE - 8 + 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_008: [The maximum size of a SASL frame is defined by MIN-MAX-FRAME-SIZE.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_049: [If any error occurs while decoding a frame, the decoder shall call the error_callback and pass to it the callback_context, both of those being the ones given to sasl_frame_codec_create.] */
TEST_FUNCTION(when_a_sasl_frame_of_513_bytes_with_4_type_specific_bytes_is_received_decoding_fails)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    unsigned char test_extra_bytes[4] = { 0x42, 0x43 };
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_sasl_frame_codec_error(TEST_CONTEXT));

    // act
    saved_on_frame_received(saved_callback_context, test_extra_bytes, sizeof(test_extra_bytes), test_sasl_frame_value, TEST_MIX_MAX_FRAME_SIZE - 10 + 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_008: [The maximum size of a SASL frame is defined by MIN-MAX-FRAME-SIZE.] */
TEST_FUNCTION(when_the_frame_size_is_exactly_MIN_MAX_FRAME_SIZE_decoding_succeeds)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, NULL);
    size_t i;
    unsigned char test_extra_bytes[2] = { 0x42, 0x43 };
    unsigned char big_frame[512 - 8] = { 0x42, 0x43 };
    umock_c_reset_all_calls();

    test_sasl_frame_value_size = sizeof(big_frame);
    for (i = 0; i < sizeof(big_frame); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    STRICT_EXPECTED_CALL(test_on_sasl_frame_received(NULL, TEST_AMQP_VALUE));

    // act
    saved_on_frame_received(saved_callback_context, test_extra_bytes, sizeof(test_extra_bytes), big_frame, sizeof(big_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_009: [The frame body of a SASL frame MUST contain exactly one AMQP type, whose type encoding MUST have provides="sasl-frame".] */
/* Tests_SRS_SASL_FRAME_CODEC_01_049: [If any error occurs while decoding a frame, the decoder shall call the error_callback and pass to it the callback_context, both of those being the ones given to sasl_frame_codec_create.] */
TEST_FUNCTION(when_not_all_bytes_are_used_for_decoding_in_a_SASL_frame_then_decoding_fails)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    size_t i;
    unsigned char test_extra_bytes[2] = { 0x42, 0x43 };
    umock_c_reset_all_calls();

    test_sasl_frame_value_size = sizeof(test_sasl_frame_value) - 1;
    for (i = 0; i < sizeof(test_sasl_frame_value) - 1; i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE)).IgnoreAllCalls();
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_on_sasl_frame_codec_error(TEST_CONTEXT));

    // act
    saved_on_frame_received(saved_callback_context, test_extra_bytes, sizeof(test_extra_bytes), test_sasl_frame_value, sizeof(test_sasl_frame_value));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_009: [The frame body of a SASL frame MUST contain exactly one AMQP type, whose type encoding MUST have provides="sasl-frame".] */
TEST_FUNCTION(when_a_sasl_init_frame_is_received_decoding_it_succeeds)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, NULL);
    size_t i;
    umock_c_reset_all_calls();

    for (i = 0; i < sizeof(test_sasl_frame_value); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_init_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    STRICT_EXPECTED_CALL(test_on_sasl_frame_received(NULL, TEST_AMQP_VALUE));

    // act
    saved_on_frame_received(saved_callback_context, NULL, 0, test_sasl_frame_value, sizeof(test_sasl_frame_value));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_009: [The frame body of a SASL frame MUST contain exactly one AMQP type, whose type encoding MUST have provides="sasl-frame".] */
TEST_FUNCTION(when_a_sasl_challenge_frame_is_received_decoding_it_succeeds)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, NULL);
    size_t i;
    umock_c_reset_all_calls();

    for (i = 0; i < sizeof(test_sasl_frame_value); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_init_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_challenge_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    STRICT_EXPECTED_CALL(test_on_sasl_frame_received(NULL, TEST_AMQP_VALUE));

    // act
    saved_on_frame_received(saved_callback_context, NULL, 0, test_sasl_frame_value, sizeof(test_sasl_frame_value));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_009: [The frame body of a SASL frame MUST contain exactly one AMQP type, whose type encoding MUST have provides="sasl-frame".] */
TEST_FUNCTION(when_a_sasl_response_frame_is_received_decoding_it_succeeds)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, NULL);
    size_t i;
    umock_c_reset_all_calls();

    for (i = 0; i < sizeof(test_sasl_frame_value); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_init_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_challenge_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_response_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    STRICT_EXPECTED_CALL(test_on_sasl_frame_received(NULL, TEST_AMQP_VALUE));

    // act
    saved_on_frame_received(saved_callback_context, NULL, 0, test_sasl_frame_value, sizeof(test_sasl_frame_value));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_009: [The frame body of a SASL frame MUST contain exactly one AMQP type, whose type encoding MUST have provides="sasl-frame".] */
TEST_FUNCTION(when_a_sasl_outcome_frame_is_received_decoding_it_succeeds)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, NULL);
    size_t i;
    umock_c_reset_all_calls();

    for (i = 0; i < sizeof(test_sasl_frame_value); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_init_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_challenge_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_response_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_outcome_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    STRICT_EXPECTED_CALL(test_on_sasl_frame_received(NULL, TEST_AMQP_VALUE));

    // act
    saved_on_frame_received(saved_callback_context, NULL, 0, test_sasl_frame_value, sizeof(test_sasl_frame_value));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_009: [The frame body of a SASL frame MUST contain exactly one AMQP type, whose type encoding MUST have provides="sasl-frame".] */
/* Tests_SRS_SASL_FRAME_CODEC_01_049: [If any error occurs while decoding a frame, the decoder shall call the error_callback and pass to it the callback_context, both of those being the ones given to sasl_frame_codec_create.] */
TEST_FUNCTION(when_an_AMQP_value_that_is_not_a_sasl_frame_is_decoded_then_decoding_fails)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    size_t i;
    umock_c_reset_all_calls();

    for (i = 0; i < sizeof(test_sasl_frame_value); i++)
    {
        STRICT_EXPECTED_CALL(amqpvalue_decode_bytes(TEST_DECODER_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_AMQP_VALUE));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_init_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_challenge_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_response_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_outcome_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);

    STRICT_EXPECTED_CALL(test_on_sasl_frame_codec_error(TEST_CONTEXT));

    // act
    saved_on_frame_received(saved_callback_context, NULL, 0, test_sasl_frame_value, sizeof(test_sasl_frame_value));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

/* Tests_SRS_SASL_FRAME_CODEC_01_010: [Receipt of an empty frame is an irrecoverable error.] */
/* Tests_SRS_SASL_FRAME_CODEC_01_049: [If any error occurs while decoding a frame, the decoder shall call the error_callback and pass to it the callback_context, both of those being the ones given to sasl_frame_codec_create.] */
TEST_FUNCTION(when_an_empty_frame_is_received_decoding_fails)
{
    // arrange
    SASL_FRAME_CODEC_HANDLE sasl_frame_codec = sasl_frame_codec_create(TEST_FRAME_CODEC_HANDLE, test_on_sasl_frame_received, test_on_sasl_frame_codec_error, TEST_CONTEXT);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_sasl_frame_codec_error(TEST_CONTEXT));

    // act
    saved_on_frame_received(saved_callback_context, NULL, 0, test_sasl_frame_value, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_frame_codec_destroy(sasl_frame_codec);
}

END_TEST_SUITE(sasl_frame_codec_ut)
