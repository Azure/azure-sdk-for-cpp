// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstdint>
#else
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"

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
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_uamqp_c/frame_codec.h"
#include "azure_uamqp_c/amqp_frame_codec.h"
#include "azure_uamqp_c/amqpvalue_to_string.h"
#include "azure_uamqp_c/amqp_definitions.h"

#undef ENABLE_MOCKS

#include "azure_uamqp_c/connection.h"

/* Requirements implicitly tested */
/* Tests_S_R_S_CONNECTION_01_088: [Any data appearing beyond the protocol header MUST match the version indicated by the protocol header.] */
/* Tests_S_R_S_CONNECTION_01_039: [START In this state a connection exists, but nothing has been sent or received. This is the state an implementation would be in immediately after performing a socket connect or socket accept.] */
/* Tests_S_R_S_CONNECTION_01_015: [Implementations SHOULD NOT expect to be able to reuse open TCP sockets after close performatives have been exchanged.] */

/* Requirements enforced by design */
/* Tests_S_R_S_CONNECTION_01_225: [HDR_RCVD HDR OPEN] */
/* Tests_S_R_S_CONNECTION_01_224: [START HDR HDR] */
/* Tests_S_R_S_CONNECTION_01_227: [HDR_EXCH OPEN OPEN] */
/* Tests_S_R_S_CONNECTION_01_228: [OPEN_RCVD OPEN *] */
/* Tests_S_R_S_CONNECTION_01_235: [CLOSE_SENT - * TCP Close for Write] */
/* Tests_S_R_S_CONNECTION_01_234: [CLOSE_RCVD * -TCP Close for Read] */

#define TEST_IO_HANDLE                      (XIO_HANDLE)0x4242
#define TEST_FRAME_CODEC_HANDLE             (FRAME_CODEC_HANDLE)0x4243
#define TEST_AMQP_FRAME_CODEC_HANDLE        (AMQP_FRAME_CODEC_HANDLE)0x4244
#define TEST_DESCRIPTOR_AMQP_VALUE          (AMQP_VALUE)0x4245
#define TEST_LIST_ITEM_AMQP_VALUE           (AMQP_VALUE)0x4246
#define TEST_DESCRIBED_AMQP_VALUE           (AMQP_VALUE)0x4247
#define TEST_AMQP_OPEN_FRAME_HANDLE         (AMQP_OPEN_FRAME_HANDLE)0x4245
#define TEST_LIST_HANDLE                    (SINGLYLINKEDLIST_HANDLE)0x4246
#define TEST_OPEN_PERFORMATIVE              (AMQP_VALUE)0x4301
#define TEST_CLOSE_PERFORMATIVE             (AMQP_VALUE)0x4302
#define TEST_CLOSE_DESCRIPTOR_AMQP_VALUE    (AMQP_VALUE)0x4303
#define TEST_TRANSFER_PERFORMATIVE          (AMQP_VALUE)0x4304
#define TEST_PROPERTIES                     (fields)0x4255
#define TEST_CLONED_PROPERTIES              (fields)0x4256

#define TEST_CONTEXT                        (void*)(0x4242)

static const TICK_COUNTER_HANDLE test_tick_counter = (TICK_COUNTER_HANDLE)0x4305;
static const char test_container_id[] = "1234";

static const IO_INTERFACE_DESCRIPTION test_io_interface_description = { 0 };

static ON_BYTES_RECEIVED saved_on_bytes_received;
static void* saved_on_bytes_received_context;
static ON_IO_OPEN_COMPLETE saved_on_io_open_complete;
static void* saved_on_io_open_complete_context;
static ON_IO_ERROR saved_on_io_error;
static void* saved_on_io_error_context;
static uint64_t performative_ulong;
static const void** list_items = NULL;
static size_t list_item_count = 0;
static unsigned char* frame_codec_bytes = NULL;
static size_t frame_codec_byte_count = 0;
static AMQP_FRAME_RECEIVED_CALLBACK saved_frame_received_callback;
static AMQP_EMPTY_FRAME_RECEIVED_CALLBACK saved_empty_frame_received_callback;
static AMQP_FRAME_CODEC_ERROR_CALLBACK saved_amqp_frame_codec_error_callback;
static void* saved_amqp_frame_codec_callback_context;
static void* saved_on_connection_state_changed_context;
static CONNECTION_STATE saved_new_connection_state;
CONNECTION_STATE saved_previous_connection_state;

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

static char expected_stringified_io[8192];
static char actual_stringified_io[8192];

/* frame received callback */
MOCK_FUNCTION_WITH_CODE(, void, test_on_frame_received, void*, context, AMQP_VALUE, performative, uint32_t, frame_payload_size, const unsigned char*, payload_bytes)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_on_connection_state_changed, void*, context, CONNECTION_STATE, new_connection_state, CONNECTION_STATE, previous_connection_state)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_on_connection_close_received, void*, context, ERROR_HANDLE, error)
MOCK_FUNCTION_END();

static int my_xio_open(XIO_HANDLE io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    (void)io;
    saved_on_bytes_received = on_bytes_received;
    saved_on_bytes_received_context = on_bytes_received_context;
    saved_on_io_open_complete = on_io_open_complete;
    saved_on_io_open_complete_context = on_io_open_complete_context;
    saved_on_io_error = on_io_error;
    saved_on_io_error_context = on_io_error_context;

    return 0;
}

static int my_frame_codec_receive_bytes(FRAME_CODEC_HANDLE frame_codec, const unsigned char* buffer, size_t size)
{
    unsigned char* new_frame_codec_bytes = (unsigned char*)my_gballoc_realloc(frame_codec_bytes, frame_codec_byte_count + size);
    (void)frame_codec;
    if (new_frame_codec_bytes != NULL)
    {
        frame_codec_bytes = new_frame_codec_bytes;
        (void)memcpy(frame_codec_bytes + frame_codec_byte_count, buffer, size);
        frame_codec_byte_count += size;
    }
    return 0;
}

/* amqp_frame_codec */
static AMQP_FRAME_CODEC_HANDLE my_amqp_frame_codec_create(FRAME_CODEC_HANDLE frame_codec, AMQP_FRAME_RECEIVED_CALLBACK frame_received_callback, AMQP_EMPTY_FRAME_RECEIVED_CALLBACK empty_frame_received_callback, AMQP_FRAME_CODEC_ERROR_CALLBACK amqp_frame_codec_error_callback, void* callback_context)
{
    (void)frame_codec;
    saved_frame_received_callback = frame_received_callback;
    saved_empty_frame_received_callback = empty_frame_received_callback;
    saved_amqp_frame_codec_error_callback = amqp_frame_codec_error_callback;
    saved_amqp_frame_codec_callback_context = callback_context;
    return TEST_AMQP_FRAME_CODEC_HANDLE;
}

static int my_amqpvalue_get_ulong(AMQP_VALUE value, uint64_t* ulong_value)
{
    (void)value;
    *ulong_value = performative_ulong;
    return 0;
}

static LIST_ITEM_HANDLE my_singlylinkedlist_add(SINGLYLINKEDLIST_HANDLE list, const void* item)
{
    const void** items = (const void**)my_gballoc_realloc((void*)list_items, (list_item_count + 1) * sizeof(item));
    (void)list;
    if (items != NULL)
    {
        list_items = items;
        list_items[list_item_count++] = item;
    }
    return (LIST_ITEM_HANDLE)list_item_count;
}

static LIST_ITEM_HANDLE my_singlylinkedlist_find(SINGLYLINKEDLIST_HANDLE handle, LIST_MATCH_FUNCTION match_function, const void* match_context)
{
    size_t i;
    const void* found_item = NULL;
    (void)handle;
    for (i = 0; i < list_item_count; i++)
    {
        if (match_function((LIST_ITEM_HANDLE)list_items[i], match_context))
        {
            found_item = list_items[i];
            break;
        }
    }
    return (LIST_ITEM_HANDLE)found_item;
}

const void* my_singlylinkedlist_item_get_value(LIST_ITEM_HANDLE item_handle)
{
    return (const void*)item_handle;
}

static void test_on_send_complete(void* context, IO_SEND_RESULT io_send_result)
{
    (void)context;
    (void)io_send_result;
}

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(connection_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, my_gballoc_calloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_RETURN(xio_create, TEST_IO_HANDLE);
    REGISTER_GLOBAL_MOCK_HOOK(xio_open, my_xio_open);
    REGISTER_GLOBAL_MOCK_RETURN(xio_close, 0);
    REGISTER_GLOBAL_MOCK_RETURN(xio_send, 0);
    REGISTER_GLOBAL_MOCK_HOOK(frame_codec_receive_bytes, my_frame_codec_receive_bytes);
    REGISTER_GLOBAL_MOCK_RETURN(frame_codec_create, TEST_FRAME_CODEC_HANDLE);
    REGISTER_GLOBAL_MOCK_RETURN(frame_codec_set_max_frame_size, 0);
    REGISTER_GLOBAL_MOCK_HOOK(amqp_frame_codec_create, my_amqp_frame_codec_create);
    REGISTER_GLOBAL_MOCK_RETURN(amqp_frame_codec_encode_frame, 0);
    REGISTER_GLOBAL_MOCK_RETURN(amqp_frame_codec_encode_empty_frame, 0);
    REGISTER_GLOBAL_MOCK_HOOK(amqpvalue_get_ulong, my_amqpvalue_get_ulong);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_get_inplace_descriptor, TEST_DESCRIPTOR_AMQP_VALUE);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_get_string, 0);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_get_list_item, TEST_LIST_ITEM_AMQP_VALUE);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_get_inplace_described_value, TEST_DESCRIBED_AMQP_VALUE);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_to_string, NULL);
    REGISTER_GLOBAL_MOCK_RETURN(singlylinkedlist_create, TEST_LIST_HANDLE);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_add, my_singlylinkedlist_add);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_find, my_singlylinkedlist_find);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_item_get_value, my_singlylinkedlist_item_get_value);
    REGISTER_GLOBAL_MOCK_RETURN(tickcounter_create, test_tick_counter);
    REGISTER_GLOBAL_MOCK_RETURN(tickcounter_get_current_ms, 0);
    REGISTER_GLOBAL_MOCK_RETURN(fields_clone, TEST_CLONED_PROPERTIES);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_clone, TEST_CLONED_PROPERTIES);

    REGISTER_UMOCK_ALIAS_TYPE(CONNECTION_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_FRAME_CODEC_ERROR, void*);
    REGISTER_UMOCK_ALIAS_TYPE(FRAME_CODEC_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(AMQP_FRAME_RECEIVED_CALLBACK, void*);
    REGISTER_UMOCK_ALIAS_TYPE(AMQP_EMPTY_FRAME_RECEIVED_CALLBACK, void*);
    REGISTER_UMOCK_ALIAS_TYPE(AMQP_FRAME_CODEC_ERROR_CALLBACK, void*);
    REGISTER_UMOCK_ALIAS_TYPE(TICK_COUNTER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(AMQP_FRAME_CODEC_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(AMQP_VALUE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(XIO_HANDLE, void*);
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

    frame_codec_bytes = NULL;
    frame_codec_byte_count = 0;
    performative_ulong = 0x10;
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    free(frame_codec_bytes);

    TEST_MUTEX_RELEASE(g_testByTest);
}

/* connection_create */

#if 0
/* Tests_S_R_S_CONNECTION_01_001: [connection_create shall open a new connection to a specified host/port.] */
/* Tests_S_R_S_CONNECTION_01_082: [connection_create shall allocate a new frame_codec instance to be used for frame encoding/decoding.] */
/* Tests_S_R_S_CONNECTION_01_107: [connection_create shall create an amqp_frame_codec instance by calling amqp_frame_codec_create.] */
/* Tests_S_R_S_CONNECTION_01_072: [When connection_create succeeds, the state of the connection shall be CONNECTION_STATE_START.] */
TEST_FUNCTION(connection_create_with_valid_args_succeeds)
{
    // arrange
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(frame_codec_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(tickcounter_create());
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    // act
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);

    // assert
    ASSERT_IS_NOT_NULL(connection);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_001: [connection_create shall open a new connection to a specified host/port.] */
/* Tests_S_R_S_CONNECTION_01_082: [connection_create shall allocate a new frame_codec instance to be used for frame encoding/decoding.] */
/* Tests_S_R_S_CONNECTION_01_107: [connection_create shall create an amqp_frame_codec instance by calling amqp_frame_codec_create.] */
/* Tests_S_R_S_CONNECTION_01_072: [When connection_create succeeds, the state of the connection shall be CONNECTION_STATE_START.] */
TEST_FUNCTION(connection_create_with_valid_args_but_NULL_host_name_succeeds)
{
    // arrange
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(frame_codec_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    // act
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, test_container_id);

    // assert
    ASSERT_IS_NOT_NULL(connection);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_081: [If allocating the memory for the connection fails then connection_create shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_fails_then_connection_create_fails)
{
    // arrange
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);

    // act
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(connection);
}

/* Tests_S_R_S_CONNECTION_01_083: [If frame_codec_create fails then connection_create shall return NULL.] */
TEST_FUNCTION(when_frame_codec_create_fails_then_connection_create_fails)
{
    // arrange
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(frame_codec_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn((FRAME_CODEC_HANDLE)NULL);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(connection);
}

/* Tests_S_R_S_CONNECTION_01_108: [If amqp_frame_codec_create fails, connection_create shall return NULL.] */
TEST_FUNCTION(when_amqp_frame_codec_create_fails_then_connection_create_fails)
{
    // arrange
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(frame_codec_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn((AMQP_FRAME_CODEC_HANDLE)NULL);
    STRICT_EXPECTED_CALL(frame_codec_destroy(TEST_FRAME_CODEC_HANDLE));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(connection);
}

/* Tests_S_R_S_CONNECTION_01_081: [If allocating the memory for the connection fails then connection_create shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_hostname_fails_connection_create_fails)
{
    // arrange
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(frame_codec_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);
    STRICT_EXPECTED_CALL(amqp_frame_codec_destroy(TEST_AMQP_FRAME_CODEC_HANDLE));
    STRICT_EXPECTED_CALL(frame_codec_destroy(TEST_FRAME_CODEC_HANDLE));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(connection);
}

/* Tests_S_R_S_CONNECTION_01_081: [If allocating the memory for the connection fails then connection_create shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_container_id_fails_connection_create_fails)
{
    // arrange
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(frame_codec_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqp_frame_codec_destroy(TEST_AMQP_FRAME_CODEC_HANDLE));
    STRICT_EXPECTED_CALL(frame_codec_destroy(TEST_FRAME_CODEC_HANDLE));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(connection);
}

/* Tests_S_R_S_CONNECTION_01_071: [If xio or container_id is NULL, connection_create shall return NULL.] */
TEST_FUNCTION(connection_create_with_NULL_io_fails)
{
    // arrange

    // act
    CONNECTION_HANDLE connection = connection_create(NULL, "testhost", test_container_id);

    // assert
    ASSERT_IS_NULL(connection);
}

/* Tests_S_R_S_CONNECTION_01_071: [If xio or container_id is NULL, connection_create shall return NULL.] */
TEST_FUNCTION(connection_create_with_NULL_container_id_fails)
{
    // arrange

    // act
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(connection);
}

/* connection_destroy */

/* Tests_S_R_S_CONNECTION_01_073: [connection_destroy shall free all resources associated with a connection.] */
/* Tests_S_R_S_CONNECTION_01_074: [connection_destroy shall close the socket connection.] */
TEST_FUNCTION(connection_destroy_frees_resources)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_frame_codec_destroy(TEST_AMQP_FRAME_CODEC_HANDLE));
    STRICT_EXPECTED_CALL(frame_codec_destroy(TEST_FRAME_CODEC_HANDLE));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    connection_destroy(connection);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_S_R_S_CONNECTION_01_079: [If handle is NULL, connection_destroy shall do nothing.] */
TEST_FUNCTION(connection_destroy_with_NULL_handle_does_nothing)
{
    // arrange

    // act
    connection_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* connection_set_max_frame_size */

/* Tests_S_R_S_CONNECTION_01_163: [If connection is NULL, connection_set_max_frame_size shall fail and return a non-zero value.] */
TEST_FUNCTION(connection_set_max_frame_size_with_NULL_connection_fails)
{
    // arrange

    // act
    int result = connection_set_max_frame_size(NULL, 512);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_S_R_S_CONNECTION_01_148: [connection_set_max_frame_size shall set the max_frame_size associated with a connection.] */
/* Tests_S_R_S_CONNECTION_01_149: [On success connection_set_max_frame_size shall return 0.] */
TEST_FUNCTION(connection_set_max_frame_size_with_valid_connection_succeeds)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    umock_c_reset_all_calls();

    // act
    int result = connection_set_max_frame_size(connection, 512);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_150: [If the max_frame_size is invalid then connection_set_max_frame_size shall fail and return a non-zero value.] */
/* Tests_S_R_S_CONNECTION_01_167: [Both peers MUST accept frames of up to 512 (MIN-MAX-FRAME-SIZE) octets.] */
TEST_FUNCTION(connection_set_max_frame_size_with_511_bytes_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    umock_c_reset_all_calls();

    // act
    int result = connection_set_max_frame_size(connection, 511);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_164: [If connection_set_max_frame_size fails, the previous max_frame_size setting shall be retained.] */
/* Tests_S_R_S_CONNECTION_01_167: [Both peers MUST accept frames of up to 512 (MIN-MAX-FRAME-SIZE) octets.] */
TEST_FUNCTION(connection_set_max_frame_size_with_511_bytes_fails_and_previous_value_is_kept)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    (void)connection_set_max_frame_size(connection, 1042);
    umock_c_reset_all_calls();

    // act
    int result = connection_set_max_frame_size(connection, 511);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    uint32_t max_frame_size;
    (void)connection_get_max_frame_size(connection, &max_frame_size);
    ASSERT_ARE_EQUAL(uint32_t, 1042, max_frame_size);

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_157: [If connection_set_max_frame_size is called after the initial Open frame has been sent, it shall fail and return a non-zero value.] */
TEST_FUNCTION(set_max_frame_size_after_open_is_sent_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    umock_c_reset_all_calls();

    // act
    int result = connection_set_max_frame_size(connection, 1024);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* connection_get_max_frame_size */

/* Tests_S_R_S_CONNECTION_01_170: [If connection or max_frame_size is NULL, connection_get_max_frame_size shall fail and return a non-zero value.] */
TEST_FUNCTION(connection_get_max_frame_size_with_NULL_connection_fails)
{
    // arrange
    uint32_t max_frame_size;

    // act
    int result = connection_get_max_frame_size(NULL, &max_frame_size);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_S_R_S_CONNECTION_01_170: [If connection or max_frame_size is NULL, connection_get_max_frame_size shall fail and return a non-zero value.] */
TEST_FUNCTION(connection_get_max_frame_size_with_NULL_max_frame_size_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    umock_c_reset_all_calls();

    // act
    int result = connection_get_max_frame_size(connection, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_168: [connection_get_max_frame_size shall return in the max_frame_size argument the current max frame size setting.] */
/* Tests_S_R_S_CONNECTION_01_169: [On success, connection_get_max_frame_size shall return 0.] */
/* Tests_S_R_S_CONNECTION_01_173: [<field name="max-frame-size" type="uint" default="4294967295"/>] */
TEST_FUNCTION(connection_get_max_frame_size_with_valid_arguments_succeeds)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    umock_c_reset_all_calls();
    uint32_t max_frame_size;

    // act
    int result = connection_get_max_frame_size(connection, &max_frame_size);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(uint32_t, 4294967295, max_frame_size);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* connection_set_channel_max */

/* Tests_S_R_S_CONNECTION_01_181: [If connection is NULL then connection_set_channel_max shall fail and return a non-zero value.] */
TEST_FUNCTION(connection_set_channel_max_with_NULL_connection_fails)
{
    // arrange

    // act
    int result = connection_set_channel_max(NULL, 10);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_S_R_S_CONNECTION_01_153: [connection_set_channel_max shall set the channel_max associated with a connection.] */
/* Tests_S_R_S_CONNECTION_01_154: [On success connection_set_channel_max shall return 0.] */
TEST_FUNCTION(connection_set_channel_max_with_valid_connection_succeeds)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    umock_c_reset_all_calls();

    // act
    int result = connection_set_channel_max(connection, 10);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_156: [If connection_set_channel_max is called after the initial Open frame has been sent, it shall fail and return a non-zero value.] */
TEST_FUNCTION(set_channel_max_after_open_is_sent_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    umock_c_reset_all_calls();

    // act
    int result = connection_set_channel_max(connection, 1024);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* connection_get_channel_max */

/* Tests_S_R_S_CONNECTION_01_184: [If connection or channel_max is NULL, connection_get_channel_max shall fail and return a non-zero value.] */
TEST_FUNCTION(connection_get_channel_max_with_NULL_connection_fails)
{
    // arrange
    uint16_t channel_max;

    // act
    int result = connection_get_channel_max(NULL, &channel_max);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_S_R_S_CONNECTION_01_184: [If connection or channel_max is NULL, connection_get_channel_max shall fail and return a non-zero value.] */
TEST_FUNCTION(connection_get_channel_max_with_NULL_channel_max_argument_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    umock_c_reset_all_calls();

    // act
    int result = connection_get_channel_max(connection, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_182: [connection_get_channel_max shall return in the channel_max argument the current channel_max setting.] */
/* Tests_S_R_S_CONNECTION_01_183: [On success, connection_get_channel_max shall return 0.] */
TEST_FUNCTION(connection_get_channel_max_with_valid_argument_succeeds)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    (void)connection_set_channel_max(connection, 12);
    umock_c_reset_all_calls();
    uint16_t channel_max;

    // act
    int result = connection_get_channel_max(connection, &channel_max);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(uint32_t, 12, (uint32_t)channel_max);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_182: [connection_get_channel_max shall return in the channel_max argument the current channel_max setting.] */
/* Tests_S_R_S_CONNECTION_01_183: [On success, connection_get_channel_max shall return 0.] */
/* Tests_S_R_S_CONNECTION_01_174: [<field name="channel-max" type="ushort" default="65535"/>] */
TEST_FUNCTION(connection_get_channel_max_default_value_succeeds)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    umock_c_reset_all_calls();
    uint16_t channel_max;

    // act
    int result = connection_get_channel_max(connection, &channel_max);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(uint32_t, 65535, (uint32_t)channel_max);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* connection_set_idle_timeout */

/* Tests_S_R_S_CONNECTION_01_191: [If connection is NULL, connection_set_idle_timeout shall fail and return a non-zero value.] */
TEST_FUNCTION(connection_set_idle_timeout_with_NULL_connection_fails)
{
    // arrange

    // act
    int result = connection_set_idle_timeout(NULL, 1000);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_S_R_S_CONNECTION_01_159: [connection_set_idle_timeout shall set the idle_timeout associated with a connection.] */
/* Tests_S_R_S_CONNECTION_01_160: [On success connection_set_idle_timeout shall return 0.] */
TEST_FUNCTION(connection_set_idle_timeout_with_valid_connection_succeeds)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    umock_c_reset_all_calls();

    // act
    int result = connection_set_idle_timeout(connection, 1000);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_158: [If connection_set_idle_timeout is called after the initial Open frame has been sent, it shall fail and return a non-zero value.] */
TEST_FUNCTION(set_idle_timeout_after_open_is_sent_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    umock_c_reset_all_calls();

    // act
    int result = connection_set_idle_timeout(connection, 1000);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* connection_get_idle_timeout */

/* Tests_S_R_S_CONNECTION_01_190: [If connection or idle_timeout is NULL, connection_get_idle_timeout shall fail and return a non-zero value.]  */
TEST_FUNCTION(connection_get_idle_timeout_with_NULL_connection_fails)
{
    // arrange
    milliseconds idle_timeout;

    // act
    int result = connection_get_idle_timeout(NULL, &idle_timeout);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_S_R_S_CONNECTION_01_190: [If connection or idle_timeout is NULL, connection_get_idle_timeout shall fail and return a non-zero value.]  */
TEST_FUNCTION(connection_get_idle_timeout_with_NULL_idle_timeout_argument_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    umock_c_reset_all_calls();

    // act
    int result = connection_get_idle_timeout(connection, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_188: [connection_get_idle_timeout shall return in the idle_timeout argument the current idle_timeout setting.] */
/* Tests_S_R_S_CONNECTION_01_189: [On success, connection_get_idle_timeout shall return 0.] */
TEST_FUNCTION(connection_get_idle_timeout_with_valid_argument_succeeds)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    (void)connection_set_idle_timeout(connection, 12);
    umock_c_reset_all_calls();
    milliseconds idle_timeout;

    // act
    int result = connection_get_idle_timeout(connection, &idle_timeout);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(uint32_t, 12, (uint32_t)idle_timeout);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_188: [connection_get_idle_timeout shall return in the idle_timeout argument the current idle_timeout setting.] */
/* Tests_S_R_S_CONNECTION_01_189: [On success, connection_get_idle_timeout shall return 0.] */
/* Tests_S_R_S_CONNECTION_01_175: [<field name="idle-time-out" type="milliseconds"/>] */
/* Tests_S_R_S_CONNECTION_01_192: [A value of zero is the same as if it was not set (null).] */
TEST_FUNCTION(connection_get_idle_timeout_default_value_succeeds)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    umock_c_reset_all_calls();
    milliseconds idle_timeout;

    // act
    int result = connection_get_idle_timeout(connection, &idle_timeout);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(uint32_t, 0, (uint32_t)idle_timeout);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* connection_dowork */

/* Tests_S_R_S_CONNECTION_01_078: [If handle is NULL, connection_dowork shall do nothing.] */
TEST_FUNCTION(connection_dowork_with_NULL_handle_does_nothing)
{
    // arrange

    // act
    connection_dowork(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_: [If the io has not been opened before, connection_dowork shall attempt to open the io by calling xio_open.] */
TEST_FUNCTION(when_io_state_is_not_open_connection_dowork_opens_the_io)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_dowork(TEST_IO_HANDLE));

    // act
    connection_dowork(connection);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_204: [If io_open_fails, no more work shall be done by connection_dowork and the connection shall be considered in the END state.] */
TEST_FUNCTION(when_io_open_fails_the_connection_state_shall_be_set_to_END)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(xio_dowork(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(test_on_connection_state_changed(TEST_CONTEXT, CONNECTION_STATE_END, CONNECTION_STATE_START));

    // act
    connection_dowork(connection);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_076: [connection_dowork shall schedule the underlying IO interface to do its work by calling xio_dowork.] */
/* Tests_S_R_S_CONNECTION_01_084: [The connection state machine implementing the protocol requirements shall be run as part of connection_dowork.] */
/* Tests_S_R_S_CONNECTION_01_086: [Prior to sending any frames on a connection, each peer MUST start by sending a protocol header that indicates the protocol version used on the connection.] */
/* Tests_S_R_S_CONNECTION_01_087: [The protocol header consists of the upper case ASCII letters "AMQP" followed by a protocol id of zero, followed by three unsigned bytes representing the major, minor, and revision of the protocol version (currently 1 (MAJOR), 0 (MINOR), 0 (REVISION)). In total this is an 8-octet sequence] */
/* Tests_S_R_S_CONNECTION_01_091: [The AMQP peer which acted in the role of the TCP client (i.e. the peer that actively opened the connection) MUST immediately send its outgoing protocol header on establishment of the TCP connection.] */
/* Tests_S_R_S_CONNECTION_01_093: [_ When the client opens a new socket connection to a server, it MUST send a protocol header with the client's preferred protocol version.] */
/* Tests_S_R_S_CONNECTION_01_104: [Sending the protocol header shall be done by using xio_send.] */
/* Tests_S_R_S_CONNECTION_01_041: [HDR SENT In this state the connection header has been sent to the peer but no connection header has been received.] */
TEST_FUNCTION(connection_dowork_when_state_is_start_sends_the_AMQP_header_and_triggers_io_dowork)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    umock_c_reset_all_calls();
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, amqp_header, sizeof(amqp_header), IGNORED_PTR_ARG, NULL))
        .ValidateArgumentBuffer(2, amqp_header, sizeof(amqp_header));
    STRICT_EXPECTED_CALL(xio_dowork(TEST_IO_HANDLE));

    connection_dowork(connection);

    // act
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_202: [If the io notifies the connection instance of an IO_STATE_ERROR state the connection shall be closed and the state set to END.] */
TEST_FUNCTION(when_io_state_changes_to_ERROR_the_io_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    connection_dowork(connection);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(test_on_connection_state_changed(TEST_CONTEXT, CONNECTION_STATE_END, CONNECTION_STATE_START));

    // act
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_ERROR, IO_STATE_NOT_OPEN);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_057: [END In this state it is illegal for either endpoint to write anything more onto the connection. The connection can be safely closed and discarded.] */
/* Tests_S_R_S_CONNECTION_01_106: [When sending the protocol header fails, the connection shall be immediately closed.] */
TEST_FUNCTION(when_sending_the_header_fails_the_io_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", "1234");
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    connection_dowork(connection);
    umock_c_reset_all_calls();
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };

    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, amqp_header, sizeof(amqp_header), IGNORED_PTR_ARG, NULL))
        .ValidateArgumentBuffer(2, amqp_header, sizeof(amqp_header))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(test_on_connection_state_changed(TEST_CONTEXT, CONNECTION_STATE_END, CONNECTION_STATE_START));

    // act
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_089: [If the incoming and outgoing protocol headers do not match, both peers MUST close their outgoing stream] */
TEST_FUNCTION(when_protocol_headers_do_not_match_connection_gets_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", "1234");
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'Q', 0, 1, 0, 0 };

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(test_on_connection_state_changed(TEST_CONTEXT, CONNECTION_STATE_END, CONNECTION_STATE_HDR_SENT));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_089: [If the incoming and outgoing protocol headers do not match, both peers MUST close their outgoing stream] */
TEST_FUNCTION(when_protocol_header_first_byte_does_not_match_connection_gets_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", "1234");
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();
    const unsigned char amqp_header[] = { 'B' };

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(test_on_connection_state_changed(TEST_CONTEXT, CONNECTION_STATE_END, CONNECTION_STATE_HDR_SENT));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_089: [If the incoming and outgoing protocol headers do not match, both peers MUST close their outgoing stream] */
TEST_FUNCTION(when_protocol_header_last_byte_does_not_match_connection_gets_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", "1234");
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 1 };

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(test_on_connection_state_changed(TEST_CONTEXT, CONNECTION_STATE_END, CONNECTION_STATE_HDR_SENT));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_089: [If the incoming and outgoing protocol headers do not match, both peers MUST close their outgoing stream] */
TEST_FUNCTION(when_protocol_header_first_byte_matches_but_only_1st_byte_received_no_io_close_is_done)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();
    const unsigned char amqp_header[] = { 'A' };

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_134: [The container id field shall be filled with the container id specified in connection_create.] */
/* Tests_S_R_S_CONNECTION_01_135: [If hostname has been specified by a call to connection_set_hostname, then that value shall be stamped in the open frame.] */
/* Tests_S_R_S_CONNECTION_01_205: [Sending the AMQP OPEN frame shall be done by calling amqp_frame_codec_begin_encode_frame with channel number 0, the actual performative payload and 0 as payload_size.] */
/* Tests_S_R_S_CONNECTION_01_151: [The connection max_frame_size setting shall be passed down to the frame_codec when the Open frame is sent.] */
/* Tests_S_R_S_CONNECTION_01_137: [The max_frame_size connection setting shall be set in the open frame by using open_set_max_frame_size.] */
/* Tests_S_R_S_CONNECTION_01_139: [The channel_max connection setting shall be set in the open frame by using open_set_channel_max.] */
/* Tests_S_R_S_CONNECTION_01_004: [After establishing or accepting a TCP connection and sending the protocol header, each peer MUST send an open frame before sending any other frames.] */
/* Tests_S_R_S_CONNECTION_01_002: [Each AMQP connection begins with an exchange of capabilities and limitations, including the maximum frame size.] */
/* Tests_S_R_S_CONNECTION_01_005: [The open frame describes the capabilities and limits of that peer.] */
/* Tests_S_R_S_CONNECTION_01_006: [The open frame can only be sent on channel 0.] */
TEST_FUNCTION(when_the_header_is_received_an_open_frame_is_sent_out)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(frame_codec_set_max_frame_size(TEST_FRAME_CODEC_HANDLE, 4294967295));
    STRICT_EXPECTED_CALL(open_create("1234"));
    STRICT_EXPECTED_CALL(open_set_hostname(test_open_handle, "testhost"));
    STRICT_EXPECTED_CALL(open_set_max_frame_size(test_open_handle, 4294967295));
    STRICT_EXPECTED_CALL(open_set_channel_max(test_open_handle, 65535));
    STRICT_EXPECTED_CALL(amqpvalue_create_open(test_open_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_open_amqp_value, NULL, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(open_destroy(test_open_handle));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_open_amqp_value));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_207: [If frame_codec_set_max_frame_size fails the connection shall be closed and the state set to END.] */
TEST_FUNCTION(when_setting_the_max_frame_size_fails_the_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };

    STRICT_EXPECTED_CALL(frame_codec_set_max_frame_size(TEST_FRAME_CODEC_HANDLE, 4294967295))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_208: [If the open frame cannot be constructed, the connection shall be closed and set to the END state.] */
TEST_FUNCTION(when_open_create_fails_the_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };

    STRICT_EXPECTED_CALL(frame_codec_set_max_frame_size(TEST_FRAME_CODEC_HANDLE, 4294967295));
    STRICT_EXPECTED_CALL(open_create("1234"))
        .SetReturn((OPEN_HANDLE)NULL);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_208: [If the open frame cannot be constructed, the connection shall be closed and set to the END state.] */
TEST_FUNCTION(when_open_set_hostname_fails_the_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };

    STRICT_EXPECTED_CALL(frame_codec_set_max_frame_size(TEST_FRAME_CODEC_HANDLE, 4294967295));
    STRICT_EXPECTED_CALL(open_create("1234"));
    STRICT_EXPECTED_CALL(open_set_max_frame_size(test_open_handle, 4294967295));
    STRICT_EXPECTED_CALL(open_set_channel_max(test_open_handle, 65535));
    STRICT_EXPECTED_CALL(open_set_hostname(test_open_handle, "testhost"))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(open_destroy(test_open_handle));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_208: [If the open frame cannot be constructed, the connection shall be closed and set to the END state.] */
TEST_FUNCTION(when_amqpvalue_create_open_fails_the_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };

    STRICT_EXPECTED_CALL(frame_codec_set_max_frame_size(TEST_FRAME_CODEC_HANDLE, 4294967295));
    STRICT_EXPECTED_CALL(open_create("1234"));
    STRICT_EXPECTED_CALL(open_set_hostname(test_open_handle, "testhost"));
    STRICT_EXPECTED_CALL(open_set_max_frame_size(test_open_handle, 4294967295));
    STRICT_EXPECTED_CALL(open_set_channel_max(test_open_handle, 65535));
    STRICT_EXPECTED_CALL(amqpvalue_create_open(test_open_handle))
        .SetReturn((AMQP_VALUE)NULL);
    STRICT_EXPECTED_CALL(open_destroy(test_open_handle));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_206: [If sending the frame fails, the connection shall be closed and state set to END.] */
TEST_FUNCTION(when_amqp_frame_codec_begin_encode_frame_fails_the_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };

    STRICT_EXPECTED_CALL(frame_codec_set_max_frame_size(TEST_FRAME_CODEC_HANDLE, 4294967295));
    STRICT_EXPECTED_CALL(open_create("1234"));
    STRICT_EXPECTED_CALL(open_set_hostname(test_open_handle, "testhost"));
    STRICT_EXPECTED_CALL(open_set_max_frame_size(test_open_handle, 4294967295));
    STRICT_EXPECTED_CALL(open_set_channel_max(test_open_handle, 65535));
    STRICT_EXPECTED_CALL(amqpvalue_create_open(test_open_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_open_amqp_value, NULL, 0, NULL, NULL))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_open_amqp_value));
    STRICT_EXPECTED_CALL(open_destroy(test_open_handle));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_136: [If no hostname value has been specified, no value shall be stamped in the open frame (no call to open_set_hostname shall be made).] */
TEST_FUNCTION(when_no_hostname_is_specified_no_hostname_is_stamped_on_the_open_frame)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(frame_codec_set_max_frame_size(TEST_FRAME_CODEC_HANDLE, 4294967295));
    STRICT_EXPECTED_CALL(open_create("1234"));
    STRICT_EXPECTED_CALL(open_set_max_frame_size(test_open_handle, 4294967295));
    STRICT_EXPECTED_CALL(open_set_channel_max(test_open_handle, 65535));
    STRICT_EXPECTED_CALL(amqpvalue_create_open(test_open_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_open_amqp_value, NULL, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(open_destroy(test_open_handle));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_open_amqp_value));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_137: [The max_frame_size connection setting shall be set in the open frame by using open_set_max_frame_size.] */
TEST_FUNCTION(when_max_frame_size_has_been_specified_it_shall_be_set_in_the_open_frame)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    (void)connection_set_max_frame_size(connection, 1024);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(frame_codec_set_max_frame_size(TEST_FRAME_CODEC_HANDLE, 1024));
    STRICT_EXPECTED_CALL(open_create("1234"));
    STRICT_EXPECTED_CALL(open_set_max_frame_size(test_open_handle, 1024));
    STRICT_EXPECTED_CALL(open_set_channel_max(test_open_handle, 65535));
    STRICT_EXPECTED_CALL(amqpvalue_create_open(test_open_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_open_amqp_value, NULL, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(open_destroy(test_open_handle));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_open_amqp_value));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_208: [If the open frame cannot be constructed, the connection shall be closed and setto the END state.] */
TEST_FUNCTION(when_setting_the_max_frame_size_on_the_open_frame_fails_then_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    (void)connection_set_max_frame_size(connection, 1024);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };

    STRICT_EXPECTED_CALL(frame_codec_set_max_frame_size(TEST_FRAME_CODEC_HANDLE, 1024));
    STRICT_EXPECTED_CALL(open_create("1234"));
    STRICT_EXPECTED_CALL(open_set_max_frame_size(test_open_handle, 1024))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(open_destroy(test_open_handle));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_139: [The channel_max connection setting shall be set in the open frame by using open_set_channel_max.]  */
TEST_FUNCTION(when_channel_max_has_been_specified_it_shall_be_set_in_the_open_frame)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    (void)connection_set_channel_max(connection, 1024);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(frame_codec_set_max_frame_size(TEST_FRAME_CODEC_HANDLE, 4294967295));
    STRICT_EXPECTED_CALL(open_create("1234"));
    STRICT_EXPECTED_CALL(open_set_max_frame_size(test_open_handle, 4294967295));
    STRICT_EXPECTED_CALL(open_set_channel_max(test_open_handle, 1024));
    STRICT_EXPECTED_CALL(amqpvalue_create_open(test_open_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_open_amqp_value, NULL, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(open_destroy(test_open_handle));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_open_amqp_value));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_208: [If the open frame cannot be constructed, the connection shall be closed and setto the END state.] */
TEST_FUNCTION(when_setting_the_channel_max_on_the_open_frame_fails_then_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    (void)connection_set_channel_max(connection, 1024);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };

    STRICT_EXPECTED_CALL(frame_codec_set_max_frame_size(TEST_FRAME_CODEC_HANDLE, 4294967295));
    STRICT_EXPECTED_CALL(open_create("1234"));
    STRICT_EXPECTED_CALL(open_set_max_frame_size(test_open_handle, 4294967295));
    STRICT_EXPECTED_CALL(open_set_channel_max(test_open_handle, 1024))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(open_destroy(test_open_handle));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_141: [If idle_timeout has been specified by a call to connection_set_idle_timeout, then that value shall be stamped in the open frame.] */
TEST_FUNCTION(when_idle_timeout_has_been_specified_it_shall_be_set_in_the_open_frame)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    (void)connection_set_idle_timeout(connection, 1000);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(frame_codec_set_max_frame_size(TEST_FRAME_CODEC_HANDLE, 4294967295));
    STRICT_EXPECTED_CALL(open_create("1234"));
    STRICT_EXPECTED_CALL(open_set_max_frame_size(test_open_handle, 4294967295));
    STRICT_EXPECTED_CALL(open_set_channel_max(test_open_handle, 65535));
    STRICT_EXPECTED_CALL(open_set_idle_time_out(test_open_handle, 1000));
    STRICT_EXPECTED_CALL(amqpvalue_create_open(test_open_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_open_amqp_value, NULL, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(open_destroy(test_open_handle));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_open_amqp_value));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_208: [If the open frame cannot be constructed, the connection shall be closed and setto the END state.] */
TEST_FUNCTION(when_setting_the_idle_timeout_on_the_open_frame_fails_then_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    (void)connection_set_idle_timeout(connection, 1000);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };

    STRICT_EXPECTED_CALL(frame_codec_set_max_frame_size(TEST_FRAME_CODEC_HANDLE, 4294967295));
    STRICT_EXPECTED_CALL(open_create("1234"));
    STRICT_EXPECTED_CALL(open_set_max_frame_size(test_open_handle, 4294967295));
    STRICT_EXPECTED_CALL(open_set_channel_max(test_open_handle, 65535));
    STRICT_EXPECTED_CALL(open_set_idle_time_out(test_open_handle, 1000))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(open_destroy(test_open_handle));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_212: [After the initial handshake has been done all bytes received from the io instance shall be passed to the frame_codec for decoding by calling frame_codec_receive_bytes.] */
TEST_FUNCTION(when_1_byte_is_received_from_the_io_it_is_passed_to_the_frame_codec)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(frame_codec_receive_bytes(TEST_FRAME_CODEC_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(frame_codec_receive_bytes(TEST_FRAME_CODEC_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .IgnoreAllCalls();

    // act
    unsigned char byte = 42;
    saved_on_bytes_received(saved_on_bytes_received_context, &byte, 1);

    // assert
    stringify_bytes(&byte, 1, expected_stringified_io);
    stringify_bytes(frame_codec_bytes, frame_codec_byte_count, actual_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, expected_stringified_io, actual_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_212: [After the initial handshake has been done all bytes received from the io instance shall be passed to the frame_codec for decoding by calling frame_codec_receive_bytes.] */
TEST_FUNCTION(when_2_bytes_are_received_from_the_io_it_is_passed_to_the_frame_codec)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(frame_codec_receive_bytes(TEST_FRAME_CODEC_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(frame_codec_receive_bytes(TEST_FRAME_CODEC_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .IgnoreAllCalls();

    // act
    unsigned char bytes[] = { 42, 43 };
    saved_on_bytes_received(saved_on_bytes_received_context, bytes, sizeof(bytes));

    // assert
    stringify_bytes(bytes, sizeof(bytes), expected_stringified_io);
    stringify_bytes(frame_codec_bytes, frame_codec_byte_count, actual_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, expected_stringified_io, actual_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_213: [When passing the bytes to frame_codec fails, a CLOSE frame shall be sent and the state shall be set to DISCARDING.]  */
/* Tests_S_R_S_CONNECTION_01_217: [The CLOSE frame shall be constructed by using close_create.] */
/* Tests_S_R_S_CONNECTION_01_215: [Sending the AMQP CLOSE frame shall be done by calling amqp_frame_codec_begin_encode_frame with channel number 0, the actual performative payload and 0 as payload_size.] */
/* Tests_S_R_S_CONNECTION_01_218: [The error amqp:internal-error shall be set in the error.condition field of the CLOSE frame.] */
/* Tests_S_R_S_CONNECTION_01_013: [However, implementations SHOULD send it on channel 0] */
/* Codes_S_R_S_CONNECTION_01_238: [If set, this field indicates that the connection is being closed due to an error condition.] */
TEST_FUNCTION(when_giving_the_bytes_to_frame_codec_fails_the_connection_is_closed_with_internal_error)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(frame_codec_receive_bytes(TEST_FRAME_CODEC_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(close_create());
    STRICT_EXPECTED_CALL(error_create("amqp:internal-error"));
    STRICT_EXPECTED_CALL(error_set_description(test_error_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(close_set_error(test_close_handle, test_error_handle));
    STRICT_EXPECTED_CALL(amqpvalue_create_close(test_close_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_close_amqp_value, NULL, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_close_amqp_value));
    STRICT_EXPECTED_CALL(close_destroy(test_close_handle));
    STRICT_EXPECTED_CALL(error_destroy(test_error_handle));

    // act
    unsigned char bytes[] = { 42, 43 };
    saved_on_bytes_received(saved_on_bytes_received_context, bytes, sizeof(bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_214: [If the close frame cannot be constructed or sent, the connection shall be closed and set to the END state.] */
TEST_FUNCTION(when_creating_a_close_frame_fails_then_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(frame_codec_receive_bytes(TEST_FRAME_CODEC_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(error_create("amqp:internal-error"));
    STRICT_EXPECTED_CALL(error_set_description(test_error_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(close_create())
        .SetReturn((CLOSE_HANDLE)NULL);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(error_destroy(test_error_handle));

    // act
    unsigned char bytes[] = { 42, 43 };
    saved_on_bytes_received(saved_on_bytes_received_context, bytes, sizeof(bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_214: [If the close frame cannot be constructed or sent, the connection shall be closed and set to the END state.] */
TEST_FUNCTION(when_creating_the_amqp_value_for_the_close_performative_fails_then_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(frame_codec_receive_bytes(TEST_FRAME_CODEC_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(error_create("amqp:internal-error"));
    STRICT_EXPECTED_CALL(error_set_description(test_error_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(close_create());
    STRICT_EXPECTED_CALL(close_set_error(test_close_handle, test_error_handle));
    STRICT_EXPECTED_CALL(amqpvalue_create_close(test_close_handle))
        .SetReturn((AMQP_VALUE)NULL);
    STRICT_EXPECTED_CALL(close_destroy(test_close_handle));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(error_destroy(test_error_handle));

    // act
    unsigned char bytes[] = { 42, 43 };
    saved_on_bytes_received(saved_on_bytes_received_context, bytes, sizeof(bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_214: [If the close frame cannot be constructed or sent, the connection shall be closed and set to the END state.] */
TEST_FUNCTION(when_sending_the_close_frame_fails_then_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(frame_codec_receive_bytes(TEST_FRAME_CODEC_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(error_create("amqp:internal-error"));
    STRICT_EXPECTED_CALL(error_set_description(test_error_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(close_create());
    STRICT_EXPECTED_CALL(close_set_error(test_close_handle, test_error_handle));
    STRICT_EXPECTED_CALL(amqpvalue_create_close(test_close_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_close_amqp_value, NULL, 0, NULL, NULL))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_close_amqp_value));
    STRICT_EXPECTED_CALL(close_destroy(test_close_handle));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(error_destroy(test_error_handle));

    // act
    unsigned char bytes[] = { 42, 43 };
    saved_on_bytes_received(saved_on_bytes_received_context, bytes, sizeof(bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_214: [If the close frame cannot be constructed or sent, the connection shall be closed and set to the END state.] */
TEST_FUNCTION(when_creating_the_error_object_fails_the_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(frame_codec_receive_bytes(TEST_FRAME_CODEC_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(error_create("amqp:internal-error"))
        .SetReturn((ERROR_HANDLE)NULL);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));

    // act
    unsigned char bytes[] = { 42, 43 };
    saved_on_bytes_received(saved_on_bytes_received_context, bytes, sizeof(bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_214: [If the close frame cannot be constructed or sent, the connection shall be closed and set to the END state.] */
TEST_FUNCTION(when_setting_the_error_description_on_the_error_handle_fails_the_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(frame_codec_receive_bytes(TEST_FRAME_CODEC_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(error_create("amqp:internal-error"));
    STRICT_EXPECTED_CALL(error_set_description(test_error_handle, IGNORED_PTR_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(error_destroy(test_error_handle));

    // act
    unsigned char bytes[] = { 42, 43 };
    saved_on_bytes_received(saved_on_bytes_received_context, bytes, sizeof(bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_214: [If the close frame cannot be constructed or sent, the connection shall be closed and set to the END state.] */
/* Tests_S_R_S_CONNECTION_01_218: [The error amqp:internal-error shall be set in the error.condition field of the CLOSE frame.] */
/* Tests_S_R_S_CONNECTION_01_219: [The error description shall be set to an implementation defined string.] */
TEST_FUNCTION(when_setting_the_error_on_the_close_frame_fails_the_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(frame_codec_receive_bytes(TEST_FRAME_CODEC_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(error_create("amqp:internal-error"));
    STRICT_EXPECTED_CALL(error_set_description(test_error_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(close_create());
    STRICT_EXPECTED_CALL(close_set_error(test_close_handle, test_error_handle))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(close_destroy(test_close_handle));
    STRICT_EXPECTED_CALL(error_destroy(test_error_handle));

    // act
    unsigned char bytes[] = { 42, 43 };
    saved_on_bytes_received(saved_on_bytes_received_context, bytes, sizeof(bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_212: [After the initial handshake has been done all bytes received from the io instance shall be passed to the frame_codec for decoding by calling frame_codec_receive_bytes.] */
TEST_FUNCTION(when_one_extra_byte_is_received_with_the_header_the_extra_byte_is_passed_to_the_frame_codec)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char in_bytes[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0, 42 };

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, in_bytes, sizeof(in_bytes));

    // assert
    stringify_bytes(&in_bytes[sizeof(in_bytes) - 1], 1, expected_stringified_io);
    stringify_bytes(frame_codec_bytes, frame_codec_byte_count, actual_stringified_io);
    ASSERT_ARE_EQUAL(char_ptr, expected_stringified_io, actual_stringified_io);

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_143: [If any of the values in the received open frame are invalid then the connection shall be closed.] */
/* Tests_S_R_S_CONNECTION_01_220: [The error amqp:invalid-field shall be set in the error.condition field of the CLOSE frame.] */
TEST_FUNCTION(when_an_open_frame_that_cannot_be_parsed_properly_is_received_the_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_OPEN_PERFORMATIVE));
    STRICT_EXPECTED_CALL(is_open_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_open(TEST_OPEN_PERFORMATIVE, IGNORED_PTR_ARG))
        .SetReturn(1);

    /* we expect to close because of bad OPEN */
    STRICT_EXPECTED_CALL(error_create("amqp:invalid-field"));
    STRICT_EXPECTED_CALL(error_set_description(test_error_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(close_create());
    STRICT_EXPECTED_CALL(close_set_error(test_close_handle, test_error_handle));
    STRICT_EXPECTED_CALL(amqpvalue_create_close(test_close_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_close_amqp_value, NULL, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_close_amqp_value));
    STRICT_EXPECTED_CALL(close_destroy(test_close_handle));
    STRICT_EXPECTED_CALL(error_destroy(test_error_handle));

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_143: [If any of the values in the received open frame are invalid then the connection shall be closed.] */
/* Tests_S_R_S_CONNECTION_01_220: [The error amqp:invalid-field shall be set in the error.condition field of the CLOSE frame.] */
TEST_FUNCTION(when_the_max_frame_size_cannot_be_retrieved_from_the_open_framethe_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_OPEN_PERFORMATIVE));
    STRICT_EXPECTED_CALL(is_open_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_open(TEST_OPEN_PERFORMATIVE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_open_handle, sizeof(test_open_handle));
    STRICT_EXPECTED_CALL(open_get_max_frame_size(test_open_handle, IGNORED_PTR_ARG))
        .SetReturn(1);

    /* we expect to close because of bad OPEN */
    STRICT_EXPECTED_CALL(error_create("amqp:invalid-field"));
    STRICT_EXPECTED_CALL(error_set_description(test_error_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(close_create());
    STRICT_EXPECTED_CALL(close_set_error(test_close_handle, test_error_handle));
    STRICT_EXPECTED_CALL(amqpvalue_create_close(test_close_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_close_amqp_value, NULL, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_close_amqp_value));
    STRICT_EXPECTED_CALL(close_destroy(test_close_handle));
    STRICT_EXPECTED_CALL(error_destroy(test_error_handle));
    STRICT_EXPECTED_CALL(open_destroy(test_open_handle));

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_143: [If any of the values in the received open frame are invalid then the connection shall be closed.] */
/* Tests_S_R_S_CONNECTION_01_220: [The error amqp:invalid-field shall be set in the error.condition field of the CLOSE frame.] */
/* Tests_S_R_S_CONNECTION_01_167: [Both peers MUST accept frames of up to 512 (MIN-MAX-FRAME-SIZE) octets.] */
TEST_FUNCTION(when_an_open_frame_with_max_frame_size_511_is_received_the_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_OPEN_PERFORMATIVE));
    STRICT_EXPECTED_CALL(is_open_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_open(TEST_OPEN_PERFORMATIVE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_open_handle, sizeof(test_open_handle));
    uint32_t remote_max_frame_size = 511;
    STRICT_EXPECTED_CALL(open_get_max_frame_size(test_open_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &remote_max_frame_size, sizeof(remote_max_frame_size));

    /* we expect to close because of bad OPEN */
    STRICT_EXPECTED_CALL(error_create("amqp:invalid-field"));
    STRICT_EXPECTED_CALL(error_set_description(test_error_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(close_create());
    STRICT_EXPECTED_CALL(close_set_error(test_close_handle, test_error_handle));
    STRICT_EXPECTED_CALL(amqpvalue_create_close(test_close_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_close_amqp_value, NULL, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_close_amqp_value));
    STRICT_EXPECTED_CALL(close_destroy(test_close_handle));
    STRICT_EXPECTED_CALL(error_destroy(test_error_handle));
    STRICT_EXPECTED_CALL(open_destroy(test_open_handle));

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_006: [The open frame can only be sent on channel 0.] */
/* Tests_S_R_S_CONNECTION_01_222: [If an Open frame is received in a manner violating the ISO specification, the connection shall be closed with condition amqp:not-allowed and description being an implementation defined string.] */
TEST_FUNCTION(when_an_open_frame_is_received_on_channel_1_the_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_OPEN_PERFORMATIVE));
    STRICT_EXPECTED_CALL(is_open_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));

    /* we expect to close because of bad OPEN */
    STRICT_EXPECTED_CALL(error_create("amqp:not-allowed"));
    STRICT_EXPECTED_CALL(error_set_description(test_error_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(close_create());
    STRICT_EXPECTED_CALL(close_set_error(test_close_handle, test_error_handle));
    STRICT_EXPECTED_CALL(amqpvalue_create_close(test_close_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_close_amqp_value, NULL, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_close_amqp_value));
    STRICT_EXPECTED_CALL(close_destroy(test_close_handle));
    STRICT_EXPECTED_CALL(error_destroy(test_error_handle));

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 1, TEST_OPEN_PERFORMATIVE, 0, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_223: [If the frame_received_callback is called with a NULL performative then the connection shall be closed with the error condition amqp:internal-error and an implementation defined error description.] */
TEST_FUNCTION(when_the_frame_received_callback_is_called_with_a_NULL_performative_the_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    /* we expect to close because of bad OPEN */
    STRICT_EXPECTED_CALL(error_create("amqp:internal-error"));
    STRICT_EXPECTED_CALL(error_set_description(test_error_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(close_create());
    STRICT_EXPECTED_CALL(close_set_error(test_close_handle, test_error_handle));
    STRICT_EXPECTED_CALL(amqpvalue_create_close(test_close_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_close_amqp_value, NULL, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_close_amqp_value));
    STRICT_EXPECTED_CALL(close_destroy(test_close_handle));
    STRICT_EXPECTED_CALL(error_destroy(test_error_handle));

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 1, NULL, 0, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_224: [START HDR HDR] */
TEST_FUNCTION(when_an_open_frame_is_indicated_as_received_before_even_opening_the_io_nothing_is_done)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    umock_c_reset_all_calls();

    unsigned char payload_bytes[] = { 0x42 };

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, payload_bytes, sizeof(payload_bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_226: [HDR_SENT OPEN HDR] */
TEST_FUNCTION(when_an_open_frame_is_indicated_as_received_before_the_header_exchange_the_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_226: [HDR_SENT OPEN HDR] */
TEST_FUNCTION(when_a_close_frame_is_received_in_HDR_SENT_the_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_CLOSE_PERFORMATIVE, 0, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_229: [OPEN_SENT ** OPEN] */
/* Tests_S_R_S_CONNECTION_01_008: [Prior to closing a connection, each peer MUST write a close frame with a code indicating the reason for closing.] */
/* Codes_S_R_S_CONNECTION_01_238: [If set, this field indicates that the connection is being closed due to an error condition.] */
TEST_FUNCTION(when_a_close_frame_is_received_in_OPEN_SENT_a_CLOSE_is_sent)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_CLOSE_PERFORMATIVE))
        .SetReturn(TEST_CLOSE_DESCRIPTOR_AMQP_VALUE);
    STRICT_EXPECTED_CALL(is_open_type_by_descriptor(TEST_CLOSE_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_close_type_by_descriptor(TEST_CLOSE_DESCRIPTOR_AMQP_VALUE));
    CLOSE_HANDLE received_test_close_handle = (CLOSE_HANDLE)0x4000;
    STRICT_EXPECTED_CALL(amqpvalue_get_close(TEST_CLOSE_PERFORMATIVE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &received_test_close_handle, sizeof(received_test_close_handle));
    STRICT_EXPECTED_CALL(close_destroy(received_test_close_handle));

    /* we expect to close with no error */
    STRICT_EXPECTED_CALL(close_create());
    STRICT_EXPECTED_CALL(amqpvalue_create_close(test_close_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_close_amqp_value, NULL, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_close_amqp_value));
    STRICT_EXPECTED_CALL(close_destroy(test_close_handle));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_CLOSE_PERFORMATIVE, 0, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_214: [If the close frame cannot be constructed or sent, the connection shall be closed and set to the END state.] */
TEST_FUNCTION(when_a_close_frame_is_sent_as_response_to_a_close_frame_and_creating_the_close_frame_fails_the_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_CLOSE_PERFORMATIVE))
        .SetReturn(TEST_CLOSE_DESCRIPTOR_AMQP_VALUE);
    STRICT_EXPECTED_CALL(is_open_type_by_descriptor(TEST_CLOSE_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_close_type_by_descriptor(TEST_CLOSE_DESCRIPTOR_AMQP_VALUE));
    CLOSE_HANDLE received_test_close_handle = (CLOSE_HANDLE)0x4000;
    STRICT_EXPECTED_CALL(amqpvalue_get_close(TEST_CLOSE_PERFORMATIVE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &received_test_close_handle, sizeof(received_test_close_handle));
    STRICT_EXPECTED_CALL(close_destroy(received_test_close_handle));

    /* we expect to close with no error */
    STRICT_EXPECTED_CALL(close_create())
        .SetReturn((CLOSE_HANDLE)NULL);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_CLOSE_PERFORMATIVE, 0, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_214: [If the close frame cannot be constructed or sent, the connection shall be closed and set to the END state.] */
TEST_FUNCTION(when_a_close_frame_is_sent_as_response_to_a_close_frame_and_creating_the_close_frame_AMQP_VALUE_fails_the_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_CLOSE_PERFORMATIVE))
        .SetReturn(TEST_CLOSE_DESCRIPTOR_AMQP_VALUE);
    STRICT_EXPECTED_CALL(is_open_type_by_descriptor(TEST_CLOSE_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_close_type_by_descriptor(TEST_CLOSE_DESCRIPTOR_AMQP_VALUE));
    CLOSE_HANDLE received_test_close_handle = (CLOSE_HANDLE)0x4000;
    STRICT_EXPECTED_CALL(amqpvalue_get_close(TEST_CLOSE_PERFORMATIVE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &received_test_close_handle, sizeof(received_test_close_handle));
    STRICT_EXPECTED_CALL(close_destroy(received_test_close_handle));

    /* we expect to close with no error */
    STRICT_EXPECTED_CALL(close_create());
    STRICT_EXPECTED_CALL(amqpvalue_create_close(test_close_handle))
        .SetReturn((AMQP_VALUE)NULL);
    STRICT_EXPECTED_CALL(close_destroy(test_close_handle));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_CLOSE_PERFORMATIVE, 0, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_214: [If the close frame cannot be constructed or sent, the connection shall be closed and set to the END state.] */
TEST_FUNCTION(when_a_close_frame_is_sent_as_response_to_a_close_frame_and_sending_the_frame_fails_the_connection_is_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_CLOSE_PERFORMATIVE))
        .SetReturn(TEST_CLOSE_DESCRIPTOR_AMQP_VALUE);
    STRICT_EXPECTED_CALL(is_open_type_by_descriptor(TEST_CLOSE_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_close_type_by_descriptor(TEST_CLOSE_DESCRIPTOR_AMQP_VALUE));
    CLOSE_HANDLE received_test_close_handle = (CLOSE_HANDLE)0x4000;
    STRICT_EXPECTED_CALL(amqpvalue_get_close(TEST_CLOSE_PERFORMATIVE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &received_test_close_handle, sizeof(received_test_close_handle));
    STRICT_EXPECTED_CALL(close_destroy(received_test_close_handle));

    /* we expect to close with no error */
    STRICT_EXPECTED_CALL(close_create());
    STRICT_EXPECTED_CALL(amqpvalue_create_close(test_close_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_close_amqp_value, NULL, 0, NULL, NULL))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_close_amqp_value));
    STRICT_EXPECTED_CALL(close_destroy(test_close_handle));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_CLOSE_PERFORMATIVE, 0, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_239: [If an Open frame is received in the Opened state the connection shall be closed with condition amqp:illegal-state and description being an implementation defined string.] */
TEST_FUNCTION(when_an_open_frame_is_received_in_open_the_connection_shall_be_closed_with_illegal_state)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_OPEN_PERFORMATIVE));
    STRICT_EXPECTED_CALL(is_open_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));

    STRICT_EXPECTED_CALL(error_create("amqp:illegal-state"));
    STRICT_EXPECTED_CALL(error_set_description(test_error_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(close_create());
    STRICT_EXPECTED_CALL(close_set_error(test_close_handle, test_error_handle));
    STRICT_EXPECTED_CALL(amqpvalue_create_close(test_close_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_close_amqp_value, NULL, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_close_amqp_value));
    STRICT_EXPECTED_CALL(close_destroy(test_close_handle));
    STRICT_EXPECTED_CALL(error_destroy(test_error_handle));

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_055: [DISCARDING The DISCARDING state is a variant of the CLOSE SENT state where the close is triggered by an error.] */
TEST_FUNCTION(when_an_open_frame_is_received_in_the_DISCARDING_state_the_connection_is_not_closed)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_OPEN_PERFORMATIVE));
    STRICT_EXPECTED_CALL(is_open_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_010: [After writing this frame the peer SHOULD continue to read from the connection until it receives the partner's close frame ] */
/* Tests_S_R_S_CONNECTION_01_240: [There is no requirement for an implementation to read from a socket after a close performative has been received.] */
TEST_FUNCTION(when_in_discarding_state_the_connection_still_looks_for_the_close_frame_and_then_closes_the_io)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_CLOSE_PERFORMATIVE));
    STRICT_EXPECTED_CALL(is_open_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_close_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_CLOSE_PERFORMATIVE, 0, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_012: [A close frame MAY be received on any channel up to the maximum channel number negotiated in open.] */
TEST_FUNCTION(when_a_CLOSE_frame_is_received_on_channel_1_it_is_still_valid)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_CLOSE_PERFORMATIVE));
    STRICT_EXPECTED_CALL(is_open_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_close_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    CLOSE_HANDLE received_test_close_handle = (CLOSE_HANDLE)0x4000;
    STRICT_EXPECTED_CALL(amqpvalue_get_close(TEST_CLOSE_PERFORMATIVE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &received_test_close_handle, sizeof(received_test_close_handle));
    STRICT_EXPECTED_CALL(close_destroy(received_test_close_handle));

    STRICT_EXPECTED_CALL(close_create());
    STRICT_EXPECTED_CALL(amqpvalue_create_close(test_close_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_close_amqp_value, NULL, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_close_amqp_value));
    STRICT_EXPECTED_CALL(close_destroy(test_close_handle));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 1, TEST_CLOSE_PERFORMATIVE, 0, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_242: [The connection module shall accept CLOSE frames even if they have extra payload bytes besides the Close performative.] */
TEST_FUNCTION(when_a_CLOSE_frame_with_1_byte_payload_is_received_it_is_still_valid)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_CLOSE_PERFORMATIVE));
    STRICT_EXPECTED_CALL(is_open_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_close_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    CLOSE_HANDLE received_test_close_handle = (CLOSE_HANDLE)0x4000;
    STRICT_EXPECTED_CALL(amqpvalue_get_close(TEST_CLOSE_PERFORMATIVE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &received_test_close_handle, sizeof(received_test_close_handle));
    STRICT_EXPECTED_CALL(close_destroy(received_test_close_handle));

    STRICT_EXPECTED_CALL(close_create());
    STRICT_EXPECTED_CALL(amqpvalue_create_close(test_close_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_close_amqp_value, NULL, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_close_amqp_value));
    STRICT_EXPECTED_CALL(close_destroy(test_close_handle));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));

    unsigned char payload_bytes[] = { 0x42 };

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 1, TEST_CLOSE_PERFORMATIVE, payload_bytes, sizeof(payload_bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_242: [The connection module shall accept CLOSE frames even if they have extra payload bytes besides the Close performative.] */
TEST_FUNCTION(when_an_OPEN_frame_with_1_byte_payload_is_received_it_is_still_valid)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_OPEN_PERFORMATIVE));
    STRICT_EXPECTED_CALL(is_open_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    CLOSE_HANDLE received_test_close_handle = (CLOSE_HANDLE)0x4000;
    STRICT_EXPECTED_CALL(amqpvalue_get_open(TEST_OPEN_PERFORMATIVE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_open_handle, sizeof(test_open_handle));
    uint32_t remote_max_frame_size = 1024;
    STRICT_EXPECTED_CALL(open_get_max_frame_size(test_open_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &remote_max_frame_size, sizeof(remote_max_frame_size));
    STRICT_EXPECTED_CALL(open_destroy(test_open_handle));

    unsigned char payload_bytes[] = { 0x42 };

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, payload_bytes, sizeof(payload_bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_012: [A close frame MAY be received on any channel up to the maximum channel number negotiated in open.] */
TEST_FUNCTION(when_a_CLOSE_FRAME_is_received_on_a_channel_higher_than_the_max_negotiated_channel_a_close_with_invalid_field_shall_be_done)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, NULL, "1234");
    (void)connection_set_channel_max(connection, 0);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_CLOSE_PERFORMATIVE));
    STRICT_EXPECTED_CALL(is_open_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_close_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));

    STRICT_EXPECTED_CALL(error_create("amqp:invalid-field"));
    STRICT_EXPECTED_CALL(error_set_description(test_error_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(close_create());
    STRICT_EXPECTED_CALL(close_set_error(test_close_handle, test_error_handle));
    STRICT_EXPECTED_CALL(amqpvalue_create_close(test_close_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_close_amqp_value, NULL, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_close_amqp_value));
    STRICT_EXPECTED_CALL(close_destroy(test_close_handle));
    STRICT_EXPECTED_CALL(error_destroy(test_error_handle));

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 1, TEST_CLOSE_PERFORMATIVE, 0, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* connection_create_endpoint */

/* Tests_S_R_S_CONNECTION_01_113: [If connection, frame_received_callback or connection_state_changed_callback is NULL, connection_create_endpoint shall fail and return NULL.] */
TEST_FUNCTION(connection_create_endpoint_with_NULL_conneciton_fails)
{
    // arrange

    // act
    ENDPOINT_HANDLE result = connection_create_endpoint(NULL, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_S_R_S_CONNECTION_01_113: [If connection, frame_received_callback or connection_state_changed_callback is NULL, connection_create_endpoint shall fail and return NULL.] */
TEST_FUNCTION(connection_create_endpoint_with_NULL_frame_receive_callback_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    umock_c_reset_all_calls();

    // act
    ENDPOINT_HANDLE result = connection_create_endpoint(connection, NULL, test_on_connection_state_changed, TEST_CONTEXT);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_113: [If connection, frame_received_callback or connection_state_changed_callback is NULL, connection_create_endpoint shall fail and return NULL.] */
TEST_FUNCTION(connection_create_endpoint_with_NULL_connection_state_changed_callback_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    umock_c_reset_all_calls();

    // act
    ENDPOINT_HANDLE result = connection_create_endpoint(connection, test_on_frame_received, NULL, TEST_CONTEXT);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_112: [connection_create_endpoint shall create a new endpoint that can be used by a session.] */
/* Tests_S_R_S_CONNECTION_01_127: [On success, connection_create_endpoint shall return a non-NULL handle to the newly created endpoint.] */
/* Tests_S_R_S_CONNECTION_01_197: [The newly created endpoint shall be added to the endpoints list, so that it can be tracked.] */
TEST_FUNCTION(connection_create_endpoint_with_valid_arguments_succeeds)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    // act
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);

    // assert
    ASSERT_IS_NOT_NULL(endpoint);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_196: [If memory cannot be allocated for the new endpoint, connection_create_endpoint shall fail and return NULL.] */
TEST_FUNCTION(when_allocating_memory_fails_connection_create_endpoint_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);

    // act
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);

    // assert
    ASSERT_IS_NULL(endpoint);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_198: [If adding the endpoint to the endpoints list tracked by the connection fails, connection_create_endpoint shall fail and return NULL.] */
TEST_FUNCTION(when_realloc_for_the_endpoint_list_fails_connection_create_endpoint_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);

    // assert
    ASSERT_IS_NULL(endpoint);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_193: [The context argument shall be allowed to be NULL.] */
TEST_FUNCTION(connection_create_endpoint_with_valid_arguments_and_NULL_context_succeeds)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    // act
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, NULL);

    // assert
    ASSERT_IS_NOT_NULL(endpoint);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_115: [If no more endpoints can be created due to all channels being used, connection_create_endpoint shall fail and return NULL.] */
TEST_FUNCTION(when_no_more_channels_are_available_connection_create_endpoint_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    (void)connection_set_channel_max(connection, 0);
    ENDPOINT_HANDLE endpoint0 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    umock_c_reset_all_calls();

    // act
    ENDPOINT_HANDLE endpoint1 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);

    // assert
    ASSERT_IS_NULL(endpoint1);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint0);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_115: [If no more endpoints can be created due to all channels being used, connection_create_endpoint shall fail and return NULL.] */
TEST_FUNCTION(when_no_more_channels_are_available_after_create_destroy_and_create_again_connection_create_endpoint_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    (void)connection_set_channel_max(connection, 0);
    ENDPOINT_HANDLE endpoint0 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    connection_destroy_endpoint(endpoint0);
    endpoint0 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    umock_c_reset_all_calls();

    // act
    ENDPOINT_HANDLE endpoint1 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);

    // assert
    ASSERT_IS_NULL(endpoint1);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint0);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_115: [If no more endpoints can be created due to all channels being used, connection_create_endpoint shall fail and return NULL.] */
TEST_FUNCTION(when_no_more_channels_are_available_with_channel_max_1_connection_create_endpoint_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    (void)connection_set_channel_max(connection, 1);
    ENDPOINT_HANDLE endpoint0 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    ENDPOINT_HANDLE endpoint1 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    umock_c_reset_all_calls();

    // act
    ENDPOINT_HANDLE endpoint2 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);

    // assert
    ASSERT_IS_NULL(endpoint1);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint0);
    connection_destroy_endpoint(endpoint1);
    connection_destroy(connection);
}

/* connection_destroy_endpoint */

/* Tests_S_R_S_CONNECTION_01_199: [If endpoint is NULL, connection_destroy_endpoint shall do nothing.] */
TEST_FUNCTION(connection_destroy_endpoint_with_NULL_argument_does_nothing)
{
    // arrange

    // act
    connection_destroy_endpoint(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_S_R_S_CONNECTION_01_129: [connection_destroy_endpoint shall free all resources associated with an endpoint created by connection_create_endpoint.] */
/* Tests_S_R_S_CONNECTION_01_130: [The outgoing channel associated with the endpoint shall be released by removing the endpoint from the endpoint list.] */
TEST_FUNCTION(connection_destroy_endpoint_frees_the_resources_associated_with_the_endpoint)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    connection_destroy_endpoint(endpoint);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_129: [connection_destroy_endpoint shall free all resources associated with an endpoint created by connection_create_endpoint.] */
/* Tests_S_R_S_CONNECTION_01_130: [The outgoing channel associated with the endpoint shall be released by removing the endpoint from the endpoint list.] */
/* Tests_S_R_S_CONNECTION_01_131: [Any incoming channel number associated with the endpoint shall be released.] */
TEST_FUNCTION(when_reallocating_the_endpoints_list_fails_connection_destroy_endpoint_shall_still_free_all_resources)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    connection_destroy_endpoint(endpoint);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_130: [The outgoing channel associated with the endpoint shall be released by removing the endpoint from the endpoint list.] */
TEST_FUNCTION(when_an_endpoint_is_released_another_one_can_be_created_in_its_place)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    (void)connection_set_channel_max(connection, 2);
    ENDPOINT_HANDLE endpoint0 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    ENDPOINT_HANDLE endpoint1 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    ENDPOINT_HANDLE endpoint2 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    connection_destroy_endpoint(endpoint1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    // act
    endpoint1 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);

    // assert
    ASSERT_IS_NOT_NULL(endpoint1);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint0);
    connection_destroy_endpoint(endpoint1);
    connection_destroy_endpoint(endpoint2);
    connection_destroy(connection);
}

/* connection_encode_frame */

/* Tests_S_R_S_CONNECTION_01_249: [If endpoint or performative are NULL, connection_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(connection_encode_frame_with_NULL_endpoint_fails)
{
    // arrange

    // act
    int result = connection_encode_frame(NULL, TEST_TRANSFER_PERFORMATIVE, NULL, 0, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_S_R_S_CONNECTION_01_249: [If endpoint or performative are NULL, connection_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(connection_encode_frame_with_NULL_performative_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    umock_c_reset_all_calls();

    // act
    int result = connection_encode_frame(endpoint, NULL, NULL, 0, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_247: [connection_encode_frame shall send a frame for a certain endpoint.] */
/* Tests_S_R_S_CONNECTION_01_248: [On success it shall return 0.] */
/* Tests_S_R_S_CONNECTION_01_250: [connection_encode_frame shall initiate the frame send by calling amqp_frame_codec_begin_encode_frame.] */
/* Tests_S_R_S_CONNECTION_01_251: [The channel number passed to amqp_frame_codec_begin_encode_frame shall be the outgoing channel number associated with the endpoint by connection_create_endpoint.] */
/* Tests_S_R_S_CONNECTION_01_252: [The performative passed to amqp_frame_codec_begin_encode_frame shall be the performative argument of connection_encode_frame.] */
/* Tests_S_R_S_CONNECTION_01_255: [The payload size shall be computed based on all the payload chunks passed as argument in payloads.] */
TEST_FUNCTION(connection_encode_frame_sends_the_frame)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, TEST_TRANSFER_PERFORMATIVE, NULL, 0, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    int result = connection_encode_frame(endpoint, TEST_TRANSFER_PERFORMATIVE, NULL, 0, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_255: [The payload size shall be computed based on all the payload chunks passed as argument in payloads.] */
/* Tests_S_R_S_CONNECTION_01_256: [Each payload passed in the payloads array shall be passed to amqp_frame_codec by calling amqp_frame_codec_encode_payload_bytes.] */
TEST_FUNCTION(connection_encode_frame_with_1_payload_adds_the_bytes_to_the_frame_payload)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    unsigned char test_payload[] = { 0x42 };
    PAYLOAD payload = { test_payload, sizeof(test_payload) };

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, TEST_TRANSFER_PERFORMATIVE, &payload, 1, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    int result = connection_encode_frame(endpoint, TEST_TRANSFER_PERFORMATIVE, &payload, 1, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_255: [The payload size shall be computed based on all the payload chunks passed as argument in payloads.] */
/* Tests_S_R_S_CONNECTION_01_256: [Each payload passed in the payloads array shall be passed to amqp_frame_codec by calling amqp_frame_codec_encode_payload_bytes.] */
TEST_FUNCTION(connection_encode_frame_with_1_payload_of_2_bytes_adds_the_bytes_to_the_frame_payload)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    unsigned char test_payload[] = { 0x42, 0x43 };
    PAYLOAD payload = { test_payload, sizeof(test_payload) };

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, TEST_TRANSFER_PERFORMATIVE, &payload, 1, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    int result = connection_encode_frame(endpoint, TEST_TRANSFER_PERFORMATIVE, &payload, 1, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_255: [The payload size shall be computed based on all the payload chunks passed as argument in payloads.] */
/* Tests_S_R_S_CONNECTION_01_256: [Each payload passed in the payloads array shall be passed to amqp_frame_codec by calling amqp_frame_codec_encode_payload_bytes.] */
TEST_FUNCTION(connection_encode_frame_with_2_payloads_of_1_byte_rach_adds_the_bytes_to_the_frame_payload)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    unsigned char test_payload1[] = { 0x42 };
    unsigned char test_payload2[] = { 0x43 };
    PAYLOAD payloads[] = { { test_payload1, sizeof(test_payload1) }, { test_payload2, sizeof(test_payload2) } };

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, TEST_TRANSFER_PERFORMATIVE, payloads, 2, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    int result = connection_encode_frame(endpoint, TEST_TRANSFER_PERFORMATIVE, payloads, 2, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_253: [If amqp_frame_codec_begin_encode_frame or amqp_frame_codec_encode_payload_bytes fails, then connection_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(when_amqp_frame_codec_begin_encode_frame_fails_then_connection_encode_frame_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    unsigned char test_payload1[] = { 0x42 };
    unsigned char test_payload2[] = { 0x43 };
    PAYLOAD payloads[] = { { test_payload1, sizeof(test_payload1) }, { test_payload2, sizeof(test_payload2) } };

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, TEST_TRANSFER_PERFORMATIVE, payloads, 2, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1);

    // act
    int result = connection_encode_frame(endpoint, TEST_TRANSFER_PERFORMATIVE, payloads, 2, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_254: [If connection_encode_frame is called before the connection is in the OPENED state, connection_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(connection_encode_frame_when_connection_is_not_opened_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    umock_c_reset_all_calls();

    // act
    int result = connection_encode_frame(endpoint, TEST_TRANSFER_PERFORMATIVE, NULL, 0, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_253: [If amqp_frame_codec_begin_encode_frame or amqp_frame_codec_encode_payload_bytes fails, then connection_encode_frame shall fail and return a non-zero value.] */
TEST_FUNCTION(connection_encode_frame_after_close_has_been_received_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    ENDPOINT_HANDLE endpoint = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(is_open_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);

    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_CLOSE_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    unsigned char test_payload1[] = { 0x42 };
    unsigned char test_payload2[] = { 0x43 };
    PAYLOAD payloads[] = { { test_payload1, sizeof(test_payload1) }, { test_payload2, sizeof(test_payload2) } };

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    // act
    int result = connection_encode_frame(endpoint, TEST_TRANSFER_PERFORMATIVE, payloads, 2, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_251: [The channel number passed to amqp_frame_codec_begin_encode_frame shall be the outgoing channel number associated with the endpoint by connection_create_endpoint.] */
/* Tests_S_R_S_CONNECTION_01_128: [The lowest number outgoing channel shall be associated with the newly created endpoint.] */
TEST_FUNCTION(connection_encode_frame_with_a_second_endpoint_sends_on_channel_1)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    ENDPOINT_HANDLE endpoint0 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    ENDPOINT_HANDLE endpoint1 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 1, TEST_TRANSFER_PERFORMATIVE, NULL, 0, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    int result = connection_encode_frame(endpoint1, TEST_TRANSFER_PERFORMATIVE, NULL, 0, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint0);
    connection_destroy_endpoint(endpoint1);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_251: [The channel number passed to amqp_frame_codec_begin_encode_frame shall be the outgoing channel number associated with the endpoint by connection_create_endpoint.] */
/* Tests_S_R_S_CONNECTION_01_128: [The lowest number outgoing channel shall be associated with the newly created endpoint.] */
TEST_FUNCTION(when_an_endpoint_is_destroyed_and_a_new_one_is_created_the_channel_is_reused_on_the_new_endpoint)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    ENDPOINT_HANDLE endpoint0 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    ENDPOINT_HANDLE endpoint1 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    connection_destroy_endpoint(endpoint0);
    endpoint0 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, TEST_TRANSFER_PERFORMATIVE, NULL, 0, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    int result = connection_encode_frame(endpoint0, TEST_TRANSFER_PERFORMATIVE, NULL, 0, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint0);
    connection_destroy_endpoint(endpoint1);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_258: [connection_state_changed_callback shall be invoked whenever the connection state changes.] */
/* Tests_S_R_S_CONNECTION_01_260: [Each endpoint's connection_state_changed_callback shall be called.] */
/* Tests_S_R_S_CONNECTION_01_259: [As context, the callback_context passed in connection_create_endpoint shall be given.] */
TEST_FUNCTION(when_state_changes_to_HDR_SENT_all_endpoints_are_notified)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    ENDPOINT_HANDLE endpoint0 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    ENDPOINT_HANDLE endpoint1 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, NULL);
    connection_dowork(connection);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(test_on_connection_state_changed(TEST_CONTEXT, CONNECTION_STATE_HDR_SENT, CONNECTION_STATE_START));
    STRICT_EXPECTED_CALL(test_on_connection_state_changed(NULL, CONNECTION_STATE_HDR_SENT, CONNECTION_STATE_START));

    // act
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint0);
    connection_destroy_endpoint(endpoint1);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_258: [connection_state_changed_callback shall be invoked whenever the connection state changes.] */
/* Tests_S_R_S_CONNECTION_01_260: [Each endpoint's connection_state_changed_callback shall be called.] */
/* Tests_S_R_S_CONNECTION_01_259: [As context, the callback_context passed in connection_create_endpoint shall be given.] */
TEST_FUNCTION(when_state_changes_to_HDR_EXCH_and_HDR_OPEN_SENT_all_endpoints_are_notified)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    ENDPOINT_HANDLE endpoint0 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    ENDPOINT_HANDLE endpoint1 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, NULL);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(frame_codec_set_max_frame_size(TEST_FRAME_CODEC_HANDLE, 4294967295));
    STRICT_EXPECTED_CALL(open_create("1234"));
    STRICT_EXPECTED_CALL(open_set_hostname(test_open_handle, "testhost"));
    STRICT_EXPECTED_CALL(open_set_max_frame_size(test_open_handle, 4294967295));
    STRICT_EXPECTED_CALL(open_set_channel_max(test_open_handle, 65535));
    STRICT_EXPECTED_CALL(amqpvalue_create_open(test_open_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_open_amqp_value, NULL, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(open_destroy(test_open_handle));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_open_amqp_value));

    STRICT_EXPECTED_CALL(test_on_connection_state_changed(TEST_CONTEXT, CONNECTION_STATE_HDR_EXCH, CONNECTION_STATE_HDR_SENT));
    STRICT_EXPECTED_CALL(test_on_connection_state_changed(NULL, CONNECTION_STATE_HDR_EXCH, CONNECTION_STATE_HDR_SENT));
    STRICT_EXPECTED_CALL(test_on_connection_state_changed(TEST_CONTEXT, CONNECTION_STATE_OPEN_SENT, CONNECTION_STATE_HDR_EXCH));
    STRICT_EXPECTED_CALL(test_on_connection_state_changed(NULL, CONNECTION_STATE_OPEN_SENT, CONNECTION_STATE_HDR_EXCH));

    // act
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint0);
    connection_destroy_endpoint(endpoint1);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_258: [connection_state_changed_callback shall be invoked whenever the connection state changes.] */
/* Tests_S_R_S_CONNECTION_01_260: [Each endpoint's connection_state_changed_callback shall be called.] */
/* Tests_S_R_S_CONNECTION_01_259: [As context, the callback_context passed in connection_create_endpoint shall be given.] */
TEST_FUNCTION(when_state_changes_to_OPENED_all_endpoints_are_notified)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    ENDPOINT_HANDLE endpoint0 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    ENDPOINT_HANDLE endpoint1 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, NULL);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_OPEN_PERFORMATIVE));
    STRICT_EXPECTED_CALL(is_open_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    STRICT_EXPECTED_CALL(amqpvalue_get_open(TEST_OPEN_PERFORMATIVE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &test_open_handle, sizeof(test_open_handle));
    STRICT_EXPECTED_CALL(open_get_max_frame_size(test_open_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(open_destroy(test_open_handle));

    STRICT_EXPECTED_CALL(test_on_connection_state_changed(TEST_CONTEXT, CONNECTION_STATE_OPENED, CONNECTION_STATE_OPEN_SENT));
    STRICT_EXPECTED_CALL(test_on_connection_state_changed(NULL, CONNECTION_STATE_OPENED, CONNECTION_STATE_OPEN_SENT));

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint0);
    connection_destroy_endpoint(endpoint1);
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_258: [connection_state_changed_callback shall be invoked whenever the connection state changes.] */
/* Tests_S_R_S_CONNECTION_01_260: [Each endpoint's connection_state_changed_callback shall be called.] */
/* Tests_S_R_S_CONNECTION_01_259: [As context, the callback_context passed in connection_create_endpoint shall be given.] */
TEST_FUNCTION(when_state_changes_to_CLOSE_RCVD_and_END_SENT_all_endpoints_are_notified)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id);
    ENDPOINT_HANDLE endpoint0 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, TEST_CONTEXT);
    ENDPOINT_HANDLE endpoint1 = connection_create_endpoint(connection, test_on_frame_received, test_on_connection_state_changed, NULL);
    connection_dowork(connection);
    saved_io_state_changed(saved_on_io_open_complete_context, IO_STATE_OPEN, IO_STATE_NOT_OPEN);
    const unsigned char amqp_header[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header, sizeof(amqp_header));
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_OPEN_PERFORMATIVE, 0, 0);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_to_string(IGNORED_PTR_ARG)).IgnoreAllCalls();

    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_CLOSE_PERFORMATIVE))
        .SetReturn(TEST_CLOSE_DESCRIPTOR_AMQP_VALUE);
    STRICT_EXPECTED_CALL(is_open_type_by_descriptor(TEST_CLOSE_DESCRIPTOR_AMQP_VALUE))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(is_close_type_by_descriptor(TEST_CLOSE_DESCRIPTOR_AMQP_VALUE));
    CLOSE_HANDLE received_test_close_handle = (CLOSE_HANDLE)0x4000;
    STRICT_EXPECTED_CALL(amqpvalue_get_close(TEST_CLOSE_PERFORMATIVE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &received_test_close_handle, sizeof(received_test_close_handle));
    STRICT_EXPECTED_CALL(close_destroy(received_test_close_handle));

    /* we expect to close with no error */
    STRICT_EXPECTED_CALL(close_create());
    STRICT_EXPECTED_CALL(amqpvalue_create_close(test_close_handle));
    STRICT_EXPECTED_CALL(amqp_frame_codec_encode_frame(TEST_AMQP_FRAME_CODEC_HANDLE, 0, test_close_amqp_value, NULL, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_close_amqp_value));
    STRICT_EXPECTED_CALL(close_destroy(test_close_handle));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE));

    STRICT_EXPECTED_CALL(test_on_connection_state_changed(TEST_CONTEXT, CONNECTION_STATE_CLOSE_RCVD, CONNECTION_STATE_OPENED));
    STRICT_EXPECTED_CALL(test_on_connection_state_changed(NULL, CONNECTION_STATE_CLOSE_RCVD, CONNECTION_STATE_OPENED));
    STRICT_EXPECTED_CALL(test_on_connection_state_changed(TEST_CONTEXT, CONNECTION_STATE_END, CONNECTION_STATE_CLOSE_RCVD));
    STRICT_EXPECTED_CALL(test_on_connection_state_changed(NULL, CONNECTION_STATE_END, CONNECTION_STATE_CLOSE_RCVD));

    // act
    saved_frame_received_callback(saved_amqp_frame_codec_callback_context, 0, TEST_CLOSE_PERFORMATIVE, 0, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy_endpoint(endpoint0);
    connection_destroy_endpoint(endpoint1);
    connection_destroy(connection);
}
#endif

/* connection_set_properties */

/* Tests_S_R_S_CONNECTION_01_265: [If connection is NULL, connection_set_properties shall fail and return a non-zero value.] */
TEST_FUNCTION(connection_set_properties_with_NULL_connection_fails)
{
    // arrange

    // act
    int result = connection_set_properties(NULL, TEST_PROPERTIES);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_S_R_S_CONNECTION_01_266: [connection_set_properties shall set the properties associated with a connection.] */
/* Tests_S_R_S_CONNECTION_01_267: [On success connection_set_properties shall return 0.] */
TEST_FUNCTION(connection_set_properties_with_valid_connection_succeeds)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id, NULL, NULL);
    int result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(fields_clone(TEST_PROPERTIES))
        .SetReturn(TEST_CLONED_PROPERTIES);
    // act
    result = connection_set_properties(connection, TEST_PROPERTIES);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* connection_get_properties */

/* Tests_S_R_S_CONNECTION_01_261: [If connection or properties is NULL, connection_properties_timeout shall fail and return a non-zero value.]  */
TEST_FUNCTION(connection_get_properties_with_NULL_connection_fails)
{
    // arrange
    fields properties;

    // act
    int result = connection_get_properties(NULL, &properties);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_S_R_S_CONNECTION_01_261: [If connection or properties is NULL, connection_get_properties shall fail and return a non-zero value.]  */
TEST_FUNCTION(connection_get_properties_with_NULL_properties_argument_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id, NULL, NULL);
    int result;
    umock_c_reset_all_calls();

    // act
    result = connection_get_properties(connection, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_262: [connection_get_properties shall return in the properties argument the current properties setting.] */
/* Tests_S_R_S_CONNECTION_01_263: [On success, connection_get_properties shall return 0.] */
TEST_FUNCTION(connection_get_properties_with_valid_argument_succeeds)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id, NULL, NULL);
    fields properties;
    int result;
    (void)connection_set_properties(connection, TEST_PROPERTIES);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(fields_clone(TEST_CLONED_PROPERTIES))
        .SetReturn(TEST_CLONED_PROPERTIES);
    // act
    result = connection_get_properties(connection, &properties);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, TEST_CLONED_PROPERTIES, properties);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_262: [connection_get_properties shall return in the properties argument the current properties setting.] */
/* Tests_S_R_S_CONNECTION_01_263: [On success, connection_get_properties shall return 0.] */
/* Tests_S_R_S_CONNECTION_01_264: [A value will be NULL if unset.] */
TEST_FUNCTION(connection_get_properties_default_value_succeeds)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create(TEST_IO_HANDLE, "testhost", test_container_id, NULL, NULL);
    fields properties;
    int result;
    umock_c_reset_all_calls();

    // act
    result = connection_get_properties(connection, &properties);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, NULL, properties);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

static void TEST_on_connection_state_changed(void* context, CONNECTION_STATE new_connection_state, CONNECTION_STATE previous_connection_state)
{
    saved_on_connection_state_changed_context = context;
    saved_new_connection_state = new_connection_state;
    saved_previous_connection_state = previous_connection_state;
}

static void TEST_on_io_error(void* context)
{
    saved_on_io_open_complete_context = context;
}

/* Tests_S_R_S_CONNECTION_22_002: [connection_create shall allow registering connections state and io error callbacks.] */
TEST_FUNCTION(connection_create2_with_valid_args_succeeds)
{
    // arrange
    CONNECTION_HANDLE connection;
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(frame_codec_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqp_frame_codec_create(TEST_FRAME_CODEC_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(tickcounter_create());
    STRICT_EXPECTED_CALL(tickcounter_get_current_ms(test_tick_counter, IGNORED_PTR_ARG));

    // act
    connection = connection_create2(TEST_IO_HANDLE, "testhost", test_container_id, NULL, NULL, NULL, TEST_IO_HANDLE, TEST_on_io_error, TEST_CONTEXT);

    // assert
    ASSERT_IS_NOT_NULL(connection);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_07_002: [If connection is NULL then connection_set_trace shall do nothing.] */
TEST_FUNCTION(connection_set_trace_connection_NULL_fail)
{
    // arrange
    bool traceOn = false;

    // act
    connection_set_trace(NULL, traceOn);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_S_R_S_CONNECTION_07_001: [connection_set_trace shall set the ability to turn on and off trace logging.] */
TEST_FUNCTION(connection_set_trace_succeeds)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create2(TEST_IO_HANDLE, "testhost", test_container_id, NULL, NULL, NULL, TEST_IO_HANDLE, TEST_on_io_error, TEST_CONTEXT);
    bool traceOn;
    umock_c_reset_all_calls();

    // act
    traceOn = false;
    connection_set_trace(connection, traceOn);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* connection_subscribe_on_connection_close_received */

/* Tests_S_R_S_CONNECTION_01_275: [ `connection_subscribe_on_connection_close_received` shall register the `on_connection_closed` handler to be triggered whenever a CLOSE performative is received.. ]*/
/* Tests_S_R_S_CONNECTION_01_276: [ On success, `connection_subscribe_on_connection_close_received` shall return a non-NULL handle to the event subcription. ]*/
TEST_FUNCTION(connection_subscribe_on_connection_close_received_succeeds)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create2(TEST_IO_HANDLE, "testhost", test_container_id, NULL, NULL, NULL, TEST_IO_HANDLE, TEST_on_io_error, TEST_CONTEXT);
    ON_CONNECTION_CLOSED_EVENT_SUBSCRIPTION_HANDLE result;
    umock_c_reset_all_calls();

    // act
    result = connection_subscribe_on_connection_close_received(connection, test_on_connection_close_received, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_277: [ If `connection` is NULL, `connection_subscribe_on_connection_close_received` shall fail and return NULL. ]*/
TEST_FUNCTION(connection_subscribe_on_connection_close_received_with_NULL_connection_fails)
{
    // arrange
    ON_CONNECTION_CLOSED_EVENT_SUBSCRIPTION_HANDLE result;

    // act
    result = connection_subscribe_on_connection_close_received(NULL, test_on_connection_close_received, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_S_R_S_CONNECTION_01_278: [ If `on_connection_close_received` is NULL, `connection_subscribe_on_connection_close_received` shall fail and return NULL. ]*/
TEST_FUNCTION(connection_subscribe_on_connection_close_received_with_NULL_callback_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create2(TEST_IO_HANDLE, "testhost", test_container_id, NULL, NULL, NULL, TEST_IO_HANDLE, TEST_on_io_error, TEST_CONTEXT);
    ON_CONNECTION_CLOSED_EVENT_SUBSCRIPTION_HANDLE result;
    umock_c_reset_all_calls();

    // act
    result = connection_subscribe_on_connection_close_received(connection, NULL, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_279: [ `context` shall be allowed to be NULL. ]*/
TEST_FUNCTION(connection_subscribe_on_connection_close_received_with_NULL_context_succeeds)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create2(TEST_IO_HANDLE, "testhost", test_container_id, NULL, NULL, NULL, TEST_IO_HANDLE, TEST_on_io_error, TEST_CONTEXT);
    ON_CONNECTION_CLOSED_EVENT_SUBSCRIPTION_HANDLE result;
    umock_c_reset_all_calls();

    // act
    result = connection_subscribe_on_connection_close_received(connection, test_on_connection_close_received, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_280: [ Only one subscription shall be allowed per connection, if a subsequent second even subscription is done while a subscription is active, `connection_subscribe_on_connection_close_received` shall fail and return NULL. ]*/
TEST_FUNCTION(connection_subscribe_on_connection_close_received_when_already_subscribed_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create2(TEST_IO_HANDLE, "testhost", test_container_id, NULL, NULL, NULL, TEST_IO_HANDLE, TEST_on_io_error, TEST_CONTEXT);
    ON_CONNECTION_CLOSED_EVENT_SUBSCRIPTION_HANDLE result;
    (void)connection_subscribe_on_connection_close_received(connection, test_on_connection_close_received, (void*)0x4242);
    umock_c_reset_all_calls();

    // act
    result = connection_subscribe_on_connection_close_received(connection, test_on_connection_close_received, (void*)0x4243);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_280: [ Only one subscription shall be allowed per connection, if a subsequent second even subscription is done while a subscription is active, `connection_subscribe_on_connection_close_received` shall fail and return NULL. ]*/
TEST_FUNCTION(connection_subscribe_on_connection_close_received_when_already_subscribed_with_same_arguments_fails)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create2(TEST_IO_HANDLE, "testhost", test_container_id, NULL, NULL, NULL, TEST_IO_HANDLE, TEST_on_io_error, TEST_CONTEXT);
    ON_CONNECTION_CLOSED_EVENT_SUBSCRIPTION_HANDLE result;
    (void)connection_subscribe_on_connection_close_received(connection, test_on_connection_close_received, (void*)0x4242);
    umock_c_reset_all_calls();

    // act
    result = connection_subscribe_on_connection_close_received(connection, test_on_connection_close_received, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    connection_destroy(connection);
}

/* connection_unsubscribe_on_connection_close_received */

/* Tests_S_R_S_CONNECTION_01_281: [ `connection_unsubscribe_on_connection_close_received` shall remove the subscription for the connection closed event that was made by calling `connection_subscribe_on_connection_close_received`. ]*/
TEST_FUNCTION(connection_unsubscribe_on_connection_close_received_removes_the_subscription)
{
    // arrange
    CONNECTION_HANDLE connection = connection_create2(TEST_IO_HANDLE, "testhost", test_container_id, NULL, NULL, NULL, TEST_IO_HANDLE, TEST_on_io_error, TEST_CONTEXT);
    ON_CONNECTION_CLOSED_EVENT_SUBSCRIPTION_HANDLE event_subscription = connection_subscribe_on_connection_close_received(connection, test_on_connection_close_received, (void*)0x4242);
    umock_c_reset_all_calls();

    // act
    connection_unsubscribe_on_connection_close_received(event_subscription);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    connection_destroy(connection);
}

/* Tests_S_R_S_CONNECTION_01_282: [ If `event_subscription` is NULL, `connection_unsubscribe_on_connection_close_received` shall return. ]*/
TEST_FUNCTION(connection_unsubscribe_on_connection_close_received_with_NULL_event_subscription_returns)
{
    // arrange

    // act
    connection_unsubscribe_on_connection_close_received(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(connection_ut)
