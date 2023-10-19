// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#else
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"

void* real_malloc(size_t size)
{
    return malloc(size);
}

void real_free(void* ptr)
{
    free(ptr);
}

#define ENABLE_MOCKS

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/gb_rand.h"
#include "azure_c_shared_utility/buffer_.h"

#undef ENABLE_MOCKS

#include "azure_c_shared_utility/uws_frame_encoder.h"

#ifdef __cplusplus
extern "C" {
#endif

    extern BUFFER_HANDLE real_BUFFER_new(void);
    extern void real_BUFFER_delete(BUFFER_HANDLE handle);
    extern int real_BUFFER_enlarge(BUFFER_HANDLE handle, size_t enlargeSize);
    extern int real_BUFFER_size(BUFFER_HANDLE handle, size_t* size);
    extern int real_BUFFER_content(BUFFER_HANDLE handle, const unsigned char** content);
    extern unsigned char* real_BUFFER_u_char(BUFFER_HANDLE handle);
    extern size_t real_BUFFER_length(BUFFER_HANDLE handle);

#ifdef __cplusplus
}
#endif

static TEST_MUTEX_HANDLE g_testByTest;

static char expected_encoded_str[256];
static char actual_encoded_str[256];

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

static void stringify_bytes(const unsigned char* bytes, size_t byte_count, char* output_string, size_t string_buffer_size)
{
    size_t i;
    size_t pos = 0;

    output_string[pos++] = '[';
    for (i = 0; i < byte_count; i++)
    {
        (void)snprintf(&output_string[pos], string_buffer_size, "0x%02X", bytes[i]);
        if (i < byte_count - 1)
        {
            strcat(output_string, ",");
        }
        pos = strlen(output_string);
    }
    output_string[pos++] = ']';
    output_string[pos++] = '\0';
}

BEGIN_TEST_SUITE(uws_frame_encoder_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    REGISTER_GLOBAL_MOCK_HOOK(BUFFER_new, real_BUFFER_new);
    REGISTER_GLOBAL_MOCK_HOOK(BUFFER_delete, real_BUFFER_delete);
    REGISTER_GLOBAL_MOCK_HOOK(BUFFER_u_char, real_BUFFER_u_char);
    REGISTER_GLOBAL_MOCK_HOOK(BUFFER_enlarge, real_BUFFER_enlarge);

    REGISTER_UMOCK_ALIAS_TYPE(BUFFER_HANDLE, void*);
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
        ASSERT_FAIL("Could not acquire test serialization mutex.");
    }

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* uws_frame_encoder_encode */

/* Tests_SRS_UWS_FRAME_ENCODER_01_054: [ If length is greater than 0 and payload is NULL, then uws_frame_encoder_encode shall fail and return NULL. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_with_1_length_and_NULL_payload_fails)
{
    // arrange
    BUFFER_HANDLE result;

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, NULL, 1, false, true, 0);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_001: [ uws_frame_encoder_encode shall encode the information given in opcode, payload, length, is_masked, is_final and reserved according to the RFC6455 into a new buffer.]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_044: [ On success uws_frame_encoder_encode shall return a non-NULL handle to the result buffer. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_048: [ The newly created buffer shall be created by calling BUFFER_new. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_046: [ The result buffer shall be resized accordingly using BUFFER_enlarge. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_050: [ The allocated memory shall be accessed by calling BUFFER_u_char. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_002: [ Indicates that this is the final fragment in a message. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_003: [ The first fragment MAY also be the final fragment. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_015: [ Defines whether the "Payload data" is masked. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_018: [ The length of the "Payload data", in bytes: ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_043: [ if 0-125, that is the payload length. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_zero_length_binary_frame)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x82, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_049: [ If BUFFER_new fails then uws_frame_encoder_encode shall fail and return NULL. ]*/
TEST_FUNCTION(when_BUFFER_new_fails_then_uws_frame_encoder_encode_fails)
{
    // arrange
    BUFFER_HANDLE result;

    STRICT_EXPECTED_CALL(BUFFER_new())
        .SetReturn(NULL);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_047: [ If BUFFER_enlarge fails then uws_frame_encoder_encode shall fail and return NULL. ]*/
TEST_FUNCTION(when_BUFFER_enlarge_fails_then_uws_frame_encoder_encode_fails)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_051: [ If BUFFER_u_char fails then uws_frame_encoder_encode shall fail and return a NULL. ]*/
TEST_FUNCTION(when_BUFFER_u_char_fails_then_uws_frame_encoder_encode_fails)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer)
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_002: [ Indicates that this is the final fragment in a message. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_003: [ The first fragment MAY also be the final fragment. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_zero_length_binary_frame_that_is_not_final)
{
    // arrange
    unsigned char expected_bytes[] = { 0x02, 0x00 };
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, NULL, 0, false, false, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_004: [ MUST be 0 unless an extension is negotiated that defines meanings for non-zero values. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_zero_length_binary_frame_with_reserved_bits_set)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0xF2, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, NULL, 0, false, true, 7);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_052: [ If reserved has any bits set except the lowest 3 then uws_frame_encoder_encode shall fail and return NULL. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_zero_length_binary_frame_with_reserved_bits_having_all_bits_set_fails)
{
    // arrange
    BUFFER_HANDLE result;

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, NULL, 0, false, true, 0xFF);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_004: [ MUST be 0 unless an extension is negotiated that defines meanings for non-zero values. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_zero_length_binary_frame_with_RSV1_set)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0xC2, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, NULL, 0, false, true, RESERVED_1);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_004: [ MUST be 0 unless an extension is negotiated that defines meanings for non-zero values. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_zero_length_binary_frame_with_RSV2_set)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0xA2, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, NULL, 0, false, true, RESERVED_2);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_004: [ MUST be 0 unless an extension is negotiated that defines meanings for non-zero values. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_zero_length_binary_frame_with_RSV3_set)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x92, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, NULL, 0, false, true, RESERVED_3);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_006: [ If an unknown opcode is received, the receiving endpoint MUST _Fail the WebSocket Connection_. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_with_opcode_16_fails)
{
    // arrange
    BUFFER_HANDLE result;

    // act
    result = uws_frame_encoder_encode((WS_FRAME_TYPE)0x10, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_007: [ *  %x0 denotes a continuation frame ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_continuation_frame)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x80, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_CONTINUATION_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_008: [ *  %x1 denotes a text frame ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_text_frame)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x81, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_TEXT_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_009: [ *  %x2 denotes a binary frame ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_binary_frame)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x82, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_010: [ *  %x3-7 are reserved for further non-control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_non_control_frame_3)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x83, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_RESERVED_NON_CONTROL_FRAME_3, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_010: [ *  %x3-7 are reserved for further non-control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_non_control_frame_4)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x84, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_RESERVED_NON_CONTROL_FRAME_4, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_010: [ *  %x3-7 are reserved for further non-control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_non_control_frame_5)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x85, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_RESERVED_NON_CONTROL_FRAME_5, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_010: [ *  %x3-7 are reserved for further non-control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_non_control_frame_6)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x86, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_RESERVED_NON_CONTROL_FRAME_6, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_010: [ *  %x3-7 are reserved for further non-control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_non_control_frame_7)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x87, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_RESERVED_NON_CONTROL_FRAME_7, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_011: [ *  %x8 denotes a connection close ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_close_frame)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x88, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_CLOSE_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_012: [ *  %x9 denotes a ping ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_ping_frame)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x89, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_PING_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_013: [ *  %xA denotes a pong ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_pong_frame)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x8A, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_PONG_FRAME, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_014: [ *  %xB-F are reserved for further control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_control_frame_B)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x8B, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_RESERVED_CONTROL_FRAME_B, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_014: [ *  %xB-F are reserved for further control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_control_frame_C)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x8C, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_RESERVED_CONTROL_FRAME_C, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_014: [ *  %xB-F are reserved for further control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_control_frame_D)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x8D, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_RESERVED_CONTROL_FRAME_D, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_014: [ *  %xB-F are reserved for further control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_control_frame_E)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x8E, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_RESERVED_CONTROL_FRAME_E, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_014: [ *  %xB-F are reserved for further control frames ]*/
TEST_FUNCTION(uws_frame_encoder_encodes_a_reserved_control_frame_F)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x8F, 0x00 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_RESERVED_CONTROL_FRAME_F, NULL, 0, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_015: [ Defines whether the "Payload data" is masked. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_053: [ In order to obtain a 32 bit value for masking, gb_rand shall be used 4 times (for each byte). ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_016: [ If set to 1, a masking key is present in masking-key, and this is used to unmask the "Payload data" as per Section 5.3. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_026: [ This field is present if the mask bit is set to 1 and is absent if the mask bit is set to 0. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_042: [ The payload length, indicated in the framing as frame-payload-length, does NOT include the length of the masking key. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_masked_zero_length_binary_frame)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x82, 0x80, 0xFF, 0xFF, 0xFF, 0xFF };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, sizeof(expected_bytes)))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0xFF);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0xFF);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0xFF);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0xFF);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, NULL, 0, true, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_015: [ Defines whether the "Payload data" is masked. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_053: [ In order to obtain a 32 bit value for masking, gb_rand shall be used 4 times (for each byte). ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_016: [ If set to 1, a masking key is present in masking-key, and this is used to unmask the "Payload data" as per Section 5.3. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_026: [ This field is present if the mask bit is set to 1 and is absent if the mask bit is set to 0. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_042: [ The payload length, indicated in the framing as frame-payload-length, does NOT include the length of the masking key. ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_masked_zero_length_binary_frame_different_mask)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char expected_bytes[] = { 0x82, 0x80, 0x42, 0x43, 0x44, 0x45 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, sizeof(expected_bytes)))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0x42);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0x43);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0x44);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0x45);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, NULL, 0, true, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_043: [ if 0-125, that is the payload length. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_023: [ The payload length is the length of the "Extension data" + the length of the "Application data". ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_1_byte_long_binary_frame)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char payload[] = { 0x42 };
    unsigned char expected_bytes[] = { 0x82, 0x01, 0x42 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, sizeof(expected_bytes)))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, payload, sizeof(payload), false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_043: [ if 0-125, that is the payload length. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_022: [ Note that in all cases, the minimal number of bytes MUST be used to encode the length, for example, the length of a 124-byte-long string can't be encoded as the sequence 126, 0, 124. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_023: [ The payload length is the length of the "Extension data" + the length of the "Application data". ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_125_byte_long_binary_frame)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char* payload = (unsigned char*)real_malloc(125);
    unsigned char* expected_bytes = (unsigned char*)real_malloc(125 + 2);
    char* temp_expected_str = (char*)real_malloc(200 * 5);
    char* temp_actual_str = (char*)real_malloc(200 * 5);
    size_t i;

    expected_bytes[0] = 0x82;
    expected_bytes[1] = 0x7D;

    for (i = 0; i < 125; i++)
    {
        payload[i] = (unsigned char)i;
        expected_bytes[i + 2] = (unsigned char)i;
    }

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 125 + 2))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, payload, 125, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, 125 + 2, temp_expected_str, 200 * 5);
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), temp_actual_str, 200 * 5);
    ASSERT_ARE_EQUAL(char_ptr, temp_expected_str, temp_actual_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_free(temp_expected_str);
    real_free(temp_actual_str);
    real_free(expected_bytes);
    real_free(payload);
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_019: [ If 126, the following 2 bytes interpreted as a 16-bit unsigned integer are the payload length. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_023: [ The payload length is the length of the "Extension data" + the length of the "Application data". ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_126_byte_long_binary_frame)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char* payload = (unsigned char*)real_malloc(126);
    unsigned char* expected_bytes = (unsigned char*)real_malloc(126 + 4);
    char* temp_expected_str = (char*)real_malloc(200 * 5);
    char* temp_actual_str = (char*)real_malloc(200 * 5);
    size_t i;

    expected_bytes[0] = 0x82;
    expected_bytes[1] = 0x7E;
    expected_bytes[2] = 0x00;
    expected_bytes[3] = 0x7E;

    for (i = 0; i < 126; i++)
    {
        payload[i] = (unsigned char)i;
        expected_bytes[i + 4] = (unsigned char)i;
    }

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 126 + 4))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, payload, 126, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, 126 + 4, temp_expected_str, 200 * 5);
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), temp_actual_str, 200 * 5);
    ASSERT_ARE_EQUAL(char_ptr, temp_expected_str, temp_actual_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_free(temp_expected_str);
    real_free(temp_actual_str);
    real_free(expected_bytes);
    real_free(payload);
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_019: [ If 126, the following 2 bytes interpreted as a 16-bit unsigned integer are the payload length. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_021: [ Multibyte length quantities are expressed in network byte order. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_022: [ Note that in all cases, the minimal number of bytes MUST be used to encode the length, for example, the length of a 124-byte-long string can't be encoded as the sequence 126, 0, 124. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_023: [ The payload length is the length of the "Extension data" + the length of the "Application data". ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_65535_byte_long_binary_frame)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char* payload = (unsigned char*)real_malloc(65535);
    unsigned char* expected_bytes = (unsigned char*)real_malloc(65535 + 4);
    uint32_t i;

    expected_bytes[0] = 0x82;
    expected_bytes[1] = 0x7E;
    expected_bytes[2] = 0xFF;
    expected_bytes[3] = 0xFF;

    for (i = 0; i < 65535; i++)
    {
        payload[i] = (unsigned char)i;
        expected_bytes[i + 4] = (unsigned char)i;
    }

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 65535 + 4))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, payload, 65535, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(size_t, 65535 + 4, real_BUFFER_length(result));
    ASSERT_ARE_EQUAL(int, 0, memcmp(expected_bytes, real_BUFFER_u_char(result), real_BUFFER_length(result)), "Memory compare failed");
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_free(expected_bytes);
    real_free(payload);
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_020: [ If 127, the following 8 bytes interpreted as a 64-bit unsigned integer (the most significant bit MUST be 0) are the payload length. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_021: [ Multibyte length quantities are expressed in network byte order. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_023: [ The payload length is the length of the "Extension data" + the length of the "Application data". ]*/
TEST_FUNCTION(uws_frame_encoder_encode_encodes_a_65536_byte_long_binary_frame)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char* payload = (unsigned char*)real_malloc(65536);
    unsigned char* expected_bytes = (unsigned char*)real_malloc(65536 + 10);
    uint32_t i;

    expected_bytes[0] = 0x82;
    expected_bytes[1] = 0x7F;
    expected_bytes[2] = 0x00;
    expected_bytes[3] = 0x00;
    expected_bytes[4] = 0x00;
    expected_bytes[5] = 0x00;
    expected_bytes[6] = 0x00;
    expected_bytes[7] = 0x01;
    expected_bytes[8] = 0x00;
    expected_bytes[9] = 0x00;

    for (i = 0; i < 65536; i++)
    {
        payload[i] = (unsigned char)i;
        expected_bytes[i + 10] = (unsigned char)i;
    }

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, 65536 + 10))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, payload, 65536, false, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(size_t, 65536 + 10, real_BUFFER_length(result));
    ASSERT_ARE_EQUAL(int, 0, memcmp(expected_bytes, real_BUFFER_u_char(result), real_BUFFER_length(result)), "Memory compare failed");
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_free(expected_bytes);
    real_free(payload);
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_033: [ A masked frame MUST have the field frame-masked set to 1, as defined in Section 5.2. **]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_034: [ The masking key is contained completely within the frame, as defined in Section 5.2 as frame-masking-key. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_035: [ It is used to mask the "Payload data" defined in the same section as frame-payload-data, which includes "Extension data" and "Application data". ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_036: [ The masking key is a 32-bit value chosen at random by the client. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_037: [ When preparing a masked frame, the client MUST pick a fresh masking key from the set of allowed 32-bit values. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_038: [ The masking key needs to be unpredictable; thus, the masking key MUST be derived from a strong source of entropy, and the masking key for a given frame MUST NOT make it simple for a server/proxy to predict the masking key for a subsequent frame. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_039: [ To convert masked data into unmasked data, or vice versa, the following algorithm is applied. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_040: [ The same algorithm applies regardless of the direction of the translation, e.g., the same steps are applied to mask the data as to unmask the data. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_041: [ Octet i of the transformed data ("transformed-octet-i") is the XOR of octet i of the original data ("original-octet-i") with octet at index i modulo 4 of the masking key ("masking-key-octet-j"): ]*/
TEST_FUNCTION(uws_frame_encoder_encode_masks_a_1_byte_frame_with_0_as_mask)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char payload[] = { 0x42 };
    unsigned char expected_bytes[] = { 0x82, 0x81, 0x00, 0x00, 0x00, 0x00, 0x42 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, sizeof(expected_bytes)))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0x00);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0x00);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0x00);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0x00);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, payload, sizeof(payload), true, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_033: [ A masked frame MUST have the field frame-masked set to 1, as defined in Section 5.2. **]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_034: [ The masking key is contained completely within the frame, as defined in Section 5.2 as frame-masking-key. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_035: [ It is used to mask the "Payload data" defined in the same section as frame-payload-data, which includes "Extension data" and "Application data". ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_036: [ The masking key is a 32-bit value chosen at random by the client. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_037: [ When preparing a masked frame, the client MUST pick a fresh masking key from the set of allowed 32-bit values. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_038: [ The masking key needs to be unpredictable; thus, the masking key MUST be derived from a strong source of entropy, and the masking key for a given frame MUST NOT make it simple for a server/proxy to predict the masking key for a subsequent frame. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_039: [ To convert masked data into unmasked data, or vice versa, the following algorithm is applied. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_040: [ The same algorithm applies regardless of the direction of the translation, e.g., the same steps are applied to mask the data as to unmask the data. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_041: [ Octet i of the transformed data ("transformed-octet-i") is the XOR of octet i of the original data ("original-octet-i") with octet at index i modulo 4 of the masking key ("masking-key-octet-j"): ]*/
TEST_FUNCTION(uws_frame_encoder_encode_masks_a_1_byte_frame_with_0xFF_as_mask)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char payload[] = { 0x42 };
    unsigned char expected_bytes[] = { 0x82, 0x81, 0xFF, 0x00, 0x00, 0x00, 0xBD };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, sizeof(expected_bytes)))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0xFF);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0x00);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0x00);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0x00);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, payload, sizeof(payload), true, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_033: [ A masked frame MUST have the field frame-masked set to 1, as defined in Section 5.2. **]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_034: [ The masking key is contained completely within the frame, as defined in Section 5.2 as frame-masking-key. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_035: [ It is used to mask the "Payload data" defined in the same section as frame-payload-data, which includes "Extension data" and "Application data". ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_036: [ The masking key is a 32-bit value chosen at random by the client. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_037: [ When preparing a masked frame, the client MUST pick a fresh masking key from the set of allowed 32-bit values. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_038: [ The masking key needs to be unpredictable; thus, the masking key MUST be derived from a strong source of entropy, and the masking key for a given frame MUST NOT make it simple for a server/proxy to predict the masking key for a subsequent frame. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_039: [ To convert masked data into unmasked data, or vice versa, the following algorithm is applied. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_040: [ The same algorithm applies regardless of the direction of the translation, e.g., the same steps are applied to mask the data as to unmask the data. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_041: [ Octet i of the transformed data ("transformed-octet-i") is the XOR of octet i of the original data ("original-octet-i") with octet at index i modulo 4 of the masking key ("masking-key-octet-j"): ]*/
TEST_FUNCTION(uws_frame_encoder_encode_masks_a_4_byte_frame_with_0xFF_as_mask)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char payload[] = { 0x42, 0x43, 0x44, 0x45 };
    unsigned char expected_bytes[] = { 0x82, 0x84, 0xFF, 0xFF, 0xFF, 0xFF, 0xBD, 0xBC, 0xBB, 0xBA };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, sizeof(expected_bytes)))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0xFF);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0xFF);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0xFF);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0xFF);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, payload, sizeof(payload), true, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_033: [ A masked frame MUST have the field frame-masked set to 1, as defined in Section 5.2. **]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_034: [ The masking key is contained completely within the frame, as defined in Section 5.2 as frame-masking-key. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_035: [ It is used to mask the "Payload data" defined in the same section as frame-payload-data, which includes "Extension data" and "Application data". ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_036: [ The masking key is a 32-bit value chosen at random by the client. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_037: [ When preparing a masked frame, the client MUST pick a fresh masking key from the set of allowed 32-bit values. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_038: [ The masking key needs to be unpredictable; thus, the masking key MUST be derived from a strong source of entropy, and the masking key for a given frame MUST NOT make it simple for a server/proxy to predict the masking key for a subsequent frame. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_039: [ To convert masked data into unmasked data, or vice versa, the following algorithm is applied. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_040: [ The same algorithm applies regardless of the direction of the translation, e.g., the same steps are applied to mask the data as to unmask the data. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_041: [ Octet i of the transformed data ("transformed-octet-i") is the XOR of octet i of the original data ("original-octet-i") with octet at index i modulo 4 of the masking key ("masking-key-octet-j"): ]*/
TEST_FUNCTION(uws_frame_encoder_encode_masks_a_5_byte_frame_with_0xFF_as_mask)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char payload[] = { 0x42, 0x43, 0x44, 0x45, 0x01 };
    unsigned char expected_bytes[] = { 0x82, 0x85, 0xFF, 0xFF, 0xFF, 0xFF, 0xBD, 0xBC, 0xBB, 0xBA, 0xFE };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, sizeof(expected_bytes)))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0xFF);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0xFF);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0xFF);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0xFF);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, payload, sizeof(payload), true, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

/* Tests_SRS_UWS_FRAME_ENCODER_01_033: [ A masked frame MUST have the field frame-masked set to 1, as defined in Section 5.2. **]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_034: [ The masking key is contained completely within the frame, as defined in Section 5.2 as frame-masking-key. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_035: [ It is used to mask the "Payload data" defined in the same section as frame-payload-data, which includes "Extension data" and "Application data". ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_036: [ The masking key is a 32-bit value chosen at random by the client. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_037: [ When preparing a masked frame, the client MUST pick a fresh masking key from the set of allowed 32-bit values. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_038: [ The masking key needs to be unpredictable; thus, the masking key MUST be derived from a strong source of entropy, and the masking key for a given frame MUST NOT make it simple for a server/proxy to predict the masking key for a subsequent frame. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_039: [ To convert masked data into unmasked data, or vice versa, the following algorithm is applied. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_040: [ The same algorithm applies regardless of the direction of the translation, e.g., the same steps are applied to mask the data as to unmask the data. ]*/
/* Tests_SRS_UWS_FRAME_ENCODER_01_041: [ Octet i of the transformed data ("transformed-octet-i") is the XOR of octet i of the original data ("original-octet-i") with octet at index i modulo 4 of the masking key ("masking-key-octet-j"): ]*/
TEST_FUNCTION(uws_frame_encoder_encode_masks_a_8_byte_frame_with_different_mask_bytes)
{
    // arrange
    BUFFER_HANDLE result;
    BUFFER_HANDLE newly_created_buffer;
    unsigned char payload[] = { 0x42, 0x43, 0x44, 0x45, 0x01, 0x02, 0xFF, 0xAA };
    unsigned char expected_bytes[] = { 0x82, 0x88, 0x00, 0xFF, 0xAA, 0x42, 0x42, 0xBC, 0xEE, 0x07, 0x01, 0xFD, 0x55, 0xE8 };

    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_enlarge(IGNORED_PTR_ARG, sizeof(expected_bytes)))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&newly_created_buffer);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0x00);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0xFF);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0xAA);
    STRICT_EXPECTED_CALL(gb_rand())
        .SetReturn(0x42);

    // act
    result = uws_frame_encoder_encode(WS_BINARY_FRAME, payload, sizeof(payload), true, true, 0);

    // assert
    ASSERT_IS_NOT_NULL(result);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_encoded_str, sizeof(expected_encoded_str));
    stringify_bytes(real_BUFFER_u_char(result), real_BUFFER_length(result), actual_encoded_str, sizeof(actual_encoded_str));
    ASSERT_ARE_EQUAL(char_ptr, expected_encoded_str, actual_encoded_str);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    real_BUFFER_delete(result);
}

END_TEST_SUITE(uws_frame_encoder_ut)
