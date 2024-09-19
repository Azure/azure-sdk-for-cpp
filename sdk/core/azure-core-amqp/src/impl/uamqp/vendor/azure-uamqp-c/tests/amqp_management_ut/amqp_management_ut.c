// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#else
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_bool.h"
#include "umock_c/umocktypes_stdint.h"
#include "umock_c/umock_c_negative_tests.h"

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

static int my_mallocAndStrcpy_s(char** destination, const char* source)
{
    size_t len = strlen(source);
    *destination = (char*)my_gballoc_malloc(len + 1);
    (void)strcpy(*destination, source);
    return 0;
}

#define ENABLE_MOCKS

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_uamqp_c/message.h"
#include "azure_uamqp_c/session.h"
#include "azure_uamqp_c/link.h"
#include "azure_uamqp_c/message_sender.h"
#include "azure_uamqp_c/message_receiver.h"
#include "azure_uamqp_c/messaging.h"
#include "azure_uamqp_c/message.h"
#include "azure_uamqp_c/async_operation.h"
#include "azure_uamqp_c/amqp_definitions_message_id_ulong.h"

#undef ENABLE_MOCKS

#include "azure_uamqp_c/amqp_management.h"

static SESSION_HANDLE test_session_handle = (SESSION_HANDLE)0x4242;
static SINGLYLINKEDLIST_HANDLE test_singlylinkedlist_handle = (SINGLYLINKEDLIST_HANDLE)0x4243;
static AMQP_VALUE test_source_amqp_value = (AMQP_VALUE)0x4244;
static AMQP_VALUE test_target_amqp_value = (AMQP_VALUE)0x4245;
static LINK_HANDLE test_sender_link = (LINK_HANDLE)0x4246;
static LINK_HANDLE test_receiver_link = (LINK_HANDLE)0x4247;
static MESSAGE_SENDER_HANDLE test_message_sender = (MESSAGE_SENDER_HANDLE)0x4248;
static MESSAGE_RECEIVER_HANDLE test_message_receiver = (MESSAGE_RECEIVER_HANDLE)0x424A;
static MESSAGE_HANDLE test_message = (MESSAGE_HANDLE)0x424B;
static MESSAGE_HANDLE test_cloned_message = (MESSAGE_HANDLE)0x424C;
static AMQP_VALUE test_application_properties = (AMQP_VALUE)0x4301;
static AMQP_VALUE test_operation_key = (AMQP_VALUE)0x4302;
static AMQP_VALUE test_operation_value = (AMQP_VALUE)0x4303;
static AMQP_VALUE test_type_key = (AMQP_VALUE)0x4304;
static AMQP_VALUE test_type_value = (AMQP_VALUE)0x4305;
static AMQP_VALUE test_locales_key = (AMQP_VALUE)0x4306;
static AMQP_VALUE test_locales_value = (AMQP_VALUE)0x4307;
static AMQP_VALUE test_message_id_value = (AMQP_VALUE)0x4308;
static AMQP_VALUE test_string_value = (AMQP_VALUE)0x4309;
static AMQP_VALUE test_correlation_id_value = (AMQP_VALUE)0x430A;
static AMQP_VALUE test_application_properties_map = (AMQP_VALUE)0x430B;
static PROPERTIES_HANDLE test_properties = (PROPERTIES_HANDLE)0x430C;
static ASYNC_OPERATION_HANDLE test_send_operation = (ASYNC_OPERATION_HANDLE)0x430D;

static AMQP_VALUE test_status_code_key = (AMQP_VALUE)0x4400;
static AMQP_VALUE test_status_code_value = (AMQP_VALUE)0x4401;
static AMQP_VALUE test_status_description_key = (AMQP_VALUE)0x4402;
static AMQP_VALUE test_status_description_value = (AMQP_VALUE)0x4403;
static AMQP_VALUE test_delivery_accepted = (AMQP_VALUE)0x4500;
static AMQP_VALUE test_delivery_rejected = (AMQP_VALUE)0x4501;
static AMQP_VALUE test_delivery_released = (AMQP_VALUE)0x4502;

#define SIZE_OF_OPERATION_MESSAGE_INSTANCE_STRUCT 64

MOCK_FUNCTION_WITH_CODE(, void, test_amqp_management_open_complete, void*, context, AMQP_MANAGEMENT_OPEN_RESULT, open_result)
MOCK_FUNCTION_END()

static TEST_MUTEX_HANDLE g_testByTest;

#define role_VALUES \
    role_sender,    \
    role_receiver

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

#ifndef __cplusplus
TEST_DEFINE_ENUM_TYPE(role, role_VALUES);
#endif
IMPLEMENT_UMOCK_C_ENUM_TYPE(role, role_VALUES);

TEST_DEFINE_ENUM_TYPE(AMQP_MANAGEMENT_OPEN_RESULT, AMQP_MANAGEMENT_OPEN_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(AMQP_MANAGEMENT_OPEN_RESULT, AMQP_MANAGEMENT_OPEN_RESULT_VALUES);
TEST_DEFINE_ENUM_TYPE(AMQP_MANAGEMENT_EXECUTE_OPERATION_RESULT, AMQP_MANAGEMENT_EXECUTE_OPERATION_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(AMQP_MANAGEMENT_EXECUTE_OPERATION_RESULT, AMQP_MANAGEMENT_EXECUTE_OPERATION_RESULT_VALUES);

MOCK_FUNCTION_WITH_CODE(, void, test_on_amqp_management_open_complete, void*, context, AMQP_MANAGEMENT_OPEN_RESULT, open_result);
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_on_amqp_management_error, void*, context);
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_on_amqp_management_execute_operation_complete, void*, context, AMQP_MANAGEMENT_EXECUTE_OPERATION_RESULT, execute_operation_result, unsigned int, status_code, const char*, status_description, MESSAGE_HANDLE, message)
MOCK_FUNCTION_END();

static const void** list_items = NULL;
static size_t list_item_count = 0;

static LIST_ITEM_HANDLE add_to_list(const void* item)
{
    const void** items = (const void**)my_gballoc_realloc((void*)list_items, (list_item_count + 1) * sizeof(item));
    if (items != NULL)
    {
        list_items = items;
        list_items[list_item_count++] = item;
    }
    return (LIST_ITEM_HANDLE)list_item_count;
}

static int singlylinkedlist_remove_result;
static int my_singlylinkedlist_remove(SINGLYLINKEDLIST_HANDLE list, LIST_ITEM_HANDLE item)
{
    size_t index = (size_t)item - 1;
    (void)list;
    (void)memmove((void*)&list_items[index], &list_items[index + 1], sizeof(const void*) * (list_item_count - index - 1));
    list_item_count--;
    if (list_item_count == 0)
    {
        my_gballoc_free((void*)list_items);
        list_items = NULL;
    }
    return singlylinkedlist_remove_result;
}

static LIST_ITEM_HANDLE my_singlylinkedlist_get_head_item(SINGLYLINKEDLIST_HANDLE list)
{
    LIST_ITEM_HANDLE list_item_handle = NULL;
    (void)list;
    if (list_item_count > 0)
    {
        list_item_handle = (LIST_ITEM_HANDLE)1;
    }
    else
    {
        list_item_handle = NULL;
    }
    return list_item_handle;
}

static LIST_ITEM_HANDLE my_singlylinkedlist_add(SINGLYLINKEDLIST_HANDLE list, const void* item)
{
    (void)list;
    return add_to_list(item);
}

static const void* my_singlylinkedlist_item_get_value(LIST_ITEM_HANDLE item_handle)
{
    return (const void*)list_items[(size_t)item_handle - 1];
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

static LIST_ITEM_HANDLE my_singlylinkedlist_get_next_item(LIST_ITEM_HANDLE list_item)
{
    LIST_ITEM_HANDLE result = NULL;
    if ((size_t)list_item < list_item_count)
    {
        result = (LIST_ITEM_HANDLE)((size_t)((unsigned char*)list_item) + 1);
    }
    else
    {
        result = NULL;
    }
    return result;
}

static int my_singlylinkedlist_remove_if(SINGLYLINKEDLIST_HANDLE list, LIST_CONDITION_FUNCTION condition_function, const void* match_context)
{
    bool continue_processing = true;

    for (size_t index = 0; continue_processing && index < list_item_count; index++)
    {
        if (condition_function(list_items[index], match_context, &continue_processing))
        {
            (void)mock_hook_singlylinkedlist_remove(list, (LIST_ITEM_HANDLE)(index + 1)); // See my_singlylinkedlist_remove to see why.
        }
    }

    return 0;
}


static ON_MESSAGE_SENDER_STATE_CHANGED saved_on_message_sender_state_changed;
static void* saved_on_message_sender_state_changed_context;
static ON_MESSAGE_RECEIVER_STATE_CHANGED saved_on_message_receiver_state_changed;
static void* saved_on_message_receiver_state_changed_context;
static ON_MESSAGE_RECEIVED saved_on_message_received;
static void* saved_on_message_received_context;
static ON_MESSAGE_SEND_COMPLETE saved_on_message_send_complete;
static void* saved_on_message_send_complete_context;
static MESSAGE_SENDER_STATE messagesender_close_on_message_sender_state_changed_new_state;
static MESSAGE_SENDER_STATE messagesender_close_on_message_sender_state_changed_previous_state;

static MESSAGE_SENDER_HANDLE my_messagesender_create(LINK_HANDLE link, ON_MESSAGE_SENDER_STATE_CHANGED on_message_sender_state_changed, void* context)
{
    (void)link;
    saved_on_message_sender_state_changed = on_message_sender_state_changed;
    saved_on_message_sender_state_changed_context = context;
    return test_message_sender;
}

static MESSAGE_RECEIVER_HANDLE my_messagereceiver_create(LINK_HANDLE link, ON_MESSAGE_RECEIVER_STATE_CHANGED on_message_receiver_state_changed, void* context)
{
    (void)link;
    saved_on_message_receiver_state_changed = on_message_receiver_state_changed;
    saved_on_message_receiver_state_changed_context = context;
    return test_message_receiver;
}

static int my_messagereceiver_open(MESSAGE_RECEIVER_HANDLE message_receiver, ON_MESSAGE_RECEIVED on_message_received, void* callback_context)
{
    (void)message_receiver;
    saved_on_message_received = on_message_received;
    saved_on_message_received_context = callback_context;
    return 0;
}

static int my_messagesender_close(MESSAGE_SENDER_HANDLE message_sender)
{
    (void)message_sender;

    if (saved_on_message_sender_state_changed != NULL)
    {
        saved_on_message_sender_state_changed(
            saved_on_message_sender_state_changed_context,
            messagesender_close_on_message_sender_state_changed_new_state,
            messagesender_close_on_message_sender_state_changed_previous_state);
    }

    return 0;
}

static ASYNC_OPERATION_HANDLE my_messagesender_send_async(MESSAGE_SENDER_HANDLE message_sender, MESSAGE_HANDLE message, ON_MESSAGE_SEND_COMPLETE on_message_send_complete, void* callback_context, tickcounter_ms_t timeout)
{
    (void)message_sender;
    (void)message;
    (void)timeout;
    saved_on_message_send_complete = on_message_send_complete;
    saved_on_message_send_complete_context = callback_context;
    return test_send_operation;
}

static void ASYNC_OPERATION_HANDLE_ToString(char* string, size_t bufferSize, ASYNC_OPERATION_HANDLE val)
{
    (void)bufferSize;
    (void)sprintf(string, "%p", val);
}

static int ASYNC_OPERATION_HANDLE_Compare(ASYNC_OPERATION_HANDLE left, ASYNC_OPERATION_HANDLE right)
{
    return left != right;
}

typedef struct ASYNC_OPERATION_CONTEXT_STRUCT_TEST_TAG
{
    ASYNC_OPERATION_CANCEL_HANDLER_FUNC async_operation_cancel_handler;
    unsigned char context[SIZE_OF_OPERATION_MESSAGE_INSTANCE_STRUCT]; // This block of memory will be used in amqp_management.c for the OPERATION_MESSAGE_INSTANCE instance.
} ASYNC_OPERATION_CONTEXT_STRUCT_TEST;

static ASYNC_OPERATION_HANDLE my_async_operation_create(ASYNC_OPERATION_CANCEL_HANDLER_FUNC async_operation_cancel_handler, size_t context_size)
{
    (void)context_size;
    ASYNC_OPERATION_CONTEXT_STRUCT_TEST* result = my_gballoc_malloc(sizeof(ASYNC_OPERATION_CONTEXT_STRUCT_TEST));
    memset(result, 0, sizeof(ASYNC_OPERATION_CONTEXT_STRUCT_TEST));
    result->async_operation_cancel_handler = async_operation_cancel_handler;

    return (ASYNC_OPERATION_HANDLE)result;
}

static void my_async_operation_destroy(ASYNC_OPERATION_HANDLE async_operation)
{
    my_gballoc_free(async_operation);
}

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(amqp_management_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    result = umocktypes_bool_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_TYPE(role, role);
    REGISTER_TYPE(AMQP_MANAGEMENT_OPEN_RESULT, AMQP_MANAGEMENT_OPEN_RESULT);
    REGISTER_TYPE(AMQP_MANAGEMENT_EXECUTE_OPERATION_RESULT, AMQP_MANAGEMENT_EXECUTE_OPERATION_RESULT);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, my_gballoc_calloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_RETURN(singlylinkedlist_create, test_singlylinkedlist_handle);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_get_head_item, my_singlylinkedlist_get_head_item);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_remove, my_singlylinkedlist_remove);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_add, my_singlylinkedlist_add);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_item_get_value, my_singlylinkedlist_item_get_value);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_find, my_singlylinkedlist_find);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_get_next_item, my_singlylinkedlist_get_next_item);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_remove_if, my_singlylinkedlist_remove_if);
    REGISTER_GLOBAL_MOCK_RETURN(messaging_create_source, test_source_amqp_value);
    REGISTER_GLOBAL_MOCK_RETURN(messaging_create_target, test_target_amqp_value);
    REGISTER_GLOBAL_MOCK_HOOK(messagesender_create, my_messagesender_create);
    REGISTER_GLOBAL_MOCK_HOOK(messagesender_close, my_messagesender_close);
    REGISTER_GLOBAL_MOCK_HOOK(messagereceiver_create, my_messagereceiver_create);
    REGISTER_GLOBAL_MOCK_HOOK(messagereceiver_open, my_messagereceiver_open);
    REGISTER_GLOBAL_MOCK_HOOK(messagesender_send_async, my_messagesender_send_async);
    REGISTER_GLOBAL_MOCK_RETURN(link_create, test_sender_link);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_create_message_id_ulong, test_message_id_value);
    REGISTER_GLOBAL_MOCK_RETURN(message_get_application_properties, 0);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_create_string, test_string_value);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_set_map_value, 0);
    REGISTER_GLOBAL_MOCK_RETURN(message_set_application_properties, 0);
    REGISTER_GLOBAL_MOCK_RETURN(message_get_properties, 0);
    REGISTER_GLOBAL_MOCK_RETURN(message_set_properties, 0);
    REGISTER_GLOBAL_MOCK_RETURN(properties_set_message_id, 0);
    REGISTER_GLOBAL_MOCK_RETURN(message_clone, test_cloned_message);
    REGISTER_GLOBAL_MOCK_RETURN(message_create, test_message);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_create_map, test_application_properties);
    REGISTER_GLOBAL_MOCK_RETURN(properties_create, test_properties);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_get_inplace_described_value, test_application_properties_map);
    REGISTER_GLOBAL_MOCK_RETURN(messaging_delivery_accepted, test_delivery_accepted);
    REGISTER_GLOBAL_MOCK_RETURN(messaging_delivery_rejected, test_delivery_rejected);
    REGISTER_GLOBAL_MOCK_RETURN(messaging_delivery_released, test_delivery_released);
    REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_s, my_mallocAndStrcpy_s);
    REGISTER_GLOBAL_MOCK_HOOK(async_operation_create, my_async_operation_create);
    REGISTER_GLOBAL_MOCK_HOOK(async_operation_destroy, my_async_operation_destroy);

    REGISTER_UMOCK_ALIAS_TYPE(AMQP_MANAGEMENT_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SINGLYLINKEDLIST_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SESSION_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(AMQP_VALUE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LINK_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_MESSAGE_SENDER_STATE_CHANGED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_MESSAGE_RECEIVER_STATE_CHANGED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(MESSAGE_SENDER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(MESSAGE_RECEIVER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_MESSAGE_RECEIVED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(MESSAGE_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(PROPERTIES_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_ITEM_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_CONDITION_FUNCTION, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_MESSAGE_SEND_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(message_id_ulong, uint64_t);
    REGISTER_UMOCK_ALIAS_TYPE(ASYNC_OPERATION_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ASYNC_OPERATION_CANCEL_HANDLER_FUNC, void*);

    /* boo, we need uint_fast32_t in umock */
    REGISTER_UMOCK_ALIAS_TYPE(tickcounter_ms_t, unsigned long long);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
}

TEST_FUNCTION_INITIALIZE(test_init)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }

    umock_c_reset_all_calls();
    singlylinkedlist_remove_result = 0;
    messagesender_close_on_message_sender_state_changed_previous_state = MESSAGE_SENDER_STATE_OPEN;
    messagesender_close_on_message_sender_state_changed_new_state = MESSAGE_SENDER_STATE_CLOSING;
}

TEST_FUNCTION_CLEANUP(test_cleanup)
{
    if (list_items != NULL)
    {
        my_gballoc_free((void*)list_items);
        list_items = NULL;
    }
    list_item_count = 0;

    TEST_MUTEX_RELEASE(g_testByTest);
}

/* amqp_management_create */

/* Tests_SRS_AMQP_MANAGEMENT_01_001: [ `amqp_management_create` shall create a new CBS instance and on success return a non-NULL handle to it. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_003: [ `amqp_management_create` shall create a singly linked list for pending operations by calling `singlylinkedlist_create`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_006: [ `amqp_management_create` shall create a sender link by calling `link_create`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_010: [ The `source` argument shall be a value created by calling `messaging_create_source` with `management_node` as argument. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_011: [ The `target` argument shall be a value created by calling `messaging_create_target` with `management_node` as argument. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_015: [ `amqp_management_create` shall create a receiver link by calling `link_create`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_022: [ `amqp_management_create` shall create a message sender by calling `messagesender_create` and passing to it the sender link handle. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_023: [ `amqp_management_create` shall create a message receiver by calling `messagereceiver_create` and passing to it the receiver link handle. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_007: [ The `session` argument shall be set to `session`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_008: [ The `name` argument shall be constructed by concatenating the `management_node` value with `-sender`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_009: [ The `role` argument shall be `role_sender`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_016: [ The `session` argument shall be set to `session`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_017: [ The `name` argument shall be constructed by concatenating the `management_node` value with `-receiver`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_018: [ The `role` argument shall be `role_receiver`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_019: [ The `source` argument shall be the value created by calling `messaging_create_source`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_020: [ The `target` argument shall be the value created by calling `messaging_create_target`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_181: [ `amqp_management_create` shall set the status code key name to be used for parsing the status code to `statusCode`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_182: [ `amqp_management_create` shall set the status description key name to be used for parsing the status description to `statusDescription`. ]*/
TEST_FUNCTION(amqp_management_create_returns_a_valid_handle)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "statusCode"));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "statusDescription"));
    STRICT_EXPECTED_CALL(messaging_create_source("test_node"));
    STRICT_EXPECTED_CALL(messaging_create_target("test_node"));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(link_create(test_session_handle, "test_node-sender", role_sender, test_source_amqp_value, test_target_amqp_value))
        .SetReturn(test_sender_link);
    STRICT_EXPECTED_CALL(link_create(test_session_handle, "test_node-receiver", role_receiver, test_source_amqp_value, test_target_amqp_value))
        .SetReturn(test_receiver_link);
    STRICT_EXPECTED_CALL(messagesender_create(test_sender_link, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messagereceiver_create(test_receiver_link, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(IGNORED_PTR_ARG));

    // act
    amqp_management = amqp_management_create(test_session_handle, "test_node");

    // assert
    ASSERT_IS_NOT_NULL(amqp_management);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_002: [ If `session` or `management_node` is NULL then `amqp_management_create` shall fail and return NULL. ]*/
TEST_FUNCTION(amqp_management_create_with_NULL_session_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    // act
    amqp_management = amqp_management_create(NULL, "test_node");

    // assert
    ASSERT_IS_NULL(amqp_management);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_AMQP_MANAGEMENT_01_002: [ If `session` or `management_node` is NULL then `amqp_management_create` shall fail and return NULL. ]*/
TEST_FUNCTION(amqp_management_create_with_NULL_management_node_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    // act
    amqp_management = amqp_management_create(test_session_handle, NULL);

    // assert
    ASSERT_IS_NULL(amqp_management);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_AMQP_MANAGEMENT_01_030: [ If `management_node` is an empty string, then `amqp_management_create` shall fail and return NULL. ]*/
TEST_FUNCTION(amqp_management_create_with_empty_string_for_management_node_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    // act
    amqp_management = amqp_management_create(test_session_handle, "");

    // assert
    ASSERT_IS_NULL(amqp_management);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_AMQP_MANAGEMENT_01_004: [ If `singlylinkedlist_create` fails, `amqp_management_create` shall fail and return NULL. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_005: [ If allocating memory for the new handle fails, `amqp_management_create` shall fail and return NULL. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_012: [ If `messaging_create_source` fails then `amqp_management_create` shall fail and return NULL. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_013: [ If `messaging_create_target` fails then `amqp_management_create` shall fail and return NULL. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_014: [ If `link_create` fails when creating the sender link then `amqp_management_create` shall fail and return NULL. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_021: [ If `link_create` fails when creating the receiver link then `amqp_management_create` shall fail and return NULL. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_031: [ If `messagesender_create` fails then `amqp_management_create` shall fail and return NULL. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_032: [ If `messagereceiver_create` fails then `amqp_management_create` shall fail and return NULL. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_033: [ If any other error occurs `amqp_management_create` shall fail and return NULL. ]*/
TEST_FUNCTION(when_any_underlying_function_call_fails_amqp_management_create_fails)
{
    // arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    size_t count;
    size_t index;
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(singlylinkedlist_create())
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "statusCode"))
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "statusDescription"))
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(messaging_create_source("test_node"))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(messaging_create_target("test_node"))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(link_create(test_session_handle, "test_node-sender", role_sender, test_source_amqp_value, test_target_amqp_value))
        .SetReturn(test_sender_link)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(link_create(test_session_handle, "test_node-receiver", role_receiver, test_source_amqp_value, test_target_amqp_value))
        .SetReturn(test_receiver_link)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(messagesender_create(test_sender_link, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(messagereceiver_create(test_receiver_link, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetFailReturn(NULL);
    umock_c_negative_tests_snapshot();

    count = umock_c_negative_tests_call_count();
    for (index = 0; index < count; index++)
    {
        char tmp_msg[128];
        AMQP_MANAGEMENT_HANDLE amqp_management;
        (void)sprintf(tmp_msg, "Failure in test %u/%u", (unsigned int)(index + 1), (unsigned int)count);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        // act
        amqp_management = amqp_management_create(test_session_handle, "test_node");

        // assert
        ASSERT_IS_NULL(amqp_management, tmp_msg);
    }

    // cleanup
    umock_c_negative_tests_deinit();
}

/* amqp_management_destroy */

/* Tests_SRS_AMQP_MANAGEMENT_01_024: [ `amqp_management_destroy` shall free all the resources allocated by `amqp_management_create`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_026: [ `amqp_management_destroy` shall free the singly linked list by calling `singlylinkedlist_destroy`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_027: [ `amqp_management_destroy` shall free the sender and receiver links by calling `link_destroy`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_028: [ `amqp_management_destroy` shall free the message sender by calling `messagesender_destroy`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_029: [ `amqp_management_destroy` shall free the message receiver by calling `messagereceiver_destroy`. ]*/
TEST_FUNCTION(amqp_management_destroy_frees_all_the_allocated_resources)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "statusCode"));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "statusDescription"));
    STRICT_EXPECTED_CALL(messaging_create_source("test_node"));
    STRICT_EXPECTED_CALL(messaging_create_target("test_node"));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(link_create(test_session_handle, "test_node-sender", role_sender, test_source_amqp_value, test_target_amqp_value))
        .SetReturn(test_sender_link);
    STRICT_EXPECTED_CALL(link_create(test_session_handle, "test_node-receiver", role_receiver, test_source_amqp_value, test_target_amqp_value))
        .SetReturn(test_receiver_link);
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(messagesender_destroy(test_message_sender));
    STRICT_EXPECTED_CALL(messagereceiver_destroy(test_message_receiver));
    STRICT_EXPECTED_CALL(link_destroy(test_sender_link));
    STRICT_EXPECTED_CALL(link_destroy(test_receiver_link));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // status description key name
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // status code key name
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(test_singlylinkedlist_handle));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    amqp_management_destroy(amqp_management);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_AMQP_MANAGEMENT_01_025: [ If `amqp_management` is NULL, `amqp_management_destroy` shall do nothing. ]*/
TEST_FUNCTION(amqp_management_destroy_with_NULL_handle_does_nothing)
{
    // arrange

    // act
    amqp_management_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* amqp_management_open_async */

/* Tests_SRS_AMQP_MANAGEMENT_01_036: [ `amqp_management_open_async` shall start opening the AMQP management instance and save the callbacks so that they can be called when opening is complete. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_037: [ On success it shall return 0. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_039: [ `amqp_management_open_async` shall open the message sender by calling `messagesender_open`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_040: [ `amqp_management_open_async` shall open the message receiver by calling `messagereceiver_open`. ]*/
TEST_FUNCTION(amqp_management_open_async_opens_the_message_sender_and_message_receiver)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(messagereceiver_open(test_message_receiver, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messagesender_open(test_message_sender));

    // act
    result = amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_044: [ `on_amqp_management_open_complete_context` and `on_amqp_management_error_context` shall be allowed to be NULL. ]*/
TEST_FUNCTION(amqp_management_open_async_with_NULL_context_arguments_opens_the_message_sender_and_message_receiver)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(messagereceiver_open(test_message_receiver, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messagesender_open(test_message_sender));

    // act
    result = amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, NULL, test_on_amqp_management_error, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_042: [ If `messagereceiver_open` fails, `amqp_management_open_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_opening_the_receiver_fails_amqp_management_open_async_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(messagereceiver_open(test_message_receiver, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1);

    // act
    result = amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_041: [ If `messagesender_open` fails, `amqp_management_open_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_opening_the_sender_fails_amqp_management_open_async_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(messagereceiver_open(test_message_receiver, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messagesender_open(test_message_sender))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(messagereceiver_close(test_message_receiver));

    // act
    result = amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_038: [ If `amqp_management`, `on_amqp_management_open_complete` or `on_amqp_management_error` is NULL, `amqp_management_open_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(amqp_management_open_async_with_NULL_handle_fails)
{
    // arrange
    int result;

    // act
    result = amqp_management_open_async(NULL, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_AMQP_MANAGEMENT_01_038: [ If `amqp_management`, `on_amqp_management_open_complete` or `on_amqp_management_error` is NULL, `amqp_management_open_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(amqp_management_open_async_with_NULL_open_complete_callback_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    umock_c_reset_all_calls();

    // act
    result = amqp_management_open_async(amqp_management, NULL, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_038: [ If `amqp_management`, `on_amqp_management_open_complete` or `on_amqp_management_error` is NULL, `amqp_management_open_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(amqp_management_open_async_with_NULL_error_complete_callback_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    umock_c_reset_all_calls();

    // act
    result = amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, NULL, (void*)0x4243);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_043: [ If the AMQP management instance is already OPEN or OPENING, `amqp_management_open_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(amqp_management_open_async_when_opening_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    umock_c_reset_all_calls();

    // act
    result = amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

static void setup_calls_for_pending_operation_with_correlation_id(uint64_t correlation_id)
{
    STRICT_EXPECTED_CALL(message_clone(test_message));
    STRICT_EXPECTED_CALL(message_get_application_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("operation"))
        .SetReturn(test_operation_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_operation"))
        .SetReturn(test_operation_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_operation_key, test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("type"))
        .SetReturn(test_type_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_type"))
        .SetReturn(test_type_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_type_key, test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("locales"))
        .SetReturn(test_locales_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("en-US"))
        .SetReturn(test_locales_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_locales_key, test_locales_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_cloned_message, test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_create_message_id_ulong(correlation_id));
}

/* amqp_management_close */

/* Tests_SRS_AMQP_MANAGEMENT_01_045: [ `amqp_management_close` shall close the AMQP management instance. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_046: [ On success it shall return 0. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_050: [ `amqp_management_close` shall close the message sender by calling `messagesender_close`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_051: [ `amqp_management_close` shall close the message receiver by calling `messagereceiver_close`. ]*/
TEST_FUNCTION(amqp_management_close_closes_the_message_sender_and_message_receiver)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(messagesender_close(test_message_sender));
    STRICT_EXPECTED_CALL(messagereceiver_close(test_message_receiver));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));

    // act
    result = amqp_management_close(amqp_management);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_047: [ If `amqp_management` is NULL, `amqp_management_close` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(amqp_management_close_with_NULL_handle_fails)
{
    // arrange
    int result;

    // act
    result = amqp_management_close(NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_AMQP_MANAGEMENT_01_052: [ If `messagesender_close` fails, `amqp_management_close` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_closing_the_sender_fails_amqp_management_close_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(messagesender_close(test_message_sender))
        .SetReturn(1);

    // act
    result = amqp_management_close(amqp_management);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_053: [ If `messagereceiver_close` fails, `amqp_management_close` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_closing_the_receiver_fails_amqp_management_close_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(messagesender_close(test_message_sender));
    STRICT_EXPECTED_CALL(messagereceiver_close(test_message_receiver))
        .SetReturn(1);

    // act
    result = amqp_management_close(amqp_management);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_048: [ `amqp_management_close` on an AMQP management instance that is OPENING shall trigger the `on_amqp_management_open_complete` callback with `AMQP_MANAGEMENT_OPEN_CANCELLED`, while also passing the context passed in `amqp_management_open_async`. ]*/
TEST_FUNCTION(amqp_management_close_when_opening_indicates_an_open_complete_with_CANCELLED)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_amqp_management_open_complete((void*)0x4242, AMQP_MANAGEMENT_OPEN_CANCELLED));
    STRICT_EXPECTED_CALL(messagesender_close(test_message_sender));
    STRICT_EXPECTED_CALL(messagereceiver_close(test_message_receiver));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));

    // act
    result = amqp_management_close(amqp_management);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_049: [ `amqp_management_close` on an AMQP management instance that is not OPEN, shall fail and return a non-zero value. ]*/
TEST_FUNCTION(amqp_management_close_when_not_OPEN_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    umock_c_reset_all_calls();

    // act
    result = amqp_management_close(amqp_management);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_054: [ All pending operations shall be indicated complete with the code `AMQP_MANAGEMENT_EXECUTE_OPERATION_INSTANCE_CLOSED`. ]*/
TEST_FUNCTION(amqp_management_close_indicates_pending_operations_as_error_due_to_instance_close)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(1);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(messagesender_close(test_message_sender));
    STRICT_EXPECTED_CALL(messagereceiver_close(test_message_receiver));

    // first pending operation
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_amqp_management_execute_operation_complete((void*)0x4244, AMQP_MANAGEMENT_EXECUTE_OPERATION_INSTANCE_CLOSED, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist_handle, IGNORED_PTR_ARG));

    // second pending operation
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_amqp_management_execute_operation_complete((void*)0x4245, AMQP_MANAGEMENT_EXECUTE_OPERATION_INSTANCE_CLOSED, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));

    // act
    result = amqp_management_close(amqp_management);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_054: [ All pending operations shall be indicated complete with the code `AMQP_MANAGEMENT_EXECUTE_OPERATION_INSTANCE_CLOSED`. ]*/
TEST_FUNCTION(when_removing_the_pending_operation_fails_the_instance_is_still_closed)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    singlylinkedlist_remove_result = 1;

    STRICT_EXPECTED_CALL(messagesender_close(test_message_sender));
    STRICT_EXPECTED_CALL(messagereceiver_close(test_message_receiver));

    // first pending operation
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_amqp_management_execute_operation_complete((void*)0x4244, AMQP_MANAGEMENT_EXECUTE_OPERATION_INSTANCE_CLOSED, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));

    // act
    result = amqp_management_close(amqp_management);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    singlylinkedlist_remove_result = 0;

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* amqp_management_execute_operation_async */

/* Tests_SRS_AMQP_MANAGEMENT_01_055: [ `amqp_management_execute_operation_async` shall start an AMQP management operation. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_056: [ On success it shall return 0. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_103: [ Otherwise the existing message shall be cloned by using `message_clone` before being modified accordingly and used for the pending operation. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_082: [ `amqp_management_execute_operation_async` shall obtain the application properties from the message by calling `message_get_application_properties`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_084: [ For each of the arguments `operation`, `type` and `locales` an AMQP value of type string shall be created by calling `amqpvalue_create_string` in order to be used as key in the application properties map. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_085: [ For each of the arguments `operation`, `type` and `locales` an AMQP value of type string containing the argument value shall be created by calling `amqpvalue_create_string` in order to be used as value in the application properties map. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_086: [ The key/value pairs for `operation`, `type` and `locales` shall be added to the application properties map by calling `amqpvalue_set_map_value`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_087: [ The application properties obtained after adding the key/value pairs shall be set on the message by calling `message_set_application_properties`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_088: [ `amqp_management_execute_operation_async` shall send the message by calling `messagesender_send_async`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_094: [ In order to set the message Id on the message, the properties shall be obtained by calling `message_get_properties`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_095: [ A message Id with the next ulong value to be used shall be created by calling `amqpvalue_create_message_id_ulong`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_096: [ The message Id value shall be set on the properties by calling `properties_set_message_id`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_097: [ The properties thus modified to contain the message Id shall be set on the message by calling `message_set_properties`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_100: [ After setting the properties, the properties instance shall be freed by `properties_destroy`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_091: [ Once the request message has been sent, an entry shall be stored in the pending operations list by calling `singlylinkedlist_add`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_101: [ After setting the application properties, the application properties instance shall be freed by `amqpvalue_destroy`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_106: [ The message Id set on the message properties shall start at 0. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_058: [ Request messages have the following application-properties: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_059: [ operation string Yes The management operation to be performed. ] */
/* Tests_SRS_AMQP_MANAGEMENT_01_061: [ type string Yes The Manageable Entity Type of the Manageable Entity to be managed. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_063: [ locales string No A list of locales that the sending peer permits for incoming informational text in response messages. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_166: [ The `on_message_send_complete` callback shall be passed to the `messagesender_send_async` call. ]*/
TEST_FUNCTION(amqp_management_execute_operation_async_starts_the_operation)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    ASYNC_OPERATION_HANDLE result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_clone(test_message));
    STRICT_EXPECTED_CALL(message_get_application_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("operation"))
        .SetReturn(test_operation_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_operation"))
        .SetReturn(test_operation_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_operation_key, test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("type"))
        .SetReturn(test_type_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_type"))
        .SetReturn(test_type_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_type_key, test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("locales"))
        .SetReturn(test_locales_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("en-US"))
        .SetReturn(test_locales_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_locales_key, test_locales_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_cloned_message, test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_create_message_id_ulong(0));
    STRICT_EXPECTED_CALL(properties_set_message_id(test_properties, test_message_id_value));
    STRICT_EXPECTED_CALL(message_set_properties(test_cloned_message, test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_message_id_value));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messagesender_send_async(test_message_sender, test_cloned_message, IGNORED_PTR_ARG, IGNORED_PTR_ARG, 0));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));
    STRICT_EXPECTED_CALL(message_destroy(test_cloned_message));

    // act
    result = amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_09_004: [ The `ASYNC_OPERATION_HANDLE` cancel function shall cancel the underlying send async operation, remove this operation from the pending list, destroy this async operation. ] */
TEST_FUNCTION(when_amqp_management_execute_operation_async_is_cancelled_success)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    ASYNC_OPERATION_HANDLE execute_result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_clone(test_message));
    STRICT_EXPECTED_CALL(message_get_application_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("operation"))
        .SetReturn(test_operation_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_operation"))
        .SetReturn(test_operation_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_operation_key, test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("type"))
        .SetReturn(test_type_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_type"))
        .SetReturn(test_type_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_type_key, test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("locales"))
        .SetReturn(test_locales_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("en-US"))
        .SetReturn(test_locales_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_locales_key, test_locales_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_cloned_message, test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_create_message_id_ulong(0));
    STRICT_EXPECTED_CALL(properties_set_message_id(test_properties, test_message_id_value));
    STRICT_EXPECTED_CALL(message_set_properties(test_cloned_message, test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_message_id_value));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messagesender_send_async(test_message_sender, test_cloned_message, IGNORED_PTR_ARG, IGNORED_PTR_ARG, 0));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));
    STRICT_EXPECTED_CALL(message_destroy(test_cloned_message));
    execute_result = amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(async_operation_cancel(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(async_operation_destroy(execute_result));

    // act
    ((ASYNC_OPERATION_CONTEXT_STRUCT_TEST*)execute_result)->async_operation_cancel_handler(execute_result);
    
    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_105: [ `on_execute_operation_complete_context` shall be allowed to be NULL. ]*/
TEST_FUNCTION(amqp_management_execute_operation_async_with_NULL_context_starts_the_operation)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    ASYNC_OPERATION_HANDLE result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_clone(test_message));
    STRICT_EXPECTED_CALL(message_get_application_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("operation"))
        .SetReturn(test_operation_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_operation"))
        .SetReturn(test_operation_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_operation_key, test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("type"))
        .SetReturn(test_type_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_type"))
        .SetReturn(test_type_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_type_key, test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("locales"))
        .SetReturn(test_locales_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("en-US"))
        .SetReturn(test_locales_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_locales_key, test_locales_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_cloned_message, test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_create_message_id_ulong(0));
    STRICT_EXPECTED_CALL(properties_set_message_id(test_properties, test_message_id_value));
    STRICT_EXPECTED_CALL(message_set_properties(test_cloned_message, test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_message_id_value));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messagesender_send_async(test_message_sender, test_cloned_message, IGNORED_PTR_ARG, IGNORED_PTR_ARG, 0));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));
    STRICT_EXPECTED_CALL(message_destroy(test_cloned_message));

    // act
    result = amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_057: [ If `amqp_management`, `operation`, `type` or `on_execute_operation_complete` is NULL, `amqp_management_execute_operation_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(amqp_management_execute_operation_async_with_NULL_amqp_management_handle_fails)
{
    // arrange
    ASYNC_OPERATION_HANDLE result;

    // act
    result = amqp_management_execute_operation_async(NULL, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_AMQP_MANAGEMENT_01_057: [ If `amqp_management`, `operation`, `type` or `on_execute_operation_complete` is NULL, `amqp_management_execute_operation_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(amqp_management_execute_operation_async_with_NULL_operation_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    ASYNC_OPERATION_HANDLE result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    // act
    result = amqp_management_execute_operation_async(amqp_management, NULL, "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_057: [ If `amqp_management`, `operation`, `type` or `on_execute_operation_complete` is NULL, `amqp_management_execute_operation_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(amqp_management_execute_operation_async_with_NULL_type_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    ASYNC_OPERATION_HANDLE result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    // act
    result = amqp_management_execute_operation_async(amqp_management, "some_operation", NULL, "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_057: [ If `amqp_management`, `operation`, `type` or `on_execute_operation_complete` is NULL, `amqp_management_execute_operation_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(amqp_management_execute_operation_async_with_NULL_on_execute_operation_complete_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    ASYNC_OPERATION_HANDLE result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    // act
    result = amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, NULL, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_102: [ If `message` is NULL, a new message shall be created by calling `message_create`. ]*/
TEST_FUNCTION(amqp_management_execute_operation_async_with_NULL_message_creates_a_new_message)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    ASYNC_OPERATION_HANDLE result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_create());
    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("operation"))
        .SetReturn(test_operation_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_operation"))
        .SetReturn(test_operation_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_operation_key, test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("type"))
        .SetReturn(test_type_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_type"))
        .SetReturn(test_type_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_type_key, test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("locales"))
        .SetReturn(test_locales_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("en-US"))
        .SetReturn(test_locales_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_locales_key, test_locales_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_message, test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_create_message_id_ulong(0));
    STRICT_EXPECTED_CALL(properties_set_message_id(test_properties, test_message_id_value));
    STRICT_EXPECTED_CALL(message_set_properties(test_message, test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_message_id_value));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messagesender_send_async(test_message_sender, test_message, IGNORED_PTR_ARG, IGNORED_PTR_ARG, 0));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));
    STRICT_EXPECTED_CALL(message_destroy(test_message));

    // act
    result = amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", NULL, test_on_amqp_management_execute_operation_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_081: [ If `amqp_management_execute_operation_async` is called when not OPEN, it shall fail and return a non-zero value. ]*/
TEST_FUNCTION(amqp_management_execute_operation_async_when_not_open_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    ASYNC_OPERATION_HANDLE result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    umock_c_reset_all_calls();

    // act
    result = amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_081: [ If `amqp_management_execute_operation_async` is called when not OPEN, it shall fail and return a non-zero value. ]*/
TEST_FUNCTION(amqp_management_execute_operation_async_after_close_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    ASYNC_OPERATION_HANDLE result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    (void)amqp_management_close(amqp_management);
    umock_c_reset_all_calls();

    // act
    result = amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_104: [ If `amqp_management_execute_operation_async` is called when the AMQP management is in error, it shall fail and return a non-zero value. ]*/
TEST_FUNCTION(amqp_management_execute_operation_async_when_in_error_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    ASYNC_OPERATION_HANDLE result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_ERROR, MESSAGE_RECEIVER_STATE_OPEN);
    umock_c_reset_all_calls();

    // act
    result = amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_083: [ If no application properties were set on the message, a new application properties instance shall be created by calling `amqpvalue_create_map`; ]*/
TEST_FUNCTION(when_no_application_properties_were_set_on_the_message_a_new_map_is_created)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE NULL_test_application_properties = NULL;
    ASYNC_OPERATION_HANDLE result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_clone(test_message));
    STRICT_EXPECTED_CALL(message_get_application_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&NULL_test_application_properties, sizeof(NULL_test_application_properties));
    STRICT_EXPECTED_CALL(amqpvalue_create_map())
        .SetReturn(test_application_properties);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("operation"))
        .SetReturn(test_operation_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_operation"))
        .SetReturn(test_operation_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_operation_key, test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("type"))
        .SetReturn(test_type_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_type"))
        .SetReturn(test_type_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_type_key, test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("locales"))
        .SetReturn(test_locales_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("en-US"))
        .SetReturn(test_locales_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_locales_key, test_locales_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_cloned_message, test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_create_message_id_ulong(0));
    STRICT_EXPECTED_CALL(properties_set_message_id(test_properties, test_message_id_value));
    STRICT_EXPECTED_CALL(message_set_properties(test_cloned_message, test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_message_id_value));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messagesender_send_async(test_message_sender, test_cloned_message, IGNORED_PTR_ARG, IGNORED_PTR_ARG, 0));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));
    STRICT_EXPECTED_CALL(message_destroy(test_cloned_message));

    // act
    result = amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_093: [ If `locales` is NULL, no key/value pair shall be added for it in the application properties map. ]*/
TEST_FUNCTION(amqp_management_execute_operation_async_with_NULL_locales_does_not_add_the_locales_to_the_application_properties)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    ASYNC_OPERATION_HANDLE result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_clone(test_message));
    STRICT_EXPECTED_CALL(message_get_application_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("operation"))
        .SetReturn(test_operation_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_operation"))
        .SetReturn(test_operation_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_operation_key, test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("type"))
        .SetReturn(test_type_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_type"))
        .SetReturn(test_type_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_type_key, test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_cloned_message, test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_create_message_id_ulong(0));
    STRICT_EXPECTED_CALL(properties_set_message_id(test_properties, test_message_id_value));
    STRICT_EXPECTED_CALL(message_set_properties(test_cloned_message, test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_message_id_value));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messagesender_send_async(test_message_sender, test_cloned_message, IGNORED_PTR_ARG, IGNORED_PTR_ARG, 0));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));
    STRICT_EXPECTED_CALL(message_destroy(test_cloned_message));

    // act
    result = amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", NULL, test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_099: [ If the properties were not set on the message, a new properties instance shall be created by calling `properties_create`. ]*/
TEST_FUNCTION(when_no_properties_were_set_on_the_message_a_new_properties_instance_is_created)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    PROPERTIES_HANDLE NULL_test_properties = NULL;
    ASYNC_OPERATION_HANDLE result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_clone(test_message));
    STRICT_EXPECTED_CALL(message_get_application_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("operation"))
        .SetReturn(test_operation_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_operation"))
        .SetReturn(test_operation_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_operation_key, test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("type"))
        .SetReturn(test_type_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_type"))
        .SetReturn(test_type_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_type_key, test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("locales"))
        .SetReturn(test_locales_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("en-US"))
        .SetReturn(test_locales_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_locales_key, test_locales_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_cloned_message, test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&NULL_test_properties, sizeof(NULL_test_properties));
    STRICT_EXPECTED_CALL(properties_create());
    STRICT_EXPECTED_CALL(amqpvalue_create_message_id_ulong(0));
    STRICT_EXPECTED_CALL(properties_set_message_id(test_properties, test_message_id_value));
    STRICT_EXPECTED_CALL(message_set_properties(test_cloned_message, test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_message_id_value));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messagesender_send_async(test_message_sender, test_cloned_message, IGNORED_PTR_ARG, IGNORED_PTR_ARG, 0));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));
    STRICT_EXPECTED_CALL(message_destroy(test_cloned_message));

    // act
    result = amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_090: [ If any APIs used to create and set the application properties on the message fails, `amqp_management_execute_operation_async` shall fail and return a non-zero value. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_098: [ If any API fails while setting the message Id, `amqp_management_execute_operation_async` shall fail and return a non-zero value. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_092: [ If `singlylinkedlist_add` fails then `amqp_management_execute_operation_async` shall fail and return a non-zero value. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_089: [ If `messagesender_send_async` fails, `amqp_management_execute_operation_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_any_underlying_function_call_fails_amqp_management_execute_operation_async_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    size_t count;
    size_t index;
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_clone(test_message))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(message_get_application_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties))
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("operation"))
        .SetReturn(test_operation_key).SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_operation"))
        .SetReturn(test_operation_value).SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_operation_key, test_operation_value))
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("type"))
        .SetReturn(test_type_key).SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_type"))
        .SetReturn(test_type_value).SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_type_key, test_type_value))
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("locales"))
        .SetReturn(test_locales_key).SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("en-US"))
        .SetReturn(test_locales_value).SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_locales_key, test_locales_value))
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_cloned_message, test_application_properties))
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(message_get_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties))
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_create_message_id_ulong(0))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(properties_set_message_id(test_properties, test_message_id_value))
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(message_set_properties(test_cloned_message, test_properties))
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_message_id_value));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinkedlist_handle, IGNORED_PTR_ARG))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(messagesender_send_async(test_message_sender, test_cloned_message, IGNORED_PTR_ARG, IGNORED_PTR_ARG, 0))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));
    STRICT_EXPECTED_CALL(message_destroy(test_cloned_message));

    umock_c_negative_tests_snapshot();

    count = umock_c_negative_tests_call_count();
    for (index = 0; index < count - 2; index++)
    {
        char tmp_msg[128];
        ASYNC_OPERATION_HANDLE result;

        if ((index == 5) ||
            (index == 6) ||
            (index == 10) ||
            (index == 11) ||
            (index == 15) ||
            (index == 16) ||
            (index == 22) ||
            (index == 23))
        {
            continue;
        }

        (void)sprintf(tmp_msg, "Failure in test %u/%u", (unsigned int)(index + 1), (unsigned int)count);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        // act
        result = amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);

        // assert
        ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result, tmp_msg);
    }

    // cleanup
    amqp_management_destroy(amqp_management);
    umock_c_negative_tests_deinit();
}

/* Tests_SRS_AMQP_MANAGEMENT_01_090: [ If any APIs used to create and set the application properties on the message fails, `amqp_management_execute_operation_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_creating_the_application_properties_fails_amqp_management_execute_operation_async_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    ASYNC_OPERATION_HANDLE result;
    AMQP_VALUE NULL_test_application_properties = NULL;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_clone(test_message));
    STRICT_EXPECTED_CALL(message_get_application_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&NULL_test_application_properties, sizeof(NULL_test_application_properties));
    STRICT_EXPECTED_CALL(amqpvalue_create_map())
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(message_destroy(test_cloned_message));

    // act
    result = amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_090: [ If any APIs used to create and set the application properties on the message fails, `amqp_management_execute_operation_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_creating_the_properties_fails_amqp_management_execute_operation_async_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    PROPERTIES_HANDLE NULL_test_properties = NULL;
    ASYNC_OPERATION_HANDLE result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_clone(test_message));
    STRICT_EXPECTED_CALL(message_get_application_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("operation"))
        .SetReturn(test_operation_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_operation"))
        .SetReturn(test_operation_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_operation_key, test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("type"))
        .SetReturn(test_type_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_type"))
        .SetReturn(test_type_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_type_key, test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("locales"))
        .SetReturn(test_locales_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("en-US"))
        .SetReturn(test_locales_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_locales_key, test_locales_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_cloned_message, test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&NULL_test_properties, sizeof(NULL_test_properties));
    STRICT_EXPECTED_CALL(properties_create())
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));
    STRICT_EXPECTED_CALL(message_destroy(test_cloned_message));

    // act
    result = amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_107: [ The message Id set on the message properties shall be incremented with each operation. ]*/
TEST_FUNCTION(amqp_management_execute_operation_async_the_2nd_time_uses_the_next_message_id)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    ASYNC_OPERATION_HANDLE result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_clone(test_message));
    STRICT_EXPECTED_CALL(message_get_application_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("operation"))
        .SetReturn(test_operation_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_operation"))
        .SetReturn(test_operation_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_operation_key, test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_operation_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("type"))
        .SetReturn(test_type_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("some_type"))
        .SetReturn(test_type_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_type_key, test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_type_key));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("locales"))
        .SetReturn(test_locales_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("en-US"))
        .SetReturn(test_locales_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_application_properties, test_locales_key, test_locales_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_locales_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_cloned_message, test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_cloned_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_create_message_id_ulong(1));
    STRICT_EXPECTED_CALL(properties_set_message_id(test_properties, test_message_id_value));
    STRICT_EXPECTED_CALL(message_set_properties(test_cloned_message, test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_message_id_value));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messagesender_send_async(test_message_sender, test_cloned_message, IGNORED_PTR_ARG, IGNORED_PTR_ARG, 0));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));
    STRICT_EXPECTED_CALL(message_destroy(test_cloned_message));

    // act
    result = amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* on_message_send_complete */

/* Tests_SRS_AMQP_MANAGEMENT_01_167: [ When `on_message_send_complete` is called with a NULL context it shall return. ]*/
TEST_FUNCTION(on_message_send_complete_with_NULL_context_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    saved_on_message_send_complete(NULL, MESSAGE_SEND_OK, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_172: [ If `send_result` is different then `MESSAGE_SEND_OK`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_168: [ - `context` shall be used as a LIST_ITEM_HANDLE containing the pending operation. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_169: [ - `on_message_send_complete` shall obtain the pending operation by calling `singlylinkedlist_item_get_value`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_171: [ - `on_message_send_complete` shall removed the pending operation from the pending operations list. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_173: [ - The callback associated with the pending operation shall be called with `AMQP_MANAGEMENT_EXECUTE_OPERATION_ERROR`. ]*/
TEST_FUNCTION(when_on_message_send_complete_indicates_ERROR_the_pending_operation_is_indicated_as_complete_with_ERROR)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_amqp_management_execute_operation_complete((void*)0x4244, AMQP_MANAGEMENT_EXECUTE_OPERATION_ERROR, 0, NULL, NULL));
    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));

    // act
    saved_on_message_send_complete(saved_on_message_send_complete_context, MESSAGE_SEND_ERROR, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_172: [ If `send_result` is different then `MESSAGE_SEND_OK`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_168: [ - `context` shall be used as a LIST_ITEM_HANDLE containing the pending operation. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_169: [ - `on_message_send_complete` shall obtain the pending operation by calling `singlylinkedlist_item_get_value`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_171: [ - `on_message_send_complete` shall removed the pending operation from the pending operations list. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_173: [ - The callback associated with the pending operation shall be called with `AMQP_MANAGEMENT_EXECUTE_OPERATION_ERROR`. ]*/
TEST_FUNCTION(when_on_message_send_complete_indicates_CANCELLED_the_pending_operation_is_indicated_as_complete_with_ERROR)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));

    // act
    saved_on_message_send_complete(saved_on_message_send_complete_context, MESSAGE_SEND_CANCELLED, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_174: [ If any error occurs in removing the pending operation from the list `on_amqp_management_error` callback shall be invoked while passing the `on_amqp_management_error_context` as argument. ]*/
TEST_FUNCTION(when_obtaining_the_list_item_payload_fails_an_error_is_indicated_to_the_user)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist_handle, IGNORED_PTR_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(test_on_amqp_management_error(IGNORED_PTR_ARG));

    // act
    saved_on_message_send_complete(saved_on_message_send_complete_context, MESSAGE_SEND_ERROR, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_170: [ If `send_result` is `MESSAGE_SEND_OK`, `on_message_send_complete` shall return. ]*/
TEST_FUNCTION(when_on_send_message_complete_indicates_success_it_returns)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));

    // act
    saved_on_message_send_complete(saved_on_message_send_complete_context, MESSAGE_SEND_OK, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* on_message_received */

/* Tests_SRS_AMQP_MANAGEMENT_01_108: [ When `on_message_received` is called with a NULL context, it shall do nothing and return NULL. ]*/
TEST_FUNCTION(on_message_received_with_NULL_context_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;
    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    result = saved_on_message_received(NULL, test_message);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_109: [ `on_message_received` shall obtain the application properties from the message by calling `message_get_application_properties`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_110: [ `on_message_received` shall obtain the message properties from the message by calling `message_get_properties`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_111: [ `on_message_received` shall obtain the correlation Id from the message properties by using `properties_get_correlation_id`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_119: [ `on_message_received` shall obtain the application properties map by calling `amqpvalue_get_inplace_described_value`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_120: [ An AMQP value used to lookup the status code shall be created by calling `amqpvalue_create_string` with the status code key name (`statusCode`) as argument. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_121: [ The status code shall be looked up in the application properties by using `amqpvalue_get_map_value`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_133: [ The status code value shall be extracted from the value found in the map by using `amqpvalue_get_int`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_123: [ An AMQP value used to lookup the status description shall be created by calling `amqpvalue_create_string` with the status description key name (`statusDescription`) as argument. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_124: [ The status description shall be looked up in the application properties by using `amqpvalue_get_map_value`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_126: [ If a corresponding correlation Id is found in the pending operations list, the callback associated with the pending operation shall be called. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_134: [ The status description value shall be extracted from the value found in the map by using `amqpvalue_get_string`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_127: [ If the operation succeeded the result callback argument shall be `AMQP_MANAGEMENT_EXECUTE_OPERATION_OK`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_129: [ After calling the callback, the pending operation shall be removed from the pending operations list by calling `singlylinkedlist_remove`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_131: [ All temporary values like AMQP values used as keys shall be freed before exiting the callback. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_116: [ Each pending operation item value shall be obtained by calling `singlylinkedlist_item_get_value`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_112: [ `on_message_received` shall check if the correlation Id matches the stored message Id of any pending operation. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_068: [ The correlation-id of the response message MUST be the correlation-id from the request message (if present) ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_069: [ else the message-id from the request message. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_070: [ Response messages have the following application-properties: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_071: [ statusCode integer Yes HTTP response code [RFC2616] ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_072: [ statusDescription string No Description of the status. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_074: [ Successful operations MUST result in a statusCode in the 2xx range as defined in Section 10.2 of [RFC2616]. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_166: [ The `message` shall be passed as argument to the callback. ]*/
TEST_FUNCTION(on_message_received_with_a_valid_message_indicates_the_operation_complete)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;
    int32_t status_code = 200;
    const char* test_status_description = "my error ...";
    uint64_t correlation_id = 0;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    saved_on_message_send_complete(saved_on_message_send_complete_context, MESSAGE_SEND_OK, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(properties_get_correlation_id(test_properties, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_correlation_id_value(&test_correlation_id_value, sizeof(test_correlation_id_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(test_correlation_id_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ulong_value(&correlation_id, sizeof(correlation_id));
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_described_value(test_application_properties))
        .SetReturn(test_application_properties_map);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusCode"))
        .SetReturn(test_status_code_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_code_key))
        .SetReturn(test_status_code_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_int(test_status_code_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_int_value(&status_code, sizeof(status_code));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusDescription"))
        .SetReturn(test_status_description_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_description_key))
        .SetReturn(test_status_description_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_string(test_status_description_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_string_value(&test_status_description, sizeof(test_status_description));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_amqp_management_execute_operation_complete((void*)0x4244, AMQP_MANAGEMENT_EXECUTE_OPERATION_OK, 200, "my error ...", test_message));

    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messaging_delivery_accepted());
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_key));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_key));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_accepted, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_126: [ If a corresponding correlation Id is found in the pending operations list, the callback associated with the pending operation shall be called. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_115: [ Iterating through the pending operations shall be done by using `singlylinkedlist_get_head_item` and `singlylinkedlist_get_next_item` until the enm of the pending operations singly linked list is reached. ]*/
TEST_FUNCTION(on_message_received_for_the_second_pending_operation_with_a_valid_message_indicates_the_operation_complete)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;
    int32_t status_code = 200;
    const char* test_status_description = "my error ...";
    uint64_t correlation_id = 1;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);

    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    saved_on_message_send_complete(saved_on_message_send_complete_context, MESSAGE_SEND_OK, NULL);

    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(1);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4245);
    saved_on_message_send_complete(saved_on_message_send_complete_context, MESSAGE_SEND_OK, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(properties_get_correlation_id(test_properties, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_correlation_id_value(&test_correlation_id_value, sizeof(test_correlation_id_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(test_correlation_id_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ulong_value(&correlation_id, sizeof(correlation_id));
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_described_value(test_application_properties))
        .SetReturn(test_application_properties_map);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusCode"))
        .SetReturn(test_status_code_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_code_key))
        .SetReturn(test_status_code_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_int(test_status_code_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_int_value(&status_code, sizeof(status_code));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusDescription"))
        .SetReturn(test_status_description_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_description_key))
        .SetReturn(test_status_description_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_string(test_status_description_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_string_value(&test_status_description, sizeof(test_status_description));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_next_item(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_amqp_management_execute_operation_complete((void*)0x4245, AMQP_MANAGEMENT_EXECUTE_OPERATION_OK, 200, "my error ...", test_message));

    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messaging_delivery_accepted());
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_key));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_key));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_accepted, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_113: [ If obtaining the application properties or message properties fails, an error shall be indicated by calling `on_amqp_management_error` and passing the `on_amqp_management_error_context` to it. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_136: [ When `on_message_received` fails due to errors in parsing the response message `on_message_received` shall call `messaging_delivery_rejected` and return the created delivery AMQP value. ]*/
TEST_FUNCTION(when_getting_the_application_properties_fails_an_error_is_indicated)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));
    STRICT_EXPECTED_CALL(messaging_delivery_rejected(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_rejected, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_113: [ If obtaining the application properties or message properties fails, an error shall be indicated by calling `on_amqp_management_error` and passing the `on_amqp_management_error_context` to it. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_136: [ When `on_message_received` fails due to errors in parsing the response message `on_message_received` shall call `messaging_delivery_rejected` and return the created delivery AMQP value. ]*/
TEST_FUNCTION(when_getting_the_message_properties_fails_an_error_is_indicated)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));
    STRICT_EXPECTED_CALL(messaging_delivery_rejected(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_rejected, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_114: [ If obtaining the correlation Id fails, an error shall be indicated by calling `on_amqp_management_error` and passing the `on_amqp_management_error_context` to it. ] */
/* Tests_SRS_AMQP_MANAGEMENT_01_136: [ When `on_message_received` fails due to errors in parsing the response message `on_message_received` shall call `messaging_delivery_rejected` and return the created delivery AMQP value. ]*/
TEST_FUNCTION(when_getting_the_correlation_id_fails_an_error_is_indicated)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(properties_get_correlation_id(test_properties, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_correlation_id_value(&test_correlation_id_value, sizeof(test_correlation_id_value))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));
    STRICT_EXPECTED_CALL(messaging_delivery_rejected(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_rejected, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_132: [ If any functions manipulating AMQP values, application properties, etc., fail, an error shall be indicated to the consumer by calling the `on_amqp_management_error` and passing the `on_amqp_management_error_context` to it. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_136: [ When `on_message_received` fails due to errors in parsing the response message `on_message_received` shall call `messaging_delivery_rejected` and return the created delivery AMQP value. ]*/
TEST_FUNCTION(when_getting_the_correlation_id_ulong_fails_an_error_is_indicated)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;
    uint64_t correlation_id = 1;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(properties_get_correlation_id(test_properties, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_correlation_id_value(&test_correlation_id_value, sizeof(test_correlation_id_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(test_correlation_id_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ulong_value(&correlation_id, sizeof(correlation_id))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));
    STRICT_EXPECTED_CALL(messaging_delivery_rejected(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_correlation_id_value));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_rejected, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_132: [ If any functions manipulating AMQP values, application properties, etc., fail, an error shall be indicated to the consumer by calling the `on_amqp_management_error` and passing the `on_amqp_management_error_context` to it. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_136: [ When `on_message_received` fails due to errors in parsing the response message `on_message_received` shall call `messaging_delivery_rejected` and return the created delivery AMQP value. ]*/
TEST_FUNCTION(when_getting_the_application_properties_map_fails_an_error_is_indicated)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;
    uint64_t correlation_id = 1;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(properties_get_correlation_id(test_properties, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_correlation_id_value(&test_correlation_id_value, sizeof(test_correlation_id_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(test_correlation_id_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ulong_value(&correlation_id, sizeof(correlation_id));
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_described_value(test_application_properties))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));
    STRICT_EXPECTED_CALL(messaging_delivery_rejected(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_correlation_id_value));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_rejected, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_132: [ If any functions manipulating AMQP values, application properties, etc., fail, an error shall be indicated to the consumer by calling the `on_amqp_management_error` and passing the `on_amqp_management_error_context` to it. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_135: [ When an error occurs in creating AMQP values (for status code, etc.) `on_message_received` shall call `messaging_delivery_released` and return the created delivery AMQP value. ]*/
TEST_FUNCTION(when_creating_the_status_code_string_amqp_value_fails_an_error_is_indicated)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;
    uint64_t correlation_id = 0;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(properties_get_correlation_id(test_properties, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_correlation_id_value(&test_correlation_id_value, sizeof(test_correlation_id_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(test_correlation_id_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ulong_value(&correlation_id, sizeof(correlation_id));
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_described_value(test_application_properties))
        .SetReturn(test_application_properties_map);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusCode"))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));
    STRICT_EXPECTED_CALL(messaging_delivery_released());
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_correlation_id_value));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_released, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_122: [ If status code is not found an error shall be indicated to the consumer by calling the `on_amqp_management_error` and passing the `on_amqp_management_error_context` to it. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_136: [ When `on_message_received` fails due to errors in parsing the response message `on_message_received` shall call `messaging_delivery_rejected` and return the created delivery AMQP value. ]*/
TEST_FUNCTION(when_getting_the_map_value_for_status_code_fails_an_error_is_indicated)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;
    uint64_t correlation_id = 1;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(properties_get_correlation_id(test_properties, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_correlation_id_value(&test_correlation_id_value, sizeof(test_correlation_id_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(test_correlation_id_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ulong_value(&correlation_id, sizeof(correlation_id));
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_described_value(test_application_properties))
        .SetReturn(test_application_properties_map);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusCode"))
        .SetReturn(test_status_code_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_code_key))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));
    STRICT_EXPECTED_CALL(messaging_delivery_rejected(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_key));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_correlation_id_value));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_rejected, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_132: [ If any functions manipulating AMQP values, application properties, etc., fail, an error shall be indicated to the consumer by calling the `on_amqp_management_error` and passing the `on_amqp_management_error_context` to it. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_136: [ When `on_message_received` fails due to errors in parsing the response message `on_message_received` shall call `messaging_delivery_rejected` and return the created delivery AMQP value. ]*/
TEST_FUNCTION(when_getting_status_code_int_value_fails_an_error_is_indicated)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;
    uint64_t correlation_id = 1;
    int32_t status_code = 200;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(properties_get_correlation_id(test_properties, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_correlation_id_value(&test_correlation_id_value, sizeof(test_correlation_id_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(test_correlation_id_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ulong_value(&correlation_id, sizeof(correlation_id));
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_described_value(test_application_properties))
        .SetReturn(test_application_properties_map);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusCode"))
        .SetReturn(test_status_code_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_code_key))
        .SetReturn(test_status_code_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_int(test_status_code_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_int_value(&status_code, sizeof(status_code))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));
    STRICT_EXPECTED_CALL(messaging_delivery_rejected(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_key));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_correlation_id_value));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_rejected, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_132: [ If any functions manipulating AMQP values, application properties, etc., fail, an error shall be indicated to the consumer by calling the `on_amqp_management_error` and passing the `on_amqp_management_error_context` to it. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_135: [ When an error occurs in creating AMQP values (for status code, etc.) `on_message_received` shall call `messaging_delivery_released` and return the created delivery AMQP value. ]*/
TEST_FUNCTION(when_creating_the_status_description_amqp_value_fails_an_error_is_indicated)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;
    uint64_t correlation_id = 0;
    int32_t status_code = 200;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(properties_get_correlation_id(test_properties, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_correlation_id_value(&test_correlation_id_value, sizeof(test_correlation_id_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(test_correlation_id_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ulong_value(&correlation_id, sizeof(correlation_id));
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_described_value(test_application_properties))
        .SetReturn(test_application_properties_map);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusCode"))
        .SetReturn(test_status_code_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_code_key))
        .SetReturn(test_status_code_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_int(test_status_code_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_int_value(&status_code, sizeof(status_code));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusDescription"))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));
    STRICT_EXPECTED_CALL(messaging_delivery_released());
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_key));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_correlation_id_value));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_released, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_125: [ If status description is not found, NULL shall be passed to the user callback as `status_description` argument. ]*/
TEST_FUNCTION(when_no_description_is_found_NULL_is_indicated_as_description)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;
    uint64_t correlation_id = 0;
    int32_t status_code = 200;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(properties_get_correlation_id(test_properties, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_correlation_id_value(&test_correlation_id_value, sizeof(test_correlation_id_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(test_correlation_id_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ulong_value(&correlation_id, sizeof(correlation_id));
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_described_value(test_application_properties))
        .SetReturn(test_application_properties_map);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusCode"))
        .SetReturn(test_status_code_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_code_key))
        .SetReturn(test_status_code_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_int(test_status_code_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_int_value(&status_code, sizeof(status_code));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusDescription"))
        .SetReturn(test_status_description_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_description_key))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_amqp_management_execute_operation_complete((void*)0x4244, AMQP_MANAGEMENT_EXECUTE_OPERATION_OK, 200, NULL, test_message));

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_key));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_key));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));
    STRICT_EXPECTED_CALL(messaging_delivery_accepted());

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_accepted, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_125: [ If status description is not found, NULL shall be passed to the user callback as `status_description` argument. ]*/
TEST_FUNCTION(when_getting_the_string_for_the_description_fails_NULL_is_indicated_as_description)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;
    uint64_t correlation_id = 0;
    int32_t status_code = 200;
    const char* test_status_description = "my oh my";

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(properties_get_correlation_id(test_properties, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_correlation_id_value(&test_correlation_id_value, sizeof(test_correlation_id_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(test_correlation_id_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ulong_value(&correlation_id, sizeof(correlation_id));
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_described_value(test_application_properties))
        .SetReturn(test_application_properties_map);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusCode"))
        .SetReturn(test_status_code_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_code_key))
        .SetReturn(test_status_code_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_int(test_status_code_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_int_value(&status_code, sizeof(status_code));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusDescription"))
        .SetReturn(test_status_description_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_description_key))
        .SetReturn(test_status_description_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_string(test_status_description_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_string_value(&test_status_description, sizeof(test_status_description))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_amqp_management_execute_operation_complete((void*)0x4244, AMQP_MANAGEMENT_EXECUTE_OPERATION_OK, 200, NULL, test_message));

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messaging_delivery_accepted());
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_key));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_key));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_accepted, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_118: [ If no pending operation is found matching the correlation Id, an error shall be indicated by calling `on_amqp_management_error` and passing the `on_amqp_management_error_context` to it. ]*/
TEST_FUNCTION(when_getting_the_head_item_in_the_list_fails_an_error_is_indicated)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;
    uint64_t correlation_id = 0;
    int32_t status_code = 200;
    const char* test_status_description = "my oh my";

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(properties_get_correlation_id(test_properties, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_correlation_id_value(&test_correlation_id_value, sizeof(test_correlation_id_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(test_correlation_id_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ulong_value(&correlation_id, sizeof(correlation_id));
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_described_value(test_application_properties))
        .SetReturn(test_application_properties_map);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusCode"))
        .SetReturn(test_status_code_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_code_key))
        .SetReturn(test_status_code_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_int(test_status_code_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_int_value(&status_code, sizeof(status_code));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusDescription"))
        .SetReturn(test_status_description_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_description_key))
        .SetReturn(test_status_description_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_string(test_status_description_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_string_value(&test_status_description, sizeof(test_status_description));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));
    STRICT_EXPECTED_CALL(messaging_delivery_rejected(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_key));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_key));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_rejected, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_117: [ If iterating through the pending operations list fails, an error shall be indicated by calling `on_amqp_management_error` and passing the `on_amqp_management_error_context` to it. ]*/
TEST_FUNCTION(when_getting_the_list_item_content_fails_an_error_is_indicated)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;
    uint64_t correlation_id = 0;
    int32_t status_code = 200;
    const char* test_status_description = "my oh my";

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(properties_get_correlation_id(test_properties, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_correlation_id_value(&test_correlation_id_value, sizeof(test_correlation_id_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(test_correlation_id_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ulong_value(&correlation_id, sizeof(correlation_id));
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_described_value(test_application_properties))
        .SetReturn(test_application_properties_map);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusCode"))
        .SetReturn(test_status_code_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_code_key))
        .SetReturn(test_status_code_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_int(test_status_code_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_int_value(&status_code, sizeof(status_code));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusDescription"))
        .SetReturn(test_status_description_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_description_key))
        .SetReturn(test_status_description_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_string(test_status_description_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_string_value(&test_status_description, sizeof(test_status_description));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));
    STRICT_EXPECTED_CALL(messaging_delivery_rejected(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_key));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_key));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_rejected, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_117: [ If iterating through the pending operations list fails, an error shall be indicated by calling `on_amqp_management_error` and passing the `on_amqp_management_error_context` to it. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_118: [ If no pending operation is found matching the correlation Id, an error shall be indicated by calling `on_amqp_management_error` and passing the `on_amqp_management_error_context` to it. ]*/
TEST_FUNCTION(when_getting_the_next_element_in_the_list_yields_NULL_an_error_is_indicated)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;
    uint64_t correlation_id = 1;
    int32_t status_code = 200;
    const char* test_status_description = "my oh my";

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(properties_get_correlation_id(test_properties, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_correlation_id_value(&test_correlation_id_value, sizeof(test_correlation_id_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(test_correlation_id_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ulong_value(&correlation_id, sizeof(correlation_id));
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_described_value(test_application_properties))
        .SetReturn(test_application_properties_map);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusCode"))
        .SetReturn(test_status_code_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_code_key))
        .SetReturn(test_status_code_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_int(test_status_code_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_int_value(&status_code, sizeof(status_code));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusDescription"))
        .SetReturn(test_status_description_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_description_key))
        .SetReturn(test_status_description_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_string(test_status_description_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_string_value(&test_status_description, sizeof(test_status_description));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_next_item(IGNORED_PTR_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));
    STRICT_EXPECTED_CALL(messaging_delivery_rejected(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_key));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_key));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_rejected, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_117: [ If iterating through the pending operations list fails, an error shall be indicated by calling `on_amqp_management_error` and passing the `on_amqp_management_error_context` to it. ]*/
TEST_FUNCTION(when_removing_the_item_fails_an_error_is_indicated)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;
    uint64_t correlation_id = 0;
    int32_t status_code = 200;
    const char* test_status_description = "my oh my";

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    singlylinkedlist_remove_result = 1;

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(properties_get_correlation_id(test_properties, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_correlation_id_value(&test_correlation_id_value, sizeof(test_correlation_id_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(test_correlation_id_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ulong_value(&correlation_id, sizeof(correlation_id));
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_described_value(test_application_properties))
        .SetReturn(test_application_properties_map);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusCode"))
        .SetReturn(test_status_code_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_code_key))
        .SetReturn(test_status_code_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_int(test_status_code_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_int_value(&status_code, sizeof(status_code));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusDescription"))
        .SetReturn(test_status_description_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_description_key))
        .SetReturn(test_status_description_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_string(test_status_description_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_string_value(&test_status_description, sizeof(test_status_description));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));
    STRICT_EXPECTED_CALL(messaging_delivery_released());

    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_key));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_key));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_released, result);

    singlylinkedlist_remove_result = 0;

    // cleanup
    amqp_management_destroy(amqp_management);
}

static void setup_calls_for_response_with_status_code_and_correlation_id(int status_code, uint64_t correlation_id)
{
    static const char* test_status_description = "my error ...";

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(properties_get_correlation_id(test_properties, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_correlation_id_value(&test_correlation_id_value, sizeof(test_correlation_id_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(test_correlation_id_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ulong_value(&correlation_id, sizeof(correlation_id));
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_described_value(test_application_properties))
        .SetReturn(test_application_properties_map);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusCode"))
        .SetReturn(test_status_code_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_code_key))
        .SetReturn(test_status_code_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_int(test_status_code_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_int_value(&status_code, sizeof(status_code));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusDescription"))
        .SetReturn(test_status_description_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_description_key))
        .SetReturn(test_status_description_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_string(test_status_description_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_string_value(&test_status_description, sizeof(test_status_description));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
}

/* Tests_SRS_AMQP_MANAGEMENT_01_128: [ If the status indicates that the operation failed, the result callback argument shall be `AMQP_MANAGEMENT_EXECUTE_OPERATION_FAILED_BAD_STATUS`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_075: [ Unsuccessful operations MUST NOT result in a statusCode in the 2xx range as defined in Section 10.2 of [RFC2616]. ]*/
TEST_FUNCTION(on_message_received_with_300_indicates_failure)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    saved_on_message_send_complete(saved_on_message_send_complete_context, MESSAGE_SEND_OK, NULL);
    umock_c_reset_all_calls();

    setup_calls_for_response_with_status_code_and_correlation_id(300, 0);

    STRICT_EXPECTED_CALL(test_on_amqp_management_execute_operation_complete((void*)0x4244, AMQP_MANAGEMENT_EXECUTE_OPERATION_FAILED_BAD_STATUS, 300, "my error ...", test_message));
    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messaging_delivery_accepted());
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_key));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_key));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_accepted, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_128: [ If the status indicates that the operation failed, the result callback argument shall be `AMQP_MANAGEMENT_EXECUTE_OPERATION_FAILED_BAD_STATUS`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_075: [ Unsuccessful operations MUST NOT result in a statusCode in the 2xx range as defined in Section 10.2 of [RFC2616]. ]*/
TEST_FUNCTION(on_message_received_with_199_indicates_failure)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    saved_on_message_send_complete(saved_on_message_send_complete_context, MESSAGE_SEND_OK, NULL);
    umock_c_reset_all_calls();

    setup_calls_for_response_with_status_code_and_correlation_id(199, 0);

    STRICT_EXPECTED_CALL(test_on_amqp_management_execute_operation_complete((void*)0x4244, AMQP_MANAGEMENT_EXECUTE_OPERATION_FAILED_BAD_STATUS, 199, "my error ...", test_message));
    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messaging_delivery_accepted());
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_key));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_key));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_accepted, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_128: [ If the status indicates that the operation failed, the result callback argument shall be `AMQP_MANAGEMENT_EXECUTE_OPERATION_FAILED_BAD_STATUS`. ]*/
TEST_FUNCTION(on_message_received_with_all_valid_codes_indicates_failure)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;
    int i;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);

    for (i = 201; i < 300; i++)
    {
        umock_c_reset_all_calls();
        setup_calls_for_pending_operation_with_correlation_id(0);
        (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
        saved_on_message_send_complete(saved_on_message_send_complete_context, MESSAGE_SEND_OK, NULL);
        umock_c_reset_all_calls();

        setup_calls_for_response_with_status_code_and_correlation_id(i, i - 201);

        STRICT_EXPECTED_CALL(test_on_amqp_management_execute_operation_complete((void*)0x4244, AMQP_MANAGEMENT_EXECUTE_OPERATION_OK, i, "my error ...", test_message));
        STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(messaging_delivery_accepted());
        STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_value));
        STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_key));
        STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_value));
        STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_key));
        STRICT_EXPECTED_CALL(properties_destroy(test_properties));
        STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

        // act
        result = saved_on_message_received(saved_on_message_received_context, test_message);

        // assert
        ASSERT_ARE_EQUAL(void_ptr, test_delivery_accepted, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* on_message_sender_state_changed */

/* Tests_SRS_AMQP_MANAGEMENT_01_137: [ When `on_message_sender_state_changed` is called with NULL `context`, it shall do nothing. ]*/
TEST_FUNCTION(on_message_sender_state_changed_with_NULL_context_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    umock_c_reset_all_calls();

    // act
    saved_on_message_sender_state_changed(NULL, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_138: [ When `on_message_sender_state_changed` is called and the `new_state` is different than `previous_state`, the following actions shall be taken: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_139: [ For the current state of AMQP management being `OPENING`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_140: [ - If `new_state` is `MESSAGE_SENDER_STATE_IDLE`, `MESSAGE_SENDER_STATE_CLOSING` or `MESSAGE_SENDER_STATE_ERROR`, the `on_amqp_management_open_complete` callback shall be called with `AMQP_MANAGEMENT_OPEN_ERROR`, while also passing the context passed in `amqp_management_open_async`. ]*/
TEST_FUNCTION(on_message_sender_state_changed_when_a_new_SENDER_IDLE_state_is_detected_while_in_OPENING_triggers_open_complete_with_ERROR)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_amqp_management_open_complete((void*)0x4242, AMQP_MANAGEMENT_OPEN_ERROR));

    // act
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_IDLE, MESSAGE_SENDER_STATE_OPENING);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_138: [ When `on_message_sender_state_changed` is called and the `new_state` is different than `previous_state`, the following actions shall be taken: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_139: [ For the current state of AMQP management being `OPENING`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_140: [ - If `new_state` is `MESSAGE_SENDER_STATE_IDLE`, `MESSAGE_SENDER_STATE_CLOSING` or `MESSAGE_SENDER_STATE_ERROR`, the `on_amqp_management_open_complete` callback shall be called with `AMQP_MANAGEMENT_OPEN_ERROR`, while also passing the context passed in `amqp_management_open_async`. ]*/
TEST_FUNCTION(on_message_sender_state_changed_when_a_new_SENDER_CLOSING_state_is_detected_while_in_OPENING_triggers_open_complete_with_ERROR)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_amqp_management_open_complete((void*)0x4242, AMQP_MANAGEMENT_OPEN_ERROR));

    // act
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_CLOSING, MESSAGE_SENDER_STATE_OPENING);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

// Tests_SRS_AMQP_MANAGEMENT_09_001: [ For the current state of AMQP management being `CLOSING`: ]
// Tests_SRS_AMQP_MANAGEMENT_09_002: [ - If `new_state` is `MESSAGE_SENDER_STATE_OPEN`, `MESSAGE_SENDER_STATE_OPENING`, `MESSAGE_SENDER_STATE_ERROR` the `on_amqp_management_error` callback shall be invoked while passing the `on_amqp_management_error_context` as argument. ]
TEST_FUNCTION(on_message_sender_state_changed_when_a_new_SENDER_OPEN_state_is_detected_while_in_CLOSING_indicates_an_error)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    messagesender_close_on_message_sender_state_changed_previous_state = MESSAGE_SENDER_STATE_OPEN;
    messagesender_close_on_message_sender_state_changed_new_state = MESSAGE_SENDER_STATE_OPENING;

    STRICT_EXPECTED_CALL(messagesender_close(test_message_sender));
    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));
    STRICT_EXPECTED_CALL(messagereceiver_close(test_message_receiver));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));

    // act
    result = amqp_management_close(amqp_management);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

// Tests_SRS_AMQP_MANAGEMENT_09_001: [ For the current state of AMQP management being `CLOSING`: ]
// Tests_SRS_AMQP_MANAGEMENT_09_002: [ - If `new_state` is `MESSAGE_SENDER_STATE_OPEN`, `MESSAGE_SENDER_STATE_OPENING`, `MESSAGE_SENDER_STATE_ERROR` the `on_amqp_management_error` callback shall be invoked while passing the `on_amqp_management_error_context` as argument. ]
TEST_FUNCTION(on_message_sender_state_changed_when_a_new_SENDER_OPENING_state_is_detected_while_in_CLOSING_indicates_an_error)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    messagesender_close_on_message_sender_state_changed_previous_state = MESSAGE_SENDER_STATE_OPENING;
    messagesender_close_on_message_sender_state_changed_new_state = MESSAGE_SENDER_STATE_OPEN;

    STRICT_EXPECTED_CALL(messagesender_close(test_message_sender));
    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));
    STRICT_EXPECTED_CALL(messagereceiver_close(test_message_receiver));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));

    // act
    result = amqp_management_close(amqp_management);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

// Tests_SRS_AMQP_MANAGEMENT_09_001: [ For the current state of AMQP management being `CLOSING`: ]
// Tests_SRS_AMQP_MANAGEMENT_09_002: [ - If `new_state` is `MESSAGE_SENDER_STATE_OPEN`, `MESSAGE_SENDER_STATE_OPENING`, `MESSAGE_SENDER_STATE_ERROR` the `on_amqp_management_error` callback shall be invoked while passing the `on_amqp_management_error_context` as argument. ]
TEST_FUNCTION(on_message_sender_state_changed_when_a_new_SENDER_ERROR_state_is_detected_while_in_CLOSING_indicates_an_error)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    messagesender_close_on_message_sender_state_changed_previous_state = MESSAGE_SENDER_STATE_OPEN;
    messagesender_close_on_message_sender_state_changed_new_state = MESSAGE_SENDER_STATE_ERROR;

    STRICT_EXPECTED_CALL(messagesender_close(test_message_sender));
    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));
    STRICT_EXPECTED_CALL(messagereceiver_close(test_message_receiver));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));

    // act
    result = amqp_management_close(amqp_management);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

// Tests_SRS_AMQP_MANAGEMENT_09_001: [ For the current state of AMQP management being `CLOSING`: ]
// Tests_SRS_AMQP_MANAGEMENT_09_003: [ - If `new_state` is `MESSAGE_SENDER_STATE_CLOSING` or `MESSAGE_SENDER_STATE_IDLE`, `on_message_sender_state_changed` shall do nothing. ]
TEST_FUNCTION(on_message_sender_state_changed_when_a_new_SENDER_CLOSING_state_is_detected_while_in_CLOSING_does_not_raise_on_amqp_management_error)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    messagesender_close_on_message_sender_state_changed_previous_state = MESSAGE_SENDER_STATE_OPEN;
    messagesender_close_on_message_sender_state_changed_new_state = MESSAGE_SENDER_STATE_CLOSING;

    STRICT_EXPECTED_CALL(messagesender_close(test_message_sender));
    STRICT_EXPECTED_CALL(messagereceiver_close(test_message_receiver));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));

    // act
    result = amqp_management_close(amqp_management);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

// Tests_SRS_AMQP_MANAGEMENT_09_001: [ For the current state of AMQP management being `CLOSING`: ]
// Tests_SRS_AMQP_MANAGEMENT_09_003: [ - If `new_state` is `MESSAGE_SENDER_STATE_CLOSING` or `MESSAGE_SENDER_STATE_IDLE`, `on_message_sender_state_changed` shall do nothing. ]
TEST_FUNCTION(on_message_sender_state_changed_when_a_new_SENDER_IDLE_state_is_detected_while_in_CLOSING_does_not_raise_on_amqp_management_error)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    messagesender_close_on_message_sender_state_changed_previous_state = MESSAGE_SENDER_STATE_OPEN;
    messagesender_close_on_message_sender_state_changed_new_state = MESSAGE_SENDER_STATE_IDLE;

    STRICT_EXPECTED_CALL(messagesender_close(test_message_sender));
    STRICT_EXPECTED_CALL(messagereceiver_close(test_message_receiver));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));

    // act
    result = amqp_management_close(amqp_management);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_138: [ When `on_message_sender_state_changed` is called and the `new_state` is different than `previous_state`, the following actions shall be taken: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_139: [ For the current state of AMQP management being `OPENING`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_140: [ - If `new_state` is `MESSAGE_SENDER_STATE_IDLE`, `MESSAGE_SENDER_STATE_CLOSING` or `MESSAGE_SENDER_STATE_ERROR`, the `on_amqp_management_open_complete` callback shall be called with `AMQP_MANAGEMENT_OPEN_ERROR`, while also passing the context passed in `amqp_management_open_async`. ]*/
TEST_FUNCTION(on_message_sender_state_changed_when_a_new_SENDER_ERROR_state_is_detected_while_in_OPENING_triggers_open_complete_with_ERROR)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_amqp_management_open_complete((void*)0x4242, AMQP_MANAGEMENT_OPEN_ERROR));

    // act
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_ERROR, MESSAGE_SENDER_STATE_OPENING);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_141: [ - If `new_state` is `MESSAGE_SENDER_STATE_OPEN` and the message receiver already indicated its state as `MESSAGE_RECEIVER_STATE_OPEN`, the `on_amqp_management_open_complete` callback shall be called with `AMQP_MANAGEMENT_OPEN_OK`, while also passing the context passed in `amqp_management_open_async`. ]*/
TEST_FUNCTION(on_message_sender_state_changed_when_a_new_SENDER_OPEN_state_is_detected_while_in_OPENING_triggers_open_complete_with_OK)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_amqp_management_open_complete((void*)0x4242, AMQP_MANAGEMENT_OPEN_OK));

    // act
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_142: [ - If `new_state` is `MESSAGE_SENDER_STATE_OPEN` and the message receiver did not yet indicate its state as `MESSAGE_RECEIVER_STATE_OPEN`, the `on_amqp_management_open_complete` callback shall not be called.]*/
TEST_FUNCTION(on_message_sender_state_changed_when_a_new_SENDER_OPEN_state_is_detected_and_receiver_is_not_OPEN_while_in_OPENING_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    umock_c_reset_all_calls();

    // act
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_165: [ - If `new_state` is `MESSAGE_SENDER_STATE_OPEING` the transition shall be ignored. ]*/
TEST_FUNCTION(on_message_sender_state_changed_when_a_new_SENDER_OPENING_state_is_detected_in_OPENING_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    umock_c_reset_all_calls();

    // act
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPENING, MESSAGE_SENDER_STATE_IDLE);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_144: [ For the current state of AMQP management being `OPEN`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_143: [ - If `new_state` is `MESSAGE_SENDER_STATE_IDLE`, `MESSAGE_SENDER_STATE_OPENING`, `MESSAGE_SENDER_STATE_CLOSING` or `MESSAGE_SENDER_STATE_ERROR` the `on_amqp_management_error` callback shall be invoked while passing the `on_amqp_management_error_context` as argument. ]*/
TEST_FUNCTION(on_message_sender_state_changed_when_a_new_SENDER_IDLE_state_is_detected_in_OPEN_indicates_an_error)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));

    // act
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_IDLE, MESSAGE_SENDER_STATE_OPEN);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_144: [ For the current state of AMQP management being `OPEN`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_143: [ - If `new_state` is `MESSAGE_SENDER_STATE_IDLE`, `MESSAGE_SENDER_STATE_OPENING`, `MESSAGE_SENDER_STATE_CLOSING` or `MESSAGE_SENDER_STATE_ERROR` the `on_amqp_management_error` callback shall be invoked while passing the `on_amqp_management_error_context` as argument. ]*/
TEST_FUNCTION(on_message_sender_state_changed_when_a_new_SENDER_OPENING_state_is_detected_in_OPEN_indicates_an_error)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));

    // act
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPENING, MESSAGE_SENDER_STATE_OPEN);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_144: [ For the current state of AMQP management being `OPEN`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_143: [ - If `new_state` is `MESSAGE_SENDER_STATE_IDLE`, `MESSAGE_SENDER_STATE_OPENING`, `MESSAGE_SENDER_STATE_CLOSING` or `MESSAGE_SENDER_STATE_ERROR` the `on_amqp_management_error` callback shall be invoked while passing the `on_amqp_management_error_context` as argument. ]*/
TEST_FUNCTION(on_message_sender_state_changed_when_a_new_SENDER_CLOSING_state_is_detected_in_OPEN_indicates_an_error)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));

    // act
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_CLOSING, MESSAGE_SENDER_STATE_OPEN);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_145: [ - If `new_state` is `MESSAGE_SENDER_STATE_OPEN`, `on_message_sender_state_changed` shall do nothing. ]*/
TEST_FUNCTION(on_message_sender_state_changed_when_a_new_SENDER_OPEN_state_is_detected_in_OPEN_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    umock_c_reset_all_calls();

    // act
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_146: [ For the current state of AMQP management being `ERROR`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_147: [ - All state transitions shall be ignored. ]*/
TEST_FUNCTION(on_message_sender_state_changed_when_a_new_SENDER_IDLE_state_is_detected_in_ERROR_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_ERROR, MESSAGE_SENDER_STATE_OPEN);
    umock_c_reset_all_calls();

    // act
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_IDLE, MESSAGE_SENDER_STATE_OPEN);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_146: [ For the current state of AMQP management being `ERROR`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_147: [ - All state transitions shall be ignored. ]*/
TEST_FUNCTION(on_message_sender_state_changed_when_a_new_SENDER_ERROR_state_is_detected_in_ERROR_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_ERROR, MESSAGE_SENDER_STATE_OPEN);
    umock_c_reset_all_calls();

    // act
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_ERROR, MESSAGE_SENDER_STATE_OPEN);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_146: [ For the current state of AMQP management being `ERROR`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_147: [ - All state transitions shall be ignored. ]*/
TEST_FUNCTION(on_message_sender_state_changed_when_a_new_SENDER_OPENING_state_is_detected_in_ERROR_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_ERROR, MESSAGE_SENDER_STATE_OPEN);
    umock_c_reset_all_calls();

    // act
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPENING, MESSAGE_SENDER_STATE_OPEN);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_146: [ For the current state of AMQP management being `ERROR`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_147: [ - All state transitions shall be ignored. ]*/
TEST_FUNCTION(on_message_sender_state_changed_when_a_new_SENDER_CLOSING_state_is_detected_in_ERROR_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_ERROR, MESSAGE_SENDER_STATE_OPEN);
    umock_c_reset_all_calls();

    // act
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_CLOSING, MESSAGE_SENDER_STATE_OPEN);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_148: [ When no state change is detected, `on_message_sender_state_changed` shall do nothing. ]*/
TEST_FUNCTION(on_message_sender_state_changed_with_no_transition_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    // act
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPENING, MESSAGE_SENDER_STATE_OPENING);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* on_message_receiver_state_changed */

/* Tests_SRS_AMQP_MANAGEMENT_01_149: [ When `on_message_receiver_state_changed` is called with NULL `context`, it shall do nothing. ]*/
TEST_FUNCTION(on_message_receiver_state_changed_with_NULL_context_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    umock_c_reset_all_calls();

    // act
    saved_on_message_receiver_state_changed(NULL, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_150: [ When `on_message_receiver_state_changed` is called and the `new_state` is different than `previous_state`, the following actions shall be taken: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_151: [ For the current state of AMQP management being `OPENING`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_152: [ - If `new_state` is `MESSAGE_RECEIVER_STATE_IDLE`, `MESSAGE_RECEIVER_STATE_CLOSING` or `MESSAGE_RECEIVER_STATE_ERROR`, the `on_amqp_management_open_complete` callback shall be called with `AMQP_MANAGEMENT_OPEN_ERROR`, while also passing the context passed in `amqp_management_open_async`. ]*/
TEST_FUNCTION(on_message_receiver_state_changed_when_a_new_RECEIVER_IDLE_state_is_detected_while_in_OPENING_triggers_open_complete_with_ERROR)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_amqp_management_open_complete((void*)0x4242, AMQP_MANAGEMENT_OPEN_ERROR));

    // act
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_IDLE, MESSAGE_RECEIVER_STATE_OPENING);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_164: [ - If `new_state` is `MESSAGE_RECEIVER_STATE_OPEING` the transition shall be ignored. ]*/
TEST_FUNCTION(on_message_receiver_state_changed_when_a_new_RECEIVER_OPENING_state_is_detected_while_in_OPENING_triggers_open_complete_with_ERROR)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    umock_c_reset_all_calls();

    // act
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPENING, MESSAGE_RECEIVER_STATE_IDLE);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_150: [ When `on_message_receiver_state_changed` is called and the `new_state` is different than `previous_state`, the following actions shall be taken: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_151: [ For the current state of AMQP management being `OPENING`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_152: [ - If `new_state` is `MESSAGE_RECEIVER_STATE_IDLE`, `MESSAGE_RECEIVER_STATE_CLOSING` or `MESSAGE_RECEIVER_STATE_ERROR`, the `on_amqp_management_open_complete` callback shall be called with `AMQP_MANAGEMENT_OPEN_ERROR`, while also passing the context passed in `amqp_management_open_async`. ]*/
TEST_FUNCTION(on_message_receiver_state_changed_when_a_new_RECEIVER_CLOSING_state_is_detected_while_in_OPENING_triggers_open_complete_with_ERROR)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_amqp_management_open_complete((void*)0x4242, AMQP_MANAGEMENT_OPEN_ERROR));

    // act
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_CLOSING, MESSAGE_RECEIVER_STATE_OPENING);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_150: [ When `on_message_receiver_state_changed` is called and the `new_state` is different than `previous_state`, the following actions shall be taken: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_151: [ For the current state of AMQP management being `OPENING`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_152: [ - If `new_state` is `MESSAGE_RECEIVER_STATE_IDLE`, `MESSAGE_RECEIVER_STATE_CLOSING` or `MESSAGE_RECEIVER_STATE_ERROR`, the `on_amqp_management_open_complete` callback shall be called with `AMQP_MANAGEMENT_OPEN_ERROR`, while also passing the context passed in `amqp_management_open_async`. ]*/
TEST_FUNCTION(on_message_receiver_state_changed_when_a_new_RECEIVER_ERROR_state_is_detected_while_in_OPENING_triggers_open_complete_with_ERROR)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_amqp_management_open_complete((void*)0x4242, AMQP_MANAGEMENT_OPEN_ERROR));

    // act
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_ERROR, MESSAGE_RECEIVER_STATE_OPENING);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_153: [ - If `new_state` is `MESSAGE_RECEIVER_STATE_OPEN` and the message sender already indicated its state as `MESSAGE_RECEIVER_STATE_OPEN`, the `on_amqp_management_open_complete` callback shall be called with `AMQP_MANAGEMENT_OPEN_OK`, while also passing the context passed in `amqp_management_open_async`. ]*/
TEST_FUNCTION(on_message_receiver_state_changed_when_a_new_RECEIVER_OPEN_state_is_detected_while_in_OPENING_triggers_open_complete_with_OK)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_amqp_management_open_complete((void*)0x4242, AMQP_MANAGEMENT_OPEN_OK));

    // act
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_154: [ - If `new_state` is `MESSAGE_RECEIVER_STATE_OPEN` and the message sender did not yet indicate its state as `MESSAGE_RECEIVER_STATE_OPEN`, the `on_amqp_management_open_complete` callback shall not be called. ]*/
TEST_FUNCTION(on_message_receiver_state_changed_when_a_new_RECEIVER_OPEN_state_is_detected_and_sender_is_not_OPEN_while_in_OPENING_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    umock_c_reset_all_calls();

    // act
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_155: [ For the current state of AMQP management being `OPEN`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_156: [ - If `new_state` is `MESSAGE_RECEIVER_STATE_IDLE`, `MESSAGE_RECEIVER_STATE_OPENING`, `MESSAGE_RECEIVER_STATE_CLOSING` or `MESSAGE_RECEIVER_STATE_ERROR` the `on_amqp_management_error` callback shall be invoked while passing the `on_amqp_management_error_context` as argument. ]*/
TEST_FUNCTION(on_message_receiver_state_changed_when_a_new_RECEIVER_IDLE_state_is_detected_in_OPEN_indicates_an_error)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));

    // act
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_IDLE, MESSAGE_RECEIVER_STATE_OPEN);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_155: [ For the current state of AMQP management being `OPEN`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_156: [ - If `new_state` is `MESSAGE_RECEIVER_STATE_IDLE`, `MESSAGE_RECEIVER_STATE_OPENING`, `MESSAGE_RECEIVER_STATE_CLOSING` or `MESSAGE_RECEIVER_STATE_ERROR` the `on_amqp_management_error` callback shall be invoked while passing the `on_amqp_management_error_context` as argument. ]*/
TEST_FUNCTION(on_message_receiver_state_changed_when_a_new_RECEIVER_OPENING_state_is_detected_in_OPEN_indicates_an_error)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));

    // act
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPENING, MESSAGE_RECEIVER_STATE_OPEN);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_155: [ For the current state of AMQP management being `OPEN`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_156: [ - If `new_state` is `MESSAGE_RECEIVER_STATE_IDLE`, `MESSAGE_RECEIVER_STATE_OPENING`, `MESSAGE_RECEIVER_STATE_CLOSING` or `MESSAGE_RECEIVER_STATE_ERROR` the `on_amqp_management_error` callback shall be invoked while passing the `on_amqp_management_error_context` as argument. ]*/
TEST_FUNCTION(on_message_receiver_state_changed_when_a_new_RECEIVER_CLOSING_state_is_detected_in_OPEN_indicates_an_error)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_amqp_management_error((void*)0x4243));

    // act
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_CLOSING, MESSAGE_RECEIVER_STATE_OPEN);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_157: [ - If `new_state` is `MESSAGE_RECEIVER_STATE_OPEN`, `on_message_receiver_state_changed` shall do nothing. ]*/
TEST_FUNCTION(on_message_receiver_state_changed_when_a_new_RECEIVER_OPEN_state_is_detected_in_OPEN_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();

    // act
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_158: [ For the current state of AMQP management being `ERROR`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_159: [ - All state transitions shall be ignored. ]*/
TEST_FUNCTION(on_message_receiver_state_changed_when_a_new_RECEIVER_IDLE_state_is_detected_in_ERROR_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_ERROR, MESSAGE_RECEIVER_STATE_OPEN);
    umock_c_reset_all_calls();

    // act
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_IDLE, MESSAGE_RECEIVER_STATE_OPEN);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_158: [ For the current state of AMQP management being `ERROR`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_159: [ - All state transitions shall be ignored. ]*/
TEST_FUNCTION(on_message_receiver_state_changed_when_a_new_RECEIVER_ERROR_state_is_detected_in_ERROR_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_ERROR, MESSAGE_RECEIVER_STATE_OPEN);
    umock_c_reset_all_calls();

    // act
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_ERROR, MESSAGE_RECEIVER_STATE_OPEN);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_158: [ For the current state of AMQP management being `ERROR`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_159: [ - All state transitions shall be ignored. ]*/
TEST_FUNCTION(on_message_receiver_state_changed_when_a_new_RECEIVER_OPENING_state_is_detected_in_ERROR_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_ERROR, MESSAGE_RECEIVER_STATE_OPEN);
    umock_c_reset_all_calls();

    // act
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPENING, MESSAGE_RECEIVER_STATE_OPEN);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_158: [ For the current state of AMQP management being `ERROR`: ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_159: [ - All state transitions shall be ignored. ]*/
TEST_FUNCTION(on_message_receiver_state_changed_when_a_new_RECEIVER_CLOSING_state_is_detected_in_ERROR_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_ERROR, MESSAGE_RECEIVER_STATE_OPEN);
    umock_c_reset_all_calls();

    // act
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_CLOSING, MESSAGE_RECEIVER_STATE_OPEN);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_160: [ When no state change is detected, `on_message_receiver_state_changed` shall do nothing. ]*/
TEST_FUNCTION(on_message_receiver_state_changed_with_no_transition_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    umock_c_reset_all_calls();

    // act
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPENING, MESSAGE_RECEIVER_STATE_OPENING);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* amqp_management_set_trace */

/* Tests_SRS_AMQP_MANAGEMENT_01_161: [ `amqp_management_set_trace` shall call `messagesender_set_trace` to enable/disable tracing on the message sender. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_162: [ `amqp_management_set_trace` shall call `messagereceiver_set_trace` to enable/disable tracing on the message receiver. ]*/
TEST_FUNCTION(amqp_management_set_trace_sets_trace_to_on_for_both_receiver_and_sender)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(messagesender_set_trace(test_message_sender, true));
    STRICT_EXPECTED_CALL(messagereceiver_set_trace(test_message_receiver, true));

    // act
    amqp_management_set_trace(amqp_management, true);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_161: [ `amqp_management_set_trace` shall call `messagesender_set_trace` to enable/disable tracing on the message sender. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_162: [ `amqp_management_set_trace` shall call `messagereceiver_set_trace` to enable/disable tracing on the message receiver. ]*/
TEST_FUNCTION(amqp_management_set_trace_sets_trace_to_off_for_both_receiver_and_sender)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(messagesender_set_trace(test_message_sender, false));
    STRICT_EXPECTED_CALL(messagereceiver_set_trace(test_message_receiver, false));

    // act
    amqp_management_set_trace(amqp_management, false);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_163: [ If `amqp_management` is NULL, `amqp_management_set_trace` shal do nothing. ]*/
TEST_FUNCTION(amqp_management_set_trace_with_NULL_handle_does_nothing)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    umock_c_reset_all_calls();

    // act
    amqp_management_set_trace(NULL, false);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* amqp_management_set_override_status_code_key_name */

/* Tests_SRS_AMQP_MANAGEMENT_01_167: [ `amqp_management_set_override_status_code_key_name` shall set the status code key name used to parse the status code from the reply messages to `override_status_code_key_name`. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_170: [ On success, `amqp_management_set_override_status_code_key_name` shall return 0. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_168: [ `amqp_management_set_override_status_code_key_name` shall copy the `override_status_code_key_name` string. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_169: [ `amqp_management_set_override_status_code_key_name` shall free any string previously used for the status code key name. ]*/
TEST_FUNCTION(amqp_management_set_override_status_code_key_name_succeeds)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "xxx"));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = amqp_management_set_override_status_code_key_name(amqp_management, "xxx");

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_171: [ If `amqp_management` is NULL, `amqp_management_set_override_status_code_key_name` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(amqp_management_set_override_status_code_key_name_with_NULL_handle_fails)
{
    // arrange
    int result;

    // act
    result = amqp_management_set_override_status_code_key_name(NULL, "xxx");

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_172: [ If `override_status_code_key_name` is NULL, `amqp_management_set_override_status_code_key_name` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(amqp_management_set_override_status_code_key_name_with_NULL_string_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    umock_c_reset_all_calls();

    // act
    result = amqp_management_set_override_status_code_key_name(amqp_management, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_173: [ If any error occurs in copying the `override_status_code_key_name` string, `amqp_management_set_override_status_code_key_name` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_copying_the_string_fails_amqp_management_set_override_status_code_key_name_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "xxx"))
        .SetReturn(1);

    // act
    result = amqp_management_set_override_status_code_key_name(amqp_management, "xxx");

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_167: [ `amqp_management_set_override_status_code_key_name` shall set the status code key name used to parse the status code from the reply messages to `override_status_code_key_name`. ]*/
TEST_FUNCTION(when_amqp_management_set_override_status_code_key_name_is_called_the_override_status_code_key_name_is_used)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;
    int32_t status_code = 200;
    const char* test_status_description = "my error ...";
    uint64_t correlation_id = 0;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_set_override_status_code_key_name(amqp_management, "xxx");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    saved_on_message_send_complete(saved_on_message_send_complete_context, MESSAGE_SEND_OK, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(properties_get_correlation_id(test_properties, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_correlation_id_value(&test_correlation_id_value, sizeof(test_correlation_id_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(test_correlation_id_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ulong_value(&correlation_id, sizeof(correlation_id));
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_described_value(test_application_properties))
        .SetReturn(test_application_properties_map);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("xxx"))
        .SetReturn(test_status_code_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_code_key))
        .SetReturn(test_status_code_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_int(test_status_code_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_int_value(&status_code, sizeof(status_code));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusDescription"))
        .SetReturn(test_status_description_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_description_key))
        .SetReturn(test_status_description_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_string(test_status_description_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_string_value(&test_status_description, sizeof(test_status_description));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_amqp_management_execute_operation_complete((void*)0x4244, AMQP_MANAGEMENT_EXECUTE_OPERATION_OK, 200, "my error ...", test_message));

    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messaging_delivery_accepted());
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_key));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_key));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_accepted, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* amqp_management_set_override_status_description_key_name */

/* Tests_SRS_AMQP_MANAGEMENT_01_174: [ `amqp_management_set_override_status_description_key_name` shall set the status description key name used to parse the status description from the reply messages to `over ride_status_description_key_name`.]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_175: [ `amqp_management_set_override_status_description_key_name` shall copy the `override_status_description_key_name` string. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_176: [ `amqp_management_set_override_status_description_key_name` shall free any string previously used for the status description key name. ]*/
/* Tests_SRS_AMQP_MANAGEMENT_01_177: [ On success, `amqp_management_set_override_status_description_key_name` shall return 0. ]*/
TEST_FUNCTION(amqp_management_set_override_status_description_key_name_succeeds)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "xxx"));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = amqp_management_set_override_status_description_key_name(amqp_management, "xxx");

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_178: [ If `amqp_management` is NULL, `amqp_management_set_override_status_description_key_name` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(amqp_management_set_override_status_description_key_name_with_NULL_handle_fails)
{
    // arrange
    int result;

    // act
    result = amqp_management_set_override_status_description_key_name(NULL, "xxx");

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_179: [ If `override_status_description_key_name` is NULL, `amqp_management_set_override_status_description_key_name` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(amqp_management_set_override_status_description_key_name_with_NULL_string_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    umock_c_reset_all_calls();

    // act
    result = amqp_management_set_override_status_description_key_name(amqp_management, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_180: [ If any error occurs in copying the `override_status_description_key_name` string, `amqp_management_set_override_status_description_key_name` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_copying_the_string_fails_amqp_management_set_override_status_description_key_name_fails)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    int result;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "xxx"))
        .SetReturn(1);

    // act
    result = amqp_management_set_override_status_description_key_name(amqp_management, "xxx");

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    amqp_management_destroy(amqp_management);
}

TEST_FUNCTION(amqp_management_fails_if_response_to_operation_is_received_before_on_send_complete_is_called)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;
    int32_t status_code = 200;
    const char* test_status_description = "my error ...";
    uint64_t correlation_id = 0;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_set_override_status_description_key_name(amqp_management, "xxx");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    // Simulating no DISPOSITION received by not calling missing saved_on_message_send_complete(...)
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(properties_get_correlation_id(test_properties, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_correlation_id_value(&test_correlation_id_value, sizeof(test_correlation_id_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(test_correlation_id_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ulong_value(&correlation_id, sizeof(correlation_id));
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_described_value(test_application_properties))
        .SetReturn(test_application_properties_map);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusCode"))
        .SetReturn(test_status_code_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_code_key))
        .SetReturn(test_status_code_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_int(test_status_code_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_int_value(&status_code, sizeof(status_code));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("xxx"))
        .SetReturn(test_status_description_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_description_key))
        .SetReturn(test_status_description_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_string(test_status_description_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_string_value(&test_status_description, sizeof(test_status_description));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(async_operation_cancel(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_amqp_management_execute_operation_complete((void*)0x4244, AMQP_MANAGEMENT_EXECUTE_OPERATION_FAILED_BAD_STATUS, 200, "my error ...", test_message));

    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messaging_delivery_accepted());
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_key));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_key));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_accepted, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

/* Tests_SRS_AMQP_MANAGEMENT_01_174: [ `amqp_management_set_override_status_description_key_name` shall set the status description key name used to parse the status description from the reply messages to `over ride_status_description_key_name`.]*/
TEST_FUNCTION(when_amqp_management_set_override_status_description_key_name_is_called_the_override_status_code_key_name_is_used)
{
    // arrange
    AMQP_MANAGEMENT_HANDLE amqp_management;
    AMQP_VALUE result;
    int32_t status_code = 200;
    const char* test_status_description = "my error ...";
    uint64_t correlation_id = 0;

    amqp_management = amqp_management_create(test_session_handle, "test_node");
    (void)amqp_management_set_override_status_description_key_name(amqp_management, "xxx");
    (void)amqp_management_open_async(amqp_management, test_on_amqp_management_open_complete, (void*)0x4242, test_on_amqp_management_error, (void*)0x4243);
    saved_on_message_sender_state_changed(saved_on_message_sender_state_changed_context, MESSAGE_SENDER_STATE_OPEN, MESSAGE_SENDER_STATE_OPENING);
    saved_on_message_receiver_state_changed(saved_on_message_receiver_state_changed_context, MESSAGE_RECEIVER_STATE_OPEN, MESSAGE_RECEIVER_STATE_OPENING);
    umock_c_reset_all_calls();
    setup_calls_for_pending_operation_with_correlation_id(0);
    (void)amqp_management_execute_operation_async(amqp_management, "some_operation", "some_type", "en-US", test_message, test_on_amqp_management_execute_operation_complete, (void*)0x4244);
    saved_on_message_send_complete(saved_on_message_send_complete_context, MESSAGE_SEND_OK, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_get_application_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_application_properties(&test_application_properties, sizeof(test_application_properties));
    STRICT_EXPECTED_CALL(message_get_properties(test_message, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_properties(&test_properties, sizeof(test_properties));
    STRICT_EXPECTED_CALL(properties_get_correlation_id(test_properties, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_correlation_id_value(&test_correlation_id_value, sizeof(test_correlation_id_value));
    STRICT_EXPECTED_CALL(amqpvalue_get_ulong(test_correlation_id_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ulong_value(&correlation_id, sizeof(correlation_id));
    STRICT_EXPECTED_CALL(amqpvalue_get_inplace_described_value(test_application_properties))
        .SetReturn(test_application_properties_map);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("statusCode"))
        .SetReturn(test_status_code_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_code_key))
        .SetReturn(test_status_code_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_int(test_status_code_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_int_value(&status_code, sizeof(status_code));
    STRICT_EXPECTED_CALL(amqpvalue_create_string("xxx"))
        .SetReturn(test_status_description_key);
    STRICT_EXPECTED_CALL(amqpvalue_get_map_value(test_application_properties_map, test_status_description_key))
        .SetReturn(test_status_description_value);
    STRICT_EXPECTED_CALL(amqpvalue_get_string(test_status_description_value, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_string_value(&test_status_description, sizeof(test_status_description));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist_handle));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_amqp_management_execute_operation_complete((void*)0x4244, AMQP_MANAGEMENT_EXECUTE_OPERATION_OK, 200, "my error ...", test_message));

    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist_handle, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(messaging_delivery_accepted());
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_description_key));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_status_code_key));
    STRICT_EXPECTED_CALL(properties_destroy(test_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_application_properties));

    // act
    result = saved_on_message_received(saved_on_message_received_context, test_message);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, test_delivery_accepted, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    amqp_management_destroy(amqp_management);
}

END_TEST_SUITE(amqp_management_ut)
