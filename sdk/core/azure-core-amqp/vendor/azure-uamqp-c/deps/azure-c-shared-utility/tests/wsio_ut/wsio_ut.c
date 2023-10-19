// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#else
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_bool.h"

#define ENABLE_MOCKS

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/optionhandler.h"
#include "azure_c_shared_utility/uws_client.h"

static const void** list_items = NULL;
static size_t list_item_count = 0;
static const char* TEST_HOST_ADDRESS = "host_address.com";
static const char* TEST_RESOURCE_NAME = "/test_resource";
static const char* TEST_PROTOCOL = "test_proto";

static const SINGLYLINKEDLIST_HANDLE TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE = (SINGLYLINKEDLIST_HANDLE)0x4242;
static const LIST_ITEM_HANDLE TEST_LIST_ITEM_HANDLE = (LIST_ITEM_HANDLE)0x11;
static const UWS_CLIENT_HANDLE TEST_UWS_HANDLE = (UWS_CLIENT_HANDLE)0x4243;
static const XIO_HANDLE TEST_UNDERLYING_IO_HANDLE = (XIO_HANDLE)0x4244;
static const OPTIONHANDLER_HANDLE TEST_OPTIONHANDLER_HANDLE = (OPTIONHANDLER_HANDLE)0x4246;
static const OPTIONHANDLER_HANDLE TEST_UWS_CLIENT_OPTIONHANDLER_HANDLE = (OPTIONHANDLER_HANDLE)0x4247;
static void* TEST_UNDERLYING_IO_PARAMETERS = (void*)0x4248;
static const IO_INTERFACE_DESCRIPTION* TEST_UNDERLYING_IO_INTERFACE = (const IO_INTERFACE_DESCRIPTION*)0x4249;

IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(WS_OPEN_RESULT, WS_OPEN_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_SEND_RESULT, IO_SEND_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(WS_SEND_FRAME_RESULT, WS_SEND_FRAME_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(OPTIONHANDLER_RESULT, OPTIONHANDLER_RESULT_VALUES);

static size_t currentmalloc_call;
static size_t whenShallmalloc_fail;

static size_t currentcalloc_call;
static size_t whenShallcalloc_fail;

static void* my_gballoc_malloc(size_t size)
{
    void* result;
    currentmalloc_call++;
    if (whenShallmalloc_fail > 0)
    {
        if (currentmalloc_call == whenShallmalloc_fail)
        {
            result = NULL;
        }
        else
        {
            result = malloc(size);
        }
    }
    else
    {
        result = malloc(size);
    }
    return result;
}

static void* my_gballoc_calloc(size_t nmemb, size_t size)
{
    void* result;
    currentcalloc_call++;
    if (whenShallcalloc_fail > 0)
    {
        if (currentcalloc_call == whenShallcalloc_fail)
        {
            result = NULL;
        }
        else
        {
            result = calloc(nmemb, size);
        }
    }
    else
    {
        result = calloc(nmemb, size);
    }
    return result;
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

static LIST_ITEM_HANDLE add_to_list(const void* item)
{
    const void** items = (const void**)realloc((void*)list_items, (list_item_count + 1) * sizeof(item));
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
        free((void*)list_items);
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

int my_mallocAndStrcpy_s(char** destination, const char* source)
{
    *destination = (char*)malloc(strlen(source) + 1);
    (void)strcpy(*destination, source);
    return 0;
}

static pfCloneOption g_clone_option;
static pfDestroyOption g_destroy_option;
static pfSetOption g_set_option;

static OPTIONHANDLER_HANDLE my_OptionHandler_Create(pfCloneOption cloneOption, pfDestroyOption destroyOption, pfSetOption setOption)
{
    g_clone_option = cloneOption;
    g_destroy_option = destroyOption;
    g_set_option = setOption;

    return TEST_OPTIONHANDLER_HANDLE;
}

#include "azure_c_shared_utility/gballoc.h"

#undef ENABLE_MOCKS

#include "azure_c_shared_utility/wsio.h"

// consumer mocks
MOCK_FUNCTION_WITH_CODE(, void, test_on_io_open_complete, void*, context, IO_OPEN_RESULT, io_open_result);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_bytes_received, void*, context, const unsigned char*, buffer, size_t, size);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_io_error, void*, context);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_io_close_complete, void*, context);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_send_complete, void*, context, IO_SEND_RESULT, send_result)
MOCK_FUNCTION_END()

static ON_WS_OPEN_COMPLETE g_on_ws_open_complete;
static void* g_on_ws_open_complete_context;
static ON_WS_SEND_FRAME_COMPLETE g_on_ws_send_frame_complete;
static void* g_on_ws_send_frame_complete_context;
static ON_WS_FRAME_RECEIVED g_on_ws_frame_received;
static void* g_on_ws_frame_received_context;
static ON_WS_PEER_CLOSED g_on_ws_peer_closed;
static void* g_on_ws_peer_closed_context;
static ON_WS_ERROR g_on_ws_error;
static void* g_on_ws_error_context;
static ON_WS_CLOSE_COMPLETE g_on_ws_close_complete;
static void* g_on_ws_close_complete_context;

static int my_uws_open_async(UWS_CLIENT_HANDLE uws, ON_WS_OPEN_COMPLETE on_ws_open_complete, void* on_ws_open_complete_context, ON_WS_FRAME_RECEIVED on_ws_frame_received, void* on_ws_frame_received_context, ON_WS_PEER_CLOSED on_ws_peer_closed, void* on_ws_peer_closed_context, ON_WS_ERROR on_ws_error, void* on_ws_error_context)
{
    (void)uws;
    g_on_ws_open_complete = on_ws_open_complete;
    g_on_ws_open_complete_context = on_ws_open_complete_context;
    g_on_ws_frame_received = on_ws_frame_received;
    g_on_ws_frame_received_context = on_ws_frame_received_context;
    g_on_ws_peer_closed = on_ws_peer_closed;
    g_on_ws_peer_closed_context = on_ws_peer_closed_context;
    g_on_ws_error = on_ws_error;
    g_on_ws_error_context = on_ws_error_context;
    return 0;
}

static int my_uws_close_async(UWS_CLIENT_HANDLE uws, ON_WS_CLOSE_COMPLETE on_ws_close_complete, void* on_ws_close_complete_context)
{
    (void)uws;
    g_on_ws_close_complete = on_ws_close_complete;
    g_on_ws_close_complete_context = on_ws_close_complete_context;
    return 0;
}

static int my_uws_send_frame_async(UWS_CLIENT_HANDLE uws, unsigned char frame_type, const unsigned char* buffer, size_t size, bool is_final, ON_WS_SEND_FRAME_COMPLETE on_ws_send_frame_complete, void* on_ws_send_frame_complete_context)
{
    (void)uws;
    (void)buffer;
    (void)size;
    (void)is_final;
    (void)frame_type;
    g_on_ws_send_frame_complete = on_ws_send_frame_complete;
    g_on_ws_send_frame_complete_context = on_ws_send_frame_complete_context;
    return 0;
}

static WSIO_CONFIG default_wsio_config;

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(wsio_ut)

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

    default_wsio_config.hostname = TEST_HOST_ADDRESS;
    default_wsio_config.port = 443;
    default_wsio_config.resource_name = TEST_RESOURCE_NAME;
    default_wsio_config.protocol = TEST_PROTOCOL;
    default_wsio_config.underlying_io_interface = TEST_UNDERLYING_IO_INTERFACE;
    default_wsio_config.underlying_io_parameters = TEST_UNDERLYING_IO_PARAMETERS;

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, my_gballoc_calloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_RETURN(singlylinkedlist_create, TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_remove, my_singlylinkedlist_remove);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_get_head_item, my_singlylinkedlist_get_head_item);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_add, my_singlylinkedlist_add);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_item_get_value, my_singlylinkedlist_item_get_value);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_find, my_singlylinkedlist_find);
    REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_s, my_mallocAndStrcpy_s);
    REGISTER_GLOBAL_MOCK_HOOK(uws_client_open_async, my_uws_open_async);
    REGISTER_GLOBAL_MOCK_HOOK(uws_client_close_async, my_uws_close_async);
    REGISTER_GLOBAL_MOCK_HOOK(uws_client_send_frame_async, my_uws_send_frame_async);
    REGISTER_GLOBAL_MOCK_HOOK(OptionHandler_Create, my_OptionHandler_Create);
    REGISTER_GLOBAL_MOCK_RETURN(OptionHandler_FeedOptions, OPTIONHANDLER_OK);
    REGISTER_GLOBAL_MOCK_RETURN(OptionHandler_AddOption, OPTIONHANDLER_OK);
    REGISTER_GLOBAL_MOCK_RETURN(OptionHandler_Clone, TEST_OPTIONHANDLER_HANDLE);
    REGISTER_GLOBAL_MOCK_RETURN(uws_client_create_with_io, TEST_UWS_HANDLE);
    REGISTER_GLOBAL_MOCK_RETURN(uws_client_retrieve_options, TEST_UWS_CLIENT_OPTIONHANDLER_HANDLE);

    REGISTER_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT);
    REGISTER_TYPE(IO_SEND_RESULT, IO_SEND_RESULT);
    REGISTER_TYPE(OPTIONHANDLER_RESULT, OPTIONHANDLER_RESULT);

    REGISTER_UMOCK_ALIAS_TYPE(SINGLYLINKEDLIST_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_ITEM_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(XIO_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(UWS_CLIENT_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_WS_OPEN_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_WS_FRAME_RECEIVED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_WS_ERROR, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_WS_CLOSE_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_WS_SEND_FRAME_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(OPTIONHANDLER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_WS_PEER_CLOSED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfCloneOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfSetOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfDestroyOption, void*);
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

    currentmalloc_call = 0;
    whenShallmalloc_fail = 0;
    singlylinkedlist_remove_result = 0;
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* wsio_create */

/* Tests_SRS_WSIO_01_001: [wsio_create shall create an instance of wsio and return a non-NULL handle to it.]*/
/* Tests_SRS_WSIO_01_066: [ io_create_parameters shall be used as a WSIO_CONFIG* . ]*/
/* Tests_SRS_WSIO_01_070: [ The underlying uws instance shall be created by calling uws_client_create_with_io. ]*/
/* Tests_SRS_WSIO_01_071: [ The arguments for uws_client_create_with_io shall be: ]*/
/* Tests_SRS_WSIO_01_185: [ - underlying_io_interface shall be set to the underlying_io_interface field in the io_create_parameters passed to wsio_create. ]*/
/* Tests_SRS_WSIO_01_186: [ - underlying_io_parameters shall be set to the underlying_io_parameters field in the io_create_parameters passed to wsio_create. ]*/
/* Tests_SRS_WSIO_01_072: [ - hostname set to the hostname field in the io_create_parameters passed to wsio_create. ]*/
/* Tests_SRS_WSIO_01_130: [ - port set to the port field in the io_create_parameters passed to wsio_create. ]*/
/* Tests_SRS_WSIO_01_128: [ - resource_name set to the resource_name field in the io_create_parameters passed to wsio_create. ]*/
/* Tests_SRS_WSIO_01_129: [ - protocols shall be filled with only one structure, that shall have the protocol set to the value of the protocol field in the io_create_parameters passed to wsio_create. ]*/
/* Tests_SRS_WSIO_01_076: [ wsio_create shall create a pending send IO list that is to be used to queue send packets by calling singlylinkedlist_create. ]*/
TEST_FUNCTION(wsio_create_for_secure_connection_with_valid_args_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_client_create_with_io(TEST_UNDERLYING_IO_INTERFACE, TEST_UNDERLYING_IO_PARAMETERS, TEST_HOST_ADDRESS, 443, TEST_RESOURCE_NAME, IGNORED_PTR_ARG, 1));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());

    // act
    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);

    // assert
    ASSERT_IS_NOT_NULL(wsio);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_065: [ If the argument io_create_parameters is NULL then wsio_create shall return NULL. ]*/
TEST_FUNCTION(wsio_create_with_NULL_create_arguments_fails)
{
    // arrange

    // act
    CONCRETE_IO_HANDLE wsio = wsio_get_interface_description()->concrete_io_create(NULL);

    // assert
    ASSERT_IS_NULL(wsio);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_067: [ If any of the members hostname, resource_name or protocol is NULL in WSIO_CONFIG then wsio_create shall return NULL. ]*/
TEST_FUNCTION(wsio_create_with_NULL_hostname_field_fails)
{
    // arrange
    WSIO_CONFIG wsio_config;
    CONCRETE_IO_HANDLE wsio;

    wsio_config.hostname = NULL;
    wsio_config.port = 443;
    wsio_config.resource_name = TEST_RESOURCE_NAME;
    wsio_config.protocol = TEST_PROTOCOL;
    wsio_config.underlying_io_interface = NULL;
    wsio_config.underlying_io_parameters = NULL;

    // act
    wsio = wsio_get_interface_description()->concrete_io_create(&wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_067: [ If any of the members hostname, resource_name or protocol is NULL in WSIO_CONFIG then wsio_create shall return NULL. ]*/
TEST_FUNCTION(wsio_create_with_NULL_resource_name_field_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    WSIO_CONFIG wsio_config;
    wsio_config.hostname = TEST_HOST_ADDRESS;
    wsio_config.port = 443;
    wsio_config.resource_name = NULL;
    wsio_config.protocol = TEST_PROTOCOL;
    wsio_config.underlying_io_interface = NULL;
    wsio_config.underlying_io_parameters = NULL;

    // act
    wsio = wsio_get_interface_description()->concrete_io_create(&wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_067: [ If any of the members hostname, resource_name or protocol is NULL in WSIO_CONFIG then wsio_create shall return NULL. ]*/
TEST_FUNCTION(wsio_create_with_NULL_protocol_field_fails)
{
    // arrange
    WSIO_CONFIG wsio_config;
    CONCRETE_IO_HANDLE wsio;

    wsio_config.hostname = TEST_HOST_ADDRESS;
    wsio_config.port = 443;
    wsio_config.resource_name = TEST_RESOURCE_NAME;
    wsio_config.protocol = NULL;
    wsio_config.underlying_io_interface = NULL;
    wsio_config.underlying_io_parameters = NULL;

    // act
    wsio = wsio_get_interface_description()->concrete_io_create(&wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_068: [ If allocating memory for the new wsio instance fails then wsio_create shall return NULL. ]*/
TEST_FUNCTION(when_allocating_memory_fails_wsio_create_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_075: [ If uws_client_create_with_io fails, then wsio_create shall fail and return NULL. ]*/
TEST_FUNCTION(when_uws_create_fails_then_wsio_create_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_client_create_with_io(TEST_UNDERLYING_IO_INTERFACE, TEST_UNDERLYING_IO_PARAMETERS, TEST_HOST_ADDRESS, 443, TEST_RESOURCE_NAME, IGNORED_PTR_ARG, 1))
        .SetReturn(NULL);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_077: [ If singlylinkedlist_create fails then wsio_create shall fail and return NULL. ]*/
TEST_FUNCTION(when_singlylinkedlist_create_fails_then_wsio_create_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_client_create_with_io(TEST_UNDERLYING_IO_INTERFACE, TEST_UNDERLYING_IO_PARAMETERS, TEST_HOST_ADDRESS, 443, TEST_RESOURCE_NAME, IGNORED_PTR_ARG, 1));
    STRICT_EXPECTED_CALL(singlylinkedlist_create())
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(uws_client_destroy(TEST_UWS_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);

    // assert
    ASSERT_IS_NULL(wsio);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_071: [ The arguments for uws_client_create_with_io shall be: ]*/
/* Tests_SRS_WSIO_01_185: [ - underlying_io_interface shall be set to the underlying_io_interface field in the io_create_parameters passed to wsio_create. ]*/
/* Tests_SRS_WSIO_01_186: [ - underlying_io_parameters shall be set to the underlying_io_parameters field in the io_create_parameters passed to wsio_create. ]*/
/* Tests_SRS_WSIO_01_072: [ - hostname set to the hostname field in the io_create_parameters passed to wsio_create. ]*/
/* Tests_SRS_WSIO_01_130: [ - port set to the port field in the io_create_parameters passed to wsio_create. ]*/
/* Tests_SRS_WSIO_01_128: [ - resource_name set to the resource_name field in the io_create_parameters passed to wsio_create. ]*/
/* Tests_SRS_WSIO_01_129: [ - protocols shall be filled with only one structure, that shall have the protocol set to the value of the protocol field in the io_create_parameters passed to wsio_create. ]*/
TEST_FUNCTION(wsio_create_for_secure_connection_with_valid_args_succeeds_2)
{
    // arrange
    WSIO_CONFIG wsio_config;
    CONCRETE_IO_HANDLE wsio;

    wsio_config.hostname = "another.com";
    wsio_config.port = 80;
    wsio_config.resource_name = "haga";
    wsio_config.protocol = "LeProtocol";
    wsio_config.underlying_io_interface = TEST_UNDERLYING_IO_INTERFACE;
    wsio_config.underlying_io_parameters = NULL;

    EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_client_create_with_io(TEST_UNDERLYING_IO_INTERFACE, NULL, "another.com", 80, "haga", IGNORED_PTR_ARG, 1));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());

    // act
    wsio = wsio_get_interface_description()->concrete_io_create(&wsio_config);

    // assert
    ASSERT_IS_NOT_NULL(wsio);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* wsio_destroy */

/* Tests_SRS_WSIO_01_078: [ wsio_destroy shall free all resources associated with the wsio instance. ]*/
/* Tests_SRS_WSIO_01_080: [ wsio_destroy shall destroy the uws instance created in wsio_create by calling uws_client_destroy. ]*/
/* Tests_SRS_WSIO_01_081: [ wsio_destroy shall free the list used to track the pending send IOs by calling singlylinkedlist_destroy. ]*/
TEST_FUNCTION(wsio_destroy_frees_all_resources)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(uws_client_destroy(TEST_UWS_HANDLE));
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    wsio_get_interface_description()->concrete_io_destroy(wsio);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_079: [ If ws_io is NULL, wsio_destroy shall do nothing.  ]*/
TEST_FUNCTION(wsio_destroy_with_NULL_does_nothing)
{
    // arrange

    // act
    wsio_get_interface_description()->concrete_io_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* wsio_open */

/* Tests_SRS_WSIO_01_082: [ wsio_open shall open the underlying uws instance by calling uws_client_open_async and providing the uws handle created in wsio_create as argument. ]*/
/* Tests_SRS_WSIO_01_083: [ On success, wsio_open shall return 0. ]*/
TEST_FUNCTION(wsio_open_opens_the_underlying_uws_instance)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(uws_client_open_async(TEST_UWS_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_084: [ If opening the underlying uws instance fails then wsio_open shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_opening_the_uws_instance_fails_wsio_open_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(uws_client_open_async(TEST_UWS_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1);

    // act
    result = wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_131: [ wsio_open when already OPEN or OPENING shall fail and return a non-zero value. ]*/
TEST_FUNCTION(wsio_open_when_already_opening_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    result = wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_131: [ wsio_open when already OPEN or OPENING shall fail and return a non-zero value. ]*/
TEST_FUNCTION(wsio_open_when_already_open_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    // act
    result = wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_132: [ If any of the arguments ws_io, on_io_open_complete, on_bytes_received, on_io_error is NULL, wsio_open shall fail and return a non-zero value. ]*/
TEST_FUNCTION(wsio_open_with_NULL_handle_fails)
{
    // arrange
    int result;

    // act
    result = wsio_get_interface_description()->concrete_io_open(NULL, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_132: [ If any of the arguments ws_io, on_io_open_complete, on_bytes_received, on_io_error is NULL, wsio_open shall fail and return a non-zero value. ]*/
TEST_FUNCTION(wsio_open_with_NULL_on_open_complete_callback_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    umock_c_reset_all_calls();

    // act
    result = wsio_get_interface_description()->concrete_io_open(wsio, NULL, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_132: [ If any of the arguments ws_io, on_io_open_complete, on_bytes_received, on_io_error is NULL, wsio_open shall fail and return a non-zero value. ]*/
TEST_FUNCTION(wsio_open_with_NULL_on_bytes_received_callback_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    umock_c_reset_all_calls();

    // act
    result = wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, NULL, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_132: [ If any of the arguments ws_io, on_io_open_complete, on_bytes_received, on_io_error is NULL, wsio_open shall fail and return a non-zero value. ]*/
TEST_FUNCTION(wsio_open_with_NULL_on_io_error_callback_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    umock_c_reset_all_calls();

    // act
    result = wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, NULL, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_082: [ wsio_open shall open the underlying uws instance by calling uws_client_open_async and providing the uws handle created in wsio_create as argument. ]*/
/* Tests_SRS_WSIO_01_083: [ On success, wsio_open shall return 0. ]*/
TEST_FUNCTION(wsio_open_after_close_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);
    g_on_ws_close_complete(g_on_ws_close_complete_context);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(uws_client_open_async(TEST_UWS_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* wsio_close */

/* Tests_SRS_WSIO_01_085: [ wsio_close shall close the websockets IO if an open action is either pending or has completed successfully (if the IO is open).  ]*/
/* Tests_SRS_WSIO_01_133: [ On success wsio_close shall return 0. ]*/
/* Tests_SRS_WSIO_01_091: [ wsio_close shall obtain all the pending IO items by repetitively querying for the head of the pending IO list and freeing that head item. ]*/
/* Tests_SRS_WSIO_01_087: [ wsio_close shall call uws_client_close_async while passing as argument the IO handle created in wsio_create.  ]*/
/* Tests_SRS_WSIO_01_092: [ Obtaining the head of the pending IO list shall be done by calling singlylinkedlist_get_head_item. ]*/
/* Tests_SRS_WSIO_01_094: [ The callback context passed to the on_send_complete callback shall be the context given to wsio_send.  ]*/
TEST_FUNCTION(wsio_close_when_IO_is_open_closes_the_uws)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(uws_client_close_async(TEST_UWS_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));

    // act
    result = wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_086: [ if ws_io is NULL, wsio_close shall return a non-zero value.  ]*/
TEST_FUNCTION(wsio_close_with_NULL_handle_fails)
{
    // arrange
    int result;

    // act
    result = wsio_get_interface_description()->concrete_io_close(NULL, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_088: [ wsio_close when no open action has been issued shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_IO_is_not_open_wsio_close_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    umock_c_reset_all_calls();

    // act
    result = wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_089: [ wsio_close after a wsio_close shall fail and return a non-zero value.  ]*/
TEST_FUNCTION(wsio_close_after_wsio_close_completed_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);
    g_on_ws_close_complete(g_on_ws_close_complete_context);
    umock_c_reset_all_calls();

    // act
    result = wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_089: [ wsio_close after a wsio_close shall fail and return a non-zero value.  ]*/
TEST_FUNCTION(wsio_close_after_wsio_close_and_in_closing_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    // act
    result = wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_164: [ When uws_client_close fails, wsio_close shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_uws_close_fails_then_wsio_close_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(uws_client_close_async(TEST_UWS_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));

    // act
    result = wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_090: [ The argument on_io_close_complete shall be optional, if NULL is passed by the caller then no close complete callback shall be triggered.  ]*/
TEST_FUNCTION(wsio_close_with_NULL_close_complete_callback_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(uws_client_close_async(TEST_UWS_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));

    // act
    result = wsio_get_interface_description()->concrete_io_close(wsio, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_093: [ For each pending item the send complete callback shall be called with IO_SEND_CANCELLED.]*/
TEST_FUNCTION(wsio_close_indicates_a_pending_send_as_CANCELLED)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    unsigned char test_buffer[] = { 0x42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4343);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(uws_client_close_async(TEST_UWS_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4343, IO_SEND_CANCELLED));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));

    // act
    result = wsio_get_interface_description()->concrete_io_close(wsio, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_093: [ For each pending item the send complete callback shall be called with IO_SEND_CANCELLED.]*/
TEST_FUNCTION(wsio_close_indicates_2_pending_sends_as_CANCELLED)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    unsigned char test_buffer[] = { 0x42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4343);
    (void)wsio_get_interface_description()->concrete_io_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4343);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(uws_client_close_async(TEST_UWS_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4343, IO_SEND_CANCELLED));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4343, IO_SEND_CANCELLED));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));

    // act
    result = wsio_get_interface_description()->concrete_io_close(wsio, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* wsio_send */

/* Tests_SRS_WSIO_01_095: [ wsio_send shall call uws_client_send_frame_async, passing the buffer and size arguments as they are: ]*/
/* Tests_SRS_WSIO_01_097: [ The is_final argument shall be set to true. ]*/
/* Tests_SRS_WSIO_01_098: [ On success, wsio_send shall return 0. ]*/
/* Tests_SRS_WSIO_01_102: [ An entry shall be queued in the singly linked list by calling singlylinkedlist_add. ]*/
/* Tests_SRS_WSIO_01_103: [ The entry shall contain the on_send_complete callback and its context. ]*/
/* Tests_SRS_WSIO_01_096: [ The frame type used shall be WS_FRAME_TYPE_BINARY. ]*/
TEST_FUNCTION(wsio_send_with_1_byte_calls_uws_send_frame)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    unsigned char test_buffer[] = { 42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uws_client_send_frame_async(TEST_UWS_HANDLE, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, sizeof(test_buffer), true, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, test_buffer, sizeof(test_buffer));

    // act
    result = wsio_get_interface_description()->concrete_io_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4343);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_099: [ If the wsio is not OPEN (open has not been called or is still in progress) then wsio_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(wsio_send_when_not_open_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    unsigned char test_buffer[] = { 42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    umock_c_reset_all_calls();

    // act
    result = wsio_get_interface_description()->concrete_io_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4343);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_099: [ If the wsio is not OPEN (open has not been called or is still in progress) then wsio_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(wsio_send_when_opening_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    unsigned char test_buffer[] = { 42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    result = wsio_get_interface_description()->concrete_io_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4343);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_099: [ If the wsio is not OPEN (open has not been called or is still in progress) then wsio_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(wsio_send_after_io_is_closed_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    unsigned char test_buffer[] = { 42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    // act
    result = wsio_get_interface_description()->concrete_io_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4343);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_100: [ If any of the arguments ws_io or buffer are NULL, wsio_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(wsio_send_with_NULL_wsio_fails)
{
    // arrange
    int result;
    unsigned char test_buffer[] = { 42 };

    // act
    result = wsio_get_interface_description()->concrete_io_send(NULL, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4343);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_100: [ If any of the arguments ws_io or buffer are NULL, wsio_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(wsio_send_with_NULL_buffer_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    // act
    result = wsio_get_interface_description()->concrete_io_send(wsio, NULL, 1, test_on_send_complete, (void*)0x4343);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_101: [ If size is zero then wsio_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(wsio_send_with_zero_size_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    unsigned char test_buffer[] = { 42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    // act
    result = wsio_get_interface_description()->concrete_io_send(wsio, test_buffer, 0, test_on_send_complete, (void*)0x4343);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_134: [ If allocating memory for the pending IO data fails, wsio_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_pending_send_fails_wsio_send_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    unsigned char test_buffer[] = { 42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    result = wsio_get_interface_description()->concrete_io_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4343);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_104: [ If singlylinkedlist_add fails, wsio_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_adding_the_pending_item_to_the_list_fails_wsio_send_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    unsigned char test_buffer[] = { 42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .SetReturn(NULL);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = wsio_get_interface_description()->concrete_io_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4343);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_105: [ The argument on_send_complete shall be optional, if NULL is passed by the caller then no send complete callback shall be triggered. ]*/
TEST_FUNCTION(wsio_send_with_NULL_send_complete_callback_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;
    unsigned char test_buffer[] = { 42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uws_client_send_frame_async(TEST_UWS_HANDLE, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, sizeof(test_buffer), true, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, test_buffer, sizeof(test_buffer));

    // act
    result = wsio_get_interface_description()->concrete_io_send(wsio, test_buffer, sizeof(test_buffer), NULL, (void*)0x4343);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* wsio_dowork */

/* Tests_SRS_WSIO_01_106: [ wsio_dowork shall call uws_client_dowork with the uws handle created in wsio_create. ]*/
TEST_FUNCTION(wsio_dowork_calls_the_underlying_uws_dowork)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(uws_client_dowork(TEST_UWS_HANDLE));

    // act
    wsio_get_interface_description()->concrete_io_dowork(wsio);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_107: [ If the ws_io argument is NULL, wsio_dowork shall do nothing. ]*/
TEST_FUNCTION(wsio_dowork_with_NULL_handle_does_nothing)
{
    // arrange

    // act
    wsio_get_interface_description()->concrete_io_dowork(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_108: [ If the IO is not yet open, wsio_dowork shall do nothing. ]*/
TEST_FUNCTION(wsio_dowork_when_not_open_dows_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    umock_c_reset_all_calls();

    // act
    wsio_get_interface_description()->concrete_io_dowork(wsio);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_108: [ If the IO is not yet open, wsio_dowork shall do nothing. ]*/
TEST_FUNCTION(wsio_dowork_when_opening_calls_uws_dowork)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(uws_client_dowork(TEST_UWS_HANDLE));

    // act
    wsio_get_interface_description()->concrete_io_dowork(wsio);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_108: [ If the IO is not yet open, wsio_dowork shall do nothing. ]*/
TEST_FUNCTION(wsio_dowork_after_close_does_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);
    g_on_ws_close_complete(g_on_ws_close_complete_context);
    umock_c_reset_all_calls();

    // act
    wsio_get_interface_description()->concrete_io_dowork(wsio);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_108: [ If the IO is not yet open, wsio_dowork shall do nothing. ]*/
TEST_FUNCTION(wsio_dowork_in_closing_calls_uws_do_work)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    // act
    wsio_get_interface_description()->concrete_io_dowork(wsio);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* on_ws_error */

/* Tests_SRS_WSIO_01_121: [ When on_underlying_ws_error is called while the IO is OPEN the wsio instance shall be set to ERROR and an error shall be indicated via the on_io_error callback passed to wsio_open. ]*/
/* Tests_SRS_WSIO_01_123: [ When calling on_io_error, the on_io_error_context argument given in wsio_open shall be passed to the callback on_io_error. ]*/
TEST_FUNCTION(when_on_underlying_ws_error_is_called_in_open_the_error_is_reported_up)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    g_on_ws_error(g_on_ws_error_context, WS_ERROR_BAD_FRAME_RECEIVED);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_135: [ When on_underlying_ws_error is called with a NULL context, it shall do nothing. ]*/
TEST_FUNCTION(when_on_underlying_ws_error_with_NULL_context_does_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    // act
    g_on_ws_error(NULL, WS_ERROR_BAD_FRAME_RECEIVED);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_122: [ When on_underlying_ws_error is called while the IO is OPENING, the on_io_open_complete callback passed to wsio_open shall be called with IO_OPEN_ERROR. ]*/
TEST_FUNCTION(when_on_underlying_ws_error_is_called_while_OPENING_calls_open_complete_with_ERROR)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    g_on_ws_error(g_on_ws_error_context, WS_ERROR_BAD_FRAME_RECEIVED);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* on_underlying_ws_open_complete */

/* Tests_SRS_WSIO_01_136: [ When on_underlying_ws_open_complete is called with WS_OPEN_OK while the IO is opening, the callback on_io_open_complete shall be called with IO_OPEN_OK. ]*/
TEST_FUNCTION(when_on_underlying_ws_open_complete_is_called_with_OK_while_OPENING_the_io_open_complete_callback_is_called_with_OPEN_OK)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_OK));

    // act
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_149: [ When on_underlying_ws_open_complete is called with WS_OPEN_CANCELLED while the IO is opening, the callback on_io_open_complete shall be called with IO_OPEN_CANCELLED. ]*/
TEST_FUNCTION(when_on_underlying_ws_open_complete_is_called_with_CANCELLED_while_OPENING_the_io_open_complete_callback_is_called_with_OPEN_CANCELLED)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_CANCELLED));

    // act
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_CANCELLED);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_137: [ When on_underlying_ws_open_complete is called with any other error code while the IO is opening, the callback on_io_open_complete shall be called with IO_OPEN_ERROR. ]*/
TEST_FUNCTION(when_on_underlying_ws_open_complete_is_called_with_an_error_while_OPENING_the_io_open_complete_callback_is_called_with_OPEN_ERROR)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_ERROR_CANNOT_SEND_UPGRADE_REQUEST);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_138: [ When on_underlying_ws_open_complete is called with a NULL context, it shall do nothing. ]*/
TEST_FUNCTION(on_underlying_ws_open_complete_with_NULL_context_does_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    g_on_ws_open_complete(NULL, WS_OPEN_ERROR_CANNOT_SEND_UPGRADE_REQUEST);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_139: [ When on_underlying_ws_open_complete is called while in OPEN state it shall indicate an error by calling the on_io_error callback passed to wsio_open and switch to the ERROR state. ]*/
/* Tests_SRS_WSIO_01_140: [ When calling on_io_error, the on_io_error_context argument given in wsio_open shall be passed to the callback on_io_error. ]*/
TEST_FUNCTION(when_on_underlying_ws_open_complete_is_called_when_already_OPEN_an_error_is_indicated)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_141: [ When on_underlying_ws_open_complete is called while in the ERROR state it shall indicate an error by calling the on_io_error callback passed to wsio_open. ]*/
/* Tests_SRS_WSIO_01_140: [ When calling on_io_error, the on_io_error_context argument given in wsio_open shall be passed to the callback on_io_error. ]*/
TEST_FUNCTION(when_on_underlying_ws_open_complete_is_called_when_in_ERROR_an_error_is_indicated)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_142: [ When on_underlying_ws_open_complete is called while in the CLOSING state an error shall be indicated by calling the on_io_error callback passed to wsio_open. ]*/
TEST_FUNCTION(when_on_underlying_ws_open_complete_is_called_when_CLOSING_it_does_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* on_ws_frame_received */

/* Tests_SRS_WSIO_01_124: [ When on_underlying_ws_frame_received is called the bytes in the frame shall be indicated by calling the on_bytes_received callback passed to wsio_open. ]*/
/* Tests_SRS_WSIO_01_125: [ When calling on_bytes_received, the on_bytes_received_context argument given in wsio_open shall be passed to the callback on_bytes_received. ]*/
TEST_FUNCTION(when_on_underlying_ws_frame_received_is_called_the_frame_content_is_indicated_up)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    const unsigned char test_buffer[] = { 0x42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_bytes_received((void*)0x4243, IGNORED_PTR_ARG, sizeof(test_buffer)))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer));

    // act
    g_on_ws_frame_received(g_on_ws_frame_received_context, WS_FRAME_TYPE_BINARY, test_buffer, sizeof(test_buffer));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_125: [ When calling on_bytes_received, the on_bytes_received_context argument given in wsio_open shall be passed to the callback on_bytes_received. ]*/
TEST_FUNCTION(when_on_underlying_ws_frame_received_is_called_the_frame_content_is_indicated_up_with_the_proper_context)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    const unsigned char test_buffer[] = { 0x42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, NULL, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_bytes_received(NULL, IGNORED_PTR_ARG, sizeof(test_buffer)))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer));

    // act
    g_on_ws_frame_received(g_on_ws_frame_received_context, WS_FRAME_TYPE_BINARY, test_buffer, sizeof(test_buffer));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_126: [ If on_underlying_ws_frame_received is called while the IO is in any state other than OPEN, it shall do nothing. ]*/
TEST_FUNCTION(when_on_underlying_ws_frame_received_is_called_while_opening_it_shall_do_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    const unsigned char test_buffer[] = { 0x42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, NULL, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    g_on_ws_frame_received(g_on_ws_frame_received_context, WS_FRAME_TYPE_BINARY, test_buffer, sizeof(test_buffer));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_126: [ If on_underlying_ws_frame_received is called while the IO is in any state other than OPEN, it shall do nothing. ]*/
TEST_FUNCTION(when_on_underlying_ws_frame_received_is_called_while_closing_it_shall_do_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    const unsigned char test_buffer[] = { 0x42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, NULL, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    // act
    g_on_ws_frame_received(g_on_ws_frame_received_context, WS_FRAME_TYPE_BINARY, test_buffer, sizeof(test_buffer));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_150: [ If on_underlying_ws_frame_received is called with NULL context it shall do nothing. ]*/
TEST_FUNCTION(when_on_underlying_ws_frame_received_with_NULL_context_does_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    const unsigned char test_buffer[] = { 0x42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, NULL, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    // act
    g_on_ws_frame_received(NULL, WS_FRAME_TYPE_BINARY, test_buffer, sizeof(test_buffer));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_151: [ If the WebSocket frame type is not binary then an error shall be indicated by calling the on_io_error callback passed to wsio_open. ]*/
/* Tests_SRS_WSIO_01_152: [ When calling on_io_error, the on_io_error_context argument given in wsio_open shall be passed to the callback on_io_error. ]*/
TEST_FUNCTION(when_on_underlying_ws_frame_received_is_called_with_a_text_frame_an_error_is_indicated)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    const unsigned char test_buffer[] = { 0x42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, NULL, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    g_on_ws_frame_received(g_on_ws_frame_received_context, WS_FRAME_TYPE_TEXT, test_buffer, sizeof(test_buffer));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_153: [ When on_underlying_ws_frame_received is called with zero size, no bytes shall be indicated up as received. ]*/
TEST_FUNCTION(when_on_underlying_ws_frame_received_is_called_with_zero_bytes_no_bytes_are_reported_as_received)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    const unsigned char test_buffer[] = { 0x42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, NULL, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    // act
    g_on_ws_frame_received(g_on_ws_frame_received_context, WS_FRAME_TYPE_BINARY, test_buffer, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_154: [ When on_underlying_ws_frame_received is called with a positive size and a NULL buffer, an error shall be indicated by calling the on_io_error callback passed to wsio_open. ]*/
TEST_FUNCTION(when_on_underlying_ws_frame_received_is_called_with_positive_size_and_NULL_buffer_an_error_is_indicated)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, NULL, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    g_on_ws_frame_received(g_on_ws_frame_received_context, WS_FRAME_TYPE_BINARY, NULL, 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* on_underlying_ws_send_frame_complete */

/* Tests_SRS_WSIO_01_143: [ When on_underlying_ws_send_frame_complete is called after sending a WebSocket frame, the pending IO shall be removed from the list. ]*/
/* Tests_SRS_WSIO_01_145: [ Removing it from the list shall be done by calling singlylinkedlist_remove. ]*/
/* Tests_SRS_WSIO_01_144: [ Also the pending IO data shall be freed. ]*/
/* Tests_SRS_WSIO_01_146: [ When on_underlying_ws_send_frame_complete is called with WS_SEND_OK, the callback on_send_complete shall be called with IO_SEND_OK. ]*/
TEST_FUNCTION(wsio_send_with_1_byte_completed_indicates_the_completion_up)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    unsigned char test_buffer[] = { 42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4343);
    umock_c_reset_all_calls();

    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4343, IO_SEND_OK));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    g_on_ws_send_frame_complete(g_on_ws_send_frame_complete_context, WS_SEND_FRAME_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_147: [ When on_underlying_ws_send_frame_complete is called with WS_SEND_CANCELLED, the callback on_send_complete shall be called with IO_SEND_CANCELLED. ]*/
TEST_FUNCTION(wsio_send_with_1_byte_completed_with_CANCELLED_indicates_the_completion_up_as_CANCELLED)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    unsigned char test_buffer[] = { 42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4343);
    umock_c_reset_all_calls();

    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4343, IO_SEND_CANCELLED));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    g_on_ws_send_frame_complete(g_on_ws_send_frame_complete_context, WS_SEND_FRAME_CANCELLED);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_148: [ When on_underlying_ws_send_frame_complete is called with any other error code, the callback on_send_complete shall be called with IO_SEND_ERROR. ]*/
TEST_FUNCTION(wsio_send_with_1_byte_completed_with_ERROR_indicates_the_completion_up_as_ERROR)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    unsigned char test_buffer[] = { 42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4343);
    umock_c_reset_all_calls();

    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4343, IO_SEND_ERROR));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    g_on_ws_send_frame_complete(g_on_ws_send_frame_complete_context, WS_SEND_FRAME_ERROR);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_155: [ When on_underlying_ws_send_frame_complete is called with a NULL context it shall do nothing. ]*/
TEST_FUNCTION(on_underlying_ws_send_frame_complete_with_NULL_context_does_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    unsigned char test_buffer[] = { 42 };

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_send(wsio, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4343);
    umock_c_reset_all_calls();

    // act
    g_on_ws_send_frame_complete(NULL, WS_SEND_FRAME_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* wsio_setoption */

/* Tests_SRS_WSIO_01_109: [ If any of the arguments ws_io or option_name is NULL wsio_setoption shall return a non-zero value. ]*/
TEST_FUNCTION(wsio_setoption_with_NULL_handle_fails)
{
    // arrange
    int result;

    // act
    result = wsio_get_interface_description()->concrete_io_setoption(NULL, "option1", (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_109: [ If any of the arguments ws_io or option_name is NULL wsio_setoption shall return a non-zero value. ]*/
TEST_FUNCTION(wsio_setoption_with_NULL_option_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    umock_c_reset_all_calls();

    // act
    result = wsio_get_interface_description()->concrete_io_setoption(wsio, NULL, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_183: [ If the option name is WSIOOptions then wsio_setoption shall call OptionHandler_FeedOptions and pass to it the underlying IO handle and the value argument. ]*/
/* Tests_SRS_WSIO_01_158: [ On success, wsio_setoption shall return 0. ]*/
TEST_FUNCTION(wsio_setoption_with_WSIOOptions_feeds_the_options_to_the_underlying_layer)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(OptionHandler_FeedOptions(TEST_OPTIONHANDLER_HANDLE, TEST_UWS_HANDLE));

    // act
    result = wsio_get_interface_description()->concrete_io_setoption(wsio, "WSIOOptions", (void*)TEST_OPTIONHANDLER_HANDLE);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_184: [ If OptionHandler_FeedOptions fails, wsio_setoption shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_OptionHandler_FeedOptions_fails_then_wsio_setoption_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(OptionHandler_FeedOptions(TEST_OPTIONHANDLER_HANDLE, TEST_UWS_HANDLE))
        .SetReturn(OPTIONHANDLER_ERROR);

    // act
    result = wsio_get_interface_description()->concrete_io_setoption(wsio, "WSIOOptions", (void*)TEST_OPTIONHANDLER_HANDLE);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_156: [ Otherwise all options shall be passed as they are to uws by calling uws_client_set_option. ]*/
/* Tests_SRS_WSIO_01_158: [ On success, wsio_setoption shall return 0. ]*/
TEST_FUNCTION(wsio_setoption_passes_the_option_dows_to_uws)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(uws_client_set_option(TEST_UWS_HANDLE, "option1", (void*)0x4242));

    // act
    result = wsio_get_interface_description()->concrete_io_setoption(wsio, "option1", (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_157: [ If uws_client_set_option fails, wsio_setoption shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_uws_set_option_fails_wsio_setoption_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    int result;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(uws_client_set_option(TEST_UWS_HANDLE, "option1", (void*)0x4242))
        .SetReturn(1);

    // act
    result = wsio_get_interface_description()->concrete_io_setoption(wsio, "option1", (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* wsio_retrieveoptions */

/* Tests_SRS_WSIO_01_118: [ If parameter handle is NULL then wsio_retrieveoptions shall fail and return NULL. ]*/
TEST_FUNCTION(wsio_retrieveoptions_with_NULL_handle_fails)
{
    // arrange
    OPTIONHANDLER_HANDLE result;

    // act
    result = wsio_get_interface_description()->concrete_io_retrieveoptions(NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_WSIO_01_119: [ wsio_retrieveoptions shall call OptionHandler_Create to produce an OPTIONHANDLER_HANDLE and on success return the new OPTIONHANDLER_HANDLE handle. ]*/
/* Tests_SRS_WSIO_01_179: [ When calling uws_client_retrieve_options the uws client handle shall be passed to it. ]*/
/* Tests_SRS_WSIO_01_178: [ uws_client_retrieve_options shall add to the option handler one option, whose name shall be uWSCLientOptions and the value shall be queried by calling uws_client_retrieve_options. ]*/
/* Tests_SRS_WSIO_01_181: [ Adding the option shall be done by calling OptionHandler_AddOption. ]*/
TEST_FUNCTION(wsio_retrieveoptions_creates_an_option_handler_with_the_value_obtained_from_the_underlying_retrieve_options)
{
    // arrange
    OPTIONHANDLER_HANDLE result;
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    umock_c_reset_all_calls();

    EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uws_client_retrieve_options(TEST_UWS_HANDLE));
    STRICT_EXPECTED_CALL(OptionHandler_AddOption(TEST_OPTIONHANDLER_HANDLE, "WSIOOptions", TEST_UWS_CLIENT_OPTIONHANDLER_HANDLE));
    STRICT_EXPECTED_CALL(OptionHandler_Destroy(IGNORED_PTR_ARG));

    // act
    result = wsio_get_interface_description()->concrete_io_retrieveoptions(wsio);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_120: [ If OptionHandler_Create fails then wsio_retrieveoptions shall fail and return NULL.  ]*/
TEST_FUNCTION(when_OptionHandler_Create_fails_then_wsio_retrieveoptions_fails)
{
    // arrange
    OPTIONHANDLER_HANDLE result;
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    umock_c_reset_all_calls();

    EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(NULL);

    // act
    result = wsio_get_interface_description()->concrete_io_retrieveoptions(wsio);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_180: [ If uws_client_retrieve_options fails, uws_client_retrieve_options shall fail and return NULL. ]*/
TEST_FUNCTION(when_uws_client_retrieve_options_fails_then_wsio_retrieveoptions_fails)
{
    // arrange
    OPTIONHANDLER_HANDLE result;
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    umock_c_reset_all_calls();

    EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uws_client_retrieve_options(TEST_UWS_HANDLE))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(OptionHandler_Destroy(TEST_OPTIONHANDLER_HANDLE));

    // act
    result = wsio_get_interface_description()->concrete_io_retrieveoptions(wsio);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_182: [ If OptionHandler_AddOption fails, uws_client_retrieve_options shall fail and return NULL. ]*/
TEST_FUNCTION(when_OptionHandler_AddOption_fails_then_wsio_retrieveoptions_fails)
{
    // arrange
    OPTIONHANDLER_HANDLE result;
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    umock_c_reset_all_calls();

    EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uws_client_retrieve_options(TEST_UWS_HANDLE));
    STRICT_EXPECTED_CALL(OptionHandler_AddOption(TEST_OPTIONHANDLER_HANDLE, "WSIOOptions", TEST_UWS_CLIENT_OPTIONHANDLER_HANDLE))
        .SetReturn(OPTIONHANDLER_ERROR);
    STRICT_EXPECTED_CALL(OptionHandler_Destroy(TEST_OPTIONHANDLER_HANDLE));
    STRICT_EXPECTED_CALL(OptionHandler_Destroy(TEST_UWS_CLIENT_OPTIONHANDLER_HANDLE));

    // act
    result = wsio_get_interface_description()->concrete_io_retrieveoptions(wsio);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* wsio_clone_option */

/* Tests_SRS_WSIO_01_174: [ If wsio_clone_option is called with NULL name or value it shall return NULL. ]*/
TEST_FUNCTION(wsio_clone_option_with_NULL_name_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    void* result;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_retrieveoptions(wsio);
    umock_c_reset_all_calls();

    // act
    result = g_clone_option(NULL, (void*)0x4243);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_174: [ If wsio_clone_option is called with NULL name or value it shall return NULL. ]*/
TEST_FUNCTION(wsio_clone_option_with_NULL_value_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    void* result;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_retrieveoptions(wsio);
    umock_c_reset_all_calls();

    // act
    result = g_clone_option("WSIOOptions", NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_171: [** wsio_clone_option called with name being WSIOOptions shall return the same value. ]*/
TEST_FUNCTION(wsio_clone_option_with_WSIOOptions_clones_the_option_handler)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    void* result;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_retrieveoptions(wsio);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(OptionHandler_Clone(IGNORED_PTR_ARG));

    // act
    result = g_clone_option("WSIOOptions", (void*)0x4243);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, (void*)0x4246, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_173: [ wsio_clone_option called with any other option name than WSIOOptions shall return NULL. ]*/
TEST_FUNCTION(wsio_clone_option_with_an_unknown_option_name_fails)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;
    void* result;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_retrieveoptions(wsio);
    umock_c_reset_all_calls();

    // act
    result = g_clone_option("Cucu", (void*)0x4243);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* wsio_destroy_option */

/* Tests_SRS_WSIO_01_175: [ wsio_destroy_option called with the option name being WSIOOptions shall destroy the value by calling OptionHandler_Destroy. ]*/
TEST_FUNCTION(wsio_destroy_option_with_WSIOOptions_destroys_the_handler)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_retrieveoptions(wsio);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(OptionHandler_Destroy(TEST_OPTIONHANDLER_HANDLE));

    // act
    g_destroy_option("WSIOOptions", TEST_OPTIONHANDLER_HANDLE);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_176: [ If wsio_destroy_option is called with any other name it shall do nothing. ]*/
TEST_FUNCTION(wsio_destroy_option_with_an_unknown_option_does_no_destroy)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_retrieveoptions(wsio);
    umock_c_reset_all_calls();

    // act
    g_destroy_option("cucu", TEST_OPTIONHANDLER_HANDLE);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_177: [ If wsio_destroy_option is called with NULL name or value it shall do nothing. ]*/
TEST_FUNCTION(wsio_destroy_option_with_NULL_name_does_no_destroy)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_retrieveoptions(wsio);
    umock_c_reset_all_calls();

    // act
    g_destroy_option(NULL, TEST_OPTIONHANDLER_HANDLE);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_177: [ If wsio_destroy_option is called with NULL name or value it shall do nothing. ]*/
TEST_FUNCTION(wsio_destroy_option_with_NULL_value_does_no_destroy)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_retrieveoptions(wsio);
    umock_c_reset_all_calls();

    // act
    g_destroy_option("WSIOOptions", NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* on_underlying_ws_close_complete */

/* Tests_SRS_WSIO_01_159: [ When on_underlying_ws_close_complete while the IO is closing (after wsio_close), the close shall be indicated up by calling the on_io_close_complete callback passed to wsio_close. ]*/
/* Tests_SRS_WSIO_01_163: [ When on_io_close_complete is called, the context passed to wsio_close shall be passed as argument to on_io_close_complete. ]*/
TEST_FUNCTION(on_underlying_ws_close_complete_while_closing_triggers_the_send_complete_callback)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_close_complete((void*)0x4245));

    // act
    g_on_ws_close_complete(g_on_ws_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_160: [ If NULL was passed to wsio_close no callback shall be called. ]*/
TEST_FUNCTION(when_on_close_complete_was_NULL_on_underlying_ws_close_complete_does_not_trigger_any_callback)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_close(wsio, NULL, (void*)0x4245);
    umock_c_reset_all_calls();

    // act
    g_on_ws_close_complete(g_on_ws_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_161: [ If the context passed to on_underlying_ws_close_complete is NULL, on_underlying_ws_close_complete shall do nothing. ]*/
TEST_FUNCTION(on_underlying_ws_close_complete_with_NULL_context_does_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    // act
    g_on_ws_close_complete(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* on_underlying_ws_peer_closed */

/* Tests_SRS_WSIO_01_170: [ When on_underlying_ws_peer_closed and the state of the IO is OPENING an error shall be indicated by calling the on_io_open_complete callback passed to wsio_open with the error code WS_OPEN_ERROR. ]*/
/* Tests_SRS_WSIO_01_168: [ The close_code, extra_data and extra_data_length arguments shall be ignored. ]*/
TEST_FUNCTION(on_underlying_ws_peer_closed_when_OPENING_indicates_an_error)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    g_on_ws_peer_closed(g_on_ws_peer_closed_context, NULL, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_166: [ When on_underlying_ws_peer_closed and the state of the IO is OPEN an error shall be indicated by calling the on_io_error callback passed to wsio_open. ]*/
/* Tests_SRS_WSIO_01_168: [ The close_code, extra_data and extra_data_length arguments shall be ignored. ]*/
TEST_FUNCTION(on_underlying_ws_peer_closed_when_OPEN_indicates_an_error)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    g_on_ws_peer_closed(g_on_ws_peer_closed_context, NULL, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_166: [ When on_underlying_ws_peer_closed and the state of the IO is OPEN an error shall be indicated by calling the on_io_error callback passed to wsio_open. ]*/
/* Tests_SRS_WSIO_01_168: [ The close_code, extra_data and extra_data_length arguments shall be ignored. ]*/
TEST_FUNCTION(on_underlying_ws_peer_closed_when_CLOSING_indicates_an_error)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    g_on_ws_peer_closed(g_on_ws_peer_closed_context, NULL, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

// Tests_SRS_WSIO_07_001: [When on_underlying_ws_peer_closed and the state of the IO is NOT_OPEN an error will be raised and the io_state will remain as NOT_OPEN]
TEST_FUNCTION(on_underlying_ws_peer_closed_when_NOT_OPEN_indicates_an_error)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    g_on_ws_peer_closed(g_on_ws_peer_closed_context, NULL, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

/* Tests_SRS_WSIO_01_167: [ If on_underlying_ws_peer_closed is called with a NULL context it shall do nothing. ]*/
TEST_FUNCTION(on_underlying_ws_peer_closed_with_NULL_context_does_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE wsio;

    wsio = wsio_get_interface_description()->concrete_io_create(&default_wsio_config);
    (void)wsio_get_interface_description()->concrete_io_open(wsio, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_ws_open_complete(g_on_ws_open_complete_context, WS_OPEN_OK);
    (void)wsio_get_interface_description()->concrete_io_close(wsio, test_on_io_close_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    // act
    g_on_ws_peer_closed(NULL, NULL, NULL, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    wsio_get_interface_description()->concrete_io_destroy(wsio);
}

END_TEST_SUITE(wsio_ut)
