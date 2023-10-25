// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdint>
#include <cstdlib>
#else
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_stdint.h"
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
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_uamqp_c/amqpvalue.h"

#undef ENABLE_MOCKS

#include "azure_uamqp_c/frame_codec.h"

#define TEST_DESCRIPTION_AMQP_VALUE        (AMQP_VALUE)0x4243
#define TEST_LIST_HANDLE                (SINGLYLINKEDLIST_HANDLE)0x4246
#define TEST_SUBSCRIPTION_ITEM            (void*)0x4247
#define TEST_ERROR_CONTEXT                (void*)0x4248
#define TEST_LIST_ITEM_HANDLE            (LIST_ITEM_HANDLE)0x4249

typedef struct TEST_LIST_ITEM_TAG
{
    const void* item_value;
} TEST_LIST_ITEM;

static TEST_LIST_ITEM** list_items = NULL;
static size_t list_item_count = 0;
static unsigned char* sent_io_bytes;
static size_t sent_io_byte_count;
static char expected_stringified_io[8192];
static char actual_stringified_io[8192];

void stringify_bytes(const unsigned char* bytes, size_t byte_count, char* output_string)
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

    /* frame received callback */
MOCK_FUNCTION_WITH_CODE(, void, on_frame_received_1, void*, context, const unsigned char*, type_specific, uint32_t, type_specific_size, const unsigned char*, frame_body, uint32_t, frame_body_size)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, on_frame_received_2, void*, context, const unsigned char*, type_specific, uint32_t, type_specific_size, const unsigned char*, frame_body, uint32_t, frame_body_size)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_frame_codec_decode_error, void*, context)
MOCK_FUNCTION_END();

MOCK_FUNCTION_WITH_CODE(, void, test_on_bytes_encoded, void*, context, const unsigned char*, bytes, size_t, length, bool, encode_complete)
    unsigned char* new_bytes = (unsigned char*)my_gballoc_realloc(sent_io_bytes, sent_io_byte_count + length);
    if (new_bytes != NULL)
    {
        sent_io_bytes = new_bytes;
        (void)memcpy(sent_io_bytes + sent_io_byte_count, bytes, length);
        sent_io_byte_count += length;
    }
MOCK_FUNCTION_END();

static LIST_ITEM_HANDLE my_singlylinkedlist_add(SINGLYLINKEDLIST_HANDLE list, const void* item)
{
    TEST_LIST_ITEM** items = (TEST_LIST_ITEM**)my_gballoc_realloc(list_items, (list_item_count + 1) * sizeof(TEST_LIST_ITEM*));
    LIST_ITEM_HANDLE to_return = NULL;
    (void)list;
    if (items != NULL)
    {
        list_items = items;
        list_items[list_item_count] = (TEST_LIST_ITEM*)my_gballoc_malloc(sizeof(TEST_LIST_ITEM));
        if (list_items[list_item_count] != NULL)
        {
            list_items[list_item_count]->item_value = item;
            to_return = (LIST_ITEM_HANDLE)list_items[list_item_count];
            list_item_count++;
        }
    }
    return to_return;
}

static const void* my_singlylinkedlist_item_get_value(LIST_ITEM_HANDLE item_handle)
{
    return ((TEST_LIST_ITEM*)item_handle)->item_value;
}

static LIST_ITEM_HANDLE my_singlylinkedlist_find(SINGLYLINKEDLIST_HANDLE handle, LIST_MATCH_FUNCTION match_function, const void* match_context)
{
    size_t i;
    LIST_ITEM_HANDLE found_item = NULL;
    (void)handle;

    for (i = 0; i < list_item_count; i++)
    {
        if (match_function((LIST_ITEM_HANDLE)list_items[i], match_context))
        {
            found_item = (LIST_ITEM_HANDLE)list_items[i];
            break;
        }
    }
    return found_item;
}

static int my_singlylinkedlist_remove(SINGLYLINKEDLIST_HANDLE list, LIST_ITEM_HANDLE list_item)
{
    size_t i;
    (void)list;

    for (i = 0; i < list_item_count; i++)
    {
        if (((LIST_ITEM_HANDLE)list_items[i]) == list_item)
        {
            break;
        }
    }
    if (i < list_item_count)
    {
        my_gballoc_free(list_items[i]);
        memmove(&list_items[i], &list_items[i + 1], (list_item_count - i - 1) * sizeof(TEST_LIST_ITEM*));
        list_item_count--;
        if (list_item_count == 0)
        {
            my_gballoc_free(list_items);
            list_items = NULL;
        }
    }
    return 0;
}

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(frame_codec_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result, "Failed registering stdint types");
    result = umocktypes_bool_register_types();
    ASSERT_ARE_EQUAL(int, 0, result, "Failed registering bool types");

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, my_gballoc_calloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
    REGISTER_GLOBAL_MOCK_RETURN(singlylinkedlist_create, TEST_LIST_HANDLE);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_add, my_singlylinkedlist_add);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_item_get_value, my_singlylinkedlist_item_get_value);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_find, my_singlylinkedlist_find);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_remove, my_singlylinkedlist_remove);

    REGISTER_UMOCK_ALIAS_TYPE(SINGLYLINKEDLIST_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_MATCH_FUNCTION, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_ITEM_HANDLE, void*);
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
    size_t i;
    if (sent_io_bytes != NULL)
    {
        free(sent_io_bytes);
        sent_io_bytes = NULL;
    }
    for (i = 0; i < list_item_count; i++)
    {
        free(list_items[i]);
    }
    free(list_items);
    list_items = NULL;
    list_item_count = 0;

    sent_io_byte_count = 0;

    TEST_MUTEX_RELEASE(g_testByTest);
}

/* frame_codec_create */

/* Tests_SRS_FRAME_CODEC_01_021: [frame_codec_create shall create a new instance of frame_codec and return a non-NULL handle to it on success.] */
TEST_FUNCTION(frame_codec_create_with_valid_args_succeeds)
{
    // arrange
    FRAME_CODEC_HANDLE frame_codec;
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());

    // act
    frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);

    // assert
    ASSERT_IS_NOT_NULL(frame_codec);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_020: [If the on_frame_codec_error argument is NULL, frame_codec_create shall return NULL.] */
TEST_FUNCTION(frame_codec_create_with_NULL_on_error_decode_fails)
{
    // arrange

    // act
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(NULL, TEST_ERROR_CONTEXT);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_104: [The callback_context shall be allowed to be NULL.] */
TEST_FUNCTION(frame_codec_create_with_NULL_frame_codec_decode_error_calback_context_succeeds)
{
    // arrange
    FRAME_CODEC_HANDLE frame_codec;
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());

    // act
    frame_codec = frame_codec_create(test_frame_codec_decode_error, NULL);

    // assert
    ASSERT_IS_NOT_NULL(frame_codec);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_022: [If allocating memory for the frame_codec instance fails, frame_codec_create shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_the_frame_codec_fails_frame_code_create_fails)
{
    // arrange
    FRAME_CODEC_HANDLE frame_codec;
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);

    // assert
    ASSERT_IS_NULL(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_082: [The initial max_frame_size_shall be 512.] */
/* Tests_SRS_FRAME_CODEC_01_095: [If the frame_size needed for the frame is bigger than the maximum frame size, frame_codec_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(sending_a_frame_with_more_than_512_bytes_of_total_frame_size_fails_immediately_after_create)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char bytes[505] = { 0 };
    PAYLOAD payload;
    payload.bytes = bytes;
    payload.length = sizeof(bytes);
    umock_c_reset_all_calls();

    // act
    result = frame_codec_encode_frame(frame_codec, 0, &payload, 1, NULL, 0, NULL, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_082: [The initial max_frame_size_shall be 512.] */
/* Tests_SRS_FRAME_CODEC_01_075: [frame_codec_set_max_frame_size shall set the maximum frame size for a frame_codec.] */
/* Tests_SRS_FRAME_CODEC_01_088: [Encoded bytes shall be passed to the `on_bytes_encoded` callback in a single call, while setting the `encode complete` argument to true.] */
/* Tests_SRS_FRAME_CODEC_01_108: [ Memory shall be allocated to hold the entire frame. ]*/
TEST_FUNCTION(a_frame_of_exactly_max_frame_size_immediately_after_create_can_be_sent)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char bytes[504] = { 0 };
    unsigned char expected_bytes[512] = { 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00 };
    PAYLOAD payload;
    payload.bytes = bytes;
    payload.length = sizeof(bytes);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_bytes_encoded((void*)0x4242, IGNORED_PTR_ARG, IGNORED_NUM_ARG, true));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_encode_frame(frame_codec, 0, &payload, 1, NULL, 0, test_on_bytes_encoded, (void*)0x4242);

    // assert
    stringify_bytes(sent_io_bytes, sent_io_byte_count, actual_stringified_io);
    stringify_bytes(expected_bytes, sizeof(expected_bytes), expected_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, expected_stringified_io, actual_stringified_io);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_082: [The initial max_frame_size_shall be 512.] */
/* Tests_SRS_FRAME_CODEC_01_096: [If a frame bigger than the current max frame size is received, frame_codec_receive_bytes shall fail and return a non-zero value.] */
/* Tests_SRS_FRAME_CODEC_01_103: [Upon any decode error, if an error callback has been passed to frame_codec_create, then the error callback shall be called with the context argument being the frame_codec_error_callback_context argument passed to frame_codec_create.] */
TEST_FUNCTION(receiving_a_frame_with_more_than_512_bytes_of_total_frame_size_immediately_after_create_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x02, 0x01 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_frame_codec_decode_error(TEST_ERROR_CONTEXT));

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_082: [The initial max_frame_size_shall be 512.] */
/* Tests_SRS_FRAME_CODEC_01_096: [If a frame bigger than the current max frame size is received, frame_codec_receive_bytes shall fail and return a non-zero value.] */
TEST_FUNCTION(receiving_a_frame_with_exactly_512_bytes_of_total_frame_size_immediately_after_create_succeeds)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[512] = { 0x00, 0x00, 0x02, 0x00, 0x02, 0x00 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();
    (void)memset(frame + 6, 0, 506);

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_1(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 504))
        .ValidateArgumentBuffer(2, &frame[6], 2)
        .ValidateArgumentBuffer(4, &frame[8], 504);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* frame_codec_destroy */

/* Tests_SRS_FRAME_CODEC_01_023: [frame_codec_destroy shall free all resources associated with a frame_codec instance.] */
TEST_FUNCTION(frame_codec_destroy_frees_the_memory_for_frame_codec)
{
    // arrange
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_LIST_HANDLE));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    frame_codec_destroy(frame_codec);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_FRAME_CODEC_01_024: [If frame_codec is NULL, frame_codec_destroy shall do nothing.] */
TEST_FUNCTION(when_frame_codec_is_NULL_frame_codec_destroy_does_nothing)
{
    // arrange

    // act
    frame_codec_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_FRAME_CODEC_01_023: [frame_codec_destroy shall free all resources associated with a frame_codec instance.] */
TEST_FUNCTION(frame_codec_destroy_while_receiving_type_specific_data_frees_the_type_specific_buffer)
{
    // arrange
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x00 };
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));
    (void)frame_codec_unsubscribe(frame_codec, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_LIST_HANDLE));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    frame_codec_destroy(frame_codec);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* frame_codec_set_max_frame_size */

/* Tests_SRS_FRAME_CODEC_01_075: [frame_codec_set_max_frame_size shall set the maximum frame size for a frame_codec.] */
/* Tests_SRS_FRAME_CODEC_01_076: [On success, frame_codec_set_max_frame_size shall return 0.] */
TEST_FUNCTION(frame_codec_set_max_frame_size_with_8_succeeds)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    umock_c_reset_all_calls();

    // act
    result = frame_codec_set_max_frame_size(frame_codec, 8);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

#if 0
/* Tests_SRS_FRAME_CODEC_01_075: [frame_codec_set_max_frame_size shall set the maximum frame size for a frame_codec.] */
TEST_FUNCTION(when_a_frame_bigger_than_max_frame_size_is_sent_frame_codec_encode_frame_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    (void)frame_codec_set_max_frame_size(frame_codec, 1024);
    umock_c_reset_all_calls();

    // act
    result = frame_codec_encode_frame(frame_codec, 0, 1017, NULL, 0, NULL, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_075: [frame_codec_set_max_frame_size shall set the maximum frame size for a frame_codec.] */
TEST_FUNCTION(a_frame_of_exactly_max_frame_size_can_be_sent)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    (void)frame_codec_set_max_frame_size(frame_codec, 1024);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ExpectedAtLeastTimes(1);
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllCalls();

    // act
    result = frame_codec_encode_frame(frame_codec, 0, 1016, NULL, 0, NULL, NULL);

    // assert
    stringify_bytes(sent_io_bytes, sent_io_byte_count, actual_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, "[0x00,0x00,0x04,0x00,0x02,0x00,0x00,0x00]", actual_stringified_io);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}
#endif

/* Tests_SRS_FRAME_CODEC_01_096: [If a frame bigger than the current max frame size is received, frame_codec_receive_bytes shall fail and return a non-zero value.] */
/* Tests_SRS_FRAME_CODEC_01_103: [Upon any decode error, if an error callback has been passed to frame_codec_create, then the error callback shall be called with the context argument being the frame_codec_error_callback_context argument passed to frame_codec_create.] */
TEST_FUNCTION(receiving_a_frame_with_more_than_max_frame_size_bytes_of_total_frame_size_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x04, 0x01 };
    (void)frame_codec_set_max_frame_size(frame_codec, 1024);
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_frame_codec_decode_error(TEST_ERROR_CONTEXT));

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_096: [If a frame bigger than the current max frame size is received, frame_codec_receive_bytes shall fail and return a non-zero value.] */
TEST_FUNCTION(receiving_a_frame_with_exactly_max_frame_size_bytes_of_total_frame_size_fails_succeeds)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[1024] = { 0x00, 0x00, 0x04, 0x00, 0x02, 0x00 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    (void)frame_codec_set_max_frame_size(frame_codec, 1024);
    umock_c_reset_all_calls();
    (void)memset(frame + 6, 0, 1016);

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_1(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 1016))
        .ValidateArgumentBuffer(2, &frame[6], 2)
        .ValidateArgumentBuffer(4, &frame[8], 1016);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_077: [If frame_codec is NULL, frame_codec_set_max_frame_size shall return a non-zero value.] */
TEST_FUNCTION(when_frame_codec_is_NULL_frame_codec_set_max_frame_size_fails)
{
    // arrange

    // act
    int result = frame_codec_set_max_frame_size(NULL, 1024);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_FRAME_CODEC_01_078: [If max_frame_size is invalid according to the AMQP standard, frame_codec_set_max_frame_size shall return a non-zero value.] */
/* Tests_SRS_FRAME_CODEC_01_010: [The frame is malformed if the size is less than the size of the frame header (8 bytes).] */
TEST_FUNCTION(when_frame_codec_is_too_small_then_frame_codec_set_max_frame_size_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    umock_c_reset_all_calls();

    // act
    result = frame_codec_set_max_frame_size(frame_codec, 7);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_081: [If a frame being decoded already has a size bigger than the max_frame_size argument then frame_codec_set_max_frame_size shall return a non-zero value and the previous frame size shall be kept.] */
TEST_FUNCTION(attempting_to_set_a_max_frame_size_lower_than_the_size_of_the_currently_being_received_frame_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[1024] = { 0x00, 0x00, 0x04, 0x00, 0x02, 0x00 };
    (void)frame_codec_set_max_frame_size(frame_codec, 1024);
    (void)memset(frame + 6, 0, 1016);

    (void)frame_codec_receive_bytes(frame_codec, frame, 4);
    umock_c_reset_all_calls();

    // act
    result = frame_codec_set_max_frame_size(frame_codec, 8);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_097: [Setting a frame size on a frame_codec that had a decode error shall fail.] */
TEST_FUNCTION(setting_the_max_frame_size_on_a_codec_with_a_decode_error_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x07 };

    (void)frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));
    umock_c_reset_all_calls();

    // act
    result = frame_codec_set_max_frame_size(frame_codec, 1024);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

#if 0
/* Tests_SRS_FRAME_CODEC_01_097: [Setting a frame size on a frame_codec that had a decode error shall fail.] */
TEST_FUNCTION(setting_the_max_frame_size_on_a_codec_with_an_encode_error_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char bytes[] = { 0x42, 0x43 };
    (void)frame_codec_encode_frame(frame_codec, 0x42, sizeof(bytes), NULL, 0, NULL, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, sizeof(bytes), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(2, &bytes, sizeof(bytes))
        .SetReturn(1);

    (void)frame_codec_encode_frame_bytes(frame_codec, bytes, sizeof(bytes));

    // act
    result = frame_codec_set_max_frame_size(frame_codec, 1024);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}
#endif

/* Tests_SRS_FRAME_CODEC_01_079: [The new frame size shall take effect immediately, even for a frame that is being decoded at the time of the call.] */
/* Tests_SRS_FRAME_CODEC_01_103: [Upon any decode error, if an error callback has been passed to frame_codec_create, then the error callback shall be called with the context argument being the frame_codec_error_callback_context argument passed to frame_codec_create.] */
TEST_FUNCTION(setting_a_new_max_frame_while_the_frame_size_is_being_received_makes_the_new_frame_size_be_in_effect)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x09, 0x02, 0x00, 0x00, 0x00, 0x00 };

    (void)frame_codec_receive_bytes(frame_codec, frame, 3);
    umock_c_reset_all_calls();

    (void)frame_codec_set_max_frame_size(frame_codec, 8);

    STRICT_EXPECTED_CALL(test_frame_codec_decode_error(TEST_ERROR_CONTEXT));

    // act
    result = frame_codec_receive_bytes(frame_codec, frame + 3, sizeof(frame) - 3);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* frame_codec_receive_bytes */

/* Tests_SRS_FRAME_CODEC_01_025: [frame_codec_receive_bytes decodes a sequence of bytes into frames and on success it shall return zero.] */
/* Tests_SRS_FRAME_CODEC_01_028: [The sequence of bytes shall be decoded according to the AMQP ISO.] */
/* Tests_SRS_FRAME_CODEC_01_031: [When a complete frame is successfully decoded it shall be indicated to the upper layer by invoking the on_frame_received passed to frame_codec_subscribe.] */
/* Tests_SRS_FRAME_CODEC_01_032: [Besides passing the frame information, the callback_context value passed to frame_codec_subscribe shall be passed to the on_frame_received_1 function.] */
/* Tests_SRS_FRAME_CODEC_01_001: [Frames are divided into three distinct areas: a fixed width frame header, a variable width extended header, and a variable width frame body.] */
/* Tests_SRS_FRAME_CODEC_01_002: [frame header The frame header is a fixed size (8 byte) structure that precedes each frame.] */
/* Tests_SRS_FRAME_CODEC_01_003: [The frame header includes mandatory information necessary to parse the rest of the frame including size and type information.] */
/* Tests_SRS_FRAME_CODEC_01_004: [extended header The extended header is a variable width area preceding the frame body.] */
/* Tests_SRS_FRAME_CODEC_01_007: [frame body The frame body is a variable width sequence of bytes the format of which depends on the frame type.] */
/* Tests_SRS_FRAME_CODEC_01_008: [SIZE Bytes 0-3 of the frame header contain the frame size.] */
/* Tests_SRS_FRAME_CODEC_01_009: [This is an unsigned 32-bit integer that MUST contain the total frame size of the frame header, extended header, and frame body.] */
/* Tests_SRS_FRAME_CODEC_01_011: [DOFF Byte 4 of the frame header is the data offset.] */
/* Tests_SRS_FRAME_CODEC_01_012: [This gives the position of the body within the frame.] */
/* Tests_SRS_FRAME_CODEC_01_013: [The value of the data offset is an unsigned, 8-bit integer specifying a count of 4-byte words.] */
/* Tests_SRS_FRAME_CODEC_01_015: [TYPE Byte 5 of the frame header is a type code.] */
/* Tests_SRS_FRAME_CODEC_01_035: [After successfully registering a callback for a certain frame type, when subsequently that frame type is received the callbacks shall be invoked, passing to it the received frame and the callback_context value. */
/* Tests_SRS_FRAME_CODEC_01_100: [If the frame body size is 0, the frame_body pointer passed to on_frame_received shall be NULL.] */
TEST_FUNCTION(frame_codec_receive_bytes_decodes_one_empty_frame)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x00, 0x00 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_1(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 0))
        .ValidateArgumentBuffer(2, &frame[6], 2);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_025: [frame_codec_receive_bytes decodes a sequence of bytes into frames and on success it shall return zero.] */
/* Tests_SRS_FRAME_CODEC_01_029: [The sequence of bytes does not have to be a complete frame, frame_codec shall be responsible for maintaining decoding state between frame_codec_receive_bytes calls.] */
TEST_FUNCTION(frame_codec_receive_bytes_with_not_enough_bytes_for_a_frame_does_not_trigger_callback)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x00 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_026: [If frame_codec or buffer are NULL, frame_codec_receive_bytes shall return a non-zero value.] */
TEST_FUNCTION(frame_codec_receive_bytes_with_NULL_frame_codec_handle_fails)
{
    // arrange
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x00, 0x00 };

    // act
    int result = frame_codec_receive_bytes(NULL, frame, sizeof(frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_FRAME_CODEC_01_026: [If frame_codec or buffer are NULL, frame_codec_receive_bytes shall return a non-zero value.] */
TEST_FUNCTION(frame_codec_receive_bytes_with_NULL_buffer_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    // act
    result = frame_codec_receive_bytes(frame_codec, NULL, 1);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_027: [If size is zero, frame_codec_receive_bytes shall return a non-zero value.] */
TEST_FUNCTION(frame_codec_receive_bytes_with_zero_size_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x00, 0x00 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, 0);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_029: [The sequence of bytes does not have to be a complete frame, frame_codec shall be responsible for maintaining decoding state between frame_codec_receive_bytes calls.] */
/* Codes_SRS_FRAME_CODEC_01_005: [This is an extension point defined for future expansion.] */
/* Codes_SRS_FRAME_CODEC_01_006: [The treatment of this area depends on the frame type.] */
TEST_FUNCTION(when_frame_codec_receive_1_byte_in_one_call_and_the_rest_of_the_frame_in_another_call_yields_succesfull_decode)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x00, 0x00 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_1(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 0))
        .ValidateArgumentBuffer(2, &frame[6], 2);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    (void)frame_codec_receive_bytes(frame_codec, frame, 1);

    // act
    result = frame_codec_receive_bytes(frame_codec, frame + 1, sizeof(frame) - 1);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_029: [The sequence of bytes does not have to be a complete frame, frame_codec shall be responsible for maintaining decoding state between frame_codec_receive_bytes calls.] */
TEST_FUNCTION(when_frame_codec_receive_the_frame_bytes_in_1_byte_per_call_a_succesfull_decode_happens)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x00, 0x00 };
    size_t i;
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_1(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 0))
        .ValidateArgumentBuffer(2, &frame[6], 2);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    for (i = 0; i < sizeof(frame) - 1; i++)
    {
        (void)frame_codec_receive_bytes(frame_codec, &frame[i], 1);
    }

    // act
    result = frame_codec_receive_bytes(frame_codec, &frame[sizeof(frame) - 1], 1);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_029: [The sequence of bytes does not have to be a complete frame, frame_codec shall be responsible for maintaining decoding state between frame_codec_receive_bytes calls.] */
TEST_FUNCTION(a_frame_codec_receive_bytes_call_with_bad_args_before_any_real_frame_bytes_does_not_affect_decoding)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x00, 0x00 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_1(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 0))
        .ValidateArgumentBuffer(2, &frame[6], 2);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    (void)frame_codec_receive_bytes(frame_codec, NULL, 1);

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_029: [The sequence of bytes does not have to be a complete frame, frame_codec shall be responsible for maintaining decoding state between frame_codec_receive_bytes calls.] */
TEST_FUNCTION(a_frame_codec_receive_bytes_call_with_bad_args_in_the_middle_of_the_frame_does_not_affect_decoding)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x00, 0x00 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_1(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 0))
        .ValidateArgumentBuffer(2, &frame[6], 2);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    (void)frame_codec_receive_bytes(frame_codec, frame, 1);
    (void)frame_codec_receive_bytes(frame_codec, NULL, 1);

    // act
    result = frame_codec_receive_bytes(frame_codec, frame + 1, sizeof(frame) - 1);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_025: [frame_codec_receive_bytes decodes a sequence of bytes into frames and on success it shall return zero.] */
TEST_FUNCTION(frame_codec_receive_bytes_decodes_2_empty_frames)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame1[] = { 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x01, 0x02 };
    unsigned char frame2[] = { 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x03, 0x04 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame1[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_1(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 0))
        .ValidateArgumentBuffer(2, &frame1[6], 2);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame2[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_1(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 0))
        .ValidateArgumentBuffer(2, &frame2[6], 2);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    (void)frame_codec_receive_bytes(frame_codec, frame1, sizeof(frame1));

    // act
    result = frame_codec_receive_bytes(frame_codec, frame2, sizeof(frame2));

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_025: [frame_codec_receive_bytes decodes a sequence of bytes into frames and on success it shall return zero.] */
TEST_FUNCTION(a_call_to_frame_codec_receive_bytes_with_bad_args_between_2_frames_does_not_affect_decoding)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame1[] = { 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x01, 0x02 };
    unsigned char frame2[] = { 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x03, 0x04 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame1[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_1(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 0))
        .ValidateArgumentBuffer(2, &frame1[6], 2);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame2[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_1(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 0))
        .ValidateArgumentBuffer(2, &frame2[6], 2);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    (void)frame_codec_receive_bytes(frame_codec, frame1, sizeof(frame1));
    (void)frame_codec_receive_bytes(frame_codec, NULL, 1);

    // act
    result = frame_codec_receive_bytes(frame_codec, frame2, sizeof(frame2));

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_025: [frame_codec_receive_bytes decodes a sequence of bytes into frames and on success it shall return zero.] */
TEST_FUNCTION(when_getting_the_list_item_value_fails_no_callback_is_invoked)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x01, 0x02 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG))
        .SetReturn(NULL);

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_010: [The frame is malformed if the size is less than the size of the frame header (8 bytes).] */
/* Tests_SRS_FRAME_CODEC_01_103: [Upon any decode error, if an error callback has been passed to frame_codec_create, then the error callback shall be called with the context argument being the frame_codec_error_callback_context argument passed to frame_codec_create.] */
TEST_FUNCTION(when_frame_size_is_bad_frame_codec_receive_bytes_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x07, 0x02, 0x00, 0x01, 0x02 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_frame_codec_decode_error(TEST_ERROR_CONTEXT));

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_014: [Due to the mandatory 8-byte frame header, the frame is malformed if the value is less than 2.] */
/* Tests_SRS_FRAME_CODEC_01_103: [Upon any decode error, if an error callback has been passed to frame_codec_create, then the error callback shall be called with the context argument being the frame_codec_error_callback_context argument passed to frame_codec_create.] */
TEST_FUNCTION(when_frame_size_has_a_bad_doff_frame_codec_receive_bytes_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x01, 0x02 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_frame_codec_decode_error(TEST_ERROR_CONTEXT));

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_074: [If a decoding error is detected, any subsequent calls on frame_codec_receive_bytes shall fail.] */
TEST_FUNCTION(after_a_frame_decode_error_occurs_due_to_frame_size_a_subsequent_decode_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char bad_frame[] = { 0x00, 0x00, 0x00, 0x07, 0x02, 0x00, 0x01, 0x02 };
    unsigned char good_frame[] = { 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x01, 0x02 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);

    (void)frame_codec_receive_bytes(frame_codec, bad_frame, sizeof(bad_frame));
    umock_c_reset_all_calls();

    // act
    result = frame_codec_receive_bytes(frame_codec, good_frame, sizeof(good_frame));

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_074: [If a decoding error is detected, any subsequent calls on frame_codec_receive_bytes shall fail.] */
TEST_FUNCTION(after_a_frame_decode_error_occurs_due_to_bad_doff_size_a_subsequent_decode_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char bad_frame[] = { 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x01, 0x02 };
    unsigned char good_frame[] = { 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x01, 0x02 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);

    (void)frame_codec_receive_bytes(frame_codec, bad_frame, sizeof(bad_frame));
    umock_c_reset_all_calls();

    // act
    result = frame_codec_receive_bytes(frame_codec, good_frame, sizeof(good_frame));

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_025: [frame_codec_receive_bytes decodes a sequence of bytes into frames and on success it shall return zero.] */
/* Tests_SRS_FRAME_CODEC_01_031: [When a complete frame is successfully decoded it shall be indicated to the upper layer by invoking the on_frame_received passed to frame_codec_subscribe.] */
/* Tests_SRS_FRAME_CODEC_01_099: [A pointer to the frame_body bytes shall also be passed to the on_frame_received.] */
/* Tests_SRS_FRAME_CODEC_01_102: [frame_codec_receive_bytes shall allocate memory to hold the frame_body bytes.] */
TEST_FUNCTION(receiving_a_frame_with_1_byte_frame_body_succeeds)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x09, 0x02, 0x00, 0x01, 0x02, 0x42 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_1(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 1))
        .ValidateArgumentBuffer(2, &frame[6], 2)
        .ValidateArgumentBuffer(4, &frame[8], 1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_101: [If the memory for the frame_body bytes cannot be allocated, frame_codec_receive_bytes shall fail and return a non-zero value.] */
/* Tests_SRS_FRAME_CODEC_01_103: [Upon any decode error, if an error callback has been passed to frame_codec_create, then the error callback shall be called with the context argument being the frame_codec_error_callback_context argument passed to frame_codec_create.] */
TEST_FUNCTION(when_allocating_type_specific_data_fails_frame_codec_receive_bytes_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x09, 0x02, 0x00, 0x01, 0x02, 0x42 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    STRICT_EXPECTED_CALL(test_frame_codec_decode_error(TEST_ERROR_CONTEXT));

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_030: [If a decoding error occurs, frame_codec_receive_bytes shall return a non-zero value.] */
/* Tests_SRS_FRAME_CODEC_01_074: [If a decoding error is detected, any subsequent calls on frame_codec_data_receive_bytes shall fail.] */
TEST_FUNCTION(when_allocating_type_specific_data_fails_a_subsequent_decode_Call_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x09, 0x02, 0x00, 0x01, 0x02, 0x42 };
    frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    (void)frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));
    umock_c_reset_all_calls();

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_031: [When a complete frame is successfully decoded it shall be indicated to the upper layer by invoking the on_frame_received passed to frame_codec_subscribe.] */
TEST_FUNCTION(a_frame_with_2_bytes_received_together_with_the_header_passes_the_bytes_in_one_call)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x0A, 0x02, 0x00, 0x01, 0x02, 0x42, 0x43 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_1(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 2))
        .ValidateArgumentBuffer(2, &frame[6], 2)
        .ValidateArgumentBuffer(4, &frame[sizeof(frame) - 2], 2);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_025: [frame_codec_receive_bytes decodes a sequence of bytes into frames and on success it shall return zero.]  */
TEST_FUNCTION(two_empty_frames_received_in_the_same_call_yields_2_callbacks)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x01, 0x02,
        0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x03, 0x04 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_1(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 0))
        .ValidateArgumentBuffer(2, &frame[6], 2);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_1(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 0))
        .ValidateArgumentBuffer(2, &frame[14], 2);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_025: [frame_codec_receive_bytes decodes a sequence of bytes into frames and on success it shall return zero.]  */
TEST_FUNCTION(two_frames_with_1_byte_each_received_in_the_same_call_yields_2_callbacks)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x09, 0x02, 0x00, 0x01, 0x02, 0x42,
        0x00, 0x00, 0x00, 0x09, 0x02, 0x00, 0x03, 0x04, 0x43 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_1(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 1))
        .ValidateArgumentBuffer(2, &frame[6], 2)
        .ValidateArgumentBuffer(4, &frame[8], 1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_1(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 1))
        .ValidateArgumentBuffer(2, &frame[15], 2)
        .ValidateArgumentBuffer(4, &frame[17], 1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* frame_codec_subscribe */

/* Tests_SRS_FRAME_CODEC_01_033: [frame_codec_subscribe subscribes for a certain type of frame received by the frame_codec instance identified by frame_codec.] */
/* Tests_SRS_FRAME_CODEC_01_087: [On success, frame_codec_subscribe shall return zero.] */
TEST_FUNCTION(frame_codec_subscribe_with_valid_args_succeeds)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    uint8_t frame_type = 0;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame_type, 1);
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_LIST_HANDLE, IGNORED_PTR_ARG));

    // act
    result = frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_033: [frame_codec_subscribe subscribes for a certain type of frame received by the frame_codec instance identified by frame_codec.] */
/* Tests_SRS_FRAME_CODEC_01_087: [On success, frame_codec_subscribe shall return zero.] */
TEST_FUNCTION(when_list_find_returns_NULL_a_new_subscription_is_created)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    uint8_t frame_type = 0;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame_type, 1)
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_LIST_HANDLE, IGNORED_PTR_ARG));

    // act
    result = frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_037: [If any failure occurs while performing the subscribe operation, frame_codec_subscribe shall return a non-zero value.] */
TEST_FUNCTION(when_list_item_get_value_returns_NULL_subscribe_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    uint8_t frame_type = 0;
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame_type, 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG))
        .SetReturn(NULL);

    // act
    result = frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_034: [If any of the frame_codec or on_frame_received arguments is NULL, frame_codec_subscribe shall return a non-zero value.] */
TEST_FUNCTION(when_frame_codec_is_NULL_frame_codec_subscribe_fails)
{
    // arrange

    // act
    int result = frame_codec_subscribe(NULL, 0, on_frame_received_1, (void*)0x01);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_FRAME_CODEC_01_034: [If any of the frame_codec or on_frame_received arguments is NULL, frame_codec_subscribe shall return a non-zero value.] */
TEST_FUNCTION(when_on_frame_received_is_NULL_frame_codec_subscribe_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    umock_c_reset_all_calls();

    // act
    result = frame_codec_subscribe(frame_codec, 0, NULL, frame_codec);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_035: [After successfully registering a callback for a certain frame type, when subsequently that frame type is received the callbacks shall be invoked, passing to it the received frame and the callback_context value. */
TEST_FUNCTION(when_a_frame_type_that_has_no_subscribers_is_received_no_callback_is_called)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x08, 0x02, 0x01, 0x00, 0x00 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_035: [After successfully registering a callback for a certain frame type, when subsequently that frame type is received the callbacks shall be invoked, passing to it the received frame and the callback_context value.] */
TEST_FUNCTION(when_no_subscribe_is_done_no_callback_is_called)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x08, 0x02, 0x01, 0x00, 0x00 };
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_035: [After successfully registering a callback for a certain frame type, when subsequently that frame type is received the callbacks shall be invoked, passing to it the received frame and the callback_context value.] */
TEST_FUNCTION(when_2_subscriptions_exist_and_first_one_matches_the_callback_is_invoked)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x0A, 0x02, 0x00, 0x01, 0x02, 0x42, 0x43 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    (void)frame_codec_subscribe(frame_codec, 1, on_frame_received_2, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_1(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 2))
        .ValidateArgumentBuffer(2, &frame[6], 2)
        .ValidateArgumentBuffer(4, &frame[sizeof(frame) - 2], 2);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    (void)frame_codec_unsubscribe(frame_codec, 1);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_035: [After successfully registering a callback for a certain frame type, when subsequently that frame type is received the callbacks shall be invoked, passing to it the received frame and the callback_context value.] */
TEST_FUNCTION(when_2_subscriptions_exist_and_second_one_matches_the_callback_is_invoked)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x0A, 0x02, 0x01, 0x01, 0x02, 0x42, 0x43 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    (void)frame_codec_subscribe(frame_codec, 1, on_frame_received_2, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_2(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 2))
        .ValidateArgumentBuffer(2, &frame[6], 2)
        .ValidateArgumentBuffer(4, &frame[sizeof(frame) - 2], 2);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    (void)frame_codec_unsubscribe(frame_codec, 1);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_036: [Only one callback pair shall be allowed to be registered for a given frame type.] */
TEST_FUNCTION(when_frame_codec_subscribe_is_called_twice_for_the_same_frame_type_it_succeeds)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    uint8_t frame_type = 0;
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame_type, 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));

    // act
    result = frame_codec_subscribe(frame_codec, 0, on_frame_received_2, frame_codec);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_036: [Only one callback pair shall be allowed to be registered for a given frame type.] */
TEST_FUNCTION(the_callbacks_for_the_2nd_frame_codec_subscribe_for_the_same_frame_type_remain_in_effect)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x0A, 0x02, 0x00, 0x01, 0x02, 0x42, 0x43 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_2, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(on_frame_received_2(frame_codec, IGNORED_PTR_ARG, 2, IGNORED_PTR_ARG, 2))
        .ValidateArgumentBuffer(2, &frame[6], 2)
        .ValidateArgumentBuffer(4, &frame[sizeof(frame) - 2], 2);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_037: [If any failure occurs while performing the subscribe operation, frame_codec_subscribe shall return a non-zero value.] */
TEST_FUNCTION(when_allocating_memory_for_the_subscription_fails_frame_codec_subscribe_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    uint8_t frame_type = 0;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame_type, 1);
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    result = frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_037: [If any failure occurs while performing the subscribe operation, frame_codec_subscribe shall return a non-zero value.] */
TEST_FUNCTION(when_adding_the_subscription_fails_then_frame_codec_subscribe_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    uint8_t frame_type = 0;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame_type, 1);
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_LIST_HANDLE, IGNORED_PTR_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* frame_codec_unsubscribe */

/* Tests_SRS_FRAME_CODEC_01_038: [frame_codec_unsubscribe removes a previous subscription for frames of type type and on success it shall return 0.] */
TEST_FUNCTION(removing_an_existing_subscription_succeeds)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    uint8_t frame_type = 0;
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame_type, 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));

    // act
    result = frame_codec_unsubscribe(frame_codec, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_038: [frame_codec_unsubscribe removes a previous subscription for frames of type type and on success it shall return 0.] */
TEST_FUNCTION(removing_an_existing_subscription_does_not_trigger_callback_when_a_frame_of_that_type_is_received)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    unsigned char frame[] = { 0x00, 0x00, 0x00, 0x0A, 0x02, 0x00, 0x01, 0x02, 0x42, 0x43 };
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    (void)frame_codec_unsubscribe(frame_codec, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame[5], 1);

    // act
    result = frame_codec_receive_bytes(frame_codec, frame, sizeof(frame));

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_039: [If frame_codec is NULL, frame_codec_unsubscribe shall return a non-zero value.] */
TEST_FUNCTION(frame_codec_unsubscribe_with_NULL_frame_codec_handle_fails)
{
    // arrange

    // act
    int result = frame_codec_unsubscribe(NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_FRAME_CODEC_01_040: [If no subscription for the type frame type exists, frame_codec_unsubscribe shall return a non-zero value.] */
TEST_FUNCTION(frame_codec_unsubscribe_with_no_subscribe_call_has_been_made_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    uint8_t frame_type = 0;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame_type, 1);

    // act
    result = frame_codec_unsubscribe(frame_codec, 0);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_041: [If any failure occurs while performing the unsubscribe operation, frame_codec_unsubscribe shall return a non-zero value.] */
TEST_FUNCTION(when_list_remove_matching_item_fails_then_frame_codec_unsubscribe_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    uint8_t frame_type = 0;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame_type, 1)
        .SetReturn(NULL);

    // act
    result = frame_codec_unsubscribe(frame_codec, 0);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_038: [frame_codec_unsubscribe removes a previous subscription for frames of type type and on success it shall return 0.] */
TEST_FUNCTION(unsubscribe_one_of_2_subscriptions_succeeds)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    uint8_t frame_type = 0;
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    (void)frame_codec_subscribe(frame_codec, 1, on_frame_received_2, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame_type, 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));

    // act
    result = frame_codec_unsubscribe(frame_codec, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 1);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_038: [frame_codec_unsubscribe removes a previous subscription for frames of type type and on success it shall return 0.] */
TEST_FUNCTION(unsubscribe_2nd_out_of_2_subscriptions_succeeds)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    uint8_t frame_type = 1;
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    (void)frame_codec_subscribe(frame_codec, 1, on_frame_received_2, frame_codec);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame_type, 1);
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_LIST_HANDLE, IGNORED_PTR_ARG));

    // act
    result = frame_codec_unsubscribe(frame_codec, 1);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_038: [frame_codec_unsubscribe removes a previous subscription for frames of type type and on success it shall return 0.] */
TEST_FUNCTION(subscribe_unsubscribe_subscribe_succeeds)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    uint8_t frame_type = 0;
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    (void)frame_codec_unsubscribe(frame_codec, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame_type, 1);
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_LIST_HANDLE, IGNORED_PTR_ARG));

    // act
    result = frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    (void)frame_codec_unsubscribe(frame_codec, 0);
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_038: [frame_codec_unsubscribe removes a previous subscription for frames of type type and on success it shall return 0.] */
TEST_FUNCTION(subscribe_unsubscribe_unsubscribe_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    uint8_t frame_type = 0;
    (void)frame_codec_subscribe(frame_codec, 0, on_frame_received_1, frame_codec);
    (void)frame_codec_unsubscribe(frame_codec, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_LIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &frame_type, 1);

    // act
    result = frame_codec_unsubscribe(frame_codec, 0);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_016: [The type code indicates the format and purpose of the frame.] */
/* Tests_SRS_FRAME_CODEC_01_017: [The subsequent bytes in the frame header MAY be interpreted differently depending on the type of the frame.] */
/* Tests_SRS_FRAME_CODEC_01_018: [A type code of 0x00 indicates that the frame is an AMQP frame.] */
/* Tests_SRS_FRAME_CODEC_01_070: [The type code indicates the format and purpose of the frame.] */
/* Tests_SRS_FRAME_CODEC_01_071: [The subsequent bytes in the frame header MAY be interpreted differently depending on the type of the frame.] */
/* Tests_SRS_FRAME_CODEC_01_072: [A type code of 0x00 indicates that the frame is an AMQP frame.] */
TEST_FUNCTION(frame_type_amqp_is_zero)
{
    // arrange

    // act

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(uint8_t, 0, FRAME_TYPE_AMQP);
}

/* Tests_SRS_FRAME_CODEC_01_016: [The type code indicates the format and purpose of the frame.] */
/* Tests_SRS_FRAME_CODEC_01_017: [The subsequent bytes in the frame header MAY be interpreted differently depending on the type of the frame.] */
/* Tests_SRS_FRAME_CODEC_01_019: [A type code of 0x01 indicates that the frame is a SASL frame] */
/* Tests_SRS_FRAME_CODEC_01_070: [The type code indicates the format and purpose of the frame.] */
/* Tests_SRS_FRAME_CODEC_01_071: [The subsequent bytes in the frame header MAY be interpreted differently depending on the type of the frame.] */
/* Tests_SRS_FRAME_CODEC_01_073: [A type code of 0x01 indicates that the frame is a SASL frame] */
TEST_FUNCTION(frame_type_sasl_is_one)
{
    // arrange

    // act

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(uint8_t, 1, FRAME_TYPE_SASL);
}

/* frame_codec_encode_frame */

/* Tests_SRS_FRAME_CODEC_01_042: [frame_codec_encode_frame encodes the header, type specific bytes and frame payload of a frame that has frame_payload_size bytes.]*/
/* Tests_SRS_FRAME_CODEC_01_043: [On success it shall return 0.] */
/* Tests_SRS_FRAME_CODEC_01_088: [Encoded bytes shall be passed to the `on_bytes_encoded` callback in a single call, while setting the `encode complete` argument to true.] */
/* Tests_SRS_FRAME_CODEC_01_055: [Frames are divided into three distinct areas: a fixed width frame header, a variable width extended header, and a variable width frame body.] */
/* Tests_SRS_FRAME_CODEC_01_056: [frame header The frame header is a fixed size (8 byte) structure that precedes each frame.] */
/* Tests_SRS_FRAME_CODEC_01_057: [The frame header includes mandatory information necessary to parse the rest of the frame including size and type information.] */
/* Tests_SRS_FRAME_CODEC_01_058: [extended header The extended header is a variable width area preceding the frame body.] */
/* Tests_SRS_FRAME_CODEC_01_059: [This is an extension point defined for future expansion.] */
/* Tests_SRS_FRAME_CODEC_01_060: [The treatment of this area depends on the frame type.] */
/* Tests_SRS_FRAME_CODEC_01_062: [SIZE Bytes 0-3 of the frame header contain the frame size.] */
/* Tests_SRS_FRAME_CODEC_01_063: [This is an unsigned 32-bit integer that MUST contain the total frame size of the frame header, extended header, and frame body.] */
/* Tests_SRS_FRAME_CODEC_01_064: [The frame is malformed if the size is less than the size of the frame header (8 bytes).] */
TEST_FUNCTION(frame_codec_encode_frame_with_a_zero_frame_body_length_succeeds)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_bytes_encoded((void*)0x4242, IGNORED_PTR_ARG, IGNORED_NUM_ARG, true));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_encode_frame(frame_codec, 0, NULL, 0, NULL, 0, test_on_bytes_encoded, (void*)0x4242);

    // assert
    stringify_bytes(sent_io_bytes, sent_io_byte_count, actual_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, "[0x00,0x00,0x00,0x08,0x02,0x00,0x00,0x00]", actual_stringified_io);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_044: [If any of arguments `frame_codec` or `on_bytes_encoded` is NULL, `frame_codec_encode_frame` shall return a non-zero value.] */
TEST_FUNCTION(when_frame_codec_is_NULL_frame_codec_encode_frame_fails)
{
    // arrange

    // act
    int result = frame_codec_encode_frame(NULL, 0, NULL, 0, NULL, 0, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_FRAME_CODEC_01_044: [If any of arguments `frame_codec` or `on_bytes_encoded` is NULL, `frame_codec_encode_frame` shall return a non-zero value.] */
TEST_FUNCTION(when_on_bytes_encoded_is_NULL_frame_codec_encode_frame_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    umock_c_reset_all_calls();

    // act
    result = frame_codec_encode_frame(frame_codec, 0, NULL, 0, NULL, 0, NULL, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_091: [If the argument type_specific_size is greater than 0 and type_specific_bytes is NULL, frame_codec_encode_frame shall return a non-zero value.] */
TEST_FUNCTION(when_type_specific_size_is_positive_and_type_specific_bytes_is_NULL_frame_codec_encode_frame_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    umock_c_reset_all_calls();

    // act
    result = frame_codec_encode_frame(frame_codec, 0, NULL, 0, NULL, 1, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_092: [If type_specific_size is too big to allow encoding the frame according to the AMQP ISO then frame_codec_encode_frame shall return a non-zero value.] */
/* Tests_SRS_FRAME_CODEC_01_065: [DOFF Byte 4 of the frame header is the data offset.] */
/* Tests_SRS_FRAME_CODEC_01_066: [This gives the position of the body within the frame.] */
/* Tests_SRS_FRAME_CODEC_01_058: [extended header The extended header is a variable width area preceding the frame body.] */
TEST_FUNCTION(when_type_specific_size_is_too_big_then_frame_codec_encode_frame_fails)
{
    // arrange
    int result;
    unsigned char expected_frame[1020] = { 0x00, 0x00, 0x00, 0x0A, 0xFF, 0x00, 0x00, 0x00 };
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    (void)frame_codec_set_max_frame_size(frame_codec, 4096);
    umock_c_reset_all_calls();

    // act
    result = frame_codec_encode_frame(frame_codec, 0, NULL, 0, &expected_frame[6], 1015, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_092: [If type_specific_size is too big to allow encoding the frame according to the AMQP ISO then frame_codec_encode_frame shall return a non-zero value.] */
/* Tests_SRS_FRAME_CODEC_01_065: [DOFF Byte 4 of the frame header is the data offset.] */
/* Tests_SRS_FRAME_CODEC_01_066: [This gives the position of the body within the frame.] */
/* Tests_SRS_FRAME_CODEC_01_058: [extended header The extended header is a variable width area preceding the frame body.] */
/* Tests_SRS_FRAME_CODEC_01_065: [DOFF Byte 4 of the frame header is the data offset.] */
/* Tests_SRS_FRAME_CODEC_01_066: [This gives the position of the body within the frame.] */
/* Tests_SRS_FRAME_CODEC_01_067: [The value of the data offset is an unsigned, 8-bit integer specifying a count of 4-byte words.] */
/* Tests_SRS_FRAME_CODEC_01_063: [This is an unsigned 32-bit integer that MUST contain the total frame size of the frame header, extended header, and frame body.] */
TEST_FUNCTION(when_type_specific_size_is_max_allowed_then_frame_codec_encode_frame_succeeds)
{
    // arrange
    int result;
    unsigned char expected_frame[1020] = { 0x00, 0x00, 0x03, 0xFC, 0xFF, 0x00, 0x00, 0x00 };
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    (void)frame_codec_set_max_frame_size(frame_codec, 4096);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_bytes_encoded((void*)0x4242, IGNORED_PTR_ARG, IGNORED_NUM_ARG, true));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_encode_frame(frame_codec, 0, NULL, 0, &expected_frame[6], 1014, test_on_bytes_encoded, (void*)0x4242);

    // assert
    memset(expected_frame + 6, 0, 1020 - 6);
    stringify_bytes(expected_frame, sizeof(expected_frame), expected_stringified_io);
    stringify_bytes(sent_io_bytes, sent_io_byte_count, actual_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, expected_stringified_io, actual_stringified_io);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_090: [If the type_specific_size - 2 does not divide by 4, frame_codec_encode_frame shall pad the type_specific bytes with zeroes so that type specific data is according to the AMQP ISO.] */
/* Tests_SRS_FRAME_CODEC_01_065: [DOFF Byte 4 of the frame header is the data offset.] */
/* Tests_SRS_FRAME_CODEC_01_066: [This gives the position of the body within the frame.] */
/* Tests_SRS_FRAME_CODEC_01_067: [The value of the data offset is an unsigned, 8-bit integer specifying a count of 4-byte words.] */
/* Tests_SRS_FRAME_CODEC_01_068: [Due to the mandatory 8-byte frame header, the frame is malformed if the value is less than 2.] */
TEST_FUNCTION(one_byte_of_padding_is_added_to_type_specific_data_to_make_the_frame_header)
{
    // arrange
    int result;
    unsigned char expected_frame[] = { 0x00, 0x00, 0x00, 0x8, 0x02, 0x00, 0x42, 0x00 };
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_bytes_encoded((void*)0x4242, IGNORED_PTR_ARG, IGNORED_NUM_ARG, true));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_encode_frame(frame_codec, 0, NULL, 0, &expected_frame[6], 1, test_on_bytes_encoded, (void*)0x4242);

    // assert
    stringify_bytes(expected_frame, sizeof(expected_frame), expected_stringified_io);
    stringify_bytes(sent_io_bytes, sent_io_byte_count, actual_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, expected_stringified_io, actual_stringified_io);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_090: [If the type_specific_size - 2 does not divide by 4, frame_codec_encode_frame shall pad the type_specific bytes with zeroes so that type specific data is according to the AMQP ISO.] */
/* Tests_SRS_FRAME_CODEC_01_065: [DOFF Byte 4 of the frame header is the data offset.] */
/* Tests_SRS_FRAME_CODEC_01_066: [This gives the position of the body within the frame.] */
/* Tests_SRS_FRAME_CODEC_01_067: [The value of the data offset is an unsigned, 8-bit integer specifying a count of 4-byte words.] */
/* Tests_SRS_FRAME_CODEC_01_068: [Due to the mandatory 8-byte frame header, the frame is malformed if the value is less than 2.] */
TEST_FUNCTION(no_bytes_of_padding_are_added_to_type_specific_data_when_enough_bytes_are_there)
{
    // arrange
    int result;
    unsigned char expected_frame[] = { 0x00, 0x00, 0x00, 0x8, 0x02, 0x00, 0x42, 0x00 };
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_bytes_encoded((void*)0x4242, IGNORED_PTR_ARG, IGNORED_NUM_ARG, true));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_encode_frame(frame_codec, 0, NULL, 0, &expected_frame[6], 2, test_on_bytes_encoded, (void*)0x4242);

    // assert
    stringify_bytes(expected_frame, sizeof(expected_frame), expected_stringified_io);
    stringify_bytes(sent_io_bytes, sent_io_byte_count, actual_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, expected_stringified_io, actual_stringified_io);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_069: [TYPE Byte 5 of the frame header is a type code.] */
TEST_FUNCTION(the_type_is_placed_in_the_underlying_frame)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_bytes_encoded((void*)0x4242, IGNORED_PTR_ARG, IGNORED_NUM_ARG, true));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_encode_frame(frame_codec, 0x42, NULL, 0, NULL, 0, test_on_bytes_encoded, (void*)0x4242);

    // assert
    stringify_bytes(sent_io_bytes, sent_io_byte_count, actual_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, "[0x00,0x00,0x00,0x08,0x02,0x42,0x00,0x00]", actual_stringified_io);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_105: [The frame_payload_size shall be computed by summing up the lengths of the payload segments identified by the payloads argument.]*/
/* ----------- Tests_SRS_FRAME_CODEC_01_048: [If all bytes are successfully encoded, frame_codec_encode_frame_bytes shall return 0.] */
/* Tests_SRS_FRAME_CODEC_01_061: [frame body The frame body is a variable width sequence of bytes the format of which depends on the frame type.] */
TEST_FUNCTION(frame_codec_encode_frame_bytes_with_1_encoded_byte_succeeds)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    PAYLOAD payloads[1];
    uint8_t byte = 0x42;
    umock_c_reset_all_calls();
    payloads[0].bytes = &byte;
    payloads[0].length = 1;

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_bytes_encoded((void*)0x4242, IGNORED_PTR_ARG, IGNORED_NUM_ARG, true));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_encode_frame(frame_codec, 0x42, payloads, 1, NULL, 0, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(sent_io_bytes, sent_io_byte_count, actual_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, "[0x00,0x00,0x00,0x09,0x02,0x42,0x00,0x00,0x42]", actual_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* ----------- Tests_SRS_FRAME_CODEC_01_047: [frame_codec_encode_frame_bytes encodes the frame bytes for a frame encoding started with a frame_codec_start_encode_frame call.] */
/* ----------- Tests_SRS_FRAME_CODEC_01_048: [If all bytes are successfully encoded, frame_codec_encode_frame_bytes shall return 0.] */
TEST_FUNCTION(frame_codec_encode_frame_bytes_with_2_bytes_succeeds)
{
    // arrange
    int result;
    unsigned char bytes[] = { 0x42, 0x43 };
    PAYLOAD payloads[1];
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    payloads[0].bytes = bytes;
    payloads[0].length = sizeof(bytes);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_bytes_encoded((void*)0x4242, IGNORED_PTR_ARG, IGNORED_NUM_ARG, true));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_encode_frame(frame_codec, 0x42, payloads, 1, NULL, 0, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_110: [ If the `bytes` member of a payload entry is NULL, `frame_codec_encode_frame` shall fail and return a non-zero value. ] */
TEST_FUNCTION(frame_codec_encode_frame_bytes_with_NULL_bytes_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    PAYLOAD payloads[1];
    payloads[0].bytes = NULL;
    payloads[0].length = 1;
    umock_c_reset_all_calls();

    // act
    result = frame_codec_encode_frame(frame_codec, 0x42, payloads, 1, NULL, 0, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_111: [ If the `length` member of a payload entry is 0, `frame_codec_encode_frame` shall fail and return a non-zero value. ] */
TEST_FUNCTION(frame_codec_encode_frame_bytes_with_zero_length_fails)
{
    // arrange
    int result;
    unsigned char bytes[] = { 0x42, 0x43 };
    PAYLOAD payloads[1];
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    payloads[0].bytes = bytes;
    payloads[0].length = 0;
    umock_c_reset_all_calls();

    // act
    result = frame_codec_encode_frame(frame_codec, 0x42, payloads, 1, NULL, 0, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_105: [The frame_payload_size shall be computed by summing up the lengths of the payload segments identified by the payloads argument.] */
/* Tests_SRS_FRAME_CODEC_01_106: [All payloads shall be encoded in order as part of the frame.] */
TEST_FUNCTION(sending_only_1_byte_out_of_2_frame_body_bytes_succeeds)
{
    // arrange
    int result;
    unsigned char bytes[] = { 0x42, 0x43 };
    PAYLOAD payloads[2];
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    payloads[0].bytes = bytes;
    payloads[0].length = 1;
    payloads[1].bytes = bytes + 1;
    payloads[1].length = 1;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_bytes_encoded((void*)0x4242, IGNORED_PTR_ARG, IGNORED_NUM_ARG, true));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_encode_frame(frame_codec, 0x42, payloads, 2, NULL, 0, test_on_bytes_encoded, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(sent_io_bytes, sent_io_byte_count, actual_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, "[0x00,0x00,0x00,0x0A,0x02,0x42,0x00,0x00,0x42,0x43]", actual_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_105: [The frame_payload_size shall be computed by summing up the lengths of the payload segments identified by the payloads argument.] */
/* Tests_SRS_FRAME_CODEC_01_106: [All payloads shall be encoded in order as part of the frame.] */
TEST_FUNCTION(a_send_after_send_succeeds)
{
    // arrange
    int result;
    unsigned char bytes[] = { 0x42, 0x43 };
    PAYLOAD payloads[1];
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    payloads[0].bytes = bytes;
    payloads[0].length = 2;
    (void)frame_codec_encode_frame(frame_codec, 0x42, payloads, 1, NULL, 0, test_on_bytes_encoded, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_bytes_encoded((void*)0x4242, IGNORED_PTR_ARG, IGNORED_NUM_ARG, true));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = frame_codec_encode_frame(frame_codec, 0x42, payloads, 1, NULL, 0, test_on_bytes_encoded, (void*)0x4242);

    // assert
    stringify_bytes(sent_io_bytes, sent_io_byte_count, actual_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, "[0x00,0x00,0x00,0x0A,0x02,0x42,0x00,0x00,0x42,0x43,0x00,0x00,0x00,0x0A,0x02,0x42,0x00,0x00,0x42,0x43]", actual_stringified_io);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_109: [ If allocating memory fails, `frame_codec_encode_frame` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_encoded_frame_fails_frame_codec_encode_frame_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    result = frame_codec_encode_frame(frame_codec, 0, NULL, 0, NULL, 0, test_on_bytes_encoded, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

/* Tests_SRS_FRAME_CODEC_01_107: [If the argument `payloads` is NULL and `payload_count` is non-zero, `frame_codec_encode_frame` shall return a non-zero value.]*/
TEST_FUNCTION(frame_codec_encode_frame_with_NULL_payloads_and_non_zero_payload_count_fails)
{
    // arrange
    int result;
    FRAME_CODEC_HANDLE frame_codec = frame_codec_create(test_frame_codec_decode_error, TEST_ERROR_CONTEXT);
    umock_c_reset_all_calls();

    // act
    result = frame_codec_encode_frame(frame_codec, 0, NULL, 1, NULL, 0, test_on_bytes_encoded, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    frame_codec_destroy(frame_codec);
}

END_TEST_SUITE(frame_codec_ut)
