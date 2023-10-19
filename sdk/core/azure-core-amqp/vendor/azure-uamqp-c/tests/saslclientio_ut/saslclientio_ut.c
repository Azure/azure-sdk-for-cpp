// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstdint>
#else
#include <stdlib.h>
#include <stdint.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_stdint.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_bool.h"

#if defined _MSC_VER
#pragma warning(disable: 4054) /* MSC incorrectly fires this */
#endif

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void* my_gballoc_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

static void* my_gballoc_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

#define ENABLE_MOCKS

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_uamqp_c/amqpvalue.h"
#include "azure_uamqp_c/amqpvalue_to_string.h"
#include "azure_uamqp_c/frame_codec.h"
#include "azure_uamqp_c/sasl_frame_codec.h"
#include "azure_uamqp_c/sasl_mechanism.h"
#include "azure_uamqp_c/amqp_definitions.h"

#undef ENABLE_MOCKS

#include "azure_uamqp_c/saslclientio.h"

static XIO_HANDLE test_underlying_io = (XIO_HANDLE)0x4242;
static SASL_MECHANISM_HANDLE test_sasl_mechanism = (SASL_MECHANISM_HANDLE)0x4243;
static FRAME_CODEC_HANDLE test_frame_codec = (FRAME_CODEC_HANDLE)0x4244;
static SASL_FRAME_CODEC_HANDLE test_sasl_frame_codec = (SASL_FRAME_CODEC_HANDLE)0x4245;
static AMQP_VALUE test_descriptor_value = (AMQP_VALUE)0x4246;
static AMQP_VALUE test_sasl_server_mechanism = (AMQP_VALUE)0x4247;
static const char* test_mechanism = "test_mechanism";
static SASL_OUTCOME_HANDLE test_sasl_outcome_handle = (SASL_OUTCOME_HANDLE)0x4243;
static SASL_INIT_HANDLE test_sasl_init = (SASL_INIT_HANDLE)0x4244;
static AMQP_VALUE test_sasl_init_value = (AMQP_VALUE)0x4245;
static OPTIONHANDLER_HANDLE test_optionhandler_handle = (OPTIONHANDLER_HANDLE)0x4246;
static SASL_MECHANISMS_HANDLE test_sasl_mechanisms_handle = (SASL_MECHANISMS_HANDLE)0x5001;
static AMQP_VALUE test_sasl_server_mechanisms_value = (AMQP_VALUE)0x5002;
static SASL_CHALLENGE_HANDLE test_sasl_challenge_handle = (SASL_CHALLENGE_HANDLE)0x5003;
static SASL_RESPONSE_HANDLE test_sasl_response_handle = (SASL_RESPONSE_HANDLE)0x5004;
static AMQP_VALUE test_sasl_response_amqp_value = (AMQP_VALUE)0x5005;

static ON_IO_OPEN_COMPLETE saved_on_io_open_complete;
static void* saved_on_io_open_complete_context;
static ON_BYTES_RECEIVED saved_on_bytes_received;
static void* saved_on_bytes_received_context;
static ON_IO_ERROR saved_on_io_error;
static void* saved_on_io_error_context;
static ON_IO_CLOSE_COMPLETE saved_on_io_close_complete;
static void* saved_on_io_close_complete_context;

static ON_SASL_FRAME_RECEIVED saved_on_sasl_frame_received;
static ON_SASL_FRAME_CODEC_ERROR saved_on_sasl_frame_codec_error;
static void* saved_sasl_frame_codec_callback_context;

static ON_FRAME_RECEIVED saved_frame_received_callback;
static void* saved_frame_received_callback_context;

static ON_BYTES_ENCODED saved_on_bytes_encoded;
static void* saved_on_bytes_encoded_callback_context;

static ON_FRAME_CODEC_ERROR saved_on_frame_codec_error;
static void* saved_on_frame_codec_error_callback_context;

/* Tests_SRS_SASLCLIENTIO_01_002: [The protocol header consists of the upper case ASCII letters "AMQP" followed by a protocol id of three, followed by three unsigned bytes representing the major, minor, and revision of the specification version (currently 1 (SASL-MAJOR), 0 (SASLMINOR), 0 (SASL-REVISION)).] */
/* Tests_SRS_SASLCLIENTIO_01_124: [SASL-MAJOR 1 major protocol version.] */
/* Tests_SRS_SASLCLIENTIO_01_125: [SASL-MINOR 0 minor protocol version.] */
/* Tests_SRS_SASLCLIENTIO_01_126: [SASL-REVISION 0 protocol revision.] */
static const unsigned char sasl_header[] = { 'A', 'M', 'Q', 'P', 3, 1, 0, 0 };
static const unsigned char test_sasl_mechanisms_frame[] = { 'x', '1' }; /* these are some dummy bytes */
static const unsigned char test_sasl_outcome[] = { 'x', '2' }; /* these are some dummy bytes */
static const unsigned char test_sasl_challenge[] = { 'x', '3' }; /* these are some dummy bytes */

static AMQP_VALUE test_sasl_value = (AMQP_VALUE)0x5242;

static char expected_stringified_io[8192];
static char actual_stringified_io[8192];
static unsigned char* frame_codec_received_bytes;
static size_t frame_codec_received_byte_count;
static unsigned char* io_send_bytes;
static size_t io_send_byte_count;

static int umocktypes_copy_SASL_MECHANISM_BYTES_ptr(SASL_MECHANISM_BYTES** destination, const SASL_MECHANISM_BYTES** source)
{
    int result;

    if (*source == NULL)
    {
        *destination = NULL;
        result = 0;
    }
    else
    {
        *destination = (SASL_MECHANISM_BYTES*)my_gballoc_malloc(sizeof(SASL_MECHANISM_BYTES));
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

static void umocktypes_free_SASL_MECHANISM_BYTES_ptr(SASL_MECHANISM_BYTES** value)
{
    if (*value != NULL)
    {
        my_gballoc_free((void*)(*value)->bytes);
        my_gballoc_free(*value);
    }
}

static char* umocktypes_stringify_SASL_MECHANISM_BYTES_ptr(const SASL_MECHANISM_BYTES** value)
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
                (void)sprintf(&result[pos], "0x%02X ", ((unsigned char*)(*value)->bytes)[i]);
                pos += 5;
            }
            result[pos++] = ']';
            result[pos++] = '\0';
        }
    }

    return result;
}

static int umocktypes_are_equal_SASL_MECHANISM_BYTES_ptr(SASL_MECHANISM_BYTES** left, SASL_MECHANISM_BYTES** right)
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

static const unsigned char test_challenge_bytes[] = { 0x42 };
static const unsigned char test_response_bytes[] = { 0x43, 0x44 };
static const amqp_binary some_challenge_bytes = { test_challenge_bytes, sizeof(test_challenge_bytes) };
static const SASL_MECHANISM_BYTES sasl_mechanism_challenge_bytes = { test_challenge_bytes, sizeof(test_challenge_bytes) };
static const SASL_MECHANISM_BYTES sasl_mechanism_response_bytes = { test_response_bytes, sizeof(test_response_bytes) };
static const amqp_binary response_binary_value = { test_response_bytes, sizeof(test_response_bytes) };

static FRAME_CODEC_HANDLE my_frame_codec_create(ON_FRAME_CODEC_ERROR on_frame_codec_error, void* callback_context)
{
    saved_on_frame_codec_error = on_frame_codec_error;
    saved_on_frame_codec_error_callback_context = callback_context;
    return test_frame_codec;
}

static int my_frame_codec_receive_bytes(FRAME_CODEC_HANDLE frame_codec, const unsigned char* buffer, size_t size)
{
    unsigned char* new_bytes = (unsigned char*)my_gballoc_realloc(frame_codec_received_bytes, frame_codec_received_byte_count + size);
    (void)frame_codec;
    if (new_bytes != NULL)
    {
        frame_codec_received_bytes = new_bytes;
        (void)memcpy(frame_codec_received_bytes + frame_codec_received_byte_count, buffer, size);
        frame_codec_received_byte_count += size;
    }
    return 0;
}

static int my_frame_codec_subscribe(FRAME_CODEC_HANDLE frame_codec, uint8_t type, ON_FRAME_RECEIVED frame_received_callback, void* callback_context)
{
    (void)type;
    (void)frame_codec;
    saved_frame_received_callback = frame_received_callback;
    saved_frame_received_callback_context = callback_context;
    return 0;
}

static SASL_FRAME_CODEC_HANDLE my_sasl_frame_codec_create(FRAME_CODEC_HANDLE frame_codec, ON_SASL_FRAME_RECEIVED on_sasl_frame_received, ON_SASL_FRAME_CODEC_ERROR on_sasl_frame_codec_error, void* callback_context)
{
    (void)frame_codec;
    saved_on_sasl_frame_received = on_sasl_frame_received;
    saved_sasl_frame_codec_callback_context = callback_context;
    saved_on_sasl_frame_codec_error = on_sasl_frame_codec_error;
    return test_sasl_frame_codec;
}

static int my_sasl_frame_codec_encode_frame(SASL_FRAME_CODEC_HANDLE sasl_frame_codec, AMQP_VALUE sasl_frame_value, ON_BYTES_ENCODED on_bytes_encoded, void* callback_context)
{
    (void)sasl_frame_codec;
    (void)sasl_frame_value;
    saved_on_bytes_encoded = on_bytes_encoded;
    saved_on_bytes_encoded_callback_context = callback_context;
    return 0;
}

static int my_xio_open(XIO_HANDLE xio, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    (void)xio;
    saved_on_io_open_complete = on_io_open_complete;
    saved_on_io_open_complete_context = on_io_open_complete_context;
    saved_on_bytes_received = on_bytes_received;
    saved_on_bytes_received_context = on_bytes_received_context;
    saved_on_io_error = on_io_error;
    saved_on_io_error_context = on_io_error_context;
    return 0;
}

static int my_xio_close(XIO_HANDLE xio, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
    (void)xio;
    saved_on_io_close_complete = on_io_close_complete;
    saved_on_io_close_complete_context = callback_context;
    return 0;
}

static int my_xio_send(XIO_HANDLE xio, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    unsigned char* new_bytes = (unsigned char*)my_gballoc_realloc(io_send_bytes, io_send_byte_count + size);
    (void)callback_context;
    (void)on_send_complete;
    (void)xio;
    if (new_bytes != NULL)
    {
        io_send_bytes = new_bytes;
        (void)memcpy(io_send_bytes + io_send_byte_count, buffer, size);
        io_send_byte_count += size;
    }
    return 0;
}

MOCK_FUNCTION_WITH_CODE(, void, test_on_bytes_received, void*, context, const unsigned char*, buffer, size_t, size)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_io_open_complete, void*, context, IO_OPEN_RESULT, io_open_result)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_io_error, void*, context)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_send_complete, void*, context, IO_SEND_RESULT, send_result)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_io_close_complete, void*, context)
MOCK_FUNCTION_END()

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)
TEST_DEFINE_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);
TEST_DEFINE_ENUM_TYPE(OPTIONHANDLER_RESULT, OPTIONHANDLER_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(OPTIONHANDLER_RESULT, OPTIONHANDLER_RESULT_VALUES);

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

static int umocktypes_copy_amqp_binary(amqp_binary* destination, const amqp_binary* source)
{
    int result;

    if (source->length > 0)
    {
        destination->bytes = (unsigned char*)my_gballoc_malloc(source->length);
        if (destination->bytes == NULL)
        {
            result = MU_FAILURE;
        }
        else
        {
            if (source->length > 0)
            {
                (void)memcpy((void*)destination->bytes, source->bytes, source->length);
            }

            result = 0;
        }
    }
    else
    {
        result = 0;
    }

    if (result == 0)
    {
        destination->length = source->length;
    }

    return result;
}

static void umocktypes_free_amqp_binary(amqp_binary* value)
{
    if (value->bytes != NULL)
    {
        my_gballoc_free((void*)value->bytes);
    }
}

static char* umocktypes_stringify_amqp_binary(const amqp_binary* value)
{
    char* result;

    result = (char*)my_gballoc_malloc(3 + (5 * value->length));
    if (result != NULL)
    {
        size_t pos = 0;
        size_t i;

        result[pos++] = '[';
        for (i = 0; i < value->length; i++)
        {
            (void)sprintf(&result[pos], "0x%02X ", ((const unsigned char*)value->bytes)[i]);
            pos += 5;
        }
        result[pos++] = ']';
        result[pos++] = '\0';
    }

    return result;
}

static int umocktypes_are_equal_amqp_binary(amqp_binary* left, amqp_binary* right)
{
    int result;

    if (left->length != right->length)
    {
        result = 0;
    }
    else
    {
        if (left->length == 0)
        {
            result = 1;
        }
        else
        {
            result = (memcmp(left->bytes, right->bytes, left->length) == 0) ? 1 : 0;
        }
    }

    return result;
}

static int umocktypes_copy_bool_ptr(bool** destination, const bool** source)
{
    int result;

    *destination = (bool*)my_gballoc_malloc(sizeof(bool));
    if (*destination == NULL)
    {
        result = MU_FAILURE;
    }
    else
    {
        *(*destination) = *(*source);

        result = 0;
    }

    return result;
}

static void umocktypes_free_bool_ptr(bool** value)
{
    if (*value != NULL)
    {
        my_gballoc_free(*value);
    }
}

static char* umocktypes_stringify_bool_ptr(const bool** value)
{
    char* result;

    result = (char*)my_gballoc_malloc(8);
    if (result != NULL)
    {
        if (*value == NULL)
        {
            (void)strcpy(result, "{NULL}");
        }
        else if (*(*value) == true)
        {
            (void)strcpy(result, "{true}");
        }
        else
        {
            (void)strcpy(result, "{false}");
        }
    }

    return result;
}

static int umocktypes_are_equal_bool_ptr(bool** left, bool** right)
{
    int result;

    if (*left == *right)
    {
        result = 1;
    }
    else
    {
        if (*(*left) == *(*right))
        {
            result = 1;
        }
        else
        {
            result = 0;
        }
    }

    return result;
}

static void setup_successful_sasl_handshake(void)
{
    sasl_code sasl_outcome_code = sasl_code_ok;
    uint32_t mechanism_count = 1;
    SASL_MECHANISM_BYTES init_bytes;

    init_bytes.length = 0;
    init_bytes.bytes = NULL;

    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_mechanisms(test_sasl_value, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_mechanisms_get_sasl_server_mechanisms(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item_count(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_count(&mechanism_count, sizeof(mechanism_count));
    STRICT_EXPECTED_CALL(saslmechanism_get_mechanism_name(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_symbol(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_mechanism, sizeof(test_mechanism));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_init_create(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(saslmechanism_get_init_bytes(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &init_bytes, sizeof(init_bytes));
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_outcome, sizeof(test_sasl_outcome));
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_challenge_type_by_descriptor(test_descriptor_value))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_outcome_type_by_descriptor(test_descriptor_value))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_outcome(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_outcome_handle, sizeof(test_sasl_outcome_handle));
    STRICT_EXPECTED_CALL(sasl_outcome_get_code(test_sasl_outcome_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &sasl_outcome_code, sizeof(sasl_outcome_code));
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
}

static void setup_send_init(void)
{
    uint32_t mechanisms_count = 1;
    SASL_MECHANISM_BYTES init_bytes;
    unsigned char test_init_bytes[] = { 0x42 };

    init_bytes.bytes = test_init_bytes;
    init_bytes.length = sizeof(test_init_bytes);

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_mechanisms(test_sasl_value, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_mechanisms_get_sasl_server_mechanisms(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item_count(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_count(&mechanisms_count, sizeof(mechanisms_count));
    STRICT_EXPECTED_CALL(saslmechanism_get_mechanism_name(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_symbol(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_mechanism, sizeof(test_mechanism));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_init_create(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(saslmechanism_get_init_bytes(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &init_bytes, sizeof(init_bytes));
    STRICT_EXPECTED_CALL(amqpvalue_create_sasl_init(test_sasl_init));
    STRICT_EXPECTED_CALL(sasl_frame_codec_encode_frame(test_sasl_frame_codec, test_sasl_init_value, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_init_destroy(test_sasl_init));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_sasl_init_value));
    STRICT_EXPECTED_CALL(sasl_mechanisms_destroy(test_sasl_mechanisms_handle));
}

BEGIN_TEST_SUITE(saslclientio_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result, "Failed registering stdint types");

    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result, "Failed registering charptr types");

    result = umocktypes_bool_register_types();
    ASSERT_ARE_EQUAL(int, 0, result, "Failed registering bool types");

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, my_gballoc_calloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_HOOK(frame_codec_create, my_frame_codec_create);
    REGISTER_GLOBAL_MOCK_HOOK(frame_codec_receive_bytes, my_frame_codec_receive_bytes);
    REGISTER_GLOBAL_MOCK_HOOK(frame_codec_subscribe, my_frame_codec_subscribe);
    REGISTER_GLOBAL_MOCK_RETURN(frame_codec_unsubscribe, 0);
    REGISTER_GLOBAL_MOCK_RETURN(frame_codec_encode_frame, 0);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_create_sasl_response, test_sasl_response_amqp_value);
    REGISTER_GLOBAL_MOCK_HOOK(sasl_frame_codec_create, my_sasl_frame_codec_create);
    REGISTER_GLOBAL_MOCK_HOOK(sasl_frame_codec_encode_frame, my_sasl_frame_codec_encode_frame);
    REGISTER_GLOBAL_MOCK_HOOK(xio_open, my_xio_open);
    REGISTER_GLOBAL_MOCK_HOOK(xio_send, my_xio_send);
    REGISTER_GLOBAL_MOCK_HOOK(xio_close, my_xio_close);
    REGISTER_GLOBAL_MOCK_RETURN(xio_setoption, 0);
    REGISTER_GLOBAL_MOCK_RETURN(saslmechanism_get_init_bytes, 0);
    REGISTER_GLOBAL_MOCK_RETURN(saslmechanism_get_mechanism_name, test_mechanism);
    REGISTER_GLOBAL_MOCK_RETURN(saslmechanism_challenge, 0);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_to_string, NULL);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_get_inplace_descriptor, test_descriptor_value);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_get_array_item_count, 0);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_get_array_item, test_sasl_server_mechanism);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_get_symbol, 0);
    REGISTER_GLOBAL_MOCK_RETURN(sasl_init_create, test_sasl_init);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_create_sasl_init, test_sasl_init_value);
    REGISTER_GLOBAL_MOCK_RETURN(sasl_response_create, test_sasl_response_handle);

    REGISTER_GLOBAL_MOCK_RETURN(OptionHandler_Create, test_optionhandler_handle);

    REGISTER_TYPE(SASL_MECHANISM_BYTES*, SASL_MECHANISM_BYTES_ptr);

    REGISTER_UMOCK_ALIAS_TYPE(const SASL_MECHANISM_BYTES*, SASL_MECHANISM_BYTES*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_FRAME_CODEC_ERROR, void*);
    REGISTER_UMOCK_ALIAS_TYPE(FRAME_CODEC_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_SASL_FRAME_RECEIVED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_SASL_FRAME_CODEC_ERROR, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_OPEN_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_RECEIVED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_ERROR, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_CLOSE_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_SEND_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SASL_FRAME_CODEC_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(XIO_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(AMQP_VALUE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SASL_MECHANISMS_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SASL_MECHANISM_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SASL_OUTCOME_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SASL_INIT_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(const AMQP_VALUE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_ENCODED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfCloneOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfDestroyOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfSetOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(OPTIONHANDLER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SASL_CHALLENGE_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SASL_RESPONSE_HANDLE, void*);

    REGISTER_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT);
    REGISTER_TYPE(OPTIONHANDLER_RESULT, OPTIONHANDLER_RESULT);
    REGISTER_TYPE(amqp_binary, amqp_binary);
    REGISTER_TYPE(bool*, bool_ptr);
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
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    if (frame_codec_received_bytes != NULL)
    {
        free(frame_codec_received_bytes);
        frame_codec_received_bytes = NULL;
    }
    frame_codec_received_byte_count = 0;

    if (io_send_bytes != NULL)
    {
        free(io_send_bytes);
        io_send_bytes = NULL;
    }
    io_send_byte_count = 0;

    TEST_MUTEX_RELEASE(g_testByTest);
}

/* saslclientio_create */

/* Tests_SRS_SASLCLIENTIO_01_004: [`saslclientio_create` shall return on success a non-NULL handle to a new SASL client IO instance.] */
/* Tests_SRS_SASLCLIENTIO_01_089: [`saslclientio_create` shall create a frame codec to be used for encoding/decoding frames by calling `frame_codec_create` and passing `on_frame_codec_error` and a context as arguments.] */
/* Tests_SRS_SASLCLIENTIO_01_084: [`saslclientio_create` shall create a SASL frame codec to be used for SASL frame encoding/decoding by calling `sasl_frame_codec_create` and passing the just created frame codec as argument.] */
TEST_FUNCTION(saslclientio_create_with_valid_args_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE result;
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(frame_codec_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_frame_codec_create(test_frame_codec, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(result);
}

/* Tests_SRS_SASLCLIENTIO_01_005: [If `io_create_parameters` is NULL, `saslclientio_create` shall fail and return NULL.] */
TEST_FUNCTION(saslclientio_create_with_NULL_config_fails)
{
    // arrange

    // act
    CONCRETE_IO_HANDLE result = saslclientio_get_interface_description()->concrete_io_create(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_SASLCLIENTIO_01_006: [If memory cannot be allocated for the new instance, `saslclientio_create` shall fail and return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_the_new_instance_fails_then_saslclientio_create_fails)
{
    // arrange
    CONCRETE_IO_HANDLE result;
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    result = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_SASLCLIENTIO_01_090: [If frame_codec_create fails, then saslclientio_create shall fail and return NULL.] */
TEST_FUNCTION(when_creating_the_frame_codec_fails_then_saslclientio_create_fails)
{
    // arrange
    CONCRETE_IO_HANDLE result;
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(frame_codec_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_SASLCLIENTIO_01_085: [If `sasl_frame_codec_create` fails, then `saslclientio_create` shall fail and return NULL.] */
TEST_FUNCTION(when_creating_the_sasl_frame_codec_fails_then_saslclientio_create_fails)
{
    // arrange
    CONCRETE_IO_HANDLE result;
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(frame_codec_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_frame_codec_create(test_frame_codec, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(frame_codec_destroy(test_frame_codec));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_SASLCLIENTIO_01_092: [If any of the `sasl_mechanism` or `underlying_io` members of the configuration structure are NULL, `saslclientio_create` shall fail and return NULL.] */
TEST_FUNCTION(saslclientio_create_with_a_NULL_underlying_io_fails)
{
    // arrange
    CONCRETE_IO_HANDLE result;
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    sasl_client_io_config.underlying_io = NULL;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;

    // act
    result = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_SASLCLIENTIO_01_092: [If any of the `sasl_mechanism` or `underlying_io` members of the configuration structure are NULL, `saslclientio_create` shall fail and return NULL.] */
TEST_FUNCTION(saslclientio_create_with_a_NULL_sasl_mechanism_fails)
{
    // arrange
    CONCRETE_IO_HANDLE result;
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = NULL;

    // act
    result = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* saslclientio_destroy */

/* Tests_SRS_SASLCLIENTIO_01_007: [`saslclientio_destroy` shall free all resources associated with the SASL client IO handle.] */
/* Tests_SRS_SASLCLIENTIO_01_086: [`saslclientio_destroy` shall destroy the SASL frame codec created in `saslclientio_create` by calling `sasl_frame_codec_destroy`.] */
/* Tests_SRS_SASLCLIENTIO_01_091: [`saslclientio_destroy` shall destroy the frame codec created in `saslclientio_create` by calling `frame_codec_destroy`.] */
TEST_FUNCTION(saslclientio_destroy_frees_the_resources_allocated_in_create)
{
    // arrange
    CONCRETE_IO_HANDLE sasl_client_io;
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(sasl_frame_codec_destroy(test_sasl_frame_codec));
    STRICT_EXPECTED_CALL(frame_codec_destroy(test_frame_codec));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASLCLIENTIO_01_008: [If the argument `sasl_client_io` is NULL, `saslclientio_destroy` shall do nothing.] */
TEST_FUNCTION(saslclientio_destroy_with_NULL_argument_does_nothing)
{
    // arrange

    // act
    saslclientio_get_interface_description()->concrete_io_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* saslclientio_open_async */

/* Tests_SRS_SASLCLIENTIO_01_009: [`saslclientio_open_async` shall call `xio_open` on the `underlying_io` passed to `saslclientio_create`.] */
/* Tests_SRS_SASLCLIENTIO_01_010: [On success, `saslclientio_open_async` shall return 0.]*/
/* Tests_SRS_SASLCLIENTIO_01_013: [`saslclientio_open_async` shall pass to `xio_open` the `on_underlying_io_open_complete` as `on_io_open_complete` argument, `on_underlying_io_bytes_received` as `on_bytes_received` argument and `on_underlying_io_error` as `on_io_error` argument.] */
TEST_FUNCTION(saslclientio_open_async_with_valid_args_succeeds)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_011: [If any of the `sasl_client_io`, `on_io_open_complete`, `on_bytes_received` or `on_io_error` arguments is NULL, `saslclientio_open_async` shall fail and return a non-zero value.] */
TEST_FUNCTION(saslclientio_open_async_with_NULL_sasl_io_handle_fails)
{
    // arrange

    // act
    int result = saslclientio_get_interface_description()->concrete_io_open(NULL, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_SASLCLIENTIO_01_011: [If any of the `sasl_client_io`, `on_io_open_complete`, `on_bytes_received` or `on_io_error` arguments is NULL, `saslclientio_open_async` shall fail and return a non-zero value.] */
TEST_FUNCTION(saslclientio_open_async_with_NULL_on_io_open_complete_fails)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    umock_c_reset_all_calls();

    // act
    result = saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, NULL, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_011: [If any of the `sasl_client_io`, `on_io_open_complete`, `on_bytes_received` or `on_io_error` arguments is NULL, `saslclientio_open_async` shall fail and return a non-zero value.] */
TEST_FUNCTION(saslclientio_open_async_with_NULL_on_bytes_received_fails)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    umock_c_reset_all_calls();

    // act
    result = saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, NULL, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_011: [If any of the `sasl_client_io`, `on_io_open_complete`, `on_bytes_received` or `on_io_error` arguments is NULL, `saslclientio_open_async` shall fail and return a non-zero value.] */
TEST_FUNCTION(saslclientio_open_async_with_NULL_on_io_error_fails)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    umock_c_reset_all_calls();

    // act
    result = saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, NULL, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_012: [If the open of the `underlying_io` fails, `saslclientio_open_async` shall fail and return non-zero value.] */
TEST_FUNCTION(when_opening_the_underlying_io_fails_saslclientio_open_async_fails)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1);

    // act
    result = saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* saslclientio_close_async */

/* Tests_SRS_SASLCLIENTIO_01_015: [`saslclientio_close_async` shall close the underlying io handle passed in `saslclientio_create` by calling `xio_close`.] */
/* Tests_SRS_SASLCLIENTIO_01_098: [`saslclientio_close_async` shall only perform the close if the state is OPEN, OPENING or ERROR.] */
/* Tests_SRS_SASLCLIENTIO_01_016: [On success, `saslclientio_close_async` shall return 0.] */
TEST_FUNCTION(saslclientio_close_async_when_the_io_state_is_OPENING_closes_the_underlying_io)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = saslclientio_get_interface_description()->concrete_io_close(sasl_client_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_015: [`saslclientio_close_async` shall close the underlying io handle passed in `saslclientio_create` by calling `xio_close`.] */
/* Tests_SRS_SASLCLIENTIO_01_098: [`saslclientio_close_async` shall only perform the close if the state is OPEN, OPENING or ERROR.] */
/* Tests_SRS_SASLCLIENTIO_01_016: [On success, `saslclientio_close_async` shall return 0.] */
TEST_FUNCTION(saslclientio_close_async_when_the_io_state_is_OPEN_closes_the_underlying_io)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = saslclientio_get_interface_description()->concrete_io_close(sasl_client_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_015: [`saslclientio_close_async` shall close the underlying io handle passed in `saslclientio_create` by calling `xio_close`.] */
/* Tests_SRS_SASLCLIENTIO_01_098: [`saslclientio_close_async` shall only perform the close if the state is OPEN, OPENING or ERROR.] */
/* Tests_SRS_SASLCLIENTIO_01_016: [On success, `saslclientio_close_async` shall return 0.] */
TEST_FUNCTION(saslclientio_close_async_when_the_io_state_is_ERROR_closes_the_underlying_io)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    saved_on_io_error(saved_on_io_error_context);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = saslclientio_get_interface_description()->concrete_io_close(sasl_client_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_097: [If `saslclientio_close_async` is called when the IO is in the `IO_STATE_NOT_OPEN` state, `saslclientio_close_async` shall fail and return a non zero value.] */
TEST_FUNCTION(saslclientio_close_async_when_the_io_state_is_NOT_OPEN_fails)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    umock_c_reset_all_calls();

    // act
    result = saslclientio_get_interface_description()->concrete_io_close(sasl_client_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_097: [If `saslclientio_close_async` is called when the IO is in the `IO_STATE_NOT_OPEN` state, `saslclientio_close_async` shall fail and return a non zero value.] */
TEST_FUNCTION(saslclientio_close_async_when_the_io_state_is_NOT_OPEN_due_to_a_previous_close_succeeds_without_calling_the_underlying_io)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    (void)saslclientio_get_interface_description()->concrete_io_close(sasl_client_io, test_on_io_close_complete, (void*)0x4245);
    saved_on_io_close_complete(saved_on_io_close_complete_context);
    umock_c_reset_all_calls();

    // act
    result = saslclientio_get_interface_description()->concrete_io_close(sasl_client_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_017: [If `sasl_client_io` is NULL, `saslclientio_close_async` shall fail and return a non-zero value.] */
TEST_FUNCTION(saslclientio_close_async_with_NULL_sasl_io_fails)
{
    // arrange

    // act
    int result = saslclientio_get_interface_description()->concrete_io_close(NULL, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_SASLCLIENTIO_01_018: [If `xio_close` fails, then `saslclientio_close_async` shall return a non-zero value.] */
TEST_FUNCTION(when_xio_close_fails_saslclientio_close_async_fails)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1);

    // act
    result = saslclientio_get_interface_description()->concrete_io_close(sasl_client_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* saslclientio_send_async */

/* Tests_SRS_SASLCLIENTIO_01_019: [If `saslclientio_send_async` is called while the SASL client IO state is not `IO_STATE_OPEN`, `saslclientio_send_async` shall fail and return a non-zero value.]*/
TEST_FUNCTION(saslclientio_send_async_when_io_state_is_NOT_OPEN_fails)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    unsigned char test_buffer[] = { 0x42 };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    umock_c_reset_all_calls();

    // act
    result = saslclientio_get_interface_description()->concrete_io_send(sasl_client_io, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_019: [If `saslclientio_send_async` is called while the SASL client IO state is not `IO_STATE_OPEN`, `saslclientio_send_async` shall fail and return a non-zero value.]*/
TEST_FUNCTION(saslclientio_send_async_when_io_state_is_OPENING_fails)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    unsigned char test_buffer[] = { 0x42 };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    result = saslclientio_get_interface_description()->concrete_io_send(sasl_client_io, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_019: [If `saslclientio_send_async` is called while the SASL client IO state is not `IO_STATE_OPEN`, `saslclientio_send_async` shall fail and return a non-zero value.]*/
TEST_FUNCTION(saslclientio_send_async_when_io_state_is_ERROR_fails)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    unsigned char test_buffer[] = { 0x42 };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    saved_on_io_error(saved_on_io_error_context);
    umock_c_reset_all_calls();

    // act
    result = saslclientio_get_interface_description()->concrete_io_send(sasl_client_io, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_020: [If the SASL client IO state is `IO_STATE_OPEN`, `saslclientio_send_async` shall call `xio_send` on the `underlying_io` passed to `saslclientio_create`, while passing as arguments the `buffer`,`size`, `on_send_complete` and `callback_context`.]*/
/* Tests_SRS_SASLCLIENTIO_01_021: [On success, `saslclientio_send_async` shall return 0.]*/
TEST_FUNCTION(saslclientio_send_async_when_io_state_is_OPEN_calls_the_underlying_io_send)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    unsigned char test_buffer[] = { 0x42 };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_io, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4245))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer));

    // act
    result = saslclientio_get_interface_description()->concrete_io_send(sasl_client_io, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_020: [If the SASL client IO state is `IO_STATE_OPEN`, `saslclientio_send_async` shall call `xio_send` on the `underlying_io` passed to `saslclientio_create`, while passing as arguments the `buffer`,`size`, `on_send_complete` and `callback_context`.]*/
/* Tests_SRS_SASLCLIENTIO_01_021: [On success, `saslclientio_send_async` shall return 0.]*/
/* Tests_SRS_SASLCLIENTIO_01_127: [ `on_send_complete` shall be allowed to be NULL. ]*/
TEST_FUNCTION(saslclientio_send_async_with_NULL_on_send_complete_passes_NULL_to_the_underlying_io)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    unsigned char test_buffer[] = { 0x42 };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_io, test_buffer, sizeof(test_buffer), NULL, (void*)0x4245))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer));

    // act
    result = saslclientio_get_interface_description()->concrete_io_send(sasl_client_io, test_buffer, sizeof(test_buffer), NULL, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_020: [If the SASL client IO state is `IO_STATE_OPEN`, `saslclientio_send_async` shall call `xio_send` on the `underlying_io` passed to `saslclientio_create`, while passing as arguments the `buffer`,`size`, `on_send_complete` and `callback_context`.]*/
/* Tests_SRS_SASLCLIENTIO_01_021: [On success, `saslclientio_send_async` shall return 0.]*/
TEST_FUNCTION(saslclientio_send_async_with_NULL_on_send_complete_context_passes_NULL_to_the_underlying_io)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    unsigned char test_buffer[] = { 0x42 };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_io, test_buffer, sizeof(test_buffer), test_on_send_complete, NULL))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer));

    // act
    result = saslclientio_get_interface_description()->concrete_io_send(sasl_client_io, test_buffer, sizeof(test_buffer), test_on_send_complete, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_022: [If the `sasl_client_io` or `buffer` argument is NULL, `saslclientio_send_async` shall fail and return a non-zero value.]*/
TEST_FUNCTION(saslclientio_send_async_with_NULL_sasl_io_fails)
{
    // arrange
    unsigned char test_buffer[] = { 0x42 };

    // act
    int result = saslclientio_get_interface_description()->concrete_io_send(NULL, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_SASLCLIENTIO_01_022: [If the `sasl_client_io` or `buffer` argument is NULL, `saslclientio_send_async` shall fail and return a non-zero value.]*/
TEST_FUNCTION(saslclientio_send_async_with_NULL_buffer_fails)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    umock_c_reset_all_calls();

    // act
    result = saslclientio_get_interface_description()->concrete_io_send(sasl_client_io, NULL, 1, test_on_send_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_023: [If `size` is 0, `saslclientio_send_async` shall fail and return a non-zero value.]*/
TEST_FUNCTION(saslclientio_send_async_with_0_size_fails)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    unsigned char test_buffer[] = { 0x42 };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    umock_c_reset_all_calls();

    // act
    result = saslclientio_get_interface_description()->concrete_io_send(sasl_client_io, test_buffer, 0, test_on_send_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_024: [If the call to `xio_send` fails, then `saslclientio_send_async` shall fail and return a non-zero value.]*/
TEST_FUNCTION(when_the_underlying_xio_send_fails_then_saslclientio_send_async_fails)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    unsigned char test_buffer[] = { 0x42 };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_io, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4245))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);

    // act
    result = saslclientio_get_interface_description()->concrete_io_send(sasl_client_io, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* saslclientio_dowork */

/* Tests_SRS_SASLCLIENTIO_01_025: [`saslclientio_dowork` shall call the `xio_dowork` on the `underlying_io` passed in `saslclientio_create`.]*/
TEST_FUNCTION(when_the_io_state_is_OPEN_xio_dowork_calls_the_underlying_IO)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_dowork(test_underlying_io));

    // act
    saslclientio_get_interface_description()->concrete_io_dowork(sasl_client_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_025: [`saslclientio_dowork` shall call the `xio_dowork` on the `underlying_io` passed in `saslclientio_create`.]*/
TEST_FUNCTION(when_the_io_state_is_OPENING_xio_dowork_calls_the_underlying_IO)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_dowork(test_underlying_io));

    // act
    saslclientio_get_interface_description()->concrete_io_dowork(sasl_client_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_025: [`saslclientio_dowork` shall call the `xio_dowork` on the `underlying_io` passed in `saslclientio_create`.]*/
TEST_FUNCTION(when_the_io_state_is_NOT_OPEN_xio_dowork_does_nothing)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    umock_c_reset_all_calls();

    // act
    saslclientio_get_interface_description()->concrete_io_dowork(sasl_client_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_099: [If the state of the IO is `IO_NOT_OPEN`, `saslclientio_dowork` shall make no calls to the underlying IO.]*/
TEST_FUNCTION(when_the_io_state_is_ERROR_xio_dowork_calls_the_underlying_dowork)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    saved_on_io_error(saved_on_io_error_context);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_dowork(test_underlying_io));

    // act
    saslclientio_get_interface_description()->concrete_io_dowork(sasl_client_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_026: [If the `sasl_client_io` argument is NULL, `saslclientio_dowork` shall do nothing.]*/
TEST_FUNCTION(saslclientio_dowork_with_NULL_sasl_io_handle_does_nothing)
{
    // arrange

    // act
    saslclientio_get_interface_description()->concrete_io_dowork(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* saslclientio_get_interface_description */

/* Tests_SRS_SASLCLIENTIO_01_087: [`saslclientio_get_interface_description` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains pointers to the functions: `saslclientio_create`, `saslclientio_destroy`, `saslclientio_open_async`, `saslclientio_close_async`, `saslclientio_send_async`, `saslclientio_setoption`, `saslclientio_retrieveoptions` and `saslclientio_dowork`.]*/
TEST_FUNCTION(saslclientio_get_interface_description_returns_the_saslclientio_interface_functions)
{
    // arrange

    // act
    const IO_INTERFACE_DESCRIPTION* result = saslclientio_get_interface_description();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);
    ASSERT_IS_NOT_NULL(result->concrete_io_create);
    ASSERT_IS_NOT_NULL(result->concrete_io_destroy);
    ASSERT_IS_NOT_NULL(result->concrete_io_open);
    ASSERT_IS_NOT_NULL(result->concrete_io_close);
    ASSERT_IS_NOT_NULL(result->concrete_io_send);
    ASSERT_IS_NOT_NULL(result->concrete_io_dowork);
    ASSERT_IS_NOT_NULL(result->concrete_io_setoption);
    ASSERT_IS_NOT_NULL(result->concrete_io_retrieveoptions);
}

/* saslclientio_setoption */

/* Tests_SRS_SASLCLIENTIO_03_001: [`saslclientio_setoption` shall forward all unhandled options to underlying io by calling `xio_setoption`.]*/
/* Tests_SRS_SASLCLIENTIO_01_128: [ On success, `saslclientio_setoption` shall return 0. ]*/
TEST_FUNCTION(saslclientio_setoption_calls_the_underlying_io)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    saved_on_io_error(saved_on_io_error_context);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_setoption(test_underlying_io, "option1", (void*)0x4244));

    // act
    result = saslclientio_get_interface_description()->concrete_io_setoption(sasl_client_io, "option1", (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

TEST_FUNCTION(when_xio_setopion_fails_saslclientio_setoption_also_fails)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    saved_on_io_error(saved_on_io_error_context);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_setoption(test_underlying_io, "option1", (void*)0x4244))
        .SetReturn(1);

    // act
    result = saslclientio_get_interface_description()->concrete_io_setoption(sasl_client_io, "option1", (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_130: [ If `sasl_client_io` or `option_name` is NULL, `saslclientio_setoption`  shall fail and return a non-zero value. ]*/
TEST_FUNCTION(saslclientio_setoption_with_NULL_sasl_client_io_fails)
{
    // arrange
    int result;

    // act
    result = saslclientio_get_interface_description()->concrete_io_setoption(NULL, "option1", (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_SASLCLIENTIO_01_130: [ If `sasl_client_io` or `option_name` is NULL, `saslclientio_setoption`  shall fail and return a non-zero value. ]*/
TEST_FUNCTION(saslclientio_setoption_with_NULL_option_name_fails)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    saved_on_io_error(saved_on_io_error_context);
    umock_c_reset_all_calls();

    // act
    result = saslclientio_get_interface_description()->concrete_io_setoption(sasl_client_io, NULL, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_131: [ SASL client IO shall handle the following options: ]*/
/* Tests_SRS_SASLCLIENTIO_01_132: [ - logtrace - bool. ]*/
TEST_FUNCTION(saslclientio_setoption_with_logtrace_true_succeeds)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    bool log_trace = true;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    saved_on_io_error(saved_on_io_error_context);
    umock_c_reset_all_calls();

    // act
    result = saslclientio_get_interface_description()->concrete_io_setoption(sasl_client_io, "logtrace", &log_trace);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_131: [ SASL client IO shall handle the following options: ]*/
/* Tests_SRS_SASLCLIENTIO_01_132: [ - logtrace - bool. ]*/
TEST_FUNCTION(saslclientio_setoption_with_logtrace_false_succeeds)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    int result;
    bool log_trace = false;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    saved_on_io_error(saved_on_io_error_context);
    umock_c_reset_all_calls();

    // act
    result = saslclientio_get_interface_description()->concrete_io_setoption(sasl_client_io, "logtrace", &log_trace);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* saslclientio_retrieveoptions */

/* Tests_SRS_SASLCLIENTIO_01_133: [ `saslclientio_retrieveoptions` shall create an option handler by calling `OptionHandler_Create`. ]*/
TEST_FUNCTION(saslclientio_retrieveoptions_creates_an_option_handler)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    OPTIONHANDLER_HANDLE result;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    saved_on_io_error(saved_on_io_error_context);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = saslclientio_get_interface_description()->concrete_io_retrieveoptions(sasl_client_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_138: [ If `OptionHandler_AddOption` or `OptionHandler_Create` fails then `saslclientio_retrieveoptions` shall fail and return NULL. ]*/
TEST_FUNCTION(when_OptionHandler_Create_fails_then_saslclientio_retrieveoptions_fails)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    OPTIONHANDLER_HANDLE result;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    saved_on_io_error(saved_on_io_error_context);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(NULL);

    // act
    result = saslclientio_get_interface_description()->concrete_io_retrieveoptions(sasl_client_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_139: [ When `saslclientio_retrieveoptions` is called with NULL `sasl_client_io` it shall fail and return NULL. ]*/
TEST_FUNCTION(saslclientio_retrieveoptions_with_NULL_sasl_clientio_fails)
{
    // arrange
    OPTIONHANDLER_HANDLE result;

    // act
    result = saslclientio_get_interface_description()->concrete_io_retrieveoptions(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_SASLCLIENTIO_01_134: [ `saslclientio_retrieveoptions` shall add the handled options to it: ]*/
/* Tests_SRS_SASLCLIENTIO_01_135: [ - logtrace - bool. ]*/
/* Tests_SRS_SASLCLIENTIO_01_137: [ The options shall be added by calling `OptionHandler_AddOption`. ]*/
TEST_FUNCTION(when_logtrace_was_set_to_true_saslclientio_retrieveoptions_adds_it_to_the_OptionHandler)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    OPTIONHANDLER_HANDLE result;
    bool log_trace = true;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    saved_on_io_error(saved_on_io_error_context);
    (void)saslclientio_get_interface_description()->concrete_io_setoption(sasl_client_io, "logtrace", &log_trace);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(OptionHandler_AddOption(test_optionhandler_handle, "logtrace", &log_trace))
        .ValidateArgumentValue_value_AsType(UMOCK_TYPE(bool*));

    // act
    result = saslclientio_get_interface_description()->concrete_io_retrieveoptions(sasl_client_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_134: [ `saslclientio_retrieveoptions` shall add the handled options to it: ]*/
/* Tests_SRS_SASLCLIENTIO_01_135: [ - logtrace - bool. ]*/
/* Tests_SRS_SASLCLIENTIO_01_137: [ The options shall be added by calling `OptionHandler_AddOption`. ]*/
TEST_FUNCTION(when_logtrace_was_set_to_false_saslclientio_retrieveoptions_adds_it_to_the_OptionHandler)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    OPTIONHANDLER_HANDLE result;
    bool log_trace = false;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    saved_on_io_error(saved_on_io_error_context);
    (void)saslclientio_get_interface_description()->concrete_io_setoption(sasl_client_io, "logtrace", &log_trace);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(OptionHandler_AddOption(test_optionhandler_handle, "logtrace", &log_trace))
        .ValidateArgumentValue_value_AsType(UMOCK_TYPE(bool*));

    // act
    result = saslclientio_get_interface_description()->concrete_io_retrieveoptions(sasl_client_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_138: [ If `OptionHandler_AddOption` or `OptionHandler_Create` fails then `saslclientio_retrieveoptions` shall fail and return NULL. ]*/
TEST_FUNCTION(when_OptionHandler_AddOption_fails_saslclientio_retrieveoptions_also_fails)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    OPTIONHANDLER_HANDLE result;
    bool log_trace = false;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    saved_on_io_error(saved_on_io_error_context);
    (void)saslclientio_get_interface_description()->concrete_io_setoption(sasl_client_io, "logtrace", &log_trace);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(OptionHandler_AddOption(test_optionhandler_handle, "logtrace", &log_trace))
        .ValidateArgumentValue_value_AsType(UMOCK_TYPE(bool*))
        .SetReturn(OPTIONHANDLER_ERROR);
    STRICT_EXPECTED_CALL(OptionHandler_Destroy(test_optionhandler_handle));

    // act
    result = saslclientio_get_interface_description()->concrete_io_retrieveoptions(sasl_client_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* on_underlying_io_bytes_received */

/* Tests_SRS_SASLCLIENTIO_01_027: [When the `on_underlying_io_bytes_received` callback passed to the underlying IO is called and the SASL client IO state is `IO_STATE_OPEN`, the bytes shall be indicated to the user of SASL client IO by calling the `on_bytes_received` callback that was passed in `saslclientio_open`.]*/
TEST_FUNCTION(when_io_state_is_open_and_bytes_are_received_they_are_indicated_up)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    unsigned char test_bytes[] = { 0x42, 0x43 };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_bytes_received((void*)0x4243, test_bytes, sizeof(test_bytes)))
        .ValidateArgumentBuffer(2, test_bytes, sizeof(test_bytes));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, test_bytes, sizeof(test_bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_027: [When the `on_underlying_io_bytes_received` callback passed to the underlying IO is called and the SASL client IO state is `IO_STATE_OPEN`, the bytes shall be indicated to the user of SASL client IO by calling the `on_bytes_received` callback that was passed in `saslclientio_open`.]*/
/* Tests_SRS_SASLCLIENTIO_01_029: [The `context` argument for `on_io_error` shall be set to the `on_io_error_context` passed in `saslclientio_open`.]*/
TEST_FUNCTION(when_io_state_is_open_and_bytes_are_received_and_context_passed_to_open_was_NULL_NULL_is_passed_as_context_to_the_on_bytes_received_call)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    unsigned char test_bytes[] = { 0x42, 0x43 };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, NULL, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_bytes_received(NULL, test_bytes, sizeof(test_bytes)))
        .ValidateArgumentBuffer(2, test_bytes, sizeof(test_bytes));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, test_bytes, sizeof(test_bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_028: [If `buffer` is NULL or `size` is zero, nothing should be indicated as received, the state shall be switched to ERROR and the `on_io_error` callback shall be triggered.]*/
TEST_FUNCTION(when_io_state_is_open_and_bytes_are_received_with_bytes_NULL_nothing_is_indicated_up)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    unsigned char test_bytes[] = { 0x42, 0x43 };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, NULL, sizeof(test_bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_028: [If `buffer` is NULL or `size` is zero, nothing should be indicated as received, the state shall be switched to ERROR and the `on_io_error` callback shall be triggered.]*/
TEST_FUNCTION(when_io_state_is_open_and_bytes_are_received_with_size_zero_nothing_is_indicated_up)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    unsigned char test_bytes[] = { 0x42, 0x43 };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, test_bytes, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_031: [If bytes are received when the SASL client IO state is `IO_STATE_ERROR`, SASL client IO shall do nothing.]*/
TEST_FUNCTION(when_io_state_is_ERROR_and_bytes_are_received_nothing_is_done)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    unsigned char test_bytes[] = { 0x42, 0x43 };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    saved_on_io_error(saved_on_io_error_context);
    umock_c_reset_all_calls();

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, test_bytes, 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_030: [If bytes are received when the SASL client IO state is `IO_STATE_OPENING`, the bytes shall be consumed by the SASL client IO to satisfy the SASL handshake.]*/
/* Tests_SRS_SASLCLIENTIO_01_003: [Other than using a protocol id of three, the exchange of SASL layer headers follows the same rules specified in the version negotiation section of the transport specification (See Part 2: section 2.2).] */
TEST_FUNCTION(when_io_state_is_opening_and_1_byte_is_received_it_is_used_for_the_header)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_030: [If bytes are received when the SASL client IO state is `IO_STATE_OPENING`, the bytes shall be consumed by the SASL client IO to satisfy the SASL handshake.]*/
/* Tests_SRS_SASLCLIENTIO_01_003: [Other than using a protocol id of three, the exchange of SASL layer headers follows the same rules specified in the version negotiation section of the transport specification (See Part 2: section 2.2).] */
/* Tests_SRS_SASLCLIENTIO_01_073: [If the handshake fails (i.e. the outcome is an error) the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_io_state_is_opening_and_1_bad_byte_is_received_state_is_set_to_ERROR)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    unsigned char test_bytes[] = { 0x42 };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, test_bytes, sizeof(test_bytes));
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_030: [If bytes are received when the SASL client IO state is `IO_STATE_OPENING`, the bytes shall be consumed by the SASL client IO to satisfy the SASL handshake.]*/
/* Tests_SRS_SASLCLIENTIO_01_003: [Other than using a protocol id of three, the exchange of SASL layer headers follows the same rules specified in the version negotiation section of the transport specification (See Part 2: section 2.2).] */
/* Tests_SRS_SASLCLIENTIO_01_073: [If the handshake fails (i.e. the outcome is an error) the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_io_state_is_opening_and_the_last_header_byte_is_bad_state_is_set_to_ERROR)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    unsigned char test_bytes[] = { 'A', 'M', 'Q', 'P', 3, 1, 0, 'x' };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, test_bytes, sizeof(test_bytes));
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_030: [If bytes are received when the SASL client IO state is `IO_STATE_OPENING`, the bytes shall be consumed by the SASL client IO to satisfy the SASL handshake.]*/
/* Tests_SRS_SASLCLIENTIO_01_003: [Other than using a protocol id of three, the exchange of SASL layer headers follows the same rules specified in the version negotiation section of the transport specification (See Part 2: section 2.2).] */
/* Tests_SRS_SASLCLIENTIO_01_001: [To establish a SASL layer, each peer MUST start by sending a protocol header.] */
/* Tests_SRS_SASLCLIENTIO_01_105: [start header exchange] */
/* Tests_SRS_SASLCLIENTIO_01_078: [SASL client IO shall start the header exchange by sending the SASL header.] */
/* Tests_SRS_SASLCLIENTIO_01_095: [Sending the header shall be done by using `xio_send`.]*/
TEST_FUNCTION(when_underlying_IO_switches_the_state_to_OPEN_the_SASL_header_is_sent)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_io, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_send(test_underlying_io, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllCalls();

    // act
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    stringify_bytes(io_send_bytes, io_send_byte_count, actual_stringified_io);
    stringify_bytes(sasl_header, sizeof(sasl_header), expected_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, expected_stringified_io, actual_stringified_io);

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_073: [If the handshake fails (i.e. the outcome is an error) the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_sending_the_header_fails_state_is_set_to_ERROR)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_io, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_030: [If bytes are received when the SASL client IO state is `IO_STATE_OPENING`, the bytes shall be consumed by the SASL client IO to satisfy the SASL handshake.]*/
/* Tests_SRS_SASLCLIENTIO_01_003: [Other than using a protocol id of three, the exchange of SASL layer headers follows the same rules specified in the version negotiation section of the transport specification (See Part 2: section 2.2).] */
/* Tests_SRS_SASLCLIENTIO_01_073: [If the handshake fails (i.e. the outcome is an error) the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_a_bad_header_is_received_after_a_good_one_has_been_sent_state_is_set_to_ERROR)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    unsigned char test_bytes[] = { 'A', 'M', 'Q', 'P', 3, 1, 0, 'x' };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, test_bytes, sizeof(test_bytes));
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    stringify_bytes(io_send_bytes, io_send_byte_count, actual_stringified_io);
    stringify_bytes(sasl_header, sizeof(sasl_header), expected_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, expected_stringified_io, actual_stringified_io);

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_030: [If bytes are received when the SASL client IO state is `IO_STATE_OPENING`, the bytes shall be consumed by the SASL client IO to satisfy the SASL handshake.]*/
/* Tests_SRS_SASLCLIENTIO_01_003: [Other than using a protocol id of three, the exchange of SASL layer headers follows the same rules specified in the version negotiation section of the transport specification (See Part 2: section 2.2).] */
TEST_FUNCTION(when_a_good_header_is_received_after_the_header_has_been_sent_yields_no_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_030: [If bytes are received when the SASL client IO state is `IO_STATE_OPENING`, the bytes shall be consumed by the SASL client IO to satisfy the SASL handshake.]*/
/* Tests_SRS_SASLCLIENTIO_01_067: [The SASL frame exchange shall be started as soon as the SASL header handshake is done.] */
/* Tests_SRS_SASLCLIENTIO_01_068: [During the SASL frame exchange that constitutes the handshake the received bytes from the underlying IO shall be fed to the frame codec instance created in `saslclientio_create` by calling `frame_codec_receive_bytes`.]*/
TEST_FUNCTION(when_one_byte_is_received_after_header_handshake_it_is_sent_to_the_frame_codec)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    unsigned char test_bytes[] = { 0x42 };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(frame_codec_receive_bytes(test_frame_codec, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(frame_codec_receive_bytes(test_frame_codec, IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .IgnoreAllCalls();

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, test_bytes, sizeof(test_bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    stringify_bytes(frame_codec_received_bytes, frame_codec_received_byte_count, actual_stringified_io);
    stringify_bytes(test_bytes, sizeof(test_bytes), expected_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, expected_stringified_io, actual_stringified_io);

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_088: [If `frame_codec_receive_bytes` fails, the `on_io_error` callback shall be triggered.]*/
TEST_FUNCTION(when_frame_codec_receive_bytes_fails_then_the_state_is_switched_to_ERROR)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    unsigned char test_bytes[] = { 0x42 };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(frame_codec_receive_bytes(test_frame_codec, IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, test_bytes, sizeof(test_bytes));
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_101: [`on_open_complete` with `IO_OPEN_ERROR`]*/
TEST_FUNCTION(ERROR_received_in_the_state_OPENING_indicates_on_io_open_complete)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_io_error(saved_on_io_error_context);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_102: [raise ERROR] */
TEST_FUNCTION(ERROR_received_in_the_state_OPEN_sets_the_state_to_ERROR_and_triggers_callback)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    saved_on_io_error(saved_on_io_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_103: [do nothing] */
TEST_FUNCTION(ERROR_received_in_the_state_ERROR_does_nothing)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    saved_on_io_error(saved_on_io_error_context);
    umock_c_reset_all_calls();

    // act
    saved_on_io_error(saved_on_io_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_110: [raise ERROR] */
/* Tests_SRS_SASLCLIENTIO_01_116: [If the underlying IO indicates another open while the after the header exchange has been started an error shall be indicated by calling `on_io_error`.]*/
TEST_FUNCTION(underlying_io_open_complete_again_in_OPENING_raises_ERROR)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_111: [do nothing] */
TEST_FUNCTION(OPENING_received_in_the_state_ERROR_does_nothing)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    saved_on_io_error(saved_on_io_error_context);
    umock_c_reset_all_calls();

    // act
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_106: [raise error] */
TEST_FUNCTION(underlying_io_open_complete_again_in_OPEN_raises_ERROR)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_107: [do nothing] */
TEST_FUNCTION(underlying_io_open_complete_in_the_state_ERROR_does_nothing)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    saved_on_io_error(saved_on_io_error_context);
    umock_c_reset_all_calls();

    // act
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_077: [If sending the SASL header fails, the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_sending_the_header_with_xio_send_fails_then_the_io_state_is_set_to_ERROR)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_io, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_032: [The peer acting as the SASL server MUST announce supported authentication mechanisms using the sasl-mechanisms frame.] */
/* Tests_SRS_SASLCLIENTIO_01_040: [The peer playing the role of the SASL client and the peer playing the role of the SASL server MUST correspond to the TCP client and server respectively.] */
/* Tests_SRS_SASLCLIENTIO_01_070: [When a frame needs to be sent as part of the SASL handshake frame exchange, the send shall be done by calling `sasl_frame_codec_encode_frame`.]*/
/* Tests_SRS_SASLCLIENTIO_01_034: [<-- SASL-MECHANISMS] */
/* Tests_SRS_SASLCLIENTIO_01_035: [SASL-INIT -->] */
/* Tests_SRS_SASLCLIENTIO_01_033: [The partner MUST then choose one of the supported mechanisms and initiate a sasl exchange.] */
/* Tests_SRS_SASLCLIENTIO_01_054: [Selects the sasl mechanism and provides the initial response if needed.] */
/* Tests_SRS_SASLCLIENTIO_01_045: [The name of the SASL mechanism used for the SASL exchange.] */
TEST_FUNCTION(when_a_SASL_mechanism_is_received_after_the_header_exchange_a_sasl_init_frame_is_send_with_the_selected_mechanism)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    uint32_t mechanisms_count = 1;
    SASL_MECHANISM_BYTES init_bytes;

    init_bytes.bytes = NULL;
    init_bytes.length = 0;

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_mechanisms(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_mechanisms_handle, sizeof(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(sasl_mechanisms_get_sasl_server_mechanisms(test_sasl_mechanisms_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_sasl_server_mechanisms_value(&test_sasl_server_mechanisms_value, sizeof(test_sasl_server_mechanisms_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item_count(test_sasl_server_mechanisms_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_count(&mechanisms_count, sizeof(mechanisms_count));
    STRICT_EXPECTED_CALL(saslmechanism_get_mechanism_name(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item(test_sasl_server_mechanisms_value, 0))
        .SetReturn(test_sasl_server_mechanism);
    STRICT_EXPECTED_CALL(amqpvalue_get_symbol(test_sasl_server_mechanism, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_mechanism, sizeof(test_mechanism));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_init_create(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(saslmechanism_get_init_bytes(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &init_bytes, sizeof(init_bytes));
    STRICT_EXPECTED_CALL(amqpvalue_create_sasl_init(test_sasl_init));
    STRICT_EXPECTED_CALL(sasl_frame_codec_encode_frame(test_sasl_frame_codec, test_sasl_init_value, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_sasl_init_value));
    STRICT_EXPECTED_CALL(sasl_init_destroy(test_sasl_init));
    STRICT_EXPECTED_CALL(sasl_mechanisms_destroy(test_sasl_mechanisms_handle));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_047: [A block of opaque data passed to the security mechanism.] */
/* Tests_SRS_SASLCLIENTIO_01_048: [The contents of this data are defined by the SASL security mechanism.] */
TEST_FUNCTION(when_a_SASL_mechanism_is_received_a_sasl_init_frame_is_send_with_the_mechanism_name_and_the_init_bytes)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    unsigned char test_init_bytes[] = { 0x42, 0x43 };
    uint32_t mechanisms_count = 1;
    SASL_MECHANISM_BYTES init_bytes;
    amqp_binary expected_creds;

    init_bytes.bytes = test_init_bytes;
    init_bytes.length = sizeof(test_init_bytes);

    expected_creds.bytes = test_init_bytes;
    expected_creds.length = sizeof(test_init_bytes);

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_mechanisms(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_mechanisms_handle, sizeof(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(sasl_mechanisms_get_sasl_server_mechanisms(test_sasl_mechanisms_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_sasl_server_mechanisms_value(&test_sasl_server_mechanisms_value, sizeof(test_sasl_server_mechanisms_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item_count(test_sasl_server_mechanisms_value, &mechanisms_count))
        .CopyOutArgumentBuffer(2, &mechanisms_count, sizeof(mechanisms_count));
    STRICT_EXPECTED_CALL(saslmechanism_get_mechanism_name(test_sasl_mechanism));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item(test_sasl_server_mechanisms_value, 0));
    STRICT_EXPECTED_CALL(amqpvalue_get_symbol(test_sasl_server_mechanism, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_mechanism, sizeof(test_mechanism));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_sasl_server_mechanism));
    STRICT_EXPECTED_CALL(sasl_init_create(test_mechanism));
    STRICT_EXPECTED_CALL(saslmechanism_get_init_bytes(test_sasl_mechanism, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &init_bytes, sizeof(init_bytes));
    STRICT_EXPECTED_CALL(sasl_init_set_initial_response(test_sasl_init, expected_creds));
    STRICT_EXPECTED_CALL(amqpvalue_create_sasl_init(test_sasl_init));
    STRICT_EXPECTED_CALL(sasl_frame_codec_encode_frame(test_sasl_frame_codec, test_sasl_init_value, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_sasl_init_value));
    STRICT_EXPECTED_CALL(sasl_init_destroy(test_sasl_init));
    STRICT_EXPECTED_CALL(sasl_mechanisms_destroy(test_sasl_mechanisms_handle));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_118: [If `on_sasl_frame_received_callback` is called in the OPENING state but the header exchange has not yet been completed, then the `on_io_error` callback shall be triggered.]*/
TEST_FUNCTION(when_a_SASL_mechanism_is_received_when_header_handshake_is_not_done_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_117: [If `on_sasl_frame_received_callback` is called when the state of the IO is OPEN then the `on_io_error` callback shall be triggered.]*/
TEST_FUNCTION(when_a_SASL_mechanism_is_received_in_the_OPEN_state_the_IO_state_is_set_to_ERROR)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_119: [If any error is encountered when parsing the received frame, the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_a_SASL_mechanism_is_received_and_getting_the_descriptor_fails_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_119: [If any error is encountered when parsing the received frame, the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_a_SASL_mechanism_is_received_and_getting_the_mechanism_name_fails_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    uint32_t mechanisms_count = 1;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;

    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_mechanisms(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_mechanisms_handle, sizeof(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(sasl_mechanisms_get_sasl_server_mechanisms(test_sasl_mechanisms_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_sasl_server_mechanisms_value(&test_sasl_server_mechanisms_value, sizeof(test_sasl_server_mechanisms_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item_count(test_sasl_server_mechanisms_value, &mechanisms_count))
        .CopyOutArgumentBuffer(2, &mechanisms_count, sizeof(mechanisms_count));
    STRICT_EXPECTED_CALL(saslmechanism_get_mechanism_name(test_sasl_mechanism))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_mechanisms_destroy(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_119: [If any error is encountered when parsing the received frame, the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_a_SASL_mechanism_is_received_and_creating_the_sasl_init_value_fails_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    uint32_t mechanisms_count = 1;
    SASL_MECHANISM_BYTES init_bytes;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;

    init_bytes.length = 0;
    init_bytes.bytes = NULL;

    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_mechanisms(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_mechanisms_handle, sizeof(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(sasl_mechanisms_get_sasl_server_mechanisms(test_sasl_mechanisms_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_sasl_server_mechanisms_value(&test_sasl_server_mechanisms_value, sizeof(test_sasl_server_mechanisms_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item_count(test_sasl_server_mechanisms_value, &mechanisms_count))
        .CopyOutArgumentBuffer(2, &mechanisms_count, sizeof(mechanisms_count));
    STRICT_EXPECTED_CALL(saslmechanism_get_mechanism_name(test_sasl_mechanism));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item(test_sasl_server_mechanisms_value, 0));
    STRICT_EXPECTED_CALL(amqpvalue_get_symbol(test_sasl_server_mechanism, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_mechanism, sizeof(test_mechanism));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_sasl_server_mechanism));
    STRICT_EXPECTED_CALL(sasl_init_create(test_mechanism))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_mechanisms_destroy(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_119: [If any error is encountered when parsing the received frame, the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_a_SASL_mechanism_is_received_and_getting_the_initial_bytes_fails_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    uint32_t mechanisms_count = 1;
    SASL_MECHANISM_BYTES init_bytes;

    init_bytes.length = 0;
    init_bytes.bytes = NULL;

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value)).SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_mechanisms(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_mechanisms_handle, sizeof(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(sasl_mechanisms_get_sasl_server_mechanisms(test_sasl_mechanisms_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_sasl_server_mechanisms_value(&test_sasl_server_mechanisms_value, sizeof(test_sasl_server_mechanisms_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item_count(test_sasl_server_mechanisms_value, &mechanisms_count))
        .CopyOutArgumentBuffer(2, &mechanisms_count, sizeof(mechanisms_count));
    STRICT_EXPECTED_CALL(saslmechanism_get_mechanism_name(test_sasl_mechanism));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item(test_sasl_server_mechanisms_value, 0));
    STRICT_EXPECTED_CALL(amqpvalue_get_symbol(test_sasl_server_mechanism, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_mechanism, sizeof(test_mechanism));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_sasl_server_mechanism));
    STRICT_EXPECTED_CALL(sasl_init_create(test_mechanism));
    STRICT_EXPECTED_CALL(saslmechanism_get_init_bytes(test_sasl_mechanism, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &init_bytes, sizeof(init_bytes))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(sasl_init_destroy(test_sasl_init));
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_mechanisms_destroy(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_119: [If any error is encountered when parsing the received frame, the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_a_SASL_mechanism_is_received_and_getting_the_AMQP_VALUE_fails_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    uint32_t mechanisms_count = 1;
    SASL_MECHANISM_BYTES init_bytes;

    init_bytes.length = 0;
    init_bytes.bytes = NULL;

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value)).SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_mechanisms(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_mechanisms_handle, sizeof(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(sasl_mechanisms_get_sasl_server_mechanisms(test_sasl_mechanisms_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_sasl_server_mechanisms_value(&test_sasl_server_mechanisms_value, sizeof(test_sasl_server_mechanisms_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item_count(test_sasl_server_mechanisms_value, &mechanisms_count))
        .CopyOutArgumentBuffer(2, &mechanisms_count, sizeof(mechanisms_count));
    STRICT_EXPECTED_CALL(saslmechanism_get_mechanism_name(test_sasl_mechanism));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item(test_sasl_server_mechanisms_value, 0));
    STRICT_EXPECTED_CALL(amqpvalue_get_symbol(test_sasl_server_mechanism, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_mechanism, sizeof(test_mechanism));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_sasl_server_mechanism));
    STRICT_EXPECTED_CALL(sasl_init_create(test_mechanism));
    STRICT_EXPECTED_CALL(saslmechanism_get_init_bytes(test_sasl_mechanism, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &init_bytes, sizeof(init_bytes));
    STRICT_EXPECTED_CALL(amqpvalue_create_sasl_init(test_sasl_init))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(sasl_init_destroy(test_sasl_init));
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_mechanisms_destroy(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_071: [If `sasl_frame_codec_encode_frame` fails, then the `on_io_error` callback shall be triggered.]*/
TEST_FUNCTION(when_a_SASL_mechanism_is_received_and_encoding_the_sasl_frame_fails_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    uint32_t mechanisms_count = 1;
    SASL_MECHANISM_BYTES init_bytes;

    init_bytes.length = 0;
    init_bytes.bytes = NULL;

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value)).SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_mechanisms(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_mechanisms_handle, sizeof(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(sasl_mechanisms_get_sasl_server_mechanisms(test_sasl_mechanisms_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_sasl_server_mechanisms_value(&test_sasl_server_mechanisms_value, sizeof(test_sasl_server_mechanisms_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item_count(test_sasl_server_mechanisms_value, &mechanisms_count))
        .CopyOutArgumentBuffer(2, &mechanisms_count, sizeof(mechanisms_count));
    STRICT_EXPECTED_CALL(saslmechanism_get_mechanism_name(test_sasl_mechanism));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item(test_sasl_server_mechanisms_value, 0));
    STRICT_EXPECTED_CALL(amqpvalue_get_symbol(test_sasl_server_mechanism, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_mechanism, sizeof(test_mechanism));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_sasl_server_mechanism));
    STRICT_EXPECTED_CALL(sasl_init_create(test_mechanism));
    STRICT_EXPECTED_CALL(saslmechanism_get_init_bytes(test_sasl_mechanism, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &init_bytes, sizeof(init_bytes));
    STRICT_EXPECTED_CALL(amqpvalue_create_sasl_init(test_sasl_init));
    STRICT_EXPECTED_CALL(sasl_frame_codec_encode_frame(test_sasl_frame_codec, test_sasl_init_value, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_sasl_init_value));
    STRICT_EXPECTED_CALL(sasl_init_destroy(test_sasl_init));
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_mechanisms_destroy(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_071: [If `sasl_frame_codec_encode_frame` fails, then the `on_io_error` callback shall be triggered.]*/
TEST_FUNCTION(when_a_SASL_mechanism_is_received_and_setting_the_init_bytes_fails_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    unsigned char test_init_bytes[] = { 0x42 };
    uint32_t mechanisms_count = 1;
    SASL_MECHANISM_BYTES init_bytes;
    amqp_binary expected_creds;

    init_bytes.bytes = test_init_bytes;
    init_bytes.length = sizeof(test_init_bytes);

    expected_creds.bytes = test_init_bytes;
    expected_creds.length = sizeof(test_init_bytes);

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value)).SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_mechanisms(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_mechanisms_handle, sizeof(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(sasl_mechanisms_get_sasl_server_mechanisms(test_sasl_mechanisms_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_sasl_server_mechanisms_value(&test_sasl_server_mechanisms_value, sizeof(test_sasl_server_mechanisms_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item_count(test_sasl_server_mechanisms_value, &mechanisms_count))
        .CopyOutArgumentBuffer(2, &mechanisms_count, sizeof(mechanisms_count));
    STRICT_EXPECTED_CALL(saslmechanism_get_mechanism_name(test_sasl_mechanism));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item(test_sasl_server_mechanisms_value, 0));
    STRICT_EXPECTED_CALL(amqpvalue_get_symbol(test_sasl_server_mechanism, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_mechanism, sizeof(test_mechanism));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_sasl_server_mechanism));
    STRICT_EXPECTED_CALL(sasl_init_create(test_mechanism));
    STRICT_EXPECTED_CALL(saslmechanism_get_init_bytes(test_sasl_mechanism, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &init_bytes, sizeof(init_bytes));
    STRICT_EXPECTED_CALL(sasl_init_set_initial_response(test_sasl_init, expected_creds));
    STRICT_EXPECTED_CALL(amqpvalue_create_sasl_init(test_sasl_init));
    STRICT_EXPECTED_CALL(sasl_frame_codec_encode_frame(test_sasl_frame_codec, test_sasl_init_value, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_sasl_init_value));
    STRICT_EXPECTED_CALL(sasl_init_destroy(test_sasl_init));
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_mechanisms_destroy(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_058: [This frame indicates the outcome of the SASL dialog.] */
/* Tests_SRS_SASLCLIENTIO_01_059: [Upon successful completion of the SASL dialog the security layer has been established] */
/* Tests_SRS_SASLCLIENTIO_01_060: [A reply-code indicating the outcome of the SASL dialog.] */
/* Tests_SRS_SASLCLIENTIO_01_062: [0 Connection authentication succeeded.] */
/* Tests_SRS_SASLCLIENTIO_01_038: [<-- SASL-OUTCOME] */
/* Tests_SRS_SASLCLIENTIO_01_072: [When the SASL handshake is complete, if the handshake is successful, the SASL client IO state shall be switched to `IO_STATE_OPEN` and the `on_io_open_complete` callback shall be called with `IO_OPEN_OK`.]*/
TEST_FUNCTION(when_a_SASL_outcome_frame_is_received_with_ok_the_SASL_IO_state_is_switched_to_OPEN)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_code sasl_outcome_code = sasl_code_ok;

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();
    setup_send_init();
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_outcome, sizeof(test_sasl_outcome));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_challenge_type_by_descriptor(test_descriptor_value))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_outcome_type_by_descriptor(test_descriptor_value))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_outcome(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_outcome_handle, sizeof(test_sasl_outcome_handle));
    STRICT_EXPECTED_CALL(sasl_outcome_get_code(test_sasl_outcome_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &sasl_outcome_code, sizeof(sasl_outcome_code));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_OK));
    STRICT_EXPECTED_CALL(sasl_outcome_destroy(test_sasl_outcome_handle));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

void when_an_outcome_with_error_code_is_received_the_IO_is_closed_pending_open_complete_with_error(sasl_code test_sasl_code)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_code sasl_outcome_code = test_sasl_code;

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();
    setup_send_init();
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_outcome, sizeof(test_sasl_outcome));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_challenge_type_by_descriptor(test_descriptor_value))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_outcome_type_by_descriptor(test_descriptor_value))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_outcome(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_outcome_handle, sizeof(test_sasl_outcome_handle));
    STRICT_EXPECTED_CALL(sasl_outcome_get_code(test_sasl_outcome_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &sasl_outcome_code, sizeof(sasl_outcome_code));
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_outcome_destroy(test_sasl_outcome_handle));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_063: [1 Connection authentication failed due to an unspecified problem with the supplied credentials.] */
TEST_FUNCTION(when_a_SASL_outcome_frame_is_received_with_auth_error_code_the_SASL_IO_state_is_switched_to_ERROR)
{
    when_an_outcome_with_error_code_is_received_the_IO_is_closed_pending_open_complete_with_error(sasl_code_auth);
}

/* Tests_SRS_SASLCLIENTIO_01_064: [2 Connection authentication failed due to a system error.] */
TEST_FUNCTION(when_a_SASL_outcome_frame_is_received_with_sys_error_code_the_SASL_IO_state_is_switched_to_ERROR)
{
    when_an_outcome_with_error_code_is_received_the_IO_is_closed_pending_open_complete_with_error(sasl_code_sys);
}

/* Tests_SRS_SASLCLIENTIO_01_065: [3 Connection authentication failed due to a system error that is unlikely to be corrected without intervention.] */
TEST_FUNCTION(when_a_SASL_outcome_frame_is_received_with_sys_perm_error_code_the_SASL_IO_state_is_switched_to_ERROR)
{
    when_an_outcome_with_error_code_is_received_the_IO_is_closed_pending_open_complete_with_error(sasl_code_sys_perm);
}

/* Tests_SRS_SASLCLIENTIO_01_066: [4 Connection authentication failed due to a transient system error.] */
TEST_FUNCTION(when_a_SASL_outcome_frame_is_received_with_sys_temp_error_code_the_SASL_IO_state_is_switched_to_ERROR)
{
    when_an_outcome_with_error_code_is_received_the_IO_is_closed_pending_open_complete_with_error(sasl_code_sys_temp);
}

/* Tests_SRS_SASLCLIENTIO_01_032: [The peer acting as the SASL server MUST announce supported authentication mechanisms using the sasl-mechanisms frame.] */
TEST_FUNCTION(when_a_SASL_outcome_frame_is_received_before_mechanisms_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_outcome, sizeof(test_sasl_outcome));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_challenge_type_by_descriptor(test_descriptor_value))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_outcome_type_by_descriptor(test_descriptor_value))
        .SetReturn(true);

    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_032: [The peer acting as the SASL server MUST announce supported authentication mechanisms using the sasl-mechanisms frame.] */
TEST_FUNCTION(when_a_SASL_challenge_is_received_before_mechanisms_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_challenge, sizeof(test_sasl_challenge));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_challenge_type_by_descriptor(test_descriptor_value))
        .SetReturn(true);

    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

static void setup_succesfull_challenge_response(void)
{
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_challenge_type_by_descriptor(test_descriptor_value))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_challenge(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_challenge_handle, sizeof(test_sasl_challenge_handle));
    STRICT_EXPECTED_CALL(sasl_challenge_get_challenge(test_sasl_challenge_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &some_challenge_bytes, sizeof(some_challenge_bytes));
    STRICT_EXPECTED_CALL(saslmechanism_challenge(test_sasl_mechanism, &sasl_mechanism_challenge_bytes, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(3, &sasl_mechanism_response_bytes, sizeof(sasl_mechanism_response_bytes));
    STRICT_EXPECTED_CALL(sasl_response_create(response_binary_value));
    STRICT_EXPECTED_CALL(amqpvalue_create_sasl_response(test_sasl_response_handle));
    STRICT_EXPECTED_CALL(sasl_frame_codec_encode_frame(test_sasl_frame_codec, test_sasl_response_amqp_value, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_sasl_response_amqp_value));
    STRICT_EXPECTED_CALL(sasl_response_destroy(test_sasl_response_handle));
    STRICT_EXPECTED_CALL(sasl_challenge_destroy(test_sasl_challenge_handle));
}

/* Tests_SRS_SASLCLIENTIO_01_052: [Send the SASL challenge data as defined by the SASL specification.] */
/* Tests_SRS_SASLCLIENTIO_01_053: [Challenge information, a block of opaque binary data passed to the security mechanism.] */
/* Tests_SRS_SASLCLIENTIO_01_055: [Send the SASL response data as defined by the SASL specification.] */
/* Tests_SRS_SASLCLIENTIO_01_056: [A block of opaque data passed to the security mechanism.] */
/* Tests_SRS_SASLCLIENTIO_01_057: [The contents of this data are defined by the SASL security mechanism.] */
/* Tests_SRS_SASLCLIENTIO_01_036: [<-- SASL-CHALLENGE *] */
/* Tests_SRS_SASLCLIENTIO_01_037: [SASL-RESPONSE -->] */
/* Tests_SRS_SASLCLIENTIO_01_070: [When a frame needs to be sent as part of the SASL handshake frame exchange, the send shall be done by calling `sasl_frame_codec_encode_frame`.]*/
TEST_FUNCTION(when_a_SASL_challenge_is_received_after_the_mechanisms_the_sasl_mechanism_challenge_processing_is_invoked)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();
    setup_send_init();
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_outcome, sizeof(test_sasl_outcome));
    umock_c_reset_all_calls();

    setup_succesfull_challenge_response();

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_119: [If any error is encountered when parsing the received frame, the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_getting_the_sasl_challenge_fails_then_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();
    setup_send_init();
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_outcome, sizeof(test_sasl_outcome));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_challenge_type_by_descriptor(test_descriptor_value))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_challenge(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_challenge_handle, sizeof(test_sasl_challenge_handle))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_119: [If any error is encountered when parsing the received frame, the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_getting_the_challenge_bytes_fails_then_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();
    setup_send_init();
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_outcome, sizeof(test_sasl_outcome));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_challenge_type_by_descriptor(test_descriptor_value))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_challenge(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_challenge_handle, sizeof(test_sasl_challenge_handle));
    STRICT_EXPECTED_CALL(sasl_challenge_get_challenge(test_sasl_challenge_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &some_challenge_bytes, sizeof(some_challenge_bytes))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_challenge_destroy(test_sasl_challenge_handle));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_119: [If any error is encountered when parsing the received frame, the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_the_sasl_mechanism_challenge_response_function_fails_then_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();
    setup_send_init();
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_outcome, sizeof(test_sasl_outcome));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_challenge_type_by_descriptor(test_descriptor_value))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_challenge(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_challenge_handle, sizeof(test_sasl_challenge_handle));
    STRICT_EXPECTED_CALL(sasl_challenge_get_challenge(test_sasl_challenge_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &some_challenge_bytes, sizeof(some_challenge_bytes));
    STRICT_EXPECTED_CALL(saslmechanism_challenge(test_sasl_mechanism, &sasl_mechanism_challenge_bytes, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(3, &sasl_mechanism_response_bytes, sizeof(sasl_mechanism_response_bytes))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_challenge_destroy(test_sasl_challenge_handle));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_119: [If any error is encountered when parsing the received frame, the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_creating_the_sasl_response_fails_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();
    setup_send_init();
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_outcome, sizeof(test_sasl_outcome));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_challenge_type_by_descriptor(test_descriptor_value))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_challenge(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_challenge_handle, sizeof(test_sasl_challenge_handle));
    STRICT_EXPECTED_CALL(sasl_challenge_get_challenge(test_sasl_challenge_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &some_challenge_bytes, sizeof(some_challenge_bytes));
    STRICT_EXPECTED_CALL(saslmechanism_challenge(test_sasl_mechanism, &sasl_mechanism_challenge_bytes, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(3, &sasl_mechanism_response_bytes, sizeof(sasl_mechanism_response_bytes));
    STRICT_EXPECTED_CALL(sasl_response_create(response_binary_value))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_challenge_destroy(test_sasl_challenge_handle));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_119: [If any error is encountered when parsing the received frame, the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_creating_the_AMQP_VALUE_for_sasl_response_fails_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();
    setup_send_init();
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_outcome, sizeof(test_sasl_outcome));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_challenge_type_by_descriptor(test_descriptor_value))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_challenge(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_challenge_handle, sizeof(test_sasl_challenge_handle));
    STRICT_EXPECTED_CALL(sasl_challenge_get_challenge(test_sasl_challenge_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &some_challenge_bytes, sizeof(some_challenge_bytes));
    STRICT_EXPECTED_CALL(saslmechanism_challenge(test_sasl_mechanism, &sasl_mechanism_challenge_bytes, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(3, &sasl_mechanism_response_bytes, sizeof(sasl_mechanism_response_bytes));
    STRICT_EXPECTED_CALL(sasl_response_create(response_binary_value));
    STRICT_EXPECTED_CALL(amqpvalue_create_sasl_response(test_sasl_response_handle))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(sasl_response_destroy(test_sasl_response_handle));
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_challenge_destroy(test_sasl_challenge_handle));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_119: [If any error is encountered when parsing the received frame, the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_encoding_the_sasl_frame_for_sasl_response_fails_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();
    setup_send_init();
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_outcome, sizeof(test_sasl_outcome));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_challenge_type_by_descriptor(test_descriptor_value))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_challenge(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_challenge_handle, sizeof(test_sasl_challenge_handle));
    STRICT_EXPECTED_CALL(sasl_challenge_get_challenge(test_sasl_challenge_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &some_challenge_bytes, sizeof(some_challenge_bytes));
    STRICT_EXPECTED_CALL(saslmechanism_challenge(test_sasl_mechanism, &sasl_mechanism_challenge_bytes, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(3, &sasl_mechanism_response_bytes, sizeof(sasl_mechanism_response_bytes));
    STRICT_EXPECTED_CALL(sasl_response_create(response_binary_value));
    STRICT_EXPECTED_CALL(amqpvalue_create_sasl_response(test_sasl_response_handle));
    STRICT_EXPECTED_CALL(sasl_frame_codec_encode_frame(test_sasl_frame_codec, test_sasl_response_amqp_value, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_sasl_response_amqp_value));
    STRICT_EXPECTED_CALL(sasl_response_destroy(test_sasl_response_handle));
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_challenge_destroy(test_sasl_challenge_handle));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_039: [the SASL challenge/response step can occur zero or more times depending on the details of the SASL mechanism chosen.] */
TEST_FUNCTION(SASL_challenge_response_twice_succeed)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();
    setup_send_init();
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_outcome, sizeof(test_sasl_outcome));
    umock_c_reset_all_calls();

    setup_succesfull_challenge_response();
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);

    setup_succesfull_challenge_response();

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_039: [the SASL challenge/response step can occur zero or more times depending on the details of the SASL mechanism chosen.] */
TEST_FUNCTION(SASL_challenge_response_256_times_succeeds)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    size_t i;

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);

    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();
    setup_send_init();
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_outcome, sizeof(test_sasl_outcome));
    umock_c_reset_all_calls();

    // act
    for (i = 0; i < 256; i++)
    {
        setup_succesfull_challenge_response();
        saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    }

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_039: [the SASL challenge/response step can occur zero or more times depending on the details of the SASL mechanism chosen.] */
/* Tests_SRS_SASLCLIENTIO_01_072: [When the SASL handshake is complete, if the handshake is successful, the SASL client IO state shall be switched to `IO_STATE_OPEN` and the `on_io_open_complete` callback shall be called with `IO_OPEN_OK`.]*/
TEST_FUNCTION(SASL_challenge_response_256_times_followed_by_outcome_succeeds)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    size_t i;
    sasl_code sasl_outcome_code = sasl_code_ok;

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);

    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();
    setup_send_init();
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_outcome, sizeof(test_sasl_outcome));
    umock_c_reset_all_calls();

    for (i = 0; i < 256; i++)
    {
        setup_succesfull_challenge_response();
        saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    }

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_challenge_type_by_descriptor(test_descriptor_value))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_sasl_outcome_type_by_descriptor(test_descriptor_value))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_outcome(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_outcome_handle, sizeof(test_sasl_outcome_handle));
    STRICT_EXPECTED_CALL(sasl_outcome_get_code(test_sasl_outcome_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &sasl_outcome_code, sizeof(sasl_outcome_code));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_OK));
    STRICT_EXPECTED_CALL(sasl_outcome_destroy(test_sasl_outcome_handle));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_119: [If any error is encountered when parsing the received frame, the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_the_mechanisms_sasl_value_cannot_be_decoded_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);

    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value)).SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_mechanisms(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_mechanisms_handle, sizeof(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(sasl_mechanisms_get_sasl_server_mechanisms(test_sasl_mechanisms_handle, IGNORED_PTR_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_mechanisms_destroy(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_042: [It is invalid for this list to be null or empty.] */
TEST_FUNCTION(when_a_NULL_list_is_received_in_the_SASL_mechanisms_then_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);

    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value)).SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_mechanisms(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_mechanisms_handle, sizeof(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(sasl_mechanisms_get_sasl_server_mechanisms(test_sasl_mechanisms_handle, IGNORED_PTR_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_mechanisms_destroy(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_042: [It is invalid for this list to be null or empty.] */
TEST_FUNCTION(when_an_empty_array_is_received_in_the_SASL_mechanisms_then_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    uint32_t mechanisms_count = 0;

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);

    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value)).SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_mechanisms(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_mechanisms_handle, sizeof(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(sasl_mechanisms_get_sasl_server_mechanisms(test_sasl_mechanisms_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_sasl_server_mechanisms_value(&test_sasl_server_mechanisms_value, sizeof(test_sasl_server_mechanisms_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item_count(test_sasl_server_mechanisms_value, &mechanisms_count))
        .CopyOutArgumentBuffer(2, &mechanisms_count, sizeof(mechanisms_count));
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_mechanisms_destroy(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_119: [If any error is encountered when parsing the received frame, the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_getting_the_mechanisms_array_item_count_fails_then_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    uint32_t mechanisms_count = 1;

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);

    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value)).SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_mechanisms(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_mechanisms_handle, sizeof(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(sasl_mechanisms_get_sasl_server_mechanisms(test_sasl_mechanisms_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_sasl_server_mechanisms_value(&test_sasl_server_mechanisms_value, sizeof(test_sasl_server_mechanisms_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item_count(test_sasl_server_mechanisms_value, &mechanisms_count))
        .CopyOutArgumentBuffer(2, &mechanisms_count, sizeof(mechanisms_count))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_mechanisms_destroy(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_073: [If the handshake fails (i.e. the outcome is an error) the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_the_mechanisms_array_does_not_contain_a_usable_SASL_mechanism_then_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    uint32_t mechanisms_count = 1;
    const char* test_sasl_server_mechanism_name = "blahblah";

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);

    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value)).SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_mechanisms(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_mechanisms_handle, sizeof(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(sasl_mechanisms_get_sasl_server_mechanisms(test_sasl_mechanisms_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_sasl_server_mechanisms_value(&test_sasl_server_mechanisms_value, sizeof(test_sasl_server_mechanisms_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item_count(test_sasl_server_mechanisms_value, &mechanisms_count))
        .CopyOutArgumentBuffer(2, &mechanisms_count, sizeof(mechanisms_count));
    STRICT_EXPECTED_CALL(saslmechanism_get_mechanism_name(test_sasl_mechanism));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item(test_sasl_server_mechanisms_value, 0));
    STRICT_EXPECTED_CALL(amqpvalue_get_symbol(test_sasl_server_mechanism, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_server_mechanism_name, sizeof(test_sasl_server_mechanism_name));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_sasl_server_mechanism));
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_mechanisms_destroy(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_073: [If the handshake fails (i.e. the outcome is an error) the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_the_mechanisms_array_has_2_mechanisms_and_none_matches_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    uint32_t mechanisms_count = 2;
    const char* test_sasl_server_mechanism_name_1 = "blahblah";
    const char* test_sasl_server_mechanism_name_2 = "another_blah";

    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);

    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(test_sasl_value));
    STRICT_EXPECTED_CALL(is_sasl_mechanisms_type_by_descriptor(test_descriptor_value)).SetReturn(true);
    STRICT_EXPECTED_CALL(amqpvalue_get_sasl_mechanisms(test_sasl_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_mechanisms_handle, sizeof(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(sasl_mechanisms_get_sasl_server_mechanisms(test_sasl_mechanisms_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_sasl_server_mechanisms_value(&test_sasl_server_mechanisms_value, sizeof(test_sasl_server_mechanisms_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item_count(test_sasl_server_mechanisms_value, &mechanisms_count))
        .CopyOutArgumentBuffer(2, &mechanisms_count, sizeof(mechanisms_count));
    STRICT_EXPECTED_CALL(saslmechanism_get_mechanism_name(test_sasl_mechanism));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item(test_sasl_server_mechanisms_value, 0));
    STRICT_EXPECTED_CALL(amqpvalue_get_symbol(test_sasl_server_mechanism, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_server_mechanism_name_1, sizeof(test_sasl_server_mechanism_name_1));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_sasl_server_mechanism));
    STRICT_EXPECTED_CALL(amqpvalue_get_array_item(test_sasl_server_mechanisms_value, 1));
    STRICT_EXPECTED_CALL(amqpvalue_get_symbol(test_sasl_server_mechanism, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_sasl_server_mechanism_name_2, sizeof(test_sasl_server_mechanism_name_2));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_sasl_server_mechanism));
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(sasl_mechanisms_destroy(test_sasl_mechanisms_handle));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_120: [When SASL client IO is notified by `sasl_frame_codec` of bytes that have been encoded via the `on_bytes_encoded` callback and SASL client IO is in the state OPENING, SASL client IO shall send these bytes by using `xio_send`.]*/
TEST_FUNCTION(when_encoded_bytes_are_received_they_are_given_to_xio_send)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    unsigned char encoded_bytes[] = { 0x42, 0x43 };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);

    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    setup_send_init();
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_io, encoded_bytes, sizeof(encoded_bytes), IGNORED_PTR_ARG, NULL))
        .ValidateArgumentBuffer(2, &encoded_bytes, sizeof(encoded_bytes));

    // act
    saved_on_bytes_encoded(saved_on_bytes_encoded_callback_context, encoded_bytes, sizeof(encoded_bytes), true);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_120: [When SASL client IO is notified by `sasl_frame_codec` of bytes that have been encoded via the `on_bytes_encoded` callback and SASL client IO is in the state OPENING, SASL client IO shall send these bytes by using `xio_send`.]*/
TEST_FUNCTION(when_encoded_bytes_are_received_with_encoded_complete_flag_set_to_false_they_are_given_to_xio_send)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    unsigned char encoded_bytes[] = { 0x42, 0x43 };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);

    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    setup_send_init();
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_io, encoded_bytes, sizeof(encoded_bytes), IGNORED_PTR_ARG, NULL))
        .ValidateArgumentBuffer(2, &encoded_bytes, sizeof(encoded_bytes));

    // act
    saved_on_bytes_encoded(saved_on_bytes_encoded_callback_context, encoded_bytes, sizeof(encoded_bytes), false);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_121: [If `xio_send` fails, the `on_io_error` callback shall be triggered.]*/
TEST_FUNCTION(when_xio_send_fails_when_sending_encoded_bytes_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    unsigned char encoded_bytes[] = { 0x42, 0x43 };
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);

    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    setup_send_init();
    saved_on_sasl_frame_received(saved_sasl_frame_codec_callback_context, test_sasl_value);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_io, encoded_bytes, sizeof(encoded_bytes), IGNORED_PTR_ARG, NULL))
        .ValidateArgumentBuffer(2, &encoded_bytes, sizeof(encoded_bytes))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_bytes_encoded(saved_on_bytes_encoded_callback_context, encoded_bytes, sizeof(encoded_bytes), false);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_122: [When `on_frame_codec_error` is called while in the OPENING state the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.]*/
TEST_FUNCTION(when_the_frame_codec_triggers_an_error_in_the_OPENING_state_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);

    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_frame_codec_error(saved_on_frame_codec_error_callback_context);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_143: [ When `on_frame_codec_error` is called while in the OPEN state the `on_io_error` callback shall be triggered. ]*/
TEST_FUNCTION(when_the_frame_codec_triggers_an_error_in_the_OPEN_state_the_on_io_error_callback_is_triggered)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    saved_on_frame_codec_error(saved_on_frame_codec_error_callback_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_123: [When `on_frame_codec_error` is called in the ERROR state nothing shall be done.]*/
TEST_FUNCTION(when_the_frame_codec_triggers_an_error_in_the_ERROR_state_nothing_is_done)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    saved_on_frame_codec_error(saved_sasl_frame_codec_callback_context);
    umock_c_reset_all_calls();

    // act
    saved_on_frame_codec_error(saved_on_frame_codec_error_callback_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_141: [ When `on_sasl_frame_codec_error` is called while in the OPENING state the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`. ]*/
TEST_FUNCTION(when_the_sasl_frame_codec_triggers_an_error_in_the_OPENING_state_the_IO_is_closed_pending_open_complete_with_error)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);

    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, sasl_header, sizeof(sasl_header));
    saved_on_bytes_received(saved_on_bytes_received_context, test_sasl_mechanisms_frame, sizeof(test_sasl_mechanisms_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_sasl_frame_codec_error(saved_sasl_frame_codec_callback_context);
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_144: [ When `on_sasl_frame_codec_error` is called while OPEN state the `on_io_error` callback shall be triggered. ]*/
TEST_FUNCTION(when_the_sasl_frame_codec_triggers_an_error_in_the_OPEN_state_the_saslclientio_state_is_set_to_ERROR)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    saved_on_sasl_frame_codec_error(saved_sasl_frame_codec_callback_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

/* Tests_SRS_SASLCLIENTIO_01_142: [ When `on_sasl_frame_codec_error` is called in the ERROR state nothing shall be done. ]*/
TEST_FUNCTION(when_the_sasl_frame_codec_triggers_an_error_in_the_ERROR_state_nothing_is_done)
{
    // arrange
    SASLCLIENTIO_CONFIG sasl_client_io_config;
    CONCRETE_IO_HANDLE sasl_client_io;
    sasl_client_io_config.underlying_io = test_underlying_io;
    sasl_client_io_config.sasl_mechanism = test_sasl_mechanism;
    sasl_client_io = saslclientio_get_interface_description()->concrete_io_create(&sasl_client_io_config);
    (void)saslclientio_get_interface_description()->concrete_io_open(sasl_client_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    setup_successful_sasl_handshake();
    saved_on_sasl_frame_codec_error(saved_sasl_frame_codec_callback_context);
    umock_c_reset_all_calls();

    // act
    saved_on_sasl_frame_codec_error(saved_sasl_frame_codec_callback_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslclientio_get_interface_description()->concrete_io_destroy(sasl_client_io);
}

END_TEST_SUITE(saslclientio_ut)
