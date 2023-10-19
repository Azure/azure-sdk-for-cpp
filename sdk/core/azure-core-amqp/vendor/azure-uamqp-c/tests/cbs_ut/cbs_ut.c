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

#define ENABLE_MOCKS

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_uamqp_c/message.h"
#include "azure_uamqp_c/session.h"
#include "azure_uamqp_c/amqp_management.h"

#undef ENABLE_MOCKS

#include "azure_uamqp_c/cbs.h"

static TEST_MUTEX_HANDLE g_testByTest;

static SESSION_HANDLE test_session_handle = (SESSION_HANDLE)0x4242;
static AMQP_MANAGEMENT_HANDLE test_amqp_management_handle = (AMQP_MANAGEMENT_HANDLE)0x4243;
static MESSAGE_HANDLE test_message = (MESSAGE_HANDLE)0x4300;
static AMQP_VALUE test_token_value = (AMQP_VALUE)0x4301;
static AMQP_VALUE test_map_value = (AMQP_VALUE)0x4302;
static AMQP_VALUE test_name_propery_key = (AMQP_VALUE)0x4303;
static AMQP_VALUE test_name_propery_value = (AMQP_VALUE)0x4304;
static SINGLYLINKEDLIST_HANDLE test_singlylinkedlist = (SINGLYLINKEDLIST_HANDLE)0x4305;
static AMQP_VALUE test_default_amqp_value = (AMQP_VALUE)0x4306;
static MESSAGE_HANDLE test_response_message = (MESSAGE_HANDLE)0x4307;
static ON_AMQP_MANAGEMENT_OPEN_COMPLETE saved_on_amqp_management_open_complete;
static void* saved_on_amqp_management_open_complete_context;
static ON_AMQP_MANAGEMENT_ERROR saved_on_amqp_management_error;
static void* saved_on_amqp_management_error_context;
static ON_AMQP_MANAGEMENT_EXECUTE_OPERATION_COMPLETE saved_on_execute_operation_complete;
static void* saved_on_execute_operation_complete_context;
static ASYNC_OPERATION_HANDLE test_my_amqp_management_execute_operation_async_result = (ASYNC_OPERATION_HANDLE)0x4308;

#define SIZE_OF_CBS_OPERATION_STRUCT 48

MOCK_FUNCTION_WITH_CODE(, void, test_on_cbs_open_complete, void*, context, CBS_OPEN_COMPLETE_RESULT, open_complete_result);
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_on_cbs_error, void*, context);
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_on_cbs_put_token_complete, void*, context, CBS_OPERATION_RESULT, put_token_complete_result, unsigned int, status_code, const char*, status_description);
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_on_cbs_delete_token_complete, void*, context, CBS_OPERATION_RESULT, delete_token_complete_result, unsigned int, status_code, const char*, status_description);
MOCK_FUNCTION_END();

AMQP_MANAGEMENT_HANDLE my_amqp_management_create(SESSION_HANDLE session, const char* management_node)
{
    (void)session;
    (void)management_node;
    return test_amqp_management_handle;
}

int my_amqp_management_open_async(AMQP_MANAGEMENT_HANDLE amqp_management, ON_AMQP_MANAGEMENT_OPEN_COMPLETE on_amqp_management_open_complete, void* on_amqp_management_open_complete_context, ON_AMQP_MANAGEMENT_ERROR on_amqp_management_error, void* on_amqp_management_error_context)
{
    (void)amqp_management;
    saved_on_amqp_management_open_complete = on_amqp_management_open_complete;
    saved_on_amqp_management_open_complete_context = on_amqp_management_open_complete_context;
    saved_on_amqp_management_error = on_amqp_management_error;
    saved_on_amqp_management_error_context = on_amqp_management_error_context;
    return 0;
}


ASYNC_OPERATION_HANDLE my_amqp_management_execute_operation_async(AMQP_MANAGEMENT_HANDLE amqp_management, const char* operation, const char* type, const char* locales, MESSAGE_HANDLE message, ON_AMQP_MANAGEMENT_EXECUTE_OPERATION_COMPLETE on_execute_operation_complete, void* on_execute_operation_complete_context)
{
    (void)message;
    (void)locales;
    (void)type;
    (void)operation;
    (void)amqp_management;
    saved_on_execute_operation_complete = on_execute_operation_complete;
    saved_on_execute_operation_complete_context = on_execute_operation_complete_context;
    return test_my_amqp_management_execute_operation_async_result;
}

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
    return 0;
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

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)
TEST_DEFINE_ENUM_TYPE(CBS_OPEN_COMPLETE_RESULT, CBS_OPEN_COMPLETE_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(CBS_OPEN_COMPLETE_RESULT, CBS_OPEN_COMPLETE_RESULT_VALUES);
TEST_DEFINE_ENUM_TYPE(CBS_OPERATION_RESULT, CBS_OPERATION_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(CBS_OPERATION_RESULT, CBS_OPERATION_RESULT_VALUES);

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
    unsigned char context[SIZE_OF_CBS_OPERATION_STRUCT]; // This block of memory will be used in cbs.c for the CBS_OPERATION instance.
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

BEGIN_TEST_SUITE(cbs_ut)

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

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, my_gballoc_calloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_HOOK(amqp_management_create, my_amqp_management_create);
    REGISTER_GLOBAL_MOCK_HOOK(amqp_management_open_async, my_amqp_management_open_async);
    REGISTER_GLOBAL_MOCK_HOOK(amqp_management_execute_operation_async, my_amqp_management_execute_operation_async);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_create_string, test_default_amqp_value);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_create_map, test_default_amqp_value);
    REGISTER_GLOBAL_MOCK_RETURN(amqpvalue_set_map_value, 0);
    REGISTER_TYPE(CBS_OPEN_COMPLETE_RESULT, CBS_OPEN_COMPLETE_RESULT);
    REGISTER_TYPE(CBS_OPERATION_RESULT, CBS_OPERATION_RESULT);
    REGISTER_GLOBAL_MOCK_RETURN(message_create, test_message);
    REGISTER_GLOBAL_MOCK_RETURN(singlylinkedlist_create, test_singlylinkedlist);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_get_head_item, my_singlylinkedlist_get_head_item);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_remove, my_singlylinkedlist_remove);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_add, my_singlylinkedlist_add);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_item_get_value, my_singlylinkedlist_item_get_value);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_find, my_singlylinkedlist_find);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_remove_if, my_singlylinkedlist_remove_if);
    REGISTER_UMOCKC_PAIRED_CREATE_DESTROY_CALLS(amqp_management_create, amqp_management_destroy);
    REGISTER_UMOCKC_PAIRED_CREATE_DESTROY_CALLS(message_create, message_destroy);
    REGISTER_UMOCKC_PAIRED_CREATE_DESTROY_CALLS(properties_create, properties_destroy);
    REGISTER_GLOBAL_MOCK_HOOK(async_operation_create, my_async_operation_create);
    REGISTER_GLOBAL_MOCK_HOOK(async_operation_destroy, my_async_operation_destroy);

    REGISTER_UMOCK_ALIAS_TYPE(CBS_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SESSION_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(AMQP_MANAGEMENT_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_AMQP_MANAGEMENT_OPEN_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_AMQP_MANAGEMENT_ERROR, void*);
    REGISTER_UMOCK_ALIAS_TYPE(MESSAGE_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(AMQP_VALUE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_AMQP_MANAGEMENT_EXECUTE_OPERATION_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SINGLYLINKEDLIST_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_ITEM_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_CONDITION_FUNCTION, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ASYNC_OPERATION_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ASYNC_OPERATION_CANCEL_HANDLER_FUNC, void*);
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
}

TEST_FUNCTION_CLEANUP(test_cleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* cbs_create */

/* Tests_SRS_CBS_01_001: [ `cbs_create` shall create a new CBS instance and on success return a non-NULL handle to it. ]*/
/* Tests_SRS_CBS_01_034: [ `cbs_create` shall create an AMQP management handle by calling `amqp_management_create`. ]*/
/* Tests_SRS_CBS_01_097: [ `cbs_create` shall create a singly linked list for pending operations by calling `singlylinkedlist_create`. ]*/
/* Tests_SRS_CBS_01_002: [ Tokens are communicated between AMQP peers by sending specially-formatted AMQP messages to the Claims-based Security Node. ]*/
/* Tests_SRS_CBS_01_003: [ The mechanism follows the scheme defined in the AMQP Management specification [AMQPMAN]. ]*/
/* Tests_SRS_CBS_01_117: [ `cbs_create` shall set the override status code key name on the AMQP management handle to `status-code` by calling `amqp_management_set_override_status_code_key_name`. ]*/
/* Tests_SRS_CBS_01_118: [ `cbs_create` shall set the override status description key name on the AMQP management handle to `status-description` by calling `amqp_management_set_override_status_description_key_name`. ]*/
TEST_FUNCTION(cbs_create_returns_a_valid_handle)
{
    // arrange
    CBS_HANDLE cbs;
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(amqp_management_create(test_session_handle, "$cbs"));
    STRICT_EXPECTED_CALL(amqp_management_set_override_status_code_key_name(test_amqp_management_handle, "status-code"));
    STRICT_EXPECTED_CALL(amqp_management_set_override_status_description_key_name(test_amqp_management_handle, "status-description"));

    // act
    cbs = cbs_create(test_session_handle);

    // assert
    ASSERT_IS_NOT_NULL(cbs);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_033: [** If `session` is NULL then `cbs_create` shall fail and return NULL. ]*/
TEST_FUNCTION(cbs_create_with_NULL_session_handle_fails)
{
    // arrange
    CBS_HANDLE cbs;

    // act
    cbs = cbs_create(NULL);

    // assert
    ASSERT_IS_NULL(cbs);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_CBS_01_035: [ If `amqp_management_create` fails then `cbs_create` shall fail and return NULL. ]*/
/* Tests_SRS_CBS_01_076: [ If allocating memory for the new handle fails, `cbs_create` shall fail and return NULL. ]*/
/* Tests_SRS_CBS_01_101: [ If `singlylinkedlist_create` fails, `cbs_create` shall fail and return NULL. ]*/
/* Tests_SRS_CBS_01_116: [ If setting the override key names fails, then `cbs_create` shall fail and return NULL. ]*/
TEST_FUNCTION(when_one_of_the_functions_called_by_cbs_create_fails_then_cbs_create_fails)
{
    // arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    size_t count;
    size_t index;
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(singlylinkedlist_create())
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(amqp_management_create(test_session_handle, "$cbs"))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(amqp_management_set_override_status_code_key_name(test_amqp_management_handle, "status-code"))
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(amqp_management_set_override_status_description_key_name(test_amqp_management_handle, "status-description"))
        .SetFailReturn(1);
    umock_c_negative_tests_snapshot();

    count = umock_c_negative_tests_call_count();
    for (index = 0; index < count; index++)
    {
        char tmp_msg[128];
        CBS_HANDLE cbs;
        (void)sprintf(tmp_msg, "Failure in test %u/%u", (unsigned int)(index + 1), (unsigned int)count);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        // act
        cbs = cbs_create(test_session_handle);

        // assert
        ASSERT_IS_NULL(cbs, tmp_msg);
    }

    // cleanup
    umock_c_negative_tests_deinit();
}

/* cbs_destroy */

/* Tests_SRS_CBS_01_036: [ `cbs_destroy` shall free all resources associated with the handle `cbs`. ]*/
/* Tests_SRS_CBS_01_038: [ `cbs_destroy` shall free the AMQP management handle created in `cbs_create` by calling `amqp_management_destroy`. ]*/
/* Tests_SRS_CBS_01_098: [ `cbs_destroy` shall free the pending operations list by calling `singlylinkedlist_destroy`. ]*/
/* Tests_SRS_CBS_01_099: [ All pending operations shall be freed. ]*/
TEST_FUNCTION(cbs_destroy_frees_all_resources)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_management_destroy(test_amqp_management_handle));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist));
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(test_singlylinkedlist));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    cbs_destroy(cbs);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_CBS_01_099: [ All pending operations shall be freed. ]*/
/* Tests_SRS_CBS_01_100: [ If the CBS instance is not closed, all actions performed by `cbs_close` shall be performed. ]*/
TEST_FUNCTION(cbs_destroy_frees_all_resources_including_the_pending_operations)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    (void)cbs_put_token_async(cbs, "some_type", "my_audience", "blah_token", test_on_cbs_put_token_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_management_close(test_amqp_management_handle));
    STRICT_EXPECTED_CALL(amqp_management_destroy(test_amqp_management_handle));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_cbs_put_token_complete((void*)0x4244, CBS_OPERATION_RESULT_INSTANCE_CLOSED, 0, NULL));
    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinkedlist));
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(test_singlylinkedlist));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    cbs_destroy(cbs);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_CBS_01_037: [ If `cbs` is NULL, `cbs_destroy` shall do nothing. ]*/
TEST_FUNCTION(cbs_destroy_with_NULL_handle_does_nothing)
{
    // arrange

    // act
    cbs_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* cbs_open_async */

/* Tests_SRS_CBS_01_039: [ `cbs_open_async` shall open the cbs communication by calling `amqp_management_open_async` on the AMQP management handle created in `cbs_create`. ]*/
/* Tests_SRS_CBS_01_077: [ On success, `cbs_open_async` shall return 0. ]*/
/* Tests_SRS_CBS_01_078: [ The cbs instance shall be considered OPENING until the `on_amqp_management_open_complete` callback is called by the AMQP management instance indicating that the open has completed (succesfull or not). ]*/
TEST_FUNCTION(cbs_open_async_opens_the_amqp_management_instance)
{
    // arrange
    CBS_HANDLE cbs;
    int result;
    cbs = cbs_create(test_session_handle);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_management_open_async(test_amqp_management_handle, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_040: [ If any of the arguments `cbs`, `on_cbs_open_complete` or `on_cbs_error` is NULL, then `cbs_open_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(cbs_open_async_with_NULL_handle_fails)
{
    // arrange
    int result;
    umock_c_reset_all_calls();

    // act
    result = cbs_open_async(NULL, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_CBS_01_040: [ If any of the arguments `cbs`, `on_cbs_open_complete` or `on_cbs_error` is NULL, then `cbs_open_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(cbs_open_async_with_NULL_on_open_complete_callback_fails)
{
    // arrange
    CBS_HANDLE cbs;
    int result;
    cbs = cbs_create(test_session_handle);
    umock_c_reset_all_calls();

    // act
    result = cbs_open_async(cbs, NULL, (void*)0x4242, test_on_cbs_error, (void*)0x4243);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_040: [ If any of the arguments `cbs`, `on_cbs_open_complete` or `on_cbs_error` is NULL, then `cbs_open_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(cbs_open_async_with_NULL_on_error_callback_fails)
{
    // arrange
    CBS_HANDLE cbs;
    int result;
    cbs = cbs_create(test_session_handle);
    umock_c_reset_all_calls();

    // act
    result = cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, NULL, (void*)0x4243);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_041: [ If `amqp_management_open_async` fails, shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_amqpmanagement_open_fails_cbs_open_async_fails)
{
    // arrange
    CBS_HANDLE cbs;
    int result;
    cbs = cbs_create(test_session_handle);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_management_open_async(test_amqp_management_handle, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1);

    // act
    result = cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_042: [ `on_cbs_open_complete_context` and `on_cbs_error_context` shall be allowed to be NULL. ]*/
TEST_FUNCTION(cbs_open_async_with_NULL_on_open_complete_context_succeeds)
{
    // arrange
    CBS_HANDLE cbs;
    int result;
    cbs = cbs_create(test_session_handle);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_management_open_async(test_amqp_management_handle, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = cbs_open_async(cbs, test_on_cbs_open_complete, NULL, test_on_cbs_error, (void*)0x4243);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_042: [ `on_cbs_open_complete_context` and `on_cbs_error_context` shall be allowed to be NULL. ]*/
TEST_FUNCTION(cbs_open_async_with_NULL_on_error_context_succeeds)
{
    // arrange
    CBS_HANDLE cbs;
    int result;
    cbs = cbs_create(test_session_handle);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_management_open_async(test_amqp_management_handle, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4243, test_on_cbs_error, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_043: [ `cbs_open_async` while already open or opening shall fail and return a non-zero value. ]*/
TEST_FUNCTION(cbs_open_async_while_opening_fails)
{
    // arrange
    CBS_HANDLE cbs;
    int result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4243, test_on_cbs_error, NULL);
    umock_c_reset_all_calls();

    // act
    result = cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4243, test_on_cbs_error, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_043: [ `cbs_open_async` while already open or opening shall fail and return a non-zero value. ]*/
TEST_FUNCTION(cbs_open_async_while_already_open_fails)
{
    // arrange
    CBS_HANDLE cbs;
    int result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    // act
    result = cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4243, test_on_cbs_error, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_039: [ `cbs_open_async` shall open the cbs communication by calling `amqp_management_open_async` on the AMQP management handle created in `cbs_create`. ]*/
/* Tests_SRS_CBS_01_077: [ On success, `cbs_open_async` shall return 0. ]*/
TEST_FUNCTION(after_an_open_cancelled_due_to_amqp_management_cbs_open_async_still_succeeds)
{
    // arrange
    CBS_HANDLE cbs;
    int result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_CANCELLED);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_management_open_async(test_amqp_management_handle, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_039: [ `cbs_open_async` shall open the cbs communication by calling `amqp_management_open_async` on the AMQP management handle created in `cbs_create`. ]*/
/* Tests_SRS_CBS_01_077: [ On success, `cbs_open_async` shall return 0. ]*/
TEST_FUNCTION(after_an_open_error_due_to_amqp_management_cbs_open_async_still_succeeds)
{
    // arrange
    CBS_HANDLE cbs;
    int result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_ERROR);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_management_open_async(test_amqp_management_handle, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_039: [ `cbs_open_async` shall open the cbs communication by calling `amqp_management_open_async` on the AMQP management handle created in `cbs_create`. ]*/
/* Tests_SRS_CBS_01_077: [ On success, `cbs_open_async` shall return 0. ]*/
TEST_FUNCTION(after_an_open_error_due_to_amqp_management_error_callback_cbs_open_async_still_succeeds)
{
    // arrange
    CBS_HANDLE cbs;
    int result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_error(saved_on_amqp_management_open_complete_context);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_management_open_async(test_amqp_management_handle, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    cbs_destroy(cbs);
}

/* cbs_close */

/* Tests_SRS_CBS_01_044: [ `cbs_close` shall close the CBS instance by calling `amqp_management_close` on the underlying AMQP management handle. ]*/
/* Tests_SRS_CBS_01_080: [ On success, `cbs_close` shall return 0. ]*/
TEST_FUNCTION(cbs_close_closes_the_underlying_amqp_management_instance)
{
    // arrange
    CBS_HANDLE cbs;
    int result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_management_close(test_amqp_management_handle));

    // act
    result = cbs_close(cbs);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_079: [ If `cbs_close` is called while OPENING, the `on_cbs_open_complete` callback shall be called with `CBS_OPEN_CANCELLED`, while passing the `on_cbs_open_complete_context` as argument. ]*/
/* Tests_SRS_CBS_01_080: [ On success, `cbs_close` shall return 0. ]*/
TEST_FUNCTION(cbs_close_when_opening_cancels_the_open)
{
    // arrange
    CBS_HANDLE cbs;
    int result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_management_close(test_amqp_management_handle));
    STRICT_EXPECTED_CALL(test_on_cbs_open_complete((void*)0x4242, CBS_OPEN_CANCELLED));

    // act
    result = cbs_close(cbs);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_045: [ If the argument `cbs` is NULL, `cbs_close` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(cbs_close_with_NULL_handle_fails)
{
    // arrange
    int result;

    // act
    result = cbs_close(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_CBS_01_046: [ If `amqp_management_close` fails, `cbs_close` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_amqpmanagement_close_fails_then_cbs_close_fails)
{
    // arrange
    CBS_HANDLE cbs;
    int result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_management_close(test_amqp_management_handle))
        .SetReturn(1);

    // act
    result = cbs_close(cbs);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_047: [ `cbs_close` when closed shall fail and return a non-zero value. ]*/
TEST_FUNCTION(cbs_close_after_cbs_close_fails)
{
    // arrange
    CBS_HANDLE cbs;
    int result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    (void)cbs_close(cbs);
    umock_c_reset_all_calls();

    // act
    result = cbs_close(cbs);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_048: [ `cbs_close` when not opened shall fail and return a non-zero value. ]*/
TEST_FUNCTION(cbs_close_when_not_open_fails)
{
    // arrange
    CBS_HANDLE cbs;
    int result;
    cbs = cbs_create(test_session_handle);
    umock_c_reset_all_calls();

    // act
    result = cbs_close(cbs);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    cbs_destroy(cbs);
}

/* cbs_put_token_async */

/* Tests_SRS_CBS_01_049: [ `cbs_put_token_async` shall construct a request message for the `put-token` operation. ]*/
/* Tests_SRS_CBS_01_081: [ On success `cbs_put_token_async` shall return an ASYNC_OPERATION_HANDLE. ]*/
/* Tests_SRS_CBS_01_051: [ `cbs_put_token_async` shall start the AMQP management operation by calling `amqp_management_execute_operation_async`, while passing to it: ]*/
/* Tests_SRS_CBS_01_052: [ The `amqp_management` argument shall be the one for the AMQP management instance created in `cbs_create`. ]*/
/* Tests_SRS_CBS_01_053: [ The `operation` argument shall be `put-token`. ]*/
/* Tests_SRS_CBS_01_054: [ The `type` argument shall be set to the `type` argument. ]*/
/* Tests_SRS_CBS_01_055: [ The `locales` argument shall be set to NULL. ]*/
/* Tests_SRS_CBS_01_056: [ The `message` argument shall be the message constructed earlier according to the CBS spec. ]*/
/* Tests_SRS_CBS_01_057: [ The arguments `on_execute_operation_complete` and `context` shall be set to a callback that is to be called by the AMQP management module when the operation is complete. ]*/
/* Tests_SRS_CBS_01_005: [ operation    No    string    "put-token" ]*/
/* Tests_SRS_CBS_01_006: [ Type    No    string    The type of the token being put, e.g., "amqp:jwt". ]*/
/* Tests_SRS_CBS_01_007: [ name    No    string    The "audience" to which the token applies. ]*/
/* Tests_SRS_CBS_01_009: [ The body of the message MUST contain the token. ]*/
TEST_FUNCTION(cbs_put_token_async_creates_the_message_and_starts_the_amqp_management_operation)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_create());
    STRICT_EXPECTED_CALL(amqpvalue_create_string("blah_token"))
        .SetReturn(test_token_value);
    STRICT_EXPECTED_CALL(message_set_body_amqp_value(test_message, test_token_value));
    STRICT_EXPECTED_CALL(amqpvalue_create_map())
        .SetReturn(test_map_value);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("name"))
        .SetReturn(test_name_propery_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("my_audience"))
        .SetReturn(test_name_propery_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_map_value, test_name_propery_key, test_name_propery_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_message, test_map_value));
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinkedlist, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqp_management_execute_operation_async(test_amqp_management_handle, "put-token", "some_type", NULL, test_message, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_map_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_token_value));
    STRICT_EXPECTED_CALL(message_destroy(test_message));

    // act
    result = cbs_put_token_async(cbs, "some_type", "my_audience", "blah_token", test_on_cbs_put_token_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_09_001: [ The `ASYNC_OPERATION_HANDLE` cancel function shall cancel the underlying amqp management operation, remove this operation from the pending list, destroy this async operation. ] */
TEST_FUNCTION(when_cbs_put_token_async_is_cancelled_succeeds)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_create());
    STRICT_EXPECTED_CALL(amqpvalue_create_string("blah_token"))
        .SetReturn(test_token_value);
    STRICT_EXPECTED_CALL(message_set_body_amqp_value(test_message, test_token_value));
    STRICT_EXPECTED_CALL(amqpvalue_create_map())
        .SetReturn(test_map_value);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("name"))
        .SetReturn(test_name_propery_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("my_audience"))
        .SetReturn(test_name_propery_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_map_value, test_name_propery_key, test_name_propery_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_message, test_map_value));
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinkedlist, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqp_management_execute_operation_async(test_amqp_management_handle, "put-token", "some_type", NULL, test_message, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_map_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_token_value));
    STRICT_EXPECTED_CALL(message_destroy(test_message));
    result = cbs_put_token_async(cbs, "some_type", "my_audience", "blah_token", test_on_cbs_put_token_complete, (void*)0x4244);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(async_operation_cancel(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(async_operation_destroy(result));

    // act
    ((ASYNC_OPERATION_CONTEXT_STRUCT_TEST*)result)->async_operation_cancel_handler(result);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}


/* Tests_SRS_CBS_01_050: [ If any of the arguments `cbs`, `type`, `audience`, `token` or `on_cbs_put_token_complete` is NULL `cbs_put_token_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(cbs_put_token_async_with_NULL_cbs_handle_fails)
{
    // arrange
    ASYNC_OPERATION_HANDLE result;

    // act
    result = cbs_put_token_async(NULL, "some_type", "my_audience", "blah_token", test_on_cbs_put_token_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);
}

/* Tests_SRS_CBS_01_050: [ If any of the arguments `cbs`, `type`, `audience`, `token` or `on_cbs_put_token_complete` is NULL `cbs_put_token_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(cbs_put_token_async_with_NULL_type_fails)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    // act
    result = cbs_put_token_async(cbs, NULL, "my_audience", "blah_token", test_on_cbs_put_token_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_050: [ If any of the arguments `cbs`, `type`, `audience`, `token` or `on_cbs_put_token_complete` is NULL `cbs_put_token_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(cbs_put_token_async_with_NULL_audience_fails)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    // act
    result = cbs_put_token_async(cbs, "some_type", NULL, "blah_token", test_on_cbs_put_token_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_050: [ If any of the arguments `cbs`, `type`, `audience`, `token` or `on_cbs_put_token_complete` is NULL `cbs_put_token_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(cbs_put_token_async_with_NULL_token_fails)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    // act
    result = cbs_put_token_async(cbs, "some_type", "my_audience", NULL, test_on_cbs_put_token_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_050: [ If any of the arguments `cbs`, `type`, `audience`, `token` or `on_cbs_put_token_complete` is NULL `cbs_put_token_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(cbs_put_token_async_with_NULL_complete_callback_fails)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    // act
    result = cbs_put_token_async(cbs, "some_type", "my_audience", "blah_token", NULL, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_083: [ `on_cbs_put_token_complete_context` shall be allowed to be NULL. ]*/
TEST_FUNCTION(cbs_put_token_async_with_NULL_complete_context_succeeds)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_create());
    STRICT_EXPECTED_CALL(amqpvalue_create_string("blah_token"))
        .SetReturn(test_token_value);
    STRICT_EXPECTED_CALL(message_set_body_amqp_value(test_message, test_token_value));
    STRICT_EXPECTED_CALL(amqpvalue_create_map())
        .SetReturn(test_map_value);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("name"))
        .SetReturn(test_name_propery_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("my_audience"))
        .SetReturn(test_name_propery_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_map_value, test_name_propery_key, test_name_propery_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_message, test_map_value));
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinkedlist, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqp_management_execute_operation_async(test_amqp_management_handle, "put-token", "some_type", NULL, test_message, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_map_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_token_value));
    STRICT_EXPECTED_CALL(message_destroy(test_message));

    // act
    result = cbs_put_token_async(cbs, "some_type", "my_audience", "blah_token", test_on_cbs_put_token_complete, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_072: [ If constructing the message fails, `cbs_put_token_async` shall fail and return a non-zero value. ]*/
/* Tests_SRS_CBS_01_084: [ If `amqp_management_execute_operation_async` fails `cbs_put_token_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_any_underlying_call_fails_cbs_put_token_async_fails)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    size_t count;
    size_t index;
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_create())
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("blah_token"))
        .SetReturn(test_token_value).SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(message_set_body_amqp_value(test_message, test_token_value))
        .SetFailReturn(42);
    STRICT_EXPECTED_CALL(amqpvalue_create_map())
        .SetReturn(test_map_value).SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("name"))
        .SetReturn(test_name_propery_key).SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("my_audience"))
        .SetReturn(test_name_propery_value).SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_map_value, test_name_propery_key, test_name_propery_value))
        .SetFailReturn(42);
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_message, test_map_value))
        .SetFailReturn(42);
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinkedlist, IGNORED_PTR_ARG))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(amqp_management_execute_operation_async(test_amqp_management_handle, "put-token", "some_type", NULL, test_message, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetFailReturn(NULL);

    umock_c_negative_tests_snapshot();

    count = umock_c_negative_tests_call_count();
    for (index = 0; index < count; index++)
    {
        char tmp_msg[128];

        if ((index == 7) ||
            (index == 8))
        {
            continue;
        }

        (void)sprintf(tmp_msg, "Failure in test %u/%u", (unsigned int)(index + 1), (unsigned int)count);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        // act
        result = cbs_put_token_async(cbs, "some_type", "my_audience", "blah_token", test_on_cbs_put_token_complete, NULL);

        // assert
        ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result, tmp_msg);
    }

    // cleanup
    umock_c_negative_tests_deinit();
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_058: [ If `cbs_put_token_async` is called when the CBS instance is not yet open or in error, it shall fail and return `NULL`. ]*/
TEST_FUNCTION(cbs_put_token_async_when_not_open_fails)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    cbs = cbs_create(test_session_handle);
    umock_c_reset_all_calls();

    // act
    result = cbs_put_token_async(cbs, "some_type", "my_audience", "blah_token", test_on_cbs_put_token_complete, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_058: [ If `cbs_put_token_async` is called when the CBS instance is not yet open or in error, it shall fail and return `NULL`. ]*/
TEST_FUNCTION(cbs_put_token_async_while_opening_succeeds)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_create());
    STRICT_EXPECTED_CALL(amqpvalue_create_string("blah_token"))
        .SetReturn(test_token_value);
    STRICT_EXPECTED_CALL(message_set_body_amqp_value(test_message, test_token_value));
    STRICT_EXPECTED_CALL(amqpvalue_create_map())
        .SetReturn(test_map_value);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("name"))
        .SetReturn(test_name_propery_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("my_audience"))
        .SetReturn(test_name_propery_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_map_value, test_name_propery_key, test_name_propery_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_message, test_map_value));
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinkedlist, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqp_management_execute_operation_async(test_amqp_management_handle, "put-token", "some_type", NULL, test_message, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_map_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_token_value));
    STRICT_EXPECTED_CALL(message_destroy(test_message));

    // act
    result = cbs_put_token_async(cbs, "some_type", "my_audience", "blah_token", test_on_cbs_put_token_complete, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_058: [ If `cbs_put_token_async` is called when the CBS instance is not yet open or in error, it shall fail and return `NULL`. ]*/
TEST_FUNCTION(cbs_put_token_async_when_in_error_fails)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    saved_on_amqp_management_error(saved_on_amqp_management_error_context);
    umock_c_reset_all_calls();

    // act
    result = cbs_put_token_async(cbs, "some_type", "my_audience", "blah_token", test_on_cbs_put_token_complete, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);

    // cleanup
    cbs_destroy(cbs);
}

/* cbs_delete_token_async */

/* Tests_SRS_CBS_01_059: [ `cbs_delete_token_async` shall construct a request message for the `delete-token` operation. ]*/
/* Tests_SRS_CBS_01_082: [ On success `cbs_delete_token_async` shall return an ASYNC_OPERATION_HANDLE. ]*/
/* Tests_SRS_CBS_01_061: [ `cbs_delete_token_async` shall start the AMQP management operation by calling `amqp_management_execute_operation_async`, while passing to it: ]*/
/* Tests_SRS_CBS_01_085: [ The `amqp_management` argument shall be the one for the AMQP management instance created in `cbs_create`. ]*/
/* Tests_SRS_CBS_01_062: [ The `operation` argument shall be `delete-token`. ]*/
/* Tests_SRS_CBS_01_063: [ The `type` argument shall be set to the `type` argument. ]*/
/* Tests_SRS_CBS_01_064: [ The `locales` argument shall be set to NULL. ]*/
/* Tests_SRS_CBS_01_065: [ The `message` argument shall be the message constructed earlier according to the CBS spec. ]*/
/* Tests_SRS_CBS_01_066: [ The arguments `on_operation_complete` and `context` shall be set to a callback that is to be called by the AMQP management module when the operation is complete. ]*/
/* Tests_SRS_CBS_01_020: [ To instruct a peer to delete a token associated with a specific audience, a "delete-token" message can be sent to the CBS Node ]*/
/* Tests_SRS_CBS_01_021: [ The request message has the following application-properties: ]*/
/* Tests_SRS_CBS_01_022: [ operation    Yes    string    "delete-token" ]*/
/* Tests_SRS_CBS_01_023: [ Type    Yes    string    The type of the token being deleted, e.g., "amqp:jwt". ]*/
/* Tests_SRS_CBS_01_024: [ name    Yes    string    The "audience" of the token being deleted. ]*/
/* Tests_SRS_CBS_01_025: [ The body of the message MUST be empty. ]*/
TEST_FUNCTION(cbs_delete_token_async_creates_the_message_and_starts_the_amqp_management_operation)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_create());
    STRICT_EXPECTED_CALL(amqpvalue_create_map())
        .SetReturn(test_map_value);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("name"))
        .SetReturn(test_name_propery_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("my_audience"))
        .SetReturn(test_name_propery_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_map_value, test_name_propery_key, test_name_propery_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_message, test_map_value));
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinkedlist, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqp_management_execute_operation_async(test_amqp_management_handle, "delete-token", "some_type", NULL, test_message, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_map_value));
    STRICT_EXPECTED_CALL(message_destroy(test_message));

    // act
    result = cbs_delete_token_async(cbs, "some_type", "my_audience", test_on_cbs_delete_token_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_09_001: [ The `ASYNC_OPERATION_HANDLE` cancel function shall cancel the underlying amqp management operation, remove this operation from the pending list, destroy this async operation. ] */
TEST_FUNCTION(when_cbs_delete_token_async_is_cancelled)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_create());
    STRICT_EXPECTED_CALL(amqpvalue_create_map())
        .SetReturn(test_map_value);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("name"))
        .SetReturn(test_name_propery_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("my_audience"))
        .SetReturn(test_name_propery_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_map_value, test_name_propery_key, test_name_propery_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_message, test_map_value));
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinkedlist, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqp_management_execute_operation_async(test_amqp_management_handle, "delete-token", "some_type", NULL, test_message, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_map_value));
    STRICT_EXPECTED_CALL(message_destroy(test_message));
    result = cbs_delete_token_async(cbs, "some_type", "my_audience", test_on_cbs_delete_token_complete, (void*)0x4244);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(async_operation_cancel(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(async_operation_destroy(result));

    // act
    ((ASYNC_OPERATION_CONTEXT_STRUCT_TEST*)result)->async_operation_cancel_handler(result);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_060: [ If any of the arguments `cbs`, `type`, `audience` or `on_cbs_delete_token_complete` is NULL `cbs_put_token_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(cbs_delete_token_with_NULL_cbs_handle_fails)
{
    // arrange
    ASYNC_OPERATION_HANDLE result;

    // act
    result = cbs_delete_token_async(NULL, "test_type", "my_audience", test_on_cbs_delete_token_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);
}

/* Tests_SRS_CBS_01_060: [ If any of the arguments `cbs`, `type`, `audience` or `on_cbs_delete_token_complete` is NULL `cbs_put_token_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(cbs_delete_token_with_NULL_type_fails)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    // act
    result = cbs_delete_token_async(cbs, NULL, "my_audience", test_on_cbs_delete_token_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_060: [ If any of the arguments `cbs`, `type`, `audience` or `on_cbs_delete_token_complete` is NULL `cbs_put_token_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(cbs_delete_token_with_NULL_audience_fails)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    // act
    result = cbs_delete_token_async(cbs, "some_type", NULL, test_on_cbs_delete_token_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_060: [ If any of the arguments `cbs`, `type`, `audience` or `on_cbs_delete_token_complete` is NULL `cbs_put_token_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(cbs_delete_token_with_NULL_complete_callback_fails)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    // act
    result = cbs_delete_token_async(cbs, "some_type", "my_audience", NULL, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_086: [ `on_cbs_delete_token_complete_context` shall be allowed to be NULL. ]*/
TEST_FUNCTION(cbs_delete_token_async_with_NULL_complete_context_succeeds)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_create());
    STRICT_EXPECTED_CALL(amqpvalue_create_map())
        .SetReturn(test_map_value);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("name"))
        .SetReturn(test_name_propery_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("my_audience"))
        .SetReturn(test_name_propery_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_map_value, test_name_propery_key, test_name_propery_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_message, test_map_value));
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinkedlist, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqp_management_execute_operation_async(test_amqp_management_handle, "delete-token", "some_type", NULL, test_message, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_map_value));
    STRICT_EXPECTED_CALL(message_destroy(test_message));

    // act
    result = cbs_delete_token_async(cbs, "some_type", "my_audience", test_on_cbs_delete_token_complete, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_071: [ If constructing the message fails, `cbs_delete_token_async` shall fail and return a non-zero value. ]*/
/* Tests_SRS_CBS_01_087: [ If `amqp_management_execute_operation_async` fails `cbs_put_token_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_any_underlying_call_fails_cbs_delete_token_async_fails)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    size_t count;
    size_t index;
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_create())
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(amqpvalue_create_map())
        .SetReturn(test_map_value).SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("name"))
        .SetReturn(test_name_propery_key).SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("my_audience"))
        .SetReturn(test_name_propery_value).SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_map_value, test_name_propery_key, test_name_propery_value))
        .SetFailReturn(42);
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_message, test_map_value))
        .SetFailReturn(42);
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinkedlist, IGNORED_PTR_ARG))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(amqp_management_execute_operation_async(test_amqp_management_handle, "delete-token", "some_type", NULL, test_message, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetFailReturn(NULL);

    umock_c_negative_tests_snapshot();

    count = umock_c_negative_tests_call_count();
    for (index = 0; index < count; index++)
    {
        char tmp_msg[128];

        if ((index == 5) ||
            (index == 6))
        {
            continue;
        }

        (void)sprintf(tmp_msg, "Failure in test %u/%u", (unsigned int)(index + 1), (unsigned int)count);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        // act
        result = cbs_delete_token_async(cbs, "some_type", "my_audience", test_on_cbs_put_token_complete, NULL);

        // assert
        ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result, tmp_msg);
    }

    // cleanup
    umock_c_negative_tests_deinit();
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_067: [ If `cbs_delete_token_async` is called when the CBS instance is not yet open or in error, it shall fail and return `NULL`. ]*/
TEST_FUNCTION(cbs_delete_token_async_when_not_open_fails)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    cbs = cbs_create(test_session_handle);
    umock_c_reset_all_calls();

    // act
    result = cbs_delete_token_async(cbs, "some_type", "my_audience", test_on_cbs_delete_token_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_067: [ If `cbs_delete_token_async` is called when the CBS instance is not yet open or in error, it shall fail and return `NULL`. ]*/
TEST_FUNCTION(cbs_delete_token_async_while_opening_succeeds)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(message_create());
    STRICT_EXPECTED_CALL(amqpvalue_create_map())
        .SetReturn(test_map_value);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("name"))
        .SetReturn(test_name_propery_key);
    STRICT_EXPECTED_CALL(amqpvalue_create_string("my_audience"))
        .SetReturn(test_name_propery_value);
    STRICT_EXPECTED_CALL(amqpvalue_set_map_value(test_map_value, test_name_propery_key, test_name_propery_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_value));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_name_propery_key));
    STRICT_EXPECTED_CALL(message_set_application_properties(test_message, test_map_value));
    STRICT_EXPECTED_CALL(async_operation_create(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinkedlist, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqp_management_execute_operation_async(test_amqp_management_handle, "delete-token", "some_type", NULL, test_message, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(test_map_value));
    STRICT_EXPECTED_CALL(message_destroy(test_message));

    // act
    result = cbs_delete_token_async(cbs, "some_type", "my_audience", test_on_cbs_delete_token_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_067: [ If `cbs_delete_token_async` is called when the CBS instance is not yet open or in error, it shall fail and return `NULL`. ]*/
TEST_FUNCTION(cbs_delete_token_async_when_in_error_fails)
{
    // arrange
    CBS_HANDLE cbs;
    ASYNC_OPERATION_HANDLE result;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    saved_on_amqp_management_error(saved_on_amqp_management_error_context);
    umock_c_reset_all_calls();

    // act
    result = cbs_delete_token_async(cbs, "some_type", "my_audience", test_on_cbs_delete_token_complete, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(ASYNC_OPERATION_HANDLE, NULL, result);

    // cleanup
    cbs_destroy(cbs);
}

/* cbs_set_trace */

/* Tests_SRS_CBS_01_088: [ `cbs_set_trace` shall enable or disable tracing by calling `amqp_management_set_trace` to pass down the `trace_on` value. ]*/
TEST_FUNCTION(cbs_set_trace_with_true_succeeds)
{
    // arrange
    CBS_HANDLE cbs;
    int result;
    cbs = cbs_create(test_session_handle);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_management_set_trace(test_amqp_management_handle, true));

    // act
    result = cbs_set_trace(cbs, true);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_088: [ `cbs_set_trace` shall enable or disable tracing by calling `amqp_management_set_trace` to pass down the `trace_on` value. ]*/
/* Tests_SRS_CBS_01_089: [ On success, `cbs_set_trace` shall return 0. ]*/
TEST_FUNCTION(cbs_set_trace_with_false_succeeds)
{
    // arrange
    CBS_HANDLE cbs;
    int result;
    cbs = cbs_create(test_session_handle);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_management_set_trace(test_amqp_management_handle, false));

    // act
    result = cbs_set_trace(cbs, false);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_090: [ If the argument `cbs` is NULL, `cbs_set_trace` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(cbs_set_trace_with_NULL_handle_fails)
{
    // arrange
    CBS_HANDLE cbs;
    int result;
    cbs = cbs_create(test_session_handle);
    umock_c_reset_all_calls();

    // act
    result = cbs_set_trace(NULL, false);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    cbs_destroy(cbs);
}

/* on_amqp_management_open_complete */

/* Tests_SRS_CBS_01_105: [ When `on_amqp_management_open_complete` is called with NULL `context`, it shall do nothing. ]*/
TEST_FUNCTION(on_amqp_management_open_complete_with_NULL_context_does_nothing)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    umock_c_reset_all_calls();

    // act
    saved_on_amqp_management_open_complete(NULL, AMQP_MANAGEMENT_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_106: [ If CBS is OPENING and `open_result` is `AMQP_MANAGEMENT_OPEN_OK` the callback `on_cbs_open_complete` shall be called with `CBS_OPEN_OK` and the `on_cbs_open_complete_context` shall be passed as argument. ]*/
TEST_FUNCTION(on_amqp_management_open_complete_with_OK_when_CBS_is_OPENING_indicates_the_open_complete)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_cbs_open_complete((void*)0x4242, CBS_OPEN_OK));

    // act
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_107: [ If CBS is OPENING and `open_result` is `AMQP_MANAGEMENT_OPEN_ERROR` the callback `on_cbs_open_complete` shall be called with `CBS_OPEN_ERROR` and the `on_cbs_open_complete_context` shall be passed as argument. ]*/
/* Tests_SRS_CBS_01_113: [ When `on_amqp_management_open_complete` reports a failure, the underlying AMQP management shall be closed by calling `amqp_management_close`. ]*/
TEST_FUNCTION(on_amqp_management_open_complete_with_ERROR_when_CBS_is_OPENING_indicates_the_open_complete)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_management_close(test_amqp_management_handle));
    STRICT_EXPECTED_CALL(test_on_cbs_open_complete((void*)0x4242, CBS_OPEN_ERROR));

    // act
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_ERROR);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_108: [ If CBS is OPENING and `open_result` is `AMQP_MANAGEMENT_OPEN_CANCELLED` the callback `on_cbs_open_complete` shall be called with `CBS_OPEN_CANCELLED` and the `on_cbs_open_complete_context` shall be passed as argument. ]*/
/* Tests_SRS_CBS_01_113: [ When `on_amqp_management_open_complete` reports a failure, the underlying AMQP management shall be closed by calling `amqp_management_close`. ]*/
TEST_FUNCTION(on_amqp_management_open_complete_with_CANCELLED_when_CBS_is_OPENING_indicates_the_open_complete)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_management_close(test_amqp_management_handle));
    STRICT_EXPECTED_CALL(test_on_cbs_open_complete((void*)0x4242, CBS_OPEN_CANCELLED));

    // act
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_CANCELLED);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_109: [ When `on_amqp_management_open_complete` is called when the CBS is OPEN, the callback `on_cbs_error` shall be called and the `on_cbs_error_context` shall be passed as argument. ]*/
TEST_FUNCTION(on_amqp_management_open_complete_with_OK_when_already_OPEN_triggers_an_error)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_cbs_error((void*)0x4243));

    // act
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_109: [ When `on_amqp_management_open_complete` is called when the CBS is OPEN, the callback `on_cbs_error` shall be called and the `on_cbs_error_context` shall be passed as argument. ]*/
TEST_FUNCTION(on_amqp_management_open_complete_with_ERROR_when_already_OPEN_triggers_an_error)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_cbs_error((void*)0x4243));

    // act
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_ERROR);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* on_amqp_management_error */

/* Tests_SRS_CBS_01_110: [ When `on_amqp_management_error` is called with NULL `context`, it shall do nothing. ]*/
TEST_FUNCTION(on_amqp_management_error_with_NULL_context_does_nothing)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    // act
    saved_on_amqp_management_error(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_111: [ If CBS is OPENING the callback `on_cbs_open_complete` shall be called with `CBS_OPEN_ERROR` and the `on_cbs_open_complete_context` shall be passed as argument. ]*/
/* Tests_SRS_CBS_01_114: [ Additionally the underlying AMQP management shall be closed by calling `amqp_management_close`. ]*/
TEST_FUNCTION(on_amqp_management_error_when_OPENING_indicates_open_complete_with_error)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqp_management_close(test_amqp_management_handle));
    STRICT_EXPECTED_CALL(test_on_cbs_open_complete((void*)0x4242, CBS_OPEN_ERROR));

    // act
    saved_on_amqp_management_error(saved_on_amqp_management_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_112: [ If CBS is OPEN the callback `on_cbs_error` shall be called and the `on_cbs_error_context` shall be passed as argument. ]*/
TEST_FUNCTION(on_amqp_management_error_when_OPEN_indicates_the_error_up)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_cbs_error((void*)0x4243));

    // act
    saved_on_amqp_management_error(saved_on_amqp_management_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* on_amqp_management_operation_complete */

/* Tests_SRS_CBS_01_091: [ When `on_amqp_management_operation_complete` is called with a NULL context it shall do nothing. ]*/
TEST_FUNCTION(on_amqp_management_operation_complete_with_NULL_context_does_nothing)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    (void)cbs_put_token_async(cbs, "some_type", "my_audience", "my_token", test_on_cbs_put_token_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    saved_on_execute_operation_complete(NULL, AMQP_MANAGEMENT_EXECUTE_OPERATION_OK, 200, "blah", test_response_message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_104: [ If `singlylinkedlist_item_get_value` returns NULL, `on_amqp_management_operation_complete` shall do nothing. ]*/
TEST_FUNCTION(when_singlylinkedlist_item_get_value_fails_then_on_amqp_management_operation_complete_does_nothing_more)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    (void)cbs_put_token_async(cbs, "some_type", "my_audience", "my_token", test_on_cbs_put_token_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG))
        .SetReturn(NULL);

    // act
    saved_on_execute_operation_complete(saved_on_execute_operation_complete_context, AMQP_MANAGEMENT_EXECUTE_OPERATION_OK, 200, "blah", test_response_message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_092: [ When `on_amqp_management_operation_complete` is called with `AMQP_MANAGEMENT_EXECUTE_OPERATION_OK`, the associated cbs operation complete callback shall be called with `CBS_OPERATION_RESULT_OK` and passing the `on_cbs_put_token_complete_context` as the context argument. ]*/
/* Tests_SRS_CBS_01_095: [ `status_code` and `status_description` shall be passed as they are to the cbs operation complete callback. ]*/
/* Tests_SRS_CBS_01_103: [ The `context` shall be used to obtain the pending operation information stored in the pending operations linked list by calling `singlylinkedlist_item_get_value`. ]*/
/* Tests_SRS_CBS_01_102: [ The pending operation shall be removed from the pending operations list by calling `singlylinkedlist_remove`. ]*/
/* Tests_SRS_CBS_01_096: [ The `context` for the operation shall also be freed. ]*/
/* Tests_SRS_CBS_01_014: [ The response message has the following application-properties: ]*/
/* Tests_SRS_CBS_01_013: [ status-code    No    int    HTTP response code [RFC2616]. ]*/
/* Tests_SRS_CBS_01_015: [ status-description    Yes    string    Description of the status. ]*/
/* Tests_SRS_CBS_01_016: [ The body of the message MUST be empty. ]*/
TEST_FUNCTION(on_amqp_management_operation_complete_with_OK_triggers_the_cbs_operation_complete_with_OK)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    (void)cbs_put_token_async(cbs, "some_type", "my_audience", "my_token", test_on_cbs_put_token_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_cbs_put_token_complete((void*)0x4244, CBS_OPERATION_RESULT_OK, 200, "blah"));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));

    // act
    saved_on_execute_operation_complete(saved_on_execute_operation_complete_context, AMQP_MANAGEMENT_EXECUTE_OPERATION_OK, 200, "blah", test_response_message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_093: [ When `on_amqp_management_operation_complete` is called with `AMQP_MANAGEMENT_EXECUTE_OPERATION_ERROR`, the associated cbs operation complete callback shall be called with `CBS_OPERATION_RESULT_CBS_ERROR` and passing the `on_cbs_put_token_complete_context` as the context argument. ]*/
/* Tests_SRS_CBS_01_095: [ `status_code` and `status_description` shall be passed as they are to the cbs operation complete callback. ]*/
/* Tests_SRS_CBS_01_103: [ The `context` shall be used to obtain the pending operation information stored in the pending operations linked list by calling `singlylinkedlist_item_get_value`. ]*/
/* Tests_SRS_CBS_01_102: [ The pending operation shall be removed from the pending operations list by calling `singlylinkedlist_remove`. ]*/
/* Tests_SRS_CBS_01_096: [ The `context` for the operation shall also be freed. ]*/
TEST_FUNCTION(on_amqp_management_operation_complete_with_ERROR_triggers_the_cbs_operation_complete_with_ERROR)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    (void)cbs_put_token_async(cbs, "some_type", "my_audience", "my_token", test_on_cbs_put_token_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_cbs_put_token_complete((void*)0x4244, CBS_OPERATION_RESULT_CBS_ERROR, 401, "blah"));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));

    // act
    saved_on_execute_operation_complete(saved_on_execute_operation_complete_context, AMQP_MANAGEMENT_EXECUTE_OPERATION_ERROR, 401, "blah", test_response_message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_094: [ When `on_amqp_management_operation_complete` is called with `AMQP_MANAGEMENT_EXECUTE_OPERATION_FAILED_BAD_STATUS`, the associated cbs operation complete callback shall be called with `CBS_OPERATION_RESULT_OPERATION_FAILED` and passing the `on_cbs_put_token_complete_context` as the context argument. ]*/
/* Tests_SRS_CBS_01_095: [ `status_code` and `status_description` shall be passed as they are to the cbs operation complete callback. ]*/
/* Tests_SRS_CBS_01_103: [ The `context` shall be used to obtain the pending operation information stored in the pending operations linked list by calling `singlylinkedlist_item_get_value`. ]*/
/* Tests_SRS_CBS_01_102: [ The pending operation shall be removed from the pending operations list by calling `singlylinkedlist_remove`. ]*/
/* Tests_SRS_CBS_01_096: [ The `context` for the operation shall also be freed. ]*/
TEST_FUNCTION(on_amqp_management_operation_complete_with_OPERATION_FAILED_BAD_STATUS_triggers_the_cbs_operation_complete_with_OPERATION_FAILED)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    (void)cbs_put_token_async(cbs, "some_type", "my_audience", "my_token", test_on_cbs_put_token_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_cbs_put_token_complete((void*)0x4244, CBS_OPERATION_RESULT_OPERATION_FAILED, 0, "blah"));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));

    // act
    saved_on_execute_operation_complete(saved_on_execute_operation_complete_context, AMQP_MANAGEMENT_EXECUTE_OPERATION_FAILED_BAD_STATUS, 0, "blah", test_response_message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_115: [ When `on_amqp_management_execute_operation_complete` is called with `AMQP_MANAGEMENT_EXECUTE_OPERATION_INSTANCE_CLOSED`, the associated cbs operation complete callback shall be called with `CBS_OPERATION_RESULT_INSTANCE_CLOSED` and passing the `on_cbs_put_token_complete_context` as the context argument. ]*/
/* Tests_SRS_CBS_01_095: [ `status_code` and `status_description` shall be passed as they are to the cbs operation complete callback. ]*/
/* Tests_SRS_CBS_01_103: [ The `context` shall be used to obtain the pending operation information stored in the pending operations linked list by calling `singlylinkedlist_item_get_value`. ]*/
/* Tests_SRS_CBS_01_102: [ The pending operation shall be removed from the pending operations list by calling `singlylinkedlist_remove`. ]*/
/* Tests_SRS_CBS_01_096: [ The `context` for the operation shall also be freed. ]*/
TEST_FUNCTION(on_amqp_management_operation_complete_with_INSTANCE_CLOSED_triggers_the_cbs_operation_complete_with_INSTANCE_CLOSED)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    (void)cbs_put_token_async(cbs, "some_type", "my_audience", "my_token", test_on_cbs_put_token_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_cbs_put_token_complete((void*)0x4244, CBS_OPERATION_RESULT_INSTANCE_CLOSED, 0, "blah"));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));

    // act
    saved_on_execute_operation_complete(saved_on_execute_operation_complete_context, AMQP_MANAGEMENT_EXECUTE_OPERATION_INSTANCE_CLOSED, 0, "blah", test_response_message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_092: [ When `on_amqp_management_operation_complete` is called with `AMQP_MANAGEMENT_EXECUTE_OPERATION_OK`, the associated cbs operation complete callback shall be called with `CBS_OPERATION_RESULT_OK` and passing the `on_cbs_put_token_complete_context` as the context argument. ]*/
/* Tests_SRS_CBS_01_095: [ `status_code` and `status_description` shall be passed as they are to the cbs operation complete callback. ]*/
/* Tests_SRS_CBS_01_103: [ The `context` shall be used to obtain the pending operation information stored in the pending operations linked list by calling `singlylinkedlist_item_get_value`. ]*/
/* Tests_SRS_CBS_01_102: [ The pending operation shall be removed from the pending operations list by calling `singlylinkedlist_remove`. ]*/
/* Tests_SRS_CBS_01_096: [ The `context` for the operation shall also be freed. ]*/
/* Tests_SRS_CBS_01_026: [ The response message has the following application-properties: ]*/
/* Tests_SRS_CBS_01_027: [ status-code    Yes    int    HTTP response code [RFC2616]. ]*/
/* Tests_SRS_CBS_01_028: [ status-description    No    string    Description of the status. ]*/
/* Tests_SRS_CBS_01_029: [ The body of the message MUST be empty. ]*/
TEST_FUNCTION(on_amqp_management_operation_complete_with_OK_for_delete_token_triggers_the_cbs_operation_complete_with_OK)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    (void)cbs_delete_token_async(cbs, "some_type", "my_audience", test_on_cbs_delete_token_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_cbs_delete_token_complete((void*)0x4244, CBS_OPERATION_RESULT_OK, 200, "blah"));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));

    // act
    saved_on_execute_operation_complete(saved_on_execute_operation_complete_context, AMQP_MANAGEMENT_EXECUTE_OPERATION_OK, 200, "blah", test_response_message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_093: [ When `on_amqp_management_operation_complete` is called with `AMQP_MANAGEMENT_EXECUTE_OPERATION_ERROR`, the associated cbs operation complete callback shall be called with `CBS_OPERATION_RESULT_CBS_ERROR` and passing the `on_cbs_put_token_complete_context` as the context argument. ]*/
/* Tests_SRS_CBS_01_095: [ `status_code` and `status_description` shall be passed as they are to the cbs operation complete callback. ]*/
/* Tests_SRS_CBS_01_103: [ The `context` shall be used to obtain the pending operation information stored in the pending operations linked list by calling `singlylinkedlist_item_get_value`. ]*/
/* Tests_SRS_CBS_01_102: [ The pending operation shall be removed from the pending operations list by calling `singlylinkedlist_remove`. ]*/
/* Tests_SRS_CBS_01_096: [ The `context` for the operation shall also be freed. ]*/
TEST_FUNCTION(on_amqp_management_operation_complete_with_ERROR_for_delete_token_triggers_the_cbs_operation_complete_with_ERROR)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    (void)cbs_delete_token_async(cbs, "some_type", "my_audience", test_on_cbs_delete_token_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_cbs_delete_token_complete((void*)0x4244, CBS_OPERATION_RESULT_CBS_ERROR, 401, "blah"));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));

    // act
    saved_on_execute_operation_complete(saved_on_execute_operation_complete_context, AMQP_MANAGEMENT_EXECUTE_OPERATION_ERROR, 401, "blah", test_response_message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_094: [ When `on_amqp_management_operation_complete` is called with `AMQP_MANAGEMENT_EXECUTE_OPERATION_FAILED_BAD_STATUS`, the associated cbs operation complete callback shall be called with `CBS_OPERATION_RESULT_OPERATION_FAILED` and passing the `on_cbs_put_token_complete_context` as the context argument. ]*/
/* Tests_SRS_CBS_01_095: [ `status_code` and `status_description` shall be passed as they are to the cbs operation complete callback. ]*/
/* Tests_SRS_CBS_01_103: [ The `context` shall be used to obtain the pending operation information stored in the pending operations linked list by calling `singlylinkedlist_item_get_value`. ]*/
/* Tests_SRS_CBS_01_102: [ The pending operation shall be removed from the pending operations list by calling `singlylinkedlist_remove`. ]*/
/* Tests_SRS_CBS_01_096: [ The `context` for the operation shall also be freed. ]*/
TEST_FUNCTION(on_amqp_management_operation_complete_with_OPERATION_FAILED_BAD_STATUSfor_delete_token_triggers_the_cbs_operation_complete_with_OPERATION_FAILED)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    (void)cbs_delete_token_async(cbs, "some_type", "my_audience", test_on_cbs_delete_token_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_cbs_delete_token_complete((void*)0x4244, CBS_OPERATION_RESULT_OPERATION_FAILED, 0, "blah"));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));

    // act
    saved_on_execute_operation_complete(saved_on_execute_operation_complete_context, AMQP_MANAGEMENT_EXECUTE_OPERATION_FAILED_BAD_STATUS, 0, "blah", test_response_message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

/* Tests_SRS_CBS_01_115: [ When `on_amqp_management_execute_operation_complete` is called with `AMQP_MANAGEMENT_EXECUTE_OPERATION_INSTANCE_CLOSED`, the associated cbs operation complete callback shall be called with `CBS_OPERATION_RESULT_INSTANCE_CLOSED` and passing the `on_cbs_put_token_complete_context` as the context argument. ]*/
/* Tests_SRS_CBS_01_095: [ `status_code` and `status_description` shall be passed as they are to the cbs operation complete callback. ]*/
/* Tests_SRS_CBS_01_103: [ The `context` shall be used to obtain the pending operation information stored in the pending operations linked list by calling `singlylinkedlist_item_get_value`. ]*/
/* Tests_SRS_CBS_01_102: [ The pending operation shall be removed from the pending operations list by calling `singlylinkedlist_remove`. ]*/
/* Tests_SRS_CBS_01_096: [ The `context` for the operation shall also be freed. ]*/
TEST_FUNCTION(on_amqp_management_operation_complete_with_INSTANCE_CLOSED_for_delete_token_triggers_the_cbs_operation_complete_with_INSTANCE_CLOSED)
{
    // arrange
    CBS_HANDLE cbs;
    cbs = cbs_create(test_session_handle);
    (void)cbs_open_async(cbs, test_on_cbs_open_complete, (void*)0x4242, test_on_cbs_error, (void*)0x4243);
    saved_on_amqp_management_open_complete(saved_on_amqp_management_open_complete_context, AMQP_MANAGEMENT_OPEN_OK);
    (void)cbs_delete_token_async(cbs, "some_type", "my_audience", test_on_cbs_delete_token_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_cbs_delete_token_complete((void*)0x4244, CBS_OPERATION_RESULT_INSTANCE_CLOSED, 0, "blah"));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinkedlist, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(async_operation_destroy(IGNORED_PTR_ARG));

    // act
    saved_on_execute_operation_complete(saved_on_execute_operation_complete_context, AMQP_MANAGEMENT_EXECUTE_OPERATION_INSTANCE_CLOSED, 0, "blah", test_response_message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    cbs_destroy(cbs);
}

END_TEST_SUITE(cbs_ut)
