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
#include "azure_uamqp_c/amqp_definitions.h"
#include "azure_uamqp_c/connection.h"

#undef ENABLE_MOCKS

#include "azure_uamqp_c/session.h"

#define TEST_ENDPOINT_HANDLE            (ENDPOINT_HANDLE)0x4242
#define TEST_DESCRIBED_AMQP_VALUE        (AMQP_VALUE)0x4247
#define TEST_LIST_ITEM_AMQP_VALUE        (AMQP_VALUE)0x4246
#define TEST_DESCRIPTOR_AMQP_VALUE        (AMQP_VALUE)0x4245
#define TEST_CONNECTION_HANDLE            (CONNECTION_HANDLE)0x4248
#define TEST_DELIVERY_QUEUE_HANDLE        (DELIVERY_QUEUE_HANDLE)0x4249
#define TEST_CONTEXT                    (void*)0x4444
#define TEST_ATTACH_PERFORMATIVE        (AMQP_VALUE)0x5000
#define TEST_BEGIN_PERFORMATIVE            (AMQP_VALUE)0x5001

static TRANSFER_HANDLE test_transfer_handle = (TRANSFER_HANDLE)0x6001;
static ON_ENDPOINT_FRAME_RECEIVED saved_frame_received_callback;
static ON_CONNECTION_STATE_CHANGED saved_connection_state_changed_callback;
static void* saved_callback_context;
static uint32_t some_remote_max_frame_size = 512;

static uint64_t performative_ulong;

MOCK_FUNCTION_WITH_CODE(, void, test_frame_received_callback, void*, context, AMQP_VALUE, performative, uint32_t, frame_payload_size, const unsigned char*, payload_bytes)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_on_session_state_changed, void*, context, SESSION_STATE, new_session_state, SESSION_STATE, previous_session_state)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_on_flow_on, void*, context)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_on_send_complete, void*, context, IO_SEND_RESULT, send_result)
MOCK_FUNCTION_END();

static int my_amqpvalue_get_ulong(AMQP_VALUE value, uint64_t* ulong_value)
{
    (void)value;
    *ulong_value = performative_ulong;
    return 0;
}

static int my_connection_start_endpoint(ENDPOINT_HANDLE endpoint, ON_ENDPOINT_FRAME_RECEIVED frame_received_callback, ON_CONNECTION_STATE_CHANGED on_connection_state_changed, void* context)
{
    (void)endpoint;
    saved_frame_received_callback = frame_received_callback;
    saved_connection_state_changed_callback = on_connection_state_changed;
    saved_callback_context = context;
    return 0;
}

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(session_ut)

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
    REGISTER_GLOBAL_MOCK_HOOK(amqpvalue_get_ulong, my_amqpvalue_get_ulong);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_get_uint, 0);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_get_inplace_descriptor, TEST_DESCRIPTOR_AMQP_VALUE);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_get_string, 0);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_get_list_item, TEST_LIST_ITEM_AMQP_VALUE);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_get_inplace_described_value, TEST_DESCRIBED_AMQP_VALUE);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_get_encoded_size, 0);
    REGISTER_GLOBAL_MOCK_RETURN(connection_open, 0);
    REGISTER_GLOBAL_MOCK_RETURN(connection_close, 0);
    REGISTER_GLOBAL_MOCK_RETURN(connection_create_endpoint, TEST_ENDPOINT_HANDLE);
    REGISTER_GLOBAL_MOCK_RETURN(connection_endpoint_get_incoming_channel, 0);
    REGISTER_GLOBAL_MOCK_RETURN(connection_encode_frame, 0);
    REGISTER_GLOBAL_MOCK_RETURN(connection_get_remote_max_frame_size, 0);
    REGISTER_GLOBAL_MOCK_HOOK(connection_start_endpoint, my_connection_start_endpoint);

    REGISTER_UMOCK_ALIAS_TYPE(SESSION_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(CONNECTION_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ENDPOINT_HANDLE, void*);
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
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* session_create */

/* Tests_S_R_S_SESSION_01_030: [session_create shall create a new session instance and return a non-NULL handle to it.] */
/* Tests_S_R_S_SESSION_01_032: [session_create shall create a new session endpoint by calling connection_create_endpoint.] */
TEST_FUNCTION(session_create_with_valid_args_succeeds)
{
    // arrange
    SESSION_HANDLE session;
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(connection_create_endpoint(TEST_CONNECTION_HANDLE));

    // act
    session = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);

    // assert
    ASSERT_IS_NOT_NULL(session);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy(session);
}

/* Tests_S_R_S_SESSION_01_030: [session_create shall create a new session instance and return a non-NULL handle to it.] */
/* Tests_S_R_S_SESSION_01_032: [session_create shall create a new session endpoint by calling connection_create_endpoint.] */
TEST_FUNCTION(session_create_twice_on_the_same_connection_works)
{
    // arrange
    SESSION_HANDLE session1;
    SESSION_HANDLE session2;
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(connection_create_endpoint(TEST_CONNECTION_HANDLE));
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(connection_create_endpoint(TEST_CONNECTION_HANDLE));

    // act
    session1 = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);
    session2 = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);

    // assert
    ASSERT_IS_NOT_NULL(session1);
    ASSERT_IS_NOT_NULL(session2);
    ASSERT_ARE_NOT_EQUAL(void_ptr, session1, session2);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy(session1);
    session_destroy(session2);
}

/* Tests_S_R_S_SESSION_01_031: [If connection is NULL, session_create shall fail and return NULL.] */
TEST_FUNCTION(session_create_with_NULL_connection_fails)
{
    // arrange

    // act
    SESSION_HANDLE session = session_create(NULL, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(session);
}

/* Tests_S_R_S_SESSION_01_042: [If allocating memory for the session fails, session_create shall fail and return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_the_session_fails_session_create_fails)
{
    // arrange
    SESSION_HANDLE session;
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    session = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(session);
}

/* Tests_S_R_S_SESSION_01_033: [If connection_create_endpoint fails, session_create shall fail and return NULL.] */
TEST_FUNCTION(when_connection_create_endpoint_fails_session_create_fails)
{
    // arrange
    SESSION_HANDLE session;
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(connection_create_endpoint(TEST_CONNECTION_HANDLE))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    session = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(session);
}

/* session_destroy */

/* Tests_S_R_S_SESSION_01_034: [session_destroy shall free all resources allocated by session_create.] */
/* Tests_S_R_S_SESSION_01_035: [The endpoint created in session_create shall be freed by calling connection_destroy_endpoint.] */
TEST_FUNCTION(when_session_destroy_is_called_then_the_underlying_endpoint_is_freed)
{
    // arrange
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(connection_destroy_endpoint(TEST_ENDPOINT_HANDLE));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    session_destroy(session);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_S_R_S_SESSION_01_036: [If session is NULL, session_destroy shall do nothing.] */
TEST_FUNCTION(session_destroy_with_NULL_session_does_nothing)
{
    // arrange

    // act
    session_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* session_create_link_endpoint */

/* Tests_S_R_S_SESSION_01_043: [session_create_link_endpoint shall create a link endpoint associated with a given session and return a non-NULL handle to it.] */
/* Tests_S_R_S_SESSION_01_046: [An unused handle shall be assigned to the link endpoint.] */
TEST_FUNCTION(session_create_link_endpoint_creates_a_link_endpoint)
{
    // arrange
    LINK_ENDPOINT_HANDLE link_endpoint;
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    // act
    link_endpoint = session_create_link_endpoint(session, "1");

    // assert
    ASSERT_IS_NOT_NULL(link_endpoint);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy_link_endpoint(link_endpoint);
    session_destroy(session);
}

/* Tests_S_R_S_SESSION_01_044: [If session, name or frame_received_callback is NULL, session_create_link_endpoint shall fail and return NULL.] */
TEST_FUNCTION(session_create_with_NULL_session_fails)
{
    // arrange

    // act
    LINK_ENDPOINT_HANDLE link_endpoint = session_create_link_endpoint(NULL, "1");

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(link_endpoint);
}

/* Tests_S_R_S_SESSION_01_044: [If session, name or frame_received_callback is NULL, session_create_link_endpoint shall fail and return NULL.] */
TEST_FUNCTION(session_create_with_NULL_name_fails)
{
    // arrange
    LINK_ENDPOINT_HANDLE link_endpoint;
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);
    umock_c_reset_all_calls();

    // act
    link_endpoint = session_create_link_endpoint(session, NULL);

    // assert
    ASSERT_IS_NULL(link_endpoint);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy(session);
}

/* Tests_S_R_S_SESSION_01_045: [If allocating memory for the link endpoint fails, session_create_link_endpoint shall fail and return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_the_link_endpoint_fails_then_session_create_link_endpoint_fails)
{
    // arrange
    LINK_ENDPOINT_HANDLE link_endpoint;
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    link_endpoint = session_create_link_endpoint(session, "1");

    // assert
    ASSERT_IS_NULL(link_endpoint);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy(session);
}

/* Tests_S_R_S_SESSION_01_045: [If allocating memory for the link endpoint fails, session_create_link_endpoint shall fail and return NULL.] */
TEST_FUNCTION(when_allocating_the_link_name_fails_then_session_create_link_endpoint_fails)
{
    // arrange
    LINK_ENDPOINT_HANDLE link_endpoint;
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    link_endpoint = session_create_link_endpoint(session, "1");

    // assert
    ASSERT_IS_NULL(link_endpoint);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy(session);
}

/* Tests_S_R_S_SESSION_01_045: [If allocating memory for the link endpoint fails, session_create_link_endpoint shall fail and return NULL.] */
TEST_FUNCTION(when_reallocating_the_endpoint_array_for_the_link_endpoint_fails_then_session_create_link_endpoint_fails)
{
    // arrange
    LINK_ENDPOINT_HANDLE link_endpoint;
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    link_endpoint = session_create_link_endpoint(session, "1");

    // assert
    ASSERT_IS_NULL(link_endpoint);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy(session);
}

/* session_destroy_link_endpoint */

/* Tests_S_R_S_SESSION_01_050: [If link_endpoint is NULL, session_destroy_link_endpoint shall do nothing.] */
TEST_FUNCTION(session_destroy_link_endpoint_with_NULL_handle_does_nothing)
{
    // arrange

    // act
    session_destroy_link_endpoint(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

#if 0
/* Tests_S_R_S_SESSION_01_049: [session_destroy_link_endpoint shall detach the associated endpoint, but not free the resources of the endpoint.] */
TEST_FUNCTION(session_destroy_link_endpoint_frees_the_resources)
{
    // arrange
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);
    LINK_ENDPOINT_HANDLE link_endpoint = session_create_link_endpoint(session, "1");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    session_destroy_link_endpoint(link_endpoint);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy(session);
}

/* Tests_S_R_S_SESSION_01_049: [session_destroy_link_endpoint shall free all resources associated with the endpoint.] */
TEST_FUNCTION(session_destroy_link_endpoint_when_2_endpoints_are_there_frees_the_resources)
{
    // arrange
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);
    LINK_ENDPOINT_HANDLE link_endpoint1 = session_create_link_endpoint(session, "1");
    LINK_ENDPOINT_HANDLE link_endpoint2 = session_create_link_endpoint(session, "1");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    session_destroy_link_endpoint(link_endpoint1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy_link_endpoint(link_endpoint2);
    session_destroy(session);
}

/* session_send_transfer */

/* Tests_S_R_S_SESSION_01_051: [session_send_transfer shall send a transfer frame with the performative indicated in the transfer argument.] */
/* Tests_S_R_S_SESSION_01_053: [On success, session_send_transfer shall return 0.] */
/* Tests_S_R_S_SESSION_01_055: [The encoding of the frame shall be done by calling connection_encode_frame and passing as arguments: the connection handle associated with the session, the transfer performative and the payload chunks passed to session_send_transfer.] */
/* Tests_S_R_S_SESSION_01_057: [The delivery ids shall be assigned starting at 0.] */
TEST_FUNCTION(session_transfer_sends_the_frame_to_the_connection)
{
    // arrange
    int result;
    delivery_number delivery_id;
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE);
    LINK_ENDPOINT_HANDLE link_endpoint = session_create_link_endpoint(session, "1", test_frame_received_callback, test_on_session_state_changed, test_on_flow_on, NULL);
    saved_connection_state_changed_callback(saved_callback_context, CONNECTION_STATE_OPENED, CONNECTION_STATE_OPEN_SENT);
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_BEGIN_PERFORMATIVE));
    STRICT_EXPECTED_CALL(definition_mocks, is_begin_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    saved_frame_received_callback(saved_callback_context, TEST_BEGIN_PERFORMATIVE, 0, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(definition_mocks, transfer_set_delivery_id(test_transfer_handle, 0));
    STRICT_EXPECTED_CALL(definition_mocks, amqpvalue_create_transfer(test_transfer_handle));
    STRICT_EXPECTED_CALL(connection_get_remote_max_frame_size(TEST_CONNECTION_HANDLE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &some_remote_max_frame_size, sizeof(some_remote_max_frame_size));
    STRICT_EXPECTED_CALL(connection_encode_frame(TEST_ENDPOINT_HANDLE, test_transfer_amqp_value, NULL, 0, test_on_send_complete, (void*)0x4242));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_transfer_amqp_value));

    // act
    result = session_send_transfer(link_endpoint, test_transfer_handle, NULL, 0, &delivery_id, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy_link_endpoint(link_endpoint);
    session_destroy(session);
}
#endif

/* Tests_S_R_S_SESSION_01_054: [If link_endpoint or transfer is NULL, session_send_transfer shall fail and return a non-zero value.] */
TEST_FUNCTION(session_transfer_with_NULL_transfer_fails)
{
    // arrange
    int result;
    delivery_number delivery_id;
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);
    LINK_ENDPOINT_HANDLE link_endpoint = session_create_link_endpoint(session, "1");
    umock_c_reset_all_calls();

    // act
    result = session_send_transfer(link_endpoint, NULL, NULL, 0, &delivery_id, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy_link_endpoint(link_endpoint);
    session_destroy(session);
}

/* Tests_S_R_S_SESSION_01_054: [If link_endpoint or transfer is NULL, session_send_transfer shall fail and return a non-zero value.] */
TEST_FUNCTION(session_transfer_with_NULL_link_endpoint_fails)
{
    // arrange

    // act
    delivery_number delivery_id;
    int result = session_send_transfer(NULL, test_transfer_handle, NULL, 0, &delivery_id, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

#if 0
/* Tests_S_R_S_SESSION_01_058: [When any other error occurs, session_send_transfer shall fail and return a non-zero value.] */
TEST_FUNCTION(when_transfer_set_delivery_id_fails_then_session_transfer_fails)
{
    // arrange
    int result;
    delivery_number delivery_id;
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE);
    LINK_ENDPOINT_HANDLE link_endpoint = session_create_link_endpoint(session, "1", test_frame_received_callback, test_on_session_state_changed, test_on_flow_on, NULL);
    saved_connection_state_changed_callback(saved_callback_context, CONNECTION_STATE_OPENED, CONNECTION_STATE_OPEN_SENT);
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_BEGIN_PERFORMATIVE));
    STRICT_EXPECTED_CALL(definition_mocks, is_begin_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    saved_frame_received_callback(saved_callback_context, TEST_BEGIN_PERFORMATIVE, 0, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(definition_mocks, transfer_set_delivery_id(test_transfer_handle, 0))
        .SetReturn(1);

    // act
    result = session_send_transfer(link_endpoint, test_transfer_handle, NULL, 0, &delivery_id, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy_link_endpoint(link_endpoint);
    session_destroy(session);
}

/* Tests_S_R_S_SESSION_01_058: [When any other error occurs, session_send_transfer shall fail and return a non-zero value.] */
TEST_FUNCTION(when_amqpvalue_create_transfer_fails_then_session_transfer_fails)
{
    // arrange
    int result;
    delivery_number delivery_id;
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE);
    LINK_ENDPOINT_HANDLE link_endpoint = session_create_link_endpoint(session, "1", test_frame_received_callback, test_on_session_state_changed, test_on_flow_on, NULL);
    saved_connection_state_changed_callback(saved_callback_context, CONNECTION_STATE_OPENED, CONNECTION_STATE_OPEN_SENT);
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_BEGIN_PERFORMATIVE));
    STRICT_EXPECTED_CALL(definition_mocks, is_begin_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    saved_frame_received_callback(saved_callback_context, TEST_BEGIN_PERFORMATIVE, 0, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(definition_mocks, transfer_set_delivery_id(test_transfer_handle, 0));
    STRICT_EXPECTED_CALL(connection_get_remote_max_frame_size(TEST_CONNECTION_HANDLE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &some_remote_max_frame_size, sizeof(some_remote_max_frame_size));
    STRICT_EXPECTED_CALL(definition_mocks, amqpvalue_create_transfer(test_transfer_handle))
        .SetReturn(NULL);

    // act
    result = session_send_transfer(link_endpoint, test_transfer_handle, NULL, 0, &delivery_id, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy_link_endpoint(link_endpoint);
    session_destroy(session);
}

/* Tests_S_R_S_SESSION_01_056: [If connection_encode_frame fails then session_send_transfer shall fail and return a non-zero value.] */
TEST_FUNCTION(when_connection_encode_frame_fails_then_session_transfer_fails)
{
    // arrange
    int result;
    delivery_number delivery_id;
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE);
    LINK_ENDPOINT_HANDLE link_endpoint = session_create_link_endpoint(session, "1", test_frame_received_callback, test_on_session_state_changed, test_on_flow_on, NULL);
    saved_connection_state_changed_callback(saved_callback_context, CONNECTION_STATE_OPENED, CONNECTION_STATE_OPEN_SENT);
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_BEGIN_PERFORMATIVE));
    STRICT_EXPECTED_CALL(definition_mocks, is_begin_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    saved_frame_received_callback(saved_callback_context, TEST_BEGIN_PERFORMATIVE, 0, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(definition_mocks, transfer_set_delivery_id(test_transfer_handle, 0));
    STRICT_EXPECTED_CALL(connection_get_remote_max_frame_size(TEST_CONNECTION_HANDLE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &some_remote_max_frame_size, sizeof(some_remote_max_frame_size));
    STRICT_EXPECTED_CALL(definition_mocks, amqpvalue_create_transfer(test_transfer_handle));
    STRICT_EXPECTED_CALL(connection_encode_frame(TEST_ENDPOINT_HANDLE, test_transfer_amqp_value, NULL, 0, test_on_send_complete, (void*)0x4242))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_transfer_amqp_value));

    // act
    result = session_send_transfer(link_endpoint, test_transfer_handle, NULL, 0, &delivery_id, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy_link_endpoint(link_endpoint);
    session_destroy(session);
}
#endif

/* Tests_S_R_S_SESSION_01_059: [When session_send_transfer is called while the session is not in the MAPPED state, session_send_transfer shall fail and return a non-zero value.] */
TEST_FUNCTION(when_session_is_not_MAPPED_the_transfer_fails)
{
    // arrange
    int result;
    delivery_number delivery_id;
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);
    LINK_ENDPOINT_HANDLE link_endpoint = session_create_link_endpoint(session, "1");
    umock_c_reset_all_calls();

    // act
    result = session_send_transfer(link_endpoint, test_transfer_handle, NULL, 0, &delivery_id, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy_link_endpoint(link_endpoint);
    session_destroy(session);
}

/* on_connection_state_changed */

#if 0
/* Tests_S_R_S_SESSION_01_060: [If the previous connection state is not OPENED and the new connection state is OPENED, the BEGIN frame shall be sent out and the state shall be switched to BEGIN_SENT.] */
TEST_FUNCTION(connection_state_changed_callback_with_OPENED_triggers_sending_the_BEGIN_frame)
{
    // arrange
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE);
    LINK_ENDPOINT_HANDLE link_endpoint = session_create_link_endpoint(session, "1", test_frame_received_callback, test_on_session_state_changed, test_on_flow_on, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(definition_mocks, begin_create(0, 1, 1));
    STRICT_EXPECTED_CALL(definition_mocks, begin_set_handle_max(test_begin_handle, 4294967295));
    STRICT_EXPECTED_CALL(definition_mocks, amqpvalue_create_begin(test_begin_handle));
    STRICT_EXPECTED_CALL(connection_encode_frame(TEST_ENDPOINT_HANDLE, test_begin_amqp_value, NULL, 0, test_on_send_complete, (void*)0x4242));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_begin_amqp_value));
    STRICT_EXPECTED_CALL(definition_mocks, begin_destroy(test_begin_handle));

    STRICT_EXPECTED_CALL(test_on_session_state_changed(NULL, SESSION_STATE_BEGIN_SENT, SESSION_STATE_UNMAPPED));

    // act
    saved_connection_state_changed_callback(saved_callback_context, CONNECTION_STATE_OPENED, CONNECTION_STATE_OPEN_SENT);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy_link_endpoint(link_endpoint);
    session_destroy(session);
}

/* Tests_S_R_S_SESSION_01_060: [If the previous connection state is not OPENED and the new connection state is OPENED, the BEGIN frame shall be sent out and the state shall be switched to BEGIN_SENT.] */
TEST_FUNCTION(connection_state_changed_callback_and_new_state_is_not_OPENED_does_not_trigger_sending_the_BEGIN_frame)
{
    // arrange
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);
    LINK_ENDPOINT_HANDLE link_endpoint = session_create_link_endpoint(session, "1", test_frame_received_callback, test_on_session_state_changed, test_on_flow_on, NULL);
    umock_c_reset_all_calls();

    // act
    saved_connection_state_changed_callback(saved_callback_context, CONNECTION_STATE_OPEN_SENT, CONNECTION_STATE_START);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy_link_endpoint(link_endpoint);
    session_destroy(session);
}

/* Tests_S_R_S_SESSION_01_060: [If the previous connection state is not OPENED and the new connection state is OPENED, the BEGIN frame shall be sent out and the state shall be switched to BEGIN_SENT.] */
TEST_FUNCTION(connection_state_changed_callback_and_from_OPENED_to_OPENED_does_not_trigger_sending_the_BEGIN_frame)
{
    // arrange
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);
    LINK_ENDPOINT_HANDLE link_endpoint = session_create_link_endpoint(session, "1", test_frame_received_callback, test_on_session_state_changed, test_on_flow_on, NULL);
    umock_c_reset_all_calls();

    // act
    saved_connection_state_changed_callback(saved_callback_context, CONNECTION_STATE_OPENED, CONNECTION_STATE_OPENED);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy_link_endpoint(link_endpoint);
    session_destroy(session);
}

/* Tests_S_R_S_SESSION_01_060: [If the previous connection state is not OPENED and the new connection state is OPENED, the BEGIN frame shall be sent out and the state shall be switched to BEGIN_SENT.] */
TEST_FUNCTION(connection_state_changed_callback_to_OPENED_twice_only_triggers_sending_the_BEGIN_frame_once)
{
    // arrange
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);
    LINK_ENDPOINT_HANDLE link_endpoint = session_create_link_endpoint(session, "1", test_frame_received_callback, test_on_session_state_changed, test_on_flow_on, NULL);
    saved_connection_state_changed_callback(saved_callback_context, CONNECTION_STATE_OPENED, CONNECTION_STATE_OPEN_SENT);
    umock_c_reset_all_calls();

    // act
    saved_connection_state_changed_callback(saved_callback_context, CONNECTION_STATE_OPENED, CONNECTION_STATE_OPEN_SENT);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy_link_endpoint(link_endpoint);
    session_destroy(session);
}

/* Tests_-S_R_S_SESSION_01_061: [If the previous connection state is OPENED and the new connection state is not OPENED anymore, the state shall be switched to DISCARDING.] */
TEST_FUNCTION(connection_state_changed_callback_to_different_than_OPENED_when_in_UNMAPPED_sets_the_session_state_to_END)
{
    // arrange
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);
    LINK_ENDPOINT_HANDLE link_endpoint = session_create_link_endpoint(session, "1", test_frame_received_callback, test_on_session_state_changed, test_on_flow_on, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_session_state_changed(NULL, SESSION_STATE_DISCARDING, SESSION_STATE_UNMAPPED));

    // act
    saved_connection_state_changed_callback(saved_callback_context, CONNECTION_STATE_CLOSE_RCVD, CONNECTION_STATE_OPENED);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy_link_endpoint(link_endpoint);
    session_destroy(session);
}

/* Tests_-S_R_S_SESSION_01_061: [If the previous connection state is OPENED and the new connection state is not OPENED anymore, the state shall be switched to DISCARDING.] */
TEST_FUNCTION(connection_state_changed_callback_to_different_than_OPENED_when_in_BEGIN_SENT_sets_the_session_state_to_END)
{
    // arrange
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);
    LINK_ENDPOINT_HANDLE link_endpoint = session_create_link_endpoint(session, "1", test_frame_received_callback, test_on_session_state_changed, test_on_flow_on, NULL);
    saved_connection_state_changed_callback(saved_callback_context, CONNECTION_STATE_OPENED, CONNECTION_STATE_OPEN_SENT);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_session_state_changed(NULL, SESSION_STATE_DISCARDING, SESSION_STATE_BEGIN_SENT));

    // act
    saved_connection_state_changed_callback(saved_callback_context, CONNECTION_STATE_CLOSE_RCVD, CONNECTION_STATE_OPENED);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy_link_endpoint(link_endpoint);
    session_destroy(session);
}

/* Tests_-S_R_S_SESSION_01_061: [If the previous connection state is OPENED and the new connection state is not OPENED anymore, the state shall be switched to DISCARDING.] */
TEST_FUNCTION(connection_state_changed_callback_to_different_than_OPENED_when_in_MAPPED_sets_the_session_state_to_END)
{
    // arrange
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE, NULL, NULL);
    LINK_ENDPOINT_HANDLE link_endpoint = session_create_link_endpoint(session, "1", test_frame_received_callback, test_on_session_state_changed, test_on_flow_on, NULL);
    saved_connection_state_changed_callback(saved_callback_context, CONNECTION_STATE_OPENED, CONNECTION_STATE_OPEN_SENT);
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_BEGIN_PERFORMATIVE));
    STRICT_EXPECTED_CALL(definition_mocks, is_begin_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    saved_frame_received_callback(saved_callback_context, TEST_BEGIN_PERFORMATIVE, 0, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_session_state_changed(NULL, SESSION_STATE_DISCARDING, SESSION_STATE_MAPPED));

    // act
    saved_connection_state_changed_callback(saved_callback_context, CONNECTION_STATE_CLOSE_RCVD, CONNECTION_STATE_OPENED);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy_link_endpoint(link_endpoint);
    session_destroy(session);
}

/* Session flow control */

/* Tests_S_R_S_SESSION_01_012: [The session endpoint assigns each outgoing transfer frame an implicit transfer-id from a session scoped sequence.] */
/* Tests_S_R_S_SESSION_01_027: [sending a transfer Upon sending a transfer, the sending endpoint will increment its next-outgoing-id] */
/* Tests_S_R_S_SESSION_01_016: [next-outgoing-id The next-outgoing-id is the transfer-id to assign to the next transfer frame.] */
/* Tests_S_R_S_SESSION_01_017: [The nextoutgoing-id MAY be initialized to an arbitrary value ] */
/* Tests_S_R_S_SESSION_01_018: [is incremented after each successive transfer according to RFC-1982 [RFC1982] serial number arithmetic.] */
TEST_FUNCTION(when_2_transfers_happen_on_2_different_endpoints_2_different_delivery_ids_are_assigned)
{
    // arrange
    delivery_number delivery_id0;
    delivery_number delivery_id1;
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE);
    LINK_ENDPOINT_HANDLE link_endpoint0 = session_create_link_endpoint(session, "1", test_frame_received_callback, test_on_session_state_changed, test_on_flow_on, NULL);
    LINK_ENDPOINT_HANDLE link_endpoint1 = session_create_link_endpoint(session, "2", test_frame_received_callback, test_on_session_state_changed, test_on_flow_on, NULL);
    saved_connection_state_changed_callback(saved_callback_context, CONNECTION_STATE_OPENED, CONNECTION_STATE_OPEN_SENT);
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_BEGIN_PERFORMATIVE));
    STRICT_EXPECTED_CALL(definition_mocks, is_begin_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    saved_frame_received_callback(saved_callback_context, TEST_BEGIN_PERFORMATIVE, 0, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(definition_mocks, transfer_set_delivery_id(test_transfer_handle, 0));
    STRICT_EXPECTED_CALL(definition_mocks, amqpvalue_create_transfer(test_transfer_handle));
    STRICT_EXPECTED_CALL(connection_get_remote_max_frame_size(TEST_CONNECTION_HANDLE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &some_remote_max_frame_size, sizeof(some_remote_max_frame_size));
    STRICT_EXPECTED_CALL(connection_encode_frame(TEST_ENDPOINT_HANDLE, test_transfer_amqp_value, NULL, 0, test_on_send_complete, (void*)0x4242));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_transfer_amqp_value));

    STRICT_EXPECTED_CALL(definition_mocks, transfer_set_delivery_id(test_transfer_handle, 1));
    STRICT_EXPECTED_CALL(definition_mocks, amqpvalue_create_transfer(test_transfer_handle));
    STRICT_EXPECTED_CALL(connection_get_remote_max_frame_size(TEST_CONNECTION_HANDLE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &some_remote_max_frame_size, sizeof(some_remote_max_frame_size));
    STRICT_EXPECTED_CALL(connection_encode_frame(TEST_ENDPOINT_HANDLE, test_transfer_amqp_value, NULL, 0, test_on_send_complete, (void*)0x4242));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_transfer_amqp_value));

    // act
    (void)session_send_transfer(link_endpoint0, test_transfer_handle, NULL, 0, &delivery_id0, test_on_send_complete, (void*)0x4242);
    (void)session_send_transfer(link_endpoint0, test_transfer_handle, NULL, 0, &delivery_id1, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(uint32_t, 0, delivery_id0);
    ASSERT_ARE_EQUAL(uint32_t, 1, delivery_id1);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy_link_endpoint(link_endpoint0);
    session_destroy_link_endpoint(link_endpoint1);
    session_destroy(session);
}

/* Tests_S_R_S_SESSION_01_018: [is incremented after each successive transfer according to RFC-1982 [RFC1982] serial number arithmetic.] */
TEST_FUNCTION(when_if_sending_the_frame_to_the_connection_fails_the_next_outgoing_id_is_not_incremented)
{
    // arrange
    delivery_number delivery_id;
    SESSION_HANDLE session = session_create(TEST_CONNECTION_HANDLE);
    LINK_ENDPOINT_HANDLE link_endpoint0 = session_create_link_endpoint(session, "1", test_frame_received_callback, test_on_session_state_changed, test_on_flow_on, NULL);
    LINK_ENDPOINT_HANDLE link_endpoint1 = session_create_link_endpoint(session, "2", test_frame_received_callback, test_on_session_state_changed, test_on_flow_on, NULL);
    saved_connection_state_changed_callback(saved_callback_context, CONNECTION_STATE_OPENED, CONNECTION_STATE_OPEN_SENT);
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_descriptor(TEST_BEGIN_PERFORMATIVE));
    STRICT_EXPECTED_CALL(definition_mocks, is_begin_type_by_descriptor(TEST_DESCRIPTOR_AMQP_VALUE));
    saved_frame_received_callback(saved_callback_context, TEST_BEGIN_PERFORMATIVE, 0, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(definition_mocks, transfer_set_delivery_id(test_transfer_handle, 0));
    STRICT_EXPECTED_CALL(connection_get_remote_max_frame_size(TEST_CONNECTION_HANDLE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &some_remote_max_frame_size, sizeof(some_remote_max_frame_size));
    STRICT_EXPECTED_CALL(definition_mocks, amqpvalue_create_transfer(test_transfer_handle));
    STRICT_EXPECTED_CALL(connection_encode_frame(TEST_ENDPOINT_HANDLE, test_transfer_amqp_value, NULL, 0, test_on_send_complete, (void*)0x4242))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_transfer_amqp_value));

    STRICT_EXPECTED_CALL(definition_mocks, transfer_set_delivery_id(test_transfer_handle, 0));
    STRICT_EXPECTED_CALL(connection_get_remote_max_frame_size(TEST_CONNECTION_HANDLE, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &some_remote_max_frame_size, sizeof(some_remote_max_frame_size));
    STRICT_EXPECTED_CALL(definition_mocks, amqpvalue_create_transfer(test_transfer_handle));
    STRICT_EXPECTED_CALL(connection_encode_frame(TEST_ENDPOINT_HANDLE, test_transfer_amqp_value, NULL, 0, test_on_send_complete, (void*)0x4242));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_transfer_amqp_value));

    // act
    (void)session_send_transfer(link_endpoint0, test_transfer_handle, NULL, 0, &delivery_id, test_on_send_complete, (void*)0x4242);
    (void)session_send_transfer(link_endpoint0, test_transfer_handle, NULL, 0, &delivery_id, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(uint32_t, 0, delivery_id);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    session_destroy_link_endpoint(link_endpoint0);
    session_destroy_link_endpoint(link_endpoint1);
    session_destroy(session);
}
#endif

END_TEST_SUITE(session_ut)
