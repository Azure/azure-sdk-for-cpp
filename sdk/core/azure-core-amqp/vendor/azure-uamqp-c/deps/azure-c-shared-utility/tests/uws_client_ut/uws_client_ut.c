// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#else
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_bool.h"
#include "umock_c/umock_c_negative_tests.h"

/* Requirements not needed as they are optional:
Tests_SRS_UWS_CLIENT_01_254: [ If an endpoint receives a Ping frame and has not yet sent Pong frame(s) in response to previous Ping frame(s), the endpoint MAY elect to send a Pong frame for only the most recently processed Ping frame. ]
Tests_SRS_UWS_CLIENT_01_255: [ A Pong frame MAY be sent unsolicited. ]
Tests_SRS_UWS_CLIENT_01_256: [ A response to an unsolicited Pong frame is not expected. ]
*/

/* Requirements satisfied by the underlying TLS/socket stack
Tests_SRS_UWS_CLIENT_01_362: [ To achieve reasonable levels of protection, clients should use only Strong TLS algorithms. ]
Tests_SRS_UWS_CLIENT_01_289: [ An endpoint SHOULD use a method that cleanly closes the TCP connection, as well as the TLS session, if applicable, discarding any trailing bytes that may have been received. ]
Tests_SRS_UWS_CLIENT_01_078: [ Otherwise, all further communication on this channel MUST run through the encrypted tunnel [RFC5246]. ]
Tests_SRS_UWS_CLIENT_01_141: [ masking is done whether or not the WebSocket Protocol is running over TLS. ]
*/

/* Requirements satisfied by the way the APIs are defined
Tests_SRS_UWS_CLIENT_01_324: [ 1000 indicates a normal closure, meaning that the purpose for which the connection was established has been fulfilled. ]
Tests_SRS_UWS_CLIENT_01_325: [ 1001 indicates that an endpoint is "going away", such as a server going down or a browser having navigated away from a page. ]
Tests_SRS_UWS_CLIENT_01_326: [ 1002 indicates that an endpoint is terminating the connection due to a protocol error. ]
Tests_SRS_UWS_CLIENT_01_327: [ 1003 indicates that an endpoint is terminating the connection because it has received a type of data it cannot accept (e.g., an endpoint that understands only text data MAY send this if it receives a binary message). ]
Tests_SRS_UWS_CLIENT_01_328: [ Reserved.  The specific meaning might be defined in the future. ]
Tests_SRS_UWS_CLIENT_01_329: [ 1005 is a reserved value and MUST NOT be set as a status code in a Close control frame by an endpoint. ]
Tests_SRS_UWS_CLIENT_01_330: [ 1006 is a reserved value and MUST NOT be set as a status code in a Close control frame by an endpoint. ]
Tests_SRS_UWS_CLIENT_01_331: [ 1007 indicates that an endpoint is terminating the connection because it has received data within a message that was not consistent with the type of the message (e.g., non-UTF-8 [RFC3629] data within a text message). ]
Tests_SRS_UWS_CLIENT_01_332: [ 1008 indicates that an endpoint is terminating the connection because it has received a message that violates its policy. ]
Tests_SRS_UWS_CLIENT_01_333: [ 1009 indicates that an endpoint is terminating the connection because it has received a message that is too big for it to process. ]
Tests_SRS_UWS_CLIENT_01_334: [ 1010 indicates that an endpoint (client) is terminating the connection because it has expected the server to negotiate one or more extension, but the server didn't return them in the response message of the WebSocket handshake. ]
Tests_SRS_UWS_CLIENT_01_336: [ 1011 indicates that a server is terminating the connection because it encountered an unexpected condition that prevented it from fulfilling the request. ]
Tests_SRS_UWS_CLIENT_01_337: [ 1015 is a reserved value and MUST NOT be set as a status code in a Close control frame by an endpoint. ]
Tests_SRS_UWS_CLIENT_01_238: [ As the data is not guaranteed to be human readable, clients MUST NOT show it to end users. ]
Tests_SRS_UWS_CLIENT_01_211: [ One implication of this is that in absence of extensions, senders and receivers must not depend on the presence of specific frame boundaries. ]
*/

#define ENABLE_MOCKS

#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/uws_frame_encoder.h"
#include "azure_c_shared_utility/gb_rand.h"
#include "azure_c_shared_utility/azure_base64.h"
#include "azure_c_shared_utility/map.h"

IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_SEND_RESULT, IO_SEND_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(OPTIONHANDLER_RESULT, OPTIONHANDLER_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(WS_FRAME_TYPE, WS_FRAME_TYPE_VALUES);

static const void** list_items = NULL;
static size_t list_item_count = 0;
static const SINGLYLINKEDLIST_HANDLE TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE = (SINGLYLINKEDLIST_HANDLE)0x4242;
static const LIST_ITEM_HANDLE TEST_LIST_ITEM_HANDLE = (LIST_ITEM_HANDLE)0x4243;
static const XIO_HANDLE TEST_IO_HANDLE = (XIO_HANDLE)0x4244;
static const OPTIONHANDLER_HANDLE TEST_IO_OPTIONHANDLER_HANDLE = (OPTIONHANDLER_HANDLE)0x4446;
static const OPTIONHANDLER_HANDLE TEST_OPTIONHANDLER_HANDLE = (OPTIONHANDLER_HANDLE)0x4447;
static const STRING_HANDLE BASE64_ENCODED_STRING = (STRING_HANDLE)0x4447;
static const MAP_HANDLE TEST_REQUEST_HEADERS_MAP = (MAP_HANDLE)0x4448;

static size_t currentmalloc_call;
static size_t whenShallmalloc_fail;
static size_t currentrealloc_call;
static size_t whenShallrealloc_fail;

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

static void* my_gballoc_realloc(void* ptr, size_t size)
{
    void* result;
    currentrealloc_call++;
    if (whenShallrealloc_fail > 0)
    {
        if (currentrealloc_call == whenShallrealloc_fail)
        {
            result = NULL;
        }
        else
        {
            result = realloc(ptr, size);
        }
    }
    else
    {
        result = realloc(ptr, size);
    }
    return result;
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

static int my_mallocAndStrcpy_s(char** destination, const char* source)
{
    *destination = (char*)malloc(strlen(source) + 1);
    (void)strcpy(*destination, source);
    return 0;
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

static char* my_Map_GetInternals_keys[10];
static char* my_Map_GetInternals_values[10];
static size_t my_Map_GetInternals_count;
static MAP_RESULT my_Map_GetInternals(MAP_HANDLE handle, const char*const** keys, const char*const** values, size_t* count)
{
    (void)handle;
    *keys = (const char*const*)my_Map_GetInternals_keys;
    *values = (const char*const*)my_Map_GetInternals_values;
    *count = my_Map_GetInternals_count;
    return MAP_OK;
}


#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/utf8_checker.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/optionhandler.h"

#undef ENABLE_MOCKS

#include "azure_c_shared_utility/uws_client.h"

static const WS_PROTOCOL protocols[] = { { "test_protocol" } };

IMPLEMENT_UMOCK_C_ENUM_TYPE(WS_OPEN_RESULT, WS_OPEN_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(WS_ERROR, WS_ERROR_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(WS_SEND_FRAME_RESULT, WS_SEND_FRAME_RESULT_VALUES);

static char* umocktypes_stringify_const_SOCKETIO_CONFIG_ptr(const SOCKETIO_CONFIG** value)
{
    char* result = NULL;
    char temp_buffer[1024];
    int length;
    length = sprintf(temp_buffer, "{ hostname = %s, port = %d, accepted_socket = %p }",
        (*value)->hostname,
        (*value)->port,
        (*value)->accepted_socket);

    if (length > 0)
    {
        result = (char*)malloc(strlen(temp_buffer) + 1);
        if (result != NULL)
        {
            (void)memcpy(result, temp_buffer, strlen(temp_buffer) + 1);
        }
    }

    return result;
}

static int umocktypes_are_equal_const_SOCKETIO_CONFIG_ptr(const SOCKETIO_CONFIG** left, const SOCKETIO_CONFIG** right)
{
    int result;

    if ((left == NULL) ||
        (right == NULL))
    {
        result = -1;
    }
    else
    {
        result = ((*left)->port == (*right)->port);
        result = result && ((*left)->accepted_socket == (*right)->accepted_socket);
        if (strcmp((*left)->hostname, (*right)->hostname) != 0)
        {
            result = 0;
        }
    }

    return result;
}

static char* copy_string(const char* source)
{
    char* result;

    if (source == NULL)
    {
        result = NULL;
    }
    else
    {
        size_t length = strlen(source);
        result = (char*)malloc(length + 1);
        (void)memcpy(result, source, length + 1);
    }

    return result;
}

static int umocktypes_copy_const_SOCKETIO_CONFIG_ptr(SOCKETIO_CONFIG** destination, const SOCKETIO_CONFIG** source)
{
    int result;

    *destination = (SOCKETIO_CONFIG*)malloc(sizeof(SOCKETIO_CONFIG));
    if (*destination == NULL)
    {
        result = MU_FAILURE;
    }
    else
    {
        if ((*source)->hostname == NULL)
        {
            (*destination)->hostname = NULL;
        }
        else
        {
            (*destination)->hostname = copy_string((*source)->hostname);
            (*destination)->port = (*source)->port;
            (*destination)->accepted_socket = (*source)->accepted_socket;
        }

        result = 0;
    }

    return result;
}

static void umocktypes_free_const_SOCKETIO_CONFIG_ptr(SOCKETIO_CONFIG** value)
{
    free((void*)(*value)->hostname);
    free(*value);
}

// consumer mocks
MOCK_FUNCTION_WITH_CODE(, void, test_on_ws_open_complete, void*, context, WS_OPEN_RESULT, ws_open_result)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_ws_frame_received, void*, context, unsigned char, frame_type, const unsigned char*, buffer, size_t, size)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_ws_peer_closed, void*, context, uint16_t*, close_code, const unsigned char*, extra_data, size_t, extra_data_length)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_ws_error, void*, context, WS_ERROR, error_code);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_ws_close_complete, void*, context);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, test_on_ws_send_frame_complete, void*, context, WS_SEND_FRAME_RESULT, ws_send_frame_result)
MOCK_FUNCTION_END()

static ON_IO_OPEN_COMPLETE g_on_io_open_complete;
static void* g_on_io_open_complete_context;
static ON_SEND_COMPLETE g_on_io_send_complete;
static void* g_on_io_send_complete_context;
static int g_xio_send_result;
static ON_BYTES_RECEIVED g_on_bytes_received;
static void* g_on_bytes_received_context;
static ON_IO_ERROR g_on_io_error;
static void* g_on_io_error_context;
static ON_IO_CLOSE_COMPLETE g_on_io_close_complete;
static void* g_on_io_close_complete_context;

static int my_xio_open(XIO_HANDLE xio, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    (void)xio;
    g_on_io_open_complete = on_io_open_complete;
    g_on_io_open_complete_context = on_io_open_complete_context;
    g_on_bytes_received = on_bytes_received;
    g_on_bytes_received_context = on_bytes_received_context;
    g_on_io_error = on_io_error;
    g_on_io_error_context = on_io_error_context;
    return 0;
}

static int my_xio_close(XIO_HANDLE xio, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
    (void)xio;
    g_on_io_close_complete = on_io_close_complete;
    g_on_io_close_complete_context = callback_context;
    return 0;
}

static int my_xio_send(XIO_HANDLE xio, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    (void)xio;
    (void)buffer;
    (void)size;
    g_on_io_send_complete = on_send_complete;
    g_on_io_send_complete_context = callback_context;
    return g_xio_send_result;
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

static TEST_MUTEX_HANDLE g_testByTest;

static const IO_INTERFACE_DESCRIPTION* TEST_SOCKET_IO_INTERFACE_DESCRIPTION = (const IO_INTERFACE_DESCRIPTION*)0x4542;
static const IO_INTERFACE_DESCRIPTION* TEST_TLS_IO_INTERFACE_DESCRIPTION = (const IO_INTERFACE_DESCRIPTION*)0x4543;

#ifdef __cplusplus
extern "C" {
#endif

    extern BUFFER_HANDLE real_BUFFER_new(void);
    extern void real_BUFFER_delete(BUFFER_HANDLE handle);
    extern unsigned char* real_BUFFER_u_char(BUFFER_HANDLE handle);
    extern size_t real_BUFFER_length(BUFFER_HANDLE handle);

    BUFFER_HANDLE my_uws_frame_encoder_encode(WS_FRAME_TYPE opcode, const unsigned char* payload, size_t length, bool is_masked, bool is_final, unsigned char reserved)
    {
        (void)opcode;
        (void)payload;
        (void)length;
        (void)is_masked;
        (void)is_final;
        (void)reserved;
        return real_BUFFER_new();
    }

#ifdef __cplusplus
}
#endif

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(uws_client_ut)

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
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_s, my_mallocAndStrcpy_s);
    REGISTER_GLOBAL_MOCK_HOOK(xio_open, my_xio_open);
    REGISTER_GLOBAL_MOCK_HOOK(xio_close, my_xio_close);
    REGISTER_GLOBAL_MOCK_HOOK(xio_send, my_xio_send);
    REGISTER_GLOBAL_MOCK_RETURN(singlylinkedlist_create, TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_remove, my_singlylinkedlist_remove);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_get_head_item, my_singlylinkedlist_get_head_item);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_add, my_singlylinkedlist_add);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_item_get_value, my_singlylinkedlist_item_get_value);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_find, my_singlylinkedlist_find);
    REGISTER_GLOBAL_MOCK_HOOK(OptionHandler_Create, my_OptionHandler_Create);
    REGISTER_GLOBAL_MOCK_RETURN(socketio_get_interface_description, TEST_SOCKET_IO_INTERFACE_DESCRIPTION);
    REGISTER_GLOBAL_MOCK_RETURN(platform_get_default_tlsio, TEST_TLS_IO_INTERFACE_DESCRIPTION);
    REGISTER_GLOBAL_MOCK_RETURN(xio_create, TEST_IO_HANDLE);
    REGISTER_GLOBAL_MOCK_RETURN(xio_retrieveoptions, TEST_IO_OPTIONHANDLER_HANDLE);
    REGISTER_GLOBAL_MOCK_RETURN(utf8_checker_is_valid_utf8, true);
    REGISTER_GLOBAL_MOCK_RETURN(Azure_Base64_Encode_Bytes, BASE64_ENCODED_STRING);
    REGISTER_GLOBAL_MOCK_RETURN(OptionHandler_FeedOptions, OPTIONHANDLER_OK);
    REGISTER_GLOBAL_MOCK_RETURN(OptionHandler_AddOption, OPTIONHANDLER_OK);
    REGISTER_GLOBAL_MOCK_RETURN(OptionHandler_Clone, TEST_OPTIONHANDLER_HANDLE);
    REGISTER_GLOBAL_MOCK_HOOK(BUFFER_new, real_BUFFER_new);
    REGISTER_GLOBAL_MOCK_HOOK(BUFFER_delete, real_BUFFER_delete);
    REGISTER_GLOBAL_MOCK_HOOK(BUFFER_u_char, real_BUFFER_u_char);
    REGISTER_GLOBAL_MOCK_HOOK(BUFFER_length, real_BUFFER_length);
    REGISTER_GLOBAL_MOCK_HOOK(uws_frame_encoder_encode, my_uws_frame_encoder_encode);
    REGISTER_GLOBAL_MOCK_HOOK(Map_GetInternals, my_Map_GetInternals);
    REGISTER_GLOBAL_MOCK_RETURN(STRING_c_str, "test_str");
    REGISTER_GLOBAL_MOCK_RETURN(Map_Create, TEST_REQUEST_HEADERS_MAP);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(Map_Create, NULL);
    REGISTER_GLOBAL_MOCK_RETURN(Map_AddOrUpdate, MAP_OK);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(Map_AddOrUpdate, MAP_ERROR);
    REGISTER_GLOBAL_MOCK_RETURN(Map_GetInternals, MAP_OK);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(Map_GetInternals, MAP_ERROR);
    REGISTER_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT);
    REGISTER_TYPE(IO_SEND_RESULT, IO_SEND_RESULT);
    REGISTER_TYPE(WS_OPEN_RESULT, WS_OPEN_RESULT);
    REGISTER_TYPE(OPTIONHANDLER_RESULT, OPTIONHANDLER_RESULT);
    REGISTER_TYPE(WS_ERROR, WS_ERROR);
    REGISTER_TYPE(WS_SEND_FRAME_RESULT, WS_SEND_FRAME_RESULT);
    REGISTER_TYPE(WS_FRAME_TYPE, WS_FRAME_TYPE);
    REGISTER_TYPE(const SOCKETIO_CONFIG*, const_SOCKETIO_CONFIG_ptr);

    REGISTER_UMOCK_ALIAS_TYPE(SINGLYLINKEDLIST_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_ITEM_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_MATCH_FUNCTION, void*);
    REGISTER_UMOCK_ALIAS_TYPE(UWS_CLIENT_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(XIO_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_OPEN_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_RECEIVED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_ERROR, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_CLOSE_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_SEND_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(BUFFER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(OPTIONHANDLER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(STRING_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfCloneOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfSetOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfDestroyOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(MAP_FILTER_CALLBACK, void*);
    REGISTER_UMOCK_ALIAS_TYPE(MAP_HANDLE, void*);
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
    currentrealloc_call = 0;
    whenShallrealloc_fail = 0;
    singlylinkedlist_remove_result = 0;
    g_xio_send_result = 0;

    memset(my_Map_GetInternals_keys, 0, sizeof(my_Map_GetInternals_keys));
    memset(my_Map_GetInternals_values, 0, sizeof(my_Map_GetInternals_values));
    my_Map_GetInternals_count = 0;
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);

    umock_c_negative_tests_deinit();
}

/* uws_client_create */

/* Tests_SRS_UWS_CLIENT_01_001: [uws_client_create shall create an instance of uws and return a non-NULL handle to it.]*/
/* Tests_SRS_UWS_CLIENT_01_017: [ uws_client_create shall create a pending send IO list that is to be used to queue send packets by calling singlylinkedlist_create. ]*/
/* Tests_SRS_UWS_CLIENT_01_005: [ If use_ssl is false then uws_client_create shall obtain the interface used to create a socketio instance by calling socketio_get_interface_description. ]*/
/* Tests_SRS_UWS_CLIENT_01_008: [ The obtained interface shall be used to create the IO used as underlying IO by the newly created uws instance. ]*/
/* Tests_SRS_UWS_CLIENT_01_009: [ The underlying IO shall be created by calling xio_create. ]*/
/* Tests_SRS_UWS_CLIENT_01_010: [ The create arguments for the socket IO (when use_ssl is 0) shall have: ]*/
/* Tests_SRS_UWS_CLIENT_01_011: [ - hostname set to the hostname argument passed to uws_client_create. ]*/
/* Tests_SRS_UWS_CLIENT_01_012: [ - port set to the port argument passed to uws_client_create. ]*/
/* Tests_SRS_UWS_CLIENT_01_004: [ The argument hostname shall be copied for later use. ]*/
/* Tests_SRS_UWS_CLIENT_01_403: [ The argument port shall be copied for later use. ]*/
/* Tests_SRS_UWS_CLIENT_01_404: [ The argument resource_name shall be copied for later use. ]*/
/* Tests_SRS_UWS_CLIENT_01_413: [ The protocol information indicated by protocols and protocol_count shall be copied for later use (for constructing the upgrade request). ]*/
/* Tests_SRS_UWS_CLIENT_01_063: [ A client will need to supply a /host/, /port/, /resource name/, and a /secure/ flag, which are the components of a WebSocket URI as discussed in Section 3, along with a list of /protocols/ and /extensions/ to be used. ]*/
/* Tests_SRS_UWS_CLIENT_01_076: [ If /secure/ is true, the client MUST perform a TLS handshake over the connection after opening the connection and before sending the handshake data [RFC2818]. ]*/
TEST_FUNCTION(uws_client_create_with_valid_args_no_ssl_succeeds)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_CLIENT_HANDLE uws_client;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "111"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(Map_Create(NULL));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config))
        .IgnoreArgument_io_create_parameters();
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_protocol"))
        .IgnoreArgument_destination();

    // act
    uws_client = uws_client_create("test_host", 80, "111", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NOT_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_002: [ If any of the arguments hostname and resource_name is NULL then uws_client_create shall return NULL. ]*/
TEST_FUNCTION(uws_client_create_with_NULL_hostname_fails)
{
    // arrange

    // act
    UWS_CLIENT_HANDLE uws_client = uws_client_create(NULL, 80, "222", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_002: [ If any of the arguments hostname and resource_name is NULL then uws_client_create shall return NULL. ]*/
TEST_FUNCTION(uws_client_create_with_NULL_resource_name_fails)
{
    // arrange

    // act
    UWS_CLIENT_HANDLE uws_client = uws_client_create("testhost", 80, NULL, false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_012: [ - port set to the port argument passed to uws_client_create. ]*/
TEST_FUNCTION(uws_client_create_with_valid_args_no_ssl_port_different_than_80_succeeds)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_CLIENT_HANDLE uws_client;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 81;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "333"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(Map_Create(NULL));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config))
        .IgnoreArgument_io_create_parameters();
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_protocol"))
        .IgnoreArgument_destination();

    // act
    uws_client = uws_client_create("test_host", 81, "333", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NOT_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_410: [ The protocols argument shall be allowed to be NULL, in which case no protocol is to be specified by the client in the upgrade request. ]*/
TEST_FUNCTION(uws_client_create_with_NULL_protocols_succeeds)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_CLIENT_HANDLE uws_client;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 81;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "333"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(Map_Create(NULL));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config))
        .IgnoreArgument_io_create_parameters();

    // act
    uws_client = uws_client_create("test_host", 81, "333", false, NULL, 0);

    // assert
    ASSERT_IS_NOT_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_411: [ If protocol_count is non zero and protocols is NULL then uws_client_create shall fail and return NULL. ]*/
TEST_FUNCTION(uws_client_create_with_non_zero_protocol_count_and_NULL_protocols_fails)
{
    // arrange
    UWS_CLIENT_HANDLE uws_client;

    // act
    uws_client = uws_client_create("test_host", 81, "333", false, NULL, 1);

    // assert
    ASSERT_IS_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_412: [ If the protocol member of any of the items in the protocols argument is NULL, then uws_client_create shall fail and return NULL. ]*/
TEST_FUNCTION(uws_client_create_with_the_first_protocol_name_NULL_fails)
{
    // arrange
    UWS_CLIENT_HANDLE uws_client;
    WS_PROTOCOL NULL_test_protocol[] = { { NULL } };

    // act
    uws_client = uws_client_create("test_host", 81, "333", false, NULL_test_protocol, sizeof(NULL_test_protocol) / sizeof(NULL_test_protocol[0]));

    // assert
    ASSERT_IS_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_412: [ If the protocol member of any of the items in the protocols argument is NULL, then uws_client_create shall fail and return NULL. ]*/
TEST_FUNCTION(uws_client_create_with_the_second_protocol_name_NULL_fails)
{
    // arrange
    UWS_CLIENT_HANDLE uws_client;
    WS_PROTOCOL NULL_test_protocol[] = { { "aaa" }, { NULL } };

    // act
    uws_client = uws_client_create("test_host", 81, "333", false, NULL_test_protocol, sizeof(NULL_test_protocol) / sizeof(NULL_test_protocol[0]));

    // assert
    ASSERT_IS_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_003: [ If allocating memory for the new uws instance fails then uws_client_create shall return NULL. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_new_uws_instance_fails_then_uws_client_create_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_CLIENT_HANDLE uws_client;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    uws_client = uws_client_create("test_host", 80, "aaa", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_392: [ If allocating memory for the copy of the hostname argument fails, then uws_client_create shall return NULL. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_hostname_copy_fails_then_uws_client_create_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_CLIENT_HANDLE uws_client;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination()
        .SetReturn(1);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws_client = uws_client_create("test_host", 80, "bbb", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_405: [ If allocating memory for the copy of the resource_name argument fails, then uws_client_create shall return NULL. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_resource_name_copy_fails_then_uws_client_create_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_CLIENT_HANDLE uws_client;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_resource/1"))
        .IgnoreArgument_destination()
        .SetReturn(1);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws_client = uws_client_create("test_host", 80, "test_resource/1", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_018: [ If singlylinkedlist_create fails then uws_client_create shall fail and return NULL. ]*/
TEST_FUNCTION(when_creating_the_pending_sends_list_fails_then_uws_client_create_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_CLIENT_HANDLE uws_client;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_resource/1"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(Map_Create(NULL));
    STRICT_EXPECTED_CALL(singlylinkedlist_create())
        .SetReturn(NULL);
    EXPECTED_CALL(Map_Destroy(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws_client = uws_client_create("test_host", 80, "test_resource/1", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_007: [ If obtaining the underlying IO interface fails, then uws_client_create shall fail and return NULL. ]*/
TEST_FUNCTION(when_getting_the_socket_interface_description_fails_then_uws_client_create_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_CLIENT_HANDLE uws_client;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_resource/1"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(Map_Create(NULL));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(socketio_get_interface_description())
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(Map_Destroy(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws_client = uws_client_create("test_host", 80, "test_resource/1", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_016: [ If xio_create fails, then uws_client_create shall fail and return NULL. ]*/
TEST_FUNCTION(when_creating_the_io_handle_fails_then_uws_client_create_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_CLIENT_HANDLE uws_client;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_resource/1"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(Map_Create(NULL));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config))
        .IgnoreArgument_io_create_parameters()
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(Map_Destroy(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws_client = uws_client_create("test_host", 80, "test_resource/1", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_414: [ If allocating memory for the copied protocol information fails then uws_client_create shall fail and return NULL. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_protocols_array_fails_then_uws_client_create_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_CLIENT_HANDLE uws_client;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_resource/1"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(Map_Create(NULL));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config))
        .IgnoreArgument_io_create_parameters();
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(xio_destroy(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(Map_Destroy(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws_client = uws_client_create("test_host", 80, "test_resource/1", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_414: [ If allocating memory for the copied protocol information fails then uws_client_create shall fail and return NULL. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_first_proitocol_name_fails_then_uws_client_create_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_CLIENT_HANDLE uws_client;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_resource/1"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(Map_Create(NULL));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config))
        .IgnoreArgument_io_create_parameters();
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_protocol"))
        .IgnoreArgument_destination()
        .SetReturn(1);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_destroy(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(Map_Destroy(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws_client = uws_client_create("test_host", 80, "test_resource/1", false, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_414: [ If allocating memory for the copied protocol information fails then uws_client_create shall fail and return NULL. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_second_protocol_name_fails_then_uws_client_create_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_CLIENT_HANDLE uws_client;
    static const WS_PROTOCOL two_protocols[] = { { "test_protocol1" }, { "test_protocol2" } };

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_resource/1"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(Map_Create(NULL));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config))
        .IgnoreArgument_io_create_parameters();
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_protocol1"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_protocol2"))
        .IgnoreArgument_destination()
        .SetReturn(1);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_destroy(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(Map_Destroy(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws_client = uws_client_create("test_host", 80, "test_resource/1", false, two_protocols, sizeof(two_protocols) / sizeof(two_protocols[0]));

    // assert
    ASSERT_IS_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_006: [ If use_ssl is true then uws_client_create shall obtain the interface used to create a tlsio instance by calling platform_get_default_tlsio. ]*/
/* Tests_SRS_UWS_CLIENT_01_013: [ The create arguments for the tls IO (when use_ssl is 1) shall have: ]*/
/* Tests_SRS_UWS_CLIENT_01_014: [ - hostname set to the hostname argument passed to uws_client_create. ]*/
/* Tests_SRS_UWS_CLIENT_01_015: [ - port set to the port argument passed to uws_client_create. ]*/
/* Tests_SRS_UWS_CLIENT_01_360: [ Connection confidentiality and integrity is provided by running the WebSocket Protocol over TLS (wss URIs). ]*/
/* Tests_SRS_UWS_CLIENT_01_361: [ WebSocket implementations MUST support TLS and SHOULD employ it when communicating with their peers. ]*/
TEST_FUNCTION(uws_client_create_with_valid_args_ssl_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    SOCKETIO_CONFIG socketio_config;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 443;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 443;
    tlsio_config.underlying_io_interface = TEST_SOCKET_IO_INTERFACE_DESCRIPTION;
    tlsio_config.underlying_io_parameters = &socketio_config;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_resource/23"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(Map_Create(NULL));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(platform_get_default_tlsio());
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(TEST_TLS_IO_INTERFACE_DESCRIPTION, &tlsio_config))
        .IgnoreArgument_io_create_parameters();
    STRICT_EXPECTED_CALL(xio_setoption(IGNORED_PTR_ARG, OPTION_SET_TLS_RENEGOTIATION, IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_protocol"))
        .IgnoreArgument_destination();

    // act
    uws_client = uws_client_create("test_host", 443, "test_resource/23", true, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NOT_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_006: [ If use_ssl is true then uws_client_create shall obtain the interface used to create a tlsio instance by calling platform_get_default_tlsio. ]*/
/* Tests_SRS_UWS_CLIENT_01_013: [ The create arguments for the tls IO (when use_ssl is 1) shall have: ]*/
/* Tests_SRS_UWS_CLIENT_01_014: [ - hostname set to the hostname argument passed to uws_client_create. ]*/
/* Tests_SRS_UWS_CLIENT_01_015: [ - port set to the port argument passed to uws_client_create. ]*/
TEST_FUNCTION(uws_client_create_with_valid_args_ssl_port_different_than_443_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    SOCKETIO_CONFIG socketio_config;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 444;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;
    tlsio_config.underlying_io_interface = TEST_SOCKET_IO_INTERFACE_DESCRIPTION;
    tlsio_config.underlying_io_parameters = &socketio_config;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_resource/23"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(Map_Create(NULL));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(platform_get_default_tlsio());
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(TEST_TLS_IO_INTERFACE_DESCRIPTION, &tlsio_config))
        .IgnoreArgument_io_create_parameters();
    STRICT_EXPECTED_CALL(xio_setoption(IGNORED_PTR_ARG, OPTION_SET_TLS_RENEGOTIATION, IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_protocol"))
        .IgnoreArgument_destination();

    // act
    uws_client = uws_client_create("test_host", 444, "test_resource/23", true, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NOT_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_007: [ If obtaining the underlying IO interface fails, then uws_client_create shall fail and return NULL. ]*/
TEST_FUNCTION(when_getting_the_tlsio_interface_fails_then_uws_client_create_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_resource/23"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(Map_Create(NULL));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(platform_get_default_tlsio())
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(Map_Destroy(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws_client = uws_client_create("test_host", 444, "test_resource/23", true, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* uws_client_create_with_io */

/* Tests_SRS_UWS_CLIENT_01_515: [ uws_client_create_with_io shall create an instance of uws and return a non-NULL handle to it. ]*/
/* Tests_SRS_UWS_CLIENT_01_518: [ The argument hostname shall be copied for later use. ]*/
/* Tests_SRS_UWS_CLIENT_01_520: [ The argument port shall be copied for later use. ]*/
/* Tests_SRS_UWS_CLIENT_01_521: [ The underlying IO shall be created by calling xio_create, while passing as arguments the io_interface and io_create_parameters argument values. ]*/
/* Tests_SRS_UWS_CLIENT_01_523: [ The argument resource_name shall be copied for later use. ]*/
/* Tests_SRS_UWS_CLIENT_01_530: [ uws_client_create_with_io shall create a pending send IO list that is to be used to queue send packets by calling singlylinkedlist_create. ]*/
/* Tests_SRS_UWS_CLIENT_01_527: [ The protocol information indicated by protocols and protocol_count shall be copied for later use (for constructing the upgrade request). ]*/
TEST_FUNCTION(uws_client_create_with_io_valid_args_succeeds)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_CLIENT_HANDLE uws_client;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "my_horrible_host";
    socketio_config.port = 1122;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "111"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(Map_Create(NULL));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config))
        .IgnoreArgument_io_create_parameters();
    STRICT_EXPECTED_CALL(xio_setoption(IGNORED_PTR_ARG, OPTION_SET_TLS_RENEGOTIATION, IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_protocol"))
        .IgnoreArgument_destination();

    // act
    uws_client = uws_client_create_with_io(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config, "test_host", 80, "111", protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NOT_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_516: [ If any of the arguments io_interface, hostname and resource_name is NULL then uws_client_create_with_io shall return NULL. ]*/
TEST_FUNCTION(uws_client_create_with_io_with_NULL_io_interface_description_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_CLIENT_HANDLE uws_client;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "my_horrible_host";
    socketio_config.port = 1122;

    // act
    uws_client = uws_client_create_with_io(NULL, &socketio_config, "test_host", 80, "111", protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_516: [ If any of the arguments io_interface, hostname and resource_name is NULL then uws_client_create_with_io shall return NULL. ]*/
TEST_FUNCTION(uws_client_create_with_io_with_NULL_hostname_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_CLIENT_HANDLE uws_client;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "my_horrible_host";
    socketio_config.port = 1122;

    // act
    uws_client = uws_client_create_with_io(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config, NULL, 80, "111", protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_516: [ If any of the arguments io_interface, hostname and resource_name is NULL then uws_client_create_with_io shall return NULL. ]*/
TEST_FUNCTION(uws_client_create_with_io_with_NULL_resource_name_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_CLIENT_HANDLE uws_client;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "my_horrible_host";
    socketio_config.port = 1122;

    // act
    uws_client = uws_client_create_with_io(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config, "test_host", 80, NULL, protocols, sizeof(protocols) / sizeof(protocols[0]));

    // assert
    ASSERT_IS_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_517: [ If allocating memory for the new uws instance fails then uws_client_create_with_io shall return NULL. ]*/
/* Tests_SRS_UWS_CLIENT_01_519: [ If allocating memory for the copy of the hostname argument fails, then uws_client_create shall return NULL. ]*/
/* Tests_SRS_UWS_CLIENT_01_522: [ If xio_create fails, then uws_client_create_with_io shall fail and return NULL. ]*/
/* Tests_SRS_UWS_CLIENT_01_529: [ If allocating memory for the copy of the resource_name argument fails, then uws_client_create_with_io shall return NULL. ]*/
/* Tests_SRS_UWS_CLIENT_01_531: [ If singlylinkedlist_create fails then uws_client_create_with_io shall fail and return NULL. ]*/
/* Tests_SRS_UWS_CLIENT_01_528: [ If allocating memory for the copied protocol information fails then uws_client_create_with_io shall fail and return NULL. ]*/
TEST_FUNCTION(when_any_call_fails_uws_client_create_with_io_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_CLIENT_HANDLE uws_client;
    size_t i;
    static const WS_PROTOCOL two_protocols[] = { { "test_protocol1" },{ "test_protocol2" } };

    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "my_horrible_host";
    socketio_config.port = 1122;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination()
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "111"))
        .IgnoreArgument_destination()
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(Map_Create(NULL));
    STRICT_EXPECTED_CALL(singlylinkedlist_create())
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config))
        .IgnoreArgument_io_create_parameters()
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(xio_setoption(IGNORED_PTR_ARG, OPTION_SET_TLS_RENEGOTIATION, IGNORED_PTR_ARG)).CallCannotFail();
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_protocol1"))
        .IgnoreArgument_destination()
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_protocol2"))
        .IgnoreArgument_destination()
        .SetFailReturn(1);

    umock_c_negative_tests_snapshot();

    for (i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        if (umock_c_negative_tests_can_call_fail(i))
        {
            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            // act
            uws_client = uws_client_create_with_io(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config, "test_host", 80, "111", two_protocols, sizeof(two_protocols) / sizeof(two_protocols[0]));

            // assert
            ASSERT_IS_NULL(uws_client, "On failed call %lu", (unsigned long)i);
        }
    }
}

/* Tests_SRS_UWS_CLIENT_01_524: [ The protocols argument shall be allowed to be NULL, in which case no protocol is to be specified by the client in the upgrade request. ]*/
TEST_FUNCTION(uws_client_create_with_io_with_NULL_protocols_succeeds)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_CLIENT_HANDLE uws_client;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "my_horrible_host";
    socketio_config.port = 1122;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "111"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(Map_Create(NULL));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config))
        .IgnoreArgument_io_create_parameters();
    STRICT_EXPECTED_CALL(xio_setoption(IGNORED_PTR_ARG, OPTION_SET_TLS_RENEGOTIATION, IGNORED_PTR_ARG));

    // act
    uws_client = uws_client_create_with_io(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config, "test_host", 80, "111", NULL, 0);

    // assert
    ASSERT_IS_NOT_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_525: [ If protocol_count is non zero and protocols is NULL then uws_client_create_with_io shall fail and return NULL. ]*/
TEST_FUNCTION(uws_client_create_with_io_with_NULL_protocols_and_non_zero_protocol_count_fails)
{
    // arrange
    SOCKETIO_CONFIG socketio_config;
    UWS_CLIENT_HANDLE uws_client;

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 80;

    // act
    uws_client = uws_client_create_with_io(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config, "test_host", 80, "111", NULL, 1);

    // assert
    ASSERT_IS_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_526: [ If the protocol member of any of the items in the protocols argument is NULL, then uws_client_create_with_io shall fail and return NULL. ]*/
TEST_FUNCTION(uws_client_create_with_io_with_a_NULL_protocol_name_for_first_protocol_fails)
{
    // arrange
    UWS_CLIENT_HANDLE uws_client;
    SOCKETIO_CONFIG socketio_config;
    WS_PROTOCOL NULL_test_protocol[] = { { NULL } };

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 444;

    // act
    uws_client = uws_client_create_with_io(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config, "test_host", 80, "test_resource/23", NULL_test_protocol, sizeof(NULL_test_protocol) / sizeof(NULL_test_protocol[0]));

    // assert
    ASSERT_IS_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_526: [ If the protocol member of any of the items in the protocols argument is NULL, then uws_client_create_with_io shall fail and return NULL. ]*/
TEST_FUNCTION(uws_client_create_with_io_with_a_NULL_protocol_name_for_second_protocol_fails)
{
    // arrange
    UWS_CLIENT_HANDLE uws_client;
    SOCKETIO_CONFIG socketio_config;
    WS_PROTOCOL NULL_test_protocol[] = { { "a" },  { NULL } };

    socketio_config.accepted_socket = NULL;
    socketio_config.hostname = "test_host";
    socketio_config.port = 444;

    // act
    uws_client = uws_client_create_with_io(TEST_SOCKET_IO_INTERFACE_DESCRIPTION, &socketio_config, "test_host", 80, "test_resource/23", NULL_test_protocol, sizeof(NULL_test_protocol) / sizeof(NULL_test_protocol[0]));

    // assert
    ASSERT_IS_NULL(uws_client);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* uws_client_destroy */

/* Tests_SRS_UWS_CLIENT_01_019: [ uws_client_destroy shall free all resources associated with the uws instance. ]*/
/* Tests_SRS_UWS_CLIENT_01_023: [ uws_client_destroy shall ensure the underlying IO created in uws_client_open_async is destroyed by calling xio_destroy. ]*/
/* Tests_SRS_UWS_CLIENT_01_024: [ uws_client_destroy shall free the list used to track the pending sends by calling singlylinkedlist_destroy. ]*/
/* Tests_SRS_UWS_CLIENT_01_424: [ uws_client_destroy shall free the buffer allocated in uws_client_create by calling BUFFER_delete. ]*/
/* Tests_SRS_UWS_CLIENT_01_437: [ uws_client_destroy shall free the protocols array allocated in uws_client_create. ]*/
TEST_FUNCTION(uws_client_destroy_fress_the_resources)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_destroy(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(Map_Destroy(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws_client_destroy(uws_client);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_437: [ uws_client_destroy shall free the protocols array allocated in uws_client_create. ]*/
TEST_FUNCTION(uws_client_destroy_with_2_protocols_fress_both_protocols)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    static const WS_PROTOCOL two_protocols[] = { { "test_protocol1" },{ "test_protocol2" } };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "aaa", true, two_protocols, sizeof(two_protocols) / sizeof(two_protocols[0]));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_destroy(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(Map_Destroy(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws_client_destroy(uws_client);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_437: [ uws_client_destroy shall free the protocols array allocated in uws_client_create. ]*/
TEST_FUNCTION(uws_client_destroy_with_no_protocols_frees_all_other_resources)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "aaa", true, NULL, 0);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_destroy(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(Map_Destroy(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws_client_destroy(uws_client);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_020: [ If uws_client is NULL, uws_client_destroy shall do nothing. ]*/
TEST_FUNCTION(uws_client_destroy_with_NULL_does_nothing)
{
    // arrange

    // act
    uws_client_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_021: [ uws_client_destroy shall perform a close action if the uws instance has already been open. ]*/
/* Tests_SRS_UWS_CLIENT_01_034: [ uws_client_close_async shall obtain all the pending send frames by repetitively querying for the head of the pending IO list and freeing that head item. ]*/
/* Tests_SRS_UWS_CLIENT_01_035: [ Obtaining the head of the pending send frames list shall be done by calling singlylinkedlist_get_head_item. ]*/
TEST_FUNCTION(uws_client_destroy_also_performs_a_close)
{
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_destroy(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(Map_Destroy(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    uws_client_destroy(uws_client);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* uws_client_open_async */

/* Tests_SRS_UWS_CLIENT_01_025: [ uws_client_open_async shall open the underlying IO by calling xio_open and providing the IO handle created in uws_client_create as argument. ]*/
/* Tests_SRS_UWS_CLIENT_01_367: [ The callbacks on_underlying_io_open_complete, on_underlying_io_bytes_received and on_underlying_io_error shall be passed as arguments to xio_open. ]*/
/* Tests_SRS_UWS_CLIENT_01_026: [ On success, uws_client_open_async shall return 0. ]*/
/* Tests_SRS_UWS_CLIENT_01_061: [ To _Establish a WebSocket Connection_, a client opens a connection and sends a handshake as defined in this section. ]*/
TEST_FUNCTION(uws_client_open_async_opens_the_underlying_IO)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_027: [ If uws_client, on_ws_open_complete, on_ws_frame_received, on_ws_peer_closed or on_ws_error is NULL, uws_client_open_async shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_client_open_async_with_NULL_handle_fails)
{
    // arrange
    int result;

    // act
    result = uws_client_open_async(NULL, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_027: [ If uws_client, on_ws_open_complete, on_ws_frame_received, on_ws_peer_closed or on_ws_error is NULL, uws_client_open_async shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_client_open_async_with_NULL_on_ws_open_complete_callback_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    // act
    result = uws_client_open_async(uws_client, NULL, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_027: [ If uws_client, on_ws_open_complete, on_ws_frame_received, on_ws_peer_closed or on_ws_error is NULL, uws_client_open_async shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_client_open_async_with_NULL_on_ws_frame_received_callback_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, NULL, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_027: [ If uws_client, on_ws_open_complete, on_ws_frame_received, on_ws_peer_closed or on_ws_error is NULL, uws_client_open_async shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_client_open_async_with_NULL_on_ws_peer_closed_callback_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, NULL, (void*)0x4301, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_027: [ If uws_client, on_ws_open_complete, on_ws_frame_received, on_ws_peer_closed or on_ws_error is NULL, uws_client_open_async shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_client_open_async_with_NULL_on_ws_error_callback_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, NULL, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_393: [ The context arguments for the callbacks shall be allowed to be NULL. ]*/
TEST_FUNCTION(uws_client_open_async_with_NULL_on_ws_open_complete_context_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, NULL, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_393: [ The context arguments for the callbacks shall be allowed to be NULL. ]*/
TEST_FUNCTION(uws_client_open_async_with_NULL_on_ws_frame_received_context_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, NULL, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_393: [ The context arguments for the callbacks shall be allowed to be NULL. ]*/
TEST_FUNCTION(uws_client_open_async_with_NULL_on_ws_peer_closed_context_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, NULL, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_393: [ The context arguments for the callbacks shall be allowed to be NULL. ]*/
TEST_FUNCTION(uws_client_open_async_with_NULL_on_ws_error_context_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_028: [ If opening the underlying IO fails then uws_client_open_async shall fail and return a non-zero value. ]*/
/* Tests_SRS_UWS_CLIENT_01_075: [ If the connection could not be opened, either because a direct connection failed or because any proxy used returned an error, then the client MUST _Fail the WebSocket Connection_ and abort the connection attempt. ]*/
TEST_FUNCTION(when_opening_the_underlying_io_fails_uws_client_open_async_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context()
        .SetReturn(1);

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_394: [ uws_client_open_async while the uws instance is already OPEN or OPENING shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_client_open_async_after_uws_client_open_async_without_a_close_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_400: [ uws_client_open_async while CLOSING shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_client_open_async_while_closing_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    (void)uws_client_close_async(uws_client, test_on_ws_close_complete, NULL);
    umock_c_reset_all_calls();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_400: [ uws_client_open_async while CLOSING shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_client_open_async_while_waiting_for_CLOSE_frame_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    (void)uws_client_close_handshake_async(uws_client, 1002, "", test_on_ws_close_complete, NULL);
    umock_c_reset_all_calls();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* uws_client_close_async */

/* Tests_SRS_UWS_CLIENT_01_029: [ uws_client_close_async shall close the uws instance connection if an open action is either pending or has completed successfully (if the IO is open). ]*/
/* Tests_SRS_UWS_CLIENT_01_031: [ uws_client_close_async shall close the connection by calling xio_close while passing as argument the IO handle created in uws_client_create. ]*/
/* Tests_SRS_UWS_CLIENT_01_368: [ The callback on_underlying_io_close shall be passed as argument to xio_close. ]*/
/* Tests_SRS_UWS_CLIENT_01_396: [ On success uws_client_close_async shall return 0. ]*/
/* Tests_SRS_UWS_CLIENT_01_399: [ on_ws_close_complete and on_ws_close_complete_context shall be saved and the callback on_ws_close_complete shall be triggered when the close is complete. ]*/
/* Tests_SRS_UWS_CLIENT_01_317: [ Clients SHOULD NOT close the WebSocket connection arbitrarily. ]*/
TEST_FUNCTION(uws_client_close_async_closes_the_underlying_IO)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_close_complete()
        .IgnoreArgument_callback_context();
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));

    // act
    result = uws_client_close_async(uws_client, test_on_ws_close_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_030: [ if uws_client is NULL, uws_client_close_async shall return a non-zero value. ]*/
TEST_FUNCTION(uws_client_close_async_with_NULL_handle_fails)
{
    // arrange
    int result;

    // act
    result = uws_client_close_async(NULL, test_on_ws_close_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_397: [ The on_ws_close_complete argument shall be allowed to be NULL, in which case no callback shall be called when the close is complete. ]*/
TEST_FUNCTION(uws_client_close_async_with_NULL_close_complete_callback_is_allowed)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_close_complete()
        .IgnoreArgument_callback_context();
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));

    // act
    result = uws_client_close_async(uws_client, NULL, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_398: [ on_ws_close_complete_context shall also be allows to be NULL. ]*/
TEST_FUNCTION(uws_client_close_async_with_NULL_close_context_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_close_complete()
        .IgnoreArgument_callback_context();
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));

    // act
    result = uws_client_close_async(uws_client, test_on_ws_close_complete, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_395: [ If xio_close fails, uws_client_close_async shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_the_underlying_xio_close_fails_then_uws_client_close_async_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_close_complete()
        .IgnoreArgument_callback_context()
        .SetReturn(1);

    // act
    result = uws_client_close_async(uws_client, test_on_ws_close_complete, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_032: [ uws_client_close_async when no open action has been issued shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_client_close_async_without_open_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    // act
    result = uws_client_close_async(uws_client, test_on_ws_close_complete, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_033: [ uws_client_close_async after a uws_client_close_async shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_client_close_async_while_closing_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    (void)uws_client_close_async(uws_client, test_on_ws_close_complete, NULL);
    umock_c_reset_all_calls();

    // act
    result = uws_client_close_async(uws_client, test_on_ws_close_complete, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_033: [ uws_client_close_async after a uws_client_close_async shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_client_close_async_while_WAITING_for_close_frame_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    (void)uws_client_close_handshake_async(uws_client, 1002, "", NULL, NULL);
    umock_c_reset_all_calls();

    // act
    result = uws_client_close_async(uws_client, test_on_ws_close_complete, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_033: [ uws_client_close_async after a uws_client_close_async shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_client_close_async_after_close_complete_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    (void)uws_client_close_async(uws_client, test_on_ws_close_complete, NULL);
    g_on_io_close_complete(g_on_io_close_complete_context);
    umock_c_reset_all_calls();

    // act
    result = uws_client_close_async(uws_client, test_on_ws_close_complete, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_034: [ uws_client_close_async shall obtain all the pending send frames by repetitively querying for the head of the pending IO list and freeing that head item. ]*/
/* Tests_SRS_UWS_CLIENT_01_035: [ Obtaining the head of the pending send frames list shall be done by calling singlylinkedlist_get_head_item. ]*/
/* Tests_SRS_UWS_CLIENT_01_036: [ For each pending send frame the send complete callback shall be called with UWS_SEND_FRAME_CANCELLED. ]*/
/* Tests_SRS_UWS_CLIENT_01_037: [ When indicating pending send frames as cancelled the callback context passed to the on_ws_send_frame_complete callback shall be the context given to uws_client_send_frame_async. ]*/
TEST_FUNCTION(uws_client_close_async_with_1_pending_send_frames_indicates_the_frames_as_cancelled)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    LIST_ITEM_HANDLE list_item;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    (void)uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, NULL, 0, true, test_on_ws_send_frame_complete, (void*)0x4248);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_close_complete()
        .IgnoreArgument_callback_context();
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE))
        .CaptureReturn(&list_item);
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .ValidateArgumentValue_item_handle(&list_item);
    STRICT_EXPECTED_CALL(test_on_ws_send_frame_complete((void*)0x4248, WS_SEND_FRAME_CANCELLED));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));

    // act
    result = uws_client_close_async(uws_client, test_on_ws_close_complete, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_034: [ uws_client_close_async shall obtain all the pending send frames by repetitively querying for the head of the pending IO list and freeing that head item. ]*/
/* Tests_SRS_UWS_CLIENT_01_035: [ Obtaining the head of the pending send frames list shall be done by calling singlylinkedlist_get_head_item. ]*/
/* Tests_SRS_UWS_CLIENT_01_036: [ For each pending send frame the send complete callback shall be called with UWS_SEND_FRAME_CANCELLED. ]*/
/* Tests_SRS_UWS_CLIENT_01_037: [ When indicating pending send frames as cancelled the callback context passed to the on_ws_send_frame_complete callback shall be the context given to uws_client_send_frame_async. ]*/
TEST_FUNCTION(uws_client_close_async_with_2_pending_send_frames_indicates_the_frames_as_cancelled)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char test_frame_1[] = { 0x42 };
    const unsigned char test_frame_2[] = { 0x43, 0x44 };
    LIST_ITEM_HANDLE list_item_1;
    LIST_ITEM_HANDLE list_item_2;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    (void)uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, test_frame_1, sizeof(test_frame_2), true, test_on_ws_send_frame_complete, (void*)0x4248);
    (void)uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_TEXT, test_frame_2, sizeof(test_frame_2), true, test_on_ws_send_frame_complete, (void*)0x4249);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_close_complete()
        .IgnoreArgument_callback_context();
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE))
        .CaptureReturn(&list_item_1);
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .ValidateArgumentValue_item_handle(&list_item_1);
    STRICT_EXPECTED_CALL(test_on_ws_send_frame_complete((void*)0x4248, WS_SEND_FRAME_CANCELLED));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE))
        .CaptureReturn(&list_item_2);
    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .ValidateArgumentValue_item_handle(&list_item_2);
    STRICT_EXPECTED_CALL(test_on_ws_send_frame_complete((void*)0x4249, WS_SEND_FRAME_CANCELLED));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));

    // act
    result = uws_client_close_async(uws_client, test_on_ws_close_complete, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* uws_client_close_handshake_async */

/* Tests_SRS_UWS_CLIENT_01_465: [ uws_client_close_handshake_async shall initiate the close handshake by sending a close frame to the peer. ]*/
/* Tests_SRS_UWS_CLIENT_01_466: [ On success uws_client_close_handshake_async shall return 0. ]*/
/* Tests_SRS_UWS_CLIENT_01_468: [ on_ws_close_complete and on_ws_close_complete_context shall be saved and the callback on_ws_close_complete shall be triggered when the close is complete. ]*/
/* Tests_SRS_UWS_CLIENT_01_471: [ The callback on_underlying_io_close_sent shall be passed as argument to xio_send. ]*/
TEST_FUNCTION(uws_client_close_handshake_async_sends_the_close_frame)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char close_frame_payload[] = { 0x03, 0xEA };
    unsigned char close_frame[] = { 0x88, 0x82, 0x00, 0x00, 0x00, 0x00, 0x03, 0xEA };
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_CLOSE_FRAME, IGNORED_PTR_ARG, sizeof(close_frame_payload), true, true, 0))
        .ValidateArgumentBuffer(2, close_frame_payload, sizeof(close_frame_payload))
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(close_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(close_frame));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, close_frame, sizeof(close_frame), IGNORED_PTR_ARG, NULL))
        .ValidateArgumentBuffer(2, close_frame, sizeof(close_frame));
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));

    // act
    result = uws_client_close_handshake_async(uws_client, 1002, "", test_on_ws_close_complete, (void*)0x4445);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_467: [ if uws_client is NULL, uws_client_close_handshake_async shall return a non-zero value. ]*/
TEST_FUNCTION(uws_client_close_handshake_async_with_NULL_handle_fails)
{
    // arrange
    int result;

    // act
    result = uws_client_close_handshake_async(NULL, 1002, "", test_on_ws_close_complete, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_469: [ The on_ws_close_complete argument shall be allowed to be NULL, in which case no callback shall be called when the close is complete. ]*/
/* Tests_SRS_UWS_CLIENT_01_471: [ The callback on_underlying_io_close_sent shall be passed as argument to xio_send. ]*/
TEST_FUNCTION(uws_client_close_handshake_async_with_NULL_close_complete_callback_is_allowed)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char close_frame_payload[] = { 0x03, 0xEA };
    unsigned char close_frame[] = { 0x88, 0x82, 0x00, 0x00, 0x00, 0x00, 0x03, 0xEA };
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;
    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_CLOSE_FRAME, IGNORED_PTR_ARG, sizeof(close_frame_payload), true, true, 0))
        .ValidateArgumentBuffer(2, close_frame_payload, sizeof(close_frame_payload))
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(close_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(close_frame));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, close_frame, sizeof(close_frame), IGNORED_PTR_ARG, NULL))
        .ValidateArgumentBuffer(2, close_frame, sizeof(close_frame));
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));

    // act
    result = uws_client_close_handshake_async(uws_client, 1002, "", NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_470: [ on_ws_close_complete_context shall also be allowed to be NULL. ]*/
TEST_FUNCTION(uws_client_close_handshake_async_with_NULL_context_is_allowed)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char close_frame_payload[] = { 0x03, 0xEA };
    unsigned char close_frame[] = { 0x88, 0x82, 0x00, 0x00, 0x00, 0x00, 0x03, 0xEA };
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_CLOSE_FRAME, IGNORED_PTR_ARG, sizeof(close_frame_payload), true, true, 0))
        .ValidateArgumentBuffer(2, close_frame_payload, sizeof(close_frame_payload))
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(close_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(close_frame));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, close_frame, sizeof(close_frame), IGNORED_PTR_ARG, NULL))
        .ValidateArgumentBuffer(2, close_frame, sizeof(close_frame));
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE));

    // act
    result = uws_client_close_handshake_async(uws_client, 1002, "", test_on_ws_close_complete, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_472: [ If xio_send fails, uws_client_close_handshake_async shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_xio_send_fails_uws_client_close_handshake_async_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char close_frame_payload[] = { 0x03, 0xEA };
    unsigned char close_frame[] = { 0x88, 0x82, 0x00, 0x00, 0x00, 0x00, 0x03, 0xEA };
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_CLOSE_FRAME, IGNORED_PTR_ARG, sizeof(close_frame_payload), true, true, 0))
        .ValidateArgumentBuffer(2, close_frame_payload, sizeof(close_frame_payload))
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(close_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(close_frame));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, close_frame, sizeof(close_frame), IGNORED_PTR_ARG, NULL))
        .ValidateArgumentBuffer(2, close_frame, sizeof(close_frame))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);

    // act
    result = uws_client_close_handshake_async(uws_client, 1002, "", test_on_ws_close_complete, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_473: [ uws_client_close_handshake_async when no open action has been issued shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_client_close_handshake_async_when_not_opened_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    // act
    result = uws_client_close_handshake_async(uws_client, 1002, "", test_on_ws_close_complete, (void*)0x4445);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_474: [ uws_client_close_handshake_async when already CLOSING shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_client_close_handshake_async_when_already_SENDING_CLOSE_frame_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char close_frame[] = { 0x88, 0x02, 0x03, 0xEA };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));
    umock_c_reset_all_calls();

    // act
    result = uws_client_close_handshake_async(uws_client, 1002, "", test_on_ws_close_complete, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_474: [ uws_client_close_handshake_async when already CLOSING shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_client_close_handshake_async_when_already_CLOSING_underlying_IO_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char close_frame[] = { 0x88, 0x02, 0x03, 0xEA };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));
    g_on_io_send_complete(g_on_io_send_complete_context, IO_SEND_OK);
    umock_c_reset_all_calls();

    // act
    result = uws_client_close_handshake_async(uws_client, 1002, "", test_on_ws_close_complete, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_033: [ uws_client_close_async after a uws_client_close_async shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_client_close_handshake_async_while_WAITING_for_close_frame_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    int result;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    (void)uws_client_close_handshake_async(uws_client, 1002, "", NULL, NULL);
    umock_c_reset_all_calls();

    // act
    result = uws_client_close_handshake_async(uws_client, 1002, "", NULL, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* on_underlying_io_open_complete */

/* Tests_SRS_UWS_CLIENT_01_369: [ When on_underlying_io_open_complete is called with IO_OPEN_ERROR while uws is OPENING (uws_client_open_async was called), uws shall report that the open failed by calling the on_ws_open_complete callback passed to uws_client_open_async with WS_OPEN_ERROR_UNDERLYING_IO_OPEN_FAILED. ]*/
TEST_FUNCTION(on_underlying_io_open_complete_with_ERROR_triggers_the_ws_open_complete_callback_with_WS_OPEN_ERROR_UNDERLYING_IO_OPEN_FAILED)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_UNDERLYING_IO_OPEN_FAILED));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_ERROR);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_409: [ After any error is indicated by on_ws_open_complete, a subsequent uws_client_open_async shall be possible. ]*/
TEST_FUNCTION(uws_client_open_async_after_WS_OPEN_ERROR_UNDERLYING_IO_OPEN_FAILED_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_ERROR);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);;

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_401: [ If on_underlying_io_open_complete is called with a NULL context, on_underlying_io_open_complete shall do nothing. ]*/
TEST_FUNCTION(on_underlying_io_open_complete_with_NULL_context_does_nothing)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    g_on_io_open_complete(NULL, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_402: [ When on_underlying_io_open_complete is called with IO_OPEN_CANCELLED while uws is OPENING (uws_client_open_async was called), uws shall report that the open failed by calling the on_ws_open_complete callback passed to uws_client_open_async with WS_OPEN_ERROR_UNDERLYING_IO_OPEN_CANCELLED. ]*/
TEST_FUNCTION(on_underlying_io_open_complete_with_CANCELLED_triggers_the_ws_open_complete_callback_with_WS_OPEN_ERROR_UNDERLYING_IO_OPEN_CANCELLED)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_UNDERLYING_IO_OPEN_CANCELLED));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_CANCELLED);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_409: [ After any error is indicated by on_ws_open_complete, a subsequent uws_client_open_async shall be possible. ]*/
TEST_FUNCTION(uws_client_open_async_after_WS_OPEN_ERROR_UNDERLYING_IO_OPEN_CANCELLED_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_CANCELLED);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);;

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_371: [ When on_underlying_io_open_complete is called with IO_OPEN_OK while uws is OPENING (uws_client_open_async was called), uws shall prepare the WebSockets upgrade request. ]*/
/* Tests_SRS_UWS_CLIENT_01_372: [ Once prepared the WebSocket upgrade request shall be sent by calling xio_send. ]*/
/* Tests_SRS_UWS_CLIENT_01_080: [ Once a connection to the server has been established (including a connection via a proxy or over a TLS-encrypted tunnel), the client MUST send an opening handshake to the server. ]*/
/* Tests_SRS_UWS_CLIENT_01_081: [ The handshake consists of an HTTP Upgrade request, along with a list of required and optional header fields. ]*/
/* Tests_SRS_UWS_CLIENT_01_082: [ The handshake MUST be a valid HTTP request as specified by [RFC2616]. ]*/
/* Tests_SRS_UWS_CLIENT_01_083: [ The method of the request MUST be GET, and the HTTP version MUST be at least 1.1. ]*/
/* Tests_SRS_UWS_CLIENT_01_084: [ The "Request-URI" part of the request MUST match the /resource name/ defined in Section 3 (a relative URI) or be an absolute http/https URI that, when parsed, has a /resource name/, /host/, and /port/ that match the corresponding ws/wss URI. ]*/
/* Tests_SRS_UWS_CLIENT_01_085: [ The request MUST contain a |Host| header field whose value contains /host/ plus optionally ":" followed by /port/ (when not using the default port). ]*/
/* Tests_SRS_UWS_CLIENT_01_086: [ The request MUST contain an |Upgrade| header field whose value MUST include the "websocket" keyword. ]*/
/* Tests_SRS_UWS_CLIENT_01_087: [ The request MUST contain a |Connection| header field whose value MUST include the "Upgrade" token. ]*/
/* Tests_SRS_UWS_CLIENT_01_088: [ The request MUST include a header field with the name |Sec-WebSocket-Key|. ]*/
/* Tests_SRS_UWS_CLIENT_01_094: [ The request MUST include a header field with the name |Sec-WebSocket-Version|. ]*/
/* Tests_SRS_UWS_CLIENT_01_095: [ The value of this header field MUST be 13. ]*/
/* Tests_SRS_UWS_CLIENT_01_096: [ The request MAY include a header field with the name |Sec-WebSocket-Protocol|. ]*/
/* Tests_SRS_UWS_CLIENT_01_100: [ The request MAY include a header field with the name |Sec-WebSocket-Extensions|. ]*/
/* Tests_SRS_UWS_CLIENT_01_101: [ The request MAY include any other header fields, for example, cookies [RFC6265] and/or authentication-related header fields such as the |Authorization| header field [RFC2616], which are processed according to documents that define them. ] */
/* Tests_SRS_UWS_CLIENT_01_089: [ The value of this header field MUST be a nonce consisting of a randomly selected 16-byte value that has been base64-encoded (see Section 4 of [RFC4648]). ]*/
/* Tests_SRS_UWS_CLIENT_01_090: [ The nonce MUST be selected randomly for each connection. ]*/
/* Tests_SRS_UWS_CLIENT_01_497: [ The nonce needed for the upgrade request shall be Base64 encoded with Azure_Base64_Encode_Bytes. ]*/
TEST_FUNCTION(on_underlying_io_open_complete_with_OK_prepares_and_sends_the_WebSocket_upgrade_request)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    size_t i;
    unsigned char expected_nonce[16];
    char* req_header1_key = "Authorization";
    char* req_header1_value = "Bearer 23420939909809283488230949";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    my_Map_GetInternals_keys[0] = req_header1_key;
    my_Map_GetInternals_values[0] = req_header1_value;
    my_Map_GetInternals_count = 1;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(Map_AddOrUpdate(TEST_REQUEST_HEADERS_MAP, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    (void)uws_client_set_request_header(uws_client, req_header1_key, req_header1_value);

    umock_c_reset_all_calls();

    /* get the random 16 bytes */
    for (i = 0; i < 16; i++)
    {
        EXPECTED_CALL(gb_rand()).SetReturn((int)i);
        expected_nonce[i] = (unsigned char)i;
    }

    STRICT_EXPECTED_CALL(Azure_Base64_Encode_Bytes(IGNORED_PTR_ARG, 16))
        .ValidateArgumentBuffer(1, expected_nonce, 16);
    // get_request_headers()
    STRICT_EXPECTED_CALL(Map_GetInternals(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(strlen(req_header1_key)+strlen(req_header1_value)+2+2+1));

    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)).SetReturn("ZWRuYW1vZGU6bm9jYXBlcyE=");
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG));
    EXPECTED_CALL(free(IGNORED_PTR_ARG)); // request headers

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_498: [ If Base64 encoding the nonce for the upgrade request fails, then the uws client shall report that the open failed by calling the on_ws_open_complete callback passed to uws_client_open_async with WS_OPEN_ERROR_BASE64_ENCODE_FAILED. ]*/
TEST_FUNCTION(when_base64_encode_fails_on_underlying_io_open_complete_triggers_the_error_WS_OPEN_ERROR_BASE64_ENCODE_FAILED)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    size_t i;
    unsigned char expected_nonce[16];

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    /* get the random 16 bytes */
    for (i = 0; i < 16; i++)
    {
        EXPECTED_CALL(gb_rand()).SetReturn((int)i);
        expected_nonce[i] = (unsigned char)i;
    }

    STRICT_EXPECTED_CALL(Azure_Base64_Encode_Bytes(IGNORED_PTR_ARG, 16))
        .ValidateArgumentBuffer(1, expected_nonce, 16)
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_BASE64_ENCODE_FAILED));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_406: [ If not enough memory can be allocated to construct the WebSocket upgrade request, uws shall report that the open failed by calling the on_ws_open_complete callback passed to uws_client_open_async with WS_OPEN_ERROR_NOT_ENOUGH_MEMORY. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_websocket_upgrade_request_fails_the_error_WS_OPEN_ERROR_NOT_ENOUGH_MEMORY_is_indicated_via_the_open_complete_callback)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    size_t i;
    unsigned char expected_nonce[16];

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    /* get the random 16 bytes */
    for (i = 0; i < 16; i++)
    {
        EXPECTED_CALL(gb_rand()).SetReturn((int)i);
        expected_nonce[i] = (unsigned char)i;
    }

    STRICT_EXPECTED_CALL(Azure_Base64_Encode_Bytes(IGNORED_PTR_ARG, 16))
        .ValidateArgumentBuffer(1, expected_nonce, 16);
    // get_request_headers()
    STRICT_EXPECTED_CALL(Map_GetInternals(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));

    STRICT_EXPECTED_CALL(STRING_c_str(BASE64_ENCODED_STRING)).SetReturn("ZWRuYW1vZGU6bm9jYXBlcyE=");
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_NOT_ENOUGH_MEMORY));
    STRICT_EXPECTED_CALL(STRING_delete(BASE64_ENCODED_STRING));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // empty request headers

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_409: [ After any error is indicated by on_ws_open_complete, a subsequent uws_client_open_async shall be possible. ]*/
TEST_FUNCTION(uws_client_open_async_after_WS_OPEN_ERROR_NOT_ENOUGH_MEMORY_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    int result;
    size_t i;
    unsigned char expected_nonce[16];

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    /* get the random 16 bytes */
    for (i = 0; i < 16; i++)
    {
        EXPECTED_CALL(gb_rand()).SetReturn((int)i);
        expected_nonce[i] = (unsigned char)i;
    }

    STRICT_EXPECTED_CALL(Azure_Base64_Encode_Bytes(IGNORED_PTR_ARG, 16))
        .ValidateArgumentBuffer(1, expected_nonce, 16);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_NOT_ENOUGH_MEMORY));

    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);;

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_373: [ If xio_send fails then uws shall report that the open failed by calling the on_ws_open_complete callback passed to uws_client_open_async with WS_OPEN_ERROR_CANNOT_SEND_UPGRADE_REQUEST. ]*/
TEST_FUNCTION(when_sending_the_upgrade_request_fails_the_error_WS_OPEN_ERROR_CANNOT_SEND_UPGRADE_REQUEST_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    size_t i;
    unsigned char expected_nonce[16];

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    /* get the random 16 bytes */
    for (i = 0; i < 16; i++)
    {
        EXPECTED_CALL(gb_rand()).SetReturn((int)i);
        expected_nonce[i] = (unsigned char)i;
    }

    STRICT_EXPECTED_CALL(Azure_Base64_Encode_Bytes(IGNORED_PTR_ARG, 16))
        .ValidateArgumentBuffer(1, expected_nonce, 16);
    STRICT_EXPECTED_CALL(Map_GetInternals(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG)); // get_request_headers(), no headers
    STRICT_EXPECTED_CALL(STRING_c_str(BASE64_ENCODED_STRING)).SetReturn("ZWRuYW1vZGU6bm9jYXBlcyE=");
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_send_complete()
        .IgnoreArgument_callback_context()
        .IgnoreArgument_buffer()
        .IgnoreArgument_size()
        .SetReturn(1);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_CANNOT_SEND_UPGRADE_REQUEST));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_delete(BASE64_ENCODED_STRING));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_409: [ After any error is indicated by on_ws_open_complete, a subsequent uws_client_open_async shall be possible. ]*/
TEST_FUNCTION(uws_client_open_async_after_WS_OPEN_ERROR_CANNOT_SEND_UPGRADE_REQUEST_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    int result;
    size_t i;
    unsigned char expected_nonce[16];

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    /* get the random 16 bytes */
    for (i = 0; i < 16; i++)
    {
        EXPECTED_CALL(gb_rand()).SetReturn((int)i);
        expected_nonce[i] = (unsigned char)i;
    }

    STRICT_EXPECTED_CALL(Azure_Base64_Encode_Bytes(IGNORED_PTR_ARG, 16))
        .ValidateArgumentBuffer(1, expected_nonce, 16);
    // get_request_headers()
    STRICT_EXPECTED_CALL(Map_GetInternals(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));

    STRICT_EXPECTED_CALL(STRING_c_str(BASE64_ENCODED_STRING)).SetReturn("ZWRuYW1vZGU6bm9jYXBlcyE=");
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_send_complete()
        .IgnoreArgument_callback_context()
        .IgnoreArgument_buffer()
        .IgnoreArgument_size()
        .SetReturn(1);

    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);;

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_407: [ When on_underlying_io_open_complete is called when the uws instance has send the upgrade request but it is waiting for the response, an error shall be reported to the user by calling the on_ws_open_complete with WS_OPEN_ERROR_MULTIPLE_UNDERLYING_IO_OPEN_EVENTS. ]*/
TEST_FUNCTION(when_sending_the_upgrade_request_fails_the_error_WS_OPEN_ERROR_MULTIPLE_UNDERLYING_IO_OPEN_EVENTS_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_MULTIPLE_UNDERLYING_IO_OPEN_EVENTS));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_409: [ After any error is indicated by on_ws_open_complete, a subsequent uws_client_open_async shall be possible. ]*/
TEST_FUNCTION(uws_client_open_async_after_WS_OPEN_ERROR_MULTIPLE_UNDERLYING_IO_OPEN_EVENTS_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_MULTIPLE_UNDERLYING_IO_OPEN_EVENTS));
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);;

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* on_underlying_io_bytes_received */

/* Tests_SRS_UWS_CLIENT_01_378: [ When on_underlying_io_bytes_received is called while the uws is OPENING, the received bytes shall be accumulated in order to attempt parsing the WebSocket Upgrade response. ]*/
/* Tests_SRS_UWS_CLIENT_01_380: [ If an WebSocket Upgrade request can be parsed from the accumulated bytes, the status shall be read from the WebSocket upgrade response. ]*/
/* Tests_SRS_UWS_CLIENT_01_381: [ If the status is 101, uws shall be considered OPEN and this shall be indicated by calling the on_ws_open_complete callback passed to uws_client_open_async with IO_OPEN_OK. ]*/
/* Tests_SRS_UWS_CLIENT_01_065: [ When the client is to _Establish a WebSocket Connection_ given a set of (/host/, /port/, /resource name/, and /secure/ flag), along with a list of /protocols/ and /extensions/ to be used, and an /origin/ in the case of web browsers, it MUST open a connection, send an opening handshake, and read the server's handshake in response. ]*/
/* Tests_SRS_UWS_CLIENT_01_102: [ Once the client's opening handshake has been sent, the client MUST wait for a response from the server before sending any further data. ]*/
/* Tests_SRS_UWS_CLIENT_01_115: [ If the server's response is validated as provided for above, it is said that _The WebSocket Connection is Established_ and that the WebSocket Connection is in the OPEN state. ]*/
/* Tests_SRS_UWS_CLIENT_01_478: [ A Status-Line with a 101 response code as per RFC 2616 [RFC2616]. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_a_full_reply_after_the_upgrade_request_was_sent_indicates_open_complete)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_OK));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_382: [ If a negative status is decoded from the WebSocket upgrade request, an error shall be indicated by calling the on_ws_open_complete callback passed to uws_client_open_async with WS_OPEN_ERROR_BAD_RESPONSE_STATUS. ]*/
/* Tests_SRS_UWS_CLIENT_01_478: [ A Status-Line with a 101 response code as per RFC 2616 [RFC2616]. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_a_reply_with_a_status_code_different_than_101_indicates_an_open_complete_with_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 403\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_BAD_RESPONSE_STATUS));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_382: [ If a negative status is decoded from the WebSocket upgrade request, an error shall be indicated by calling the on_ws_open_complete callback passed to uws_client_open_async with WS_OPEN_ERROR_BAD_RESPONSE_STATUS. ]*/
/* Tests_SRS_UWS_CLIENT_01_478: [ A Status-Line with a 101 response code as per RFC 2616 [RFC2616]. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_a_reply_with_status_100_indicates_an_open_complete_with_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 100\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_BAD_RESPONSE_STATUS));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_382: [ If a negative status is decoded from the WebSocket upgrade request, an error shall be indicated by calling the on_ws_open_complete callback passed to uws_client_open_async with WS_OPEN_ERROR_BAD_RESPONSE_STATUS. ]*/
/* Tests_SRS_UWS_CLIENT_01_478: [ A Status-Line with a 101 response code as per RFC 2616 [RFC2616]. ]*/
TEST_FUNCTION(open_after_a_bad_status_is_decoded_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 100\r\n\r\n";
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_378: [ When on_underlying_io_bytes_received is called while the uws is OPENING, the received bytes shall be accumulated in order to attempt parsing the WebSocket Upgrade response. ]*/
/* Tests_SRS_UWS_CLIENT_01_380: [ If an WebSocket Upgrade request can be parsed from the accumulated bytes, the status shall be read from the WebSocket upgrade response. ]*/
/* Tests_SRS_UWS_CLIENT_01_381: [ If the status is 101, uws shall be considered OPEN and this shall be indicated by calling the on_ws_open_complete callback passed to uws_client_open_async with IO_OPEN_OK. ]*/
/* Tests_SRS_UWS_CLIENT_01_065: [ When the client is to _Establish a WebSocket Connection_ given a set of (/host/, /port/, /resource name/, and /secure/ flag), along with a list of /protocols/ and /extensions/ to be used, and an /origin/ in the case of web browsers, it MUST open a connection, send an opening handshake, and read the server's handshake in response. ]*/
/* Tests_SRS_UWS_CLIENT_01_102: [ Once the client's opening handshake has been sent, the client MUST wait for a response from the server before sending any further data. ]*/
/* Tests_SRS_UWS_CLIENT_01_115: [ If the server's response is validated as provided for above, it is said that _The WebSocket Connection is Established_ and that the WebSocket Connection is in the OPEN state. ]*/
/* Tests_SRS_UWS_CLIENT_01_478: [ A Status-Line with a 101 response code as per RFC 2616 [RFC2616]. ]*/
TEST_FUNCTION(after_a_bad_status_code_a_subsequent_open_completes)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_bad_upgrade_response[] = "HTTP/1.1 403 \r\n\r\n";
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_bad_upgrade_response, sizeof(test_bad_upgrade_response) - 1);

    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_OK));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_383: [ If the WebSocket upgrade request cannot be decoded an error shall be indicated by calling the on_ws_open_complete callback passed to uws_client_open_async with WS_OPEN_ERROR_BAD_UPGRADE_RESPONSE. ]*/
/* Tests_SRS_UWS_CLIENT_01_478: [ A Status-Line with a 101 response code as per RFC 2616 [RFC2616]. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_an_empty_reply_indicates_an_open_complete_with_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_BAD_UPGRADE_RESPONSE));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_383: [ If the WebSocket upgrade request cannot be decoded an error shall be indicated by calling the on_ws_open_complete callback passed to uws_client_open_async with WS_OPEN_ERROR_BAD_UPGRADE_RESPONSE. ]*/
/* Tests_SRS_UWS_CLIENT_01_478: [ A Status-Line with a 101 response code as per RFC 2616 [RFC2616]. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_an_imcomplete_HTTP_1_1__reply_indicates_an_open_complete_with_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_BAD_UPGRADE_RESPONSE));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_383: [ If the WebSocket upgrade request cannot be decoded an error shall be indicated by calling the on_ws_open_complete callback passed to uws_client_open_async with WS_OPEN_ERROR_BAD_UPGRADE_RESPONSE. ]*/
/* Tests_SRS_UWS_CLIENT_01_478: [ A Status-Line with a 101 response code as per RFC 2616 [RFC2616]. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_a_complete_HTTP_version_but_no_status_code_indicates_an_open_complete_with_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_BAD_UPGRADE_RESPONSE));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_478: [ A Status-Line with a 101 response code as per RFC 2616 [RFC2616]. ]*/
TEST_FUNCTION(open_completes_when_response_has_more_spaces_in_it)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1  101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_OK));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_478: [ A Status-Line with a 101 response code as per RFC 2616 [RFC2616]. ]*/
TEST_FUNCTION(open_completes_when_response_has_more_spaces_in_it_after_the_status_code)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101  Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_OK));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_478: [ A Status-Line with a 101 response code as per RFC 2616 [RFC2616]. ]*/
TEST_FUNCTION(open_completes_when_a_header_is_present_in_the_response)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\nSomeHeader:x\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_OK));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_379: [ If allocating memory for accumulating the bytes fails, uws shall report that the open failed by calling the on_ws_open_complete callback passed to uws_client_open_async with WS_OPEN_ERROR_NOT_ENOUGH_MEMORY. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_received_bytes_fails_on_underlying_io_bytes_received_indicates_open_complete_with_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_NOT_ENOUGH_MEMORY));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_378: [ When on_underlying_io_bytes_received is called while the uws is OPENING, the received bytes shall be accumulated in order to attempt parsing the WebSocket Upgrade response. ]*/
/* Tests_SRS_UWS_CLIENT_01_380: [ If an WebSocket Upgrade request can be parsed from the accumulated bytes, the status shall be read from the WebSocket upgrade response. ]*/
TEST_FUNCTION(when_only_a_byte_is_received_no_open_complete_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "H";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_415: [ If called with a NULL context argument, on_underlying_io_bytes_received shall do nothing. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_NULL_context_does_nothing)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    g_on_bytes_received(NULL, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_416: [ If called with NULL buffer or zero size and the state of the iws is OPENING, uws shall report that the open failed by calling the on_ws_open_complete callback passed to uws_client_open_async with WS_OPEN_ERROR_INVALID_BYTES_RECEIVED_ARGUMENTS. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_NULL_buffer_indicates_an_open_complete_with_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_INVALID_BYTES_RECEIVED_ARGUMENTS));

    // act
    g_on_bytes_received(g_on_bytes_received_context, NULL, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_416: [ If called with NULL buffer or zero size and the state of the iws is OPENING, uws shall report that the open failed by calling the on_ws_open_complete callback passed to uws_client_open_async with WS_OPEN_ERROR_INVALID_BYTES_RECEIVED_ARGUMENTS. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_zero_size_indicates_an_open_complete_with_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_INVALID_BYTES_RECEIVED_ARGUMENTS));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_417: [ When on_underlying_io_bytes_received is called while OPENING but before the on_underlying_io_open_complete has been called, uws shall report that the open failed by calling the on_ws_open_complete callback passed to uws_client_open_async with WS_OPEN_ERROR_BYTES_RECEIVED_BEFORE_UNDERLYING_OPEN. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_before_underlying_io_open_complete_indicates_an_open_complete_with_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_BYTES_RECEIVED_BEFORE_UNDERLYING_OPEN));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_379: [ If allocating memory for accumulating the bytes fails, uws shall report that the open failed by calling the on_ws_open_complete callback passed to uws_client_open_async with WS_OPEN_ERROR_NOT_ENOUGH_MEMORY. ]*/
TEST_FUNCTION(when_allocating_memory_for_a_second_byte_fails_open_complete_is_indicated_with_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_NOT_ENOUGH_MEMORY));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response + 1, 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

void when_only_n_bytes_are_received_from_the_response_no_open_complete_is_indicated(const char* test_upgrade_response, size_t n)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    char temp_str[32];

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, n);

    // assert
    (void)sprintf(temp_str, "Bytes = %u", (unsigned int)n);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls(), temp_str);

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_380: [ If an WebSocket Upgrade request can be parsed from the accumulated bytes, the status shall be read from the WebSocket upgrade response. ]*/
TEST_FUNCTION(when_all_but_1_bytes_are_received_from_the_response_no_open_complete_is_indicated)
{
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r";
    size_t i;

    for (i = 1; i < sizeof(test_upgrade_response); i++)
    {
        when_only_n_bytes_are_received_from_the_response_no_open_complete_is_indicated(test_upgrade_response, i);
    }
}

/* Tests_SRS_UWS_CLIENT_01_384: [ Any extra bytes that are left unconsumed after decoding a succesfull WebSocket upgrade response shall be used for decoding WebSocket frames by passing them to uws_frame_decoder_decode. ]*/
TEST_FUNCTION(when_1_extra_byte_is_received_the_open_complete_is_properly_indicated_and_the_extra_byte_is_saved_for_decoding_frames)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n\0";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_OK));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_386: [ When a WebSocket data frame is decoded succesfully it shall be indicated via the callback on_ws_frame_received. ]*/
/* Tests_SRS_UWS_CLIENT_01_385: [ If the state of the uws instance is OPEN, the received bytes shall be used for decoding WebSocket frames. ]*/
/* Tests_SRS_UWS_CLIENT_01_154: [ *  %x2 denotes a binary frame ]*/
/* Tests_SRS_UWS_CLIENT_01_163: [ The length of the "Payload data", in bytes: ]*/
/* Tests_SRS_UWS_CLIENT_01_164: [ if 0-125, that is the payload length. ]*/
/* Tests_SRS_UWS_CLIENT_01_169: [ The payload length is the length of the "Extension data" + the length of the "Application data". ]*/
/* Tests_SRS_UWS_CLIENT_01_173: [ The "Payload data" is defined as "Extension data" concatenated with "Application data". ]*/
/* Tests_SRS_UWS_CLIENT_01_147: [ Indicates that this is the final fragment in a message. ]*/
/* Tests_SRS_UWS_CLIENT_01_148: [ The first fragment MAY also be the final fragment. ]*/
/* Tests_SRS_UWS_CLIENT_01_277: [ To receive WebSocket data, an endpoint listens on the underlying network connection. ]*/
/* Tests_SRS_UWS_CLIENT_01_278: [ Incoming data MUST be parsed as WebSocket frames as defined in Section 5.2. ]*/
/* Tests_SRS_UWS_CLIENT_01_280: [ Upon receiving a data frame (Section 5.6), the endpoint MUST note the /type/ of the data as defined by the opcode (frame-opcode) from Section 5.2. ]*/
/* Tests_SRS_UWS_CLIENT_01_281: [ The "Application data" from this frame is defined as the /data/ of the message. ]*/
/* Tests_SRS_UWS_CLIENT_01_282: [ If the frame comprises an unfragmented message (Section 5.4), it is said that _A WebSocket Message Has Been Received_ with type /type/ and data /data/. ]*/
TEST_FUNCTION(when_a_1_byte_binary_frame_is_received_it_shall_be_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char test_frame[] = { 0x82, 0x01, 0x42 };
    const unsigned char expected_payload[] = { 0x42 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 1))
        .ValidateArgumentBuffer(3, expected_payload, sizeof(expected_payload));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_153: [ *  %x1 denotes a text frame ]*/
/* Tests_SRS_UWS_CLIENT_01_258: [ Currently defined opcodes for data frames include 0x1 (Text), 0x2 (Binary). ]*/
/* Tests_SRS_UWS_CLIENT_01_280: [ Upon receiving a data frame (Section 5.6), the endpoint MUST note the /type/ of the data as defined by the opcode (frame-opcode) from Section 5.2. ]*/
/* Tests_SRS_UWS_CLIENT_01_281: [ The "Application data" from this frame is defined as the /data/ of the message. ]*/
/* Tests_SRS_UWS_CLIENT_01_282: [ If the frame comprises an unfragmented message (Section 5.4), it is said that _A WebSocket Message Has Been Received_ with type /type/ and data /data/. ]*/
TEST_FUNCTION(when_a_1_byte_text_frame_is_received_it_shall_be_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char test_frame[] = { 0x81, 0x01, 'a' };
    const unsigned char expected_payload[] = { 'a' };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_TEXT, IGNORED_PTR_ARG, 1))
        .ValidateArgumentBuffer(3, expected_payload, sizeof(expected_payload));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_163: [ The length of the "Payload data", in bytes: ]*/
/* Tests_SRS_UWS_CLIENT_01_164: [ if 0-125, that is the payload length. ]*/
/* Tests_SRS_UWS_CLIENT_01_264: [ The "Payload data" is arbitrary binary data whose interpretation is solely up to the application layer. ]*/
TEST_FUNCTION(when_a_0_bytes_binary_frame_is_received_it_shall_be_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char test_frame[] = { 0x82, 0x00 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 0))
        .IgnoreArgument_buffer();

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_163: [ The length of the "Payload data", in bytes: ]*/
/* Tests_SRS_UWS_CLIENT_01_164: [ if 0-125, that is the payload length. ]*/
/* Tests_SRS_UWS_CLIENT_01_258: [ Currently defined opcodes for data frames include 0x1 (Text), 0x2 (Binary). ]*/
TEST_FUNCTION(when_a_0_bytes_text_frame_is_received_it_shall_be_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char test_frame[] = { 0x81, 0x00 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_TEXT, IGNORED_PTR_ARG, 0))
        .IgnoreArgument_buffer();

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_213: [ A fragmented message consists of a single frame with the FIN bit clear and an opcode other than 0, followed by zero or more frames with the FIN bit clear and the opcode set to 0, and terminated by a single frame with the FIN bit set and an opcode of 0. ]*/
/* Tests_SRS_UWS_CLIENT_01_147: [ Indicates that this is the final fragment in a message. ]*/
/* Tests_SRS_UWS_CLIENT_01_152: [* *  %x0 denotes a continuation frame *]*/
/* Tests_SRS_UWS_CLIENT_01_216: [ Message fragments MUST be delivered to the recipient in the order sent by the sender. ]*/
/* Tests_SRS_UWS_CLIENT_01_219: [ A sender MAY create fragments of any size for non-control messages. ]*/
/* Tests_SRS_UWS_CLIENT_01_225: [ As a consequence of these rules, all fragments of a message are of the same type, as set by the first fragment's opcode. ]*/
TEST_FUNCTION(when_a_fragmented_text_frame_is_received_it_shall_be_indicated_to_the_user_once_fully_received)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char first_fragment[125 + 2] = { 0x01, 0x7D };
    unsigned char middle_fragment[130 + 4] = { 0x00, 0x7E, 0x00, 0x82 };
    unsigned char last_fragment[2] = { 0x80, 0x00 };
    unsigned char* result_payload = (unsigned char*)malloc(255);
    size_t i;

    for (i = 0; i < 255; i++)
    {
        if (i < 125)
        {
            first_fragment[2 + i] = (unsigned char)i;
        }
        else
        {
            middle_fragment[4 + (i - 125)] = (unsigned char)i;
        }
        result_payload[i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_TEXT, IGNORED_PTR_ARG, 255))
        .ValidateArgumentBuffer(3, result_payload, 255);

    // act
    g_on_bytes_received(g_on_bytes_received_context, first_fragment, sizeof(first_fragment));
    g_on_bytes_received(g_on_bytes_received_context, middle_fragment, sizeof(middle_fragment));
    g_on_bytes_received(g_on_bytes_received_context, last_fragment, sizeof(last_fragment));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    free(result_payload);
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_213: [ A fragmented message consists of a single frame with the FIN bit clear and an opcode other than 0, followed by zero or more frames with the FIN bit clear and the opcode set to 0, and terminated by a single frame with the FIN bit set and an opcode of 0. ]*/
/* Tests_SRS_UWS_CLIENT_01_147: [ Indicates that this is the final fragment in a message. ]*/
/* Tests_SRS_UWS_CLIENT_01_152: [* *  %x0 denotes a continuation frame *]*/
/* Tests_SRS_UWS_CLIENT_01_216: [ Message fragments MUST be delivered to the recipient in the order sent by the sender. ]*/
/* Tests_SRS_UWS_CLIENT_01_219: [ A sender MAY create fragments of any size for non-control messages. ]*/
/* Tests_SRS_UWS_CLIENT_01_225: [ As a consequence of these rules, all fragments of a message are of the same type, as set by the first fragment's opcode. ]*/
TEST_FUNCTION(when_a_fragmented_binary_frame_is_received_it_shall_be_indicated_to_the_user_once_fully_received)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char first_fragment[125 + 2] = { 0x02, 0x7D };
    unsigned char middle_fragment[130 + 4] = { 0x00, 0x7E, 0x00, 0x82 };
    unsigned char last_fragment[2] = { 0x80, 0x00 };
    unsigned char* result_payload = (unsigned char*)malloc(255);
    size_t i;

    for (i = 0; i < 255; i++)
    {
        if (i < 125)
        {
            first_fragment[2 + i] = (unsigned char)i;
        }
        else
        {
            middle_fragment[4 + (i - 125)] = (unsigned char)i;
        }
        result_payload[i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 255))
        .ValidateArgumentBuffer(3, result_payload, 255);

    // act
    g_on_bytes_received(g_on_bytes_received_context, first_fragment, sizeof(first_fragment));
    g_on_bytes_received(g_on_bytes_received_context, middle_fragment, sizeof(middle_fragment));
    g_on_bytes_received(g_on_bytes_received_context, last_fragment, sizeof(last_fragment));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    free(result_payload);
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_217: [ The fragments of one message MUST NOT be interleaved between the fragments of another message unless an extension has been negotiated that can interpret the interleaving. ]*/
TEST_FUNCTION(when_a_fragmented_frame_is_interleaved_within_another_fragmented_frame_there_is_an_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char fragment1[2] = {0x02, 0x00};
    unsigned char fragment2[2] = {0x02, 0x00};

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, fragment1, sizeof(fragment1));
    g_on_bytes_received(g_on_bytes_received_context, fragment2, sizeof(fragment2));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_213: [ A fragmented message consists of a single frame with the FIN bit clear and an opcode other than 0, followed by zero or more frames with the FIN bit clear and the opcode set to 0, and terminated by a single frame with the FIN bit set and an opcode of 0. ]*/
/* Tests_SRS_UWS_CLIENT_01_147: [ Indicates that this is the final fragment in a message. ]*/
/* Tests_SRS_UWS_CLIENT_01_152: [* *  %x0 denotes a continuation frame *]*/
/* Tests_SRS_UWS_CLIENT_01_216: [ Message fragments MUST be delivered to the recipient in the order sent by the sender. ]*/
/* Tests_SRS_UWS_CLIENT_01_219: [ A sender MAY create fragments of any size for non-control messages. ]*/
/* Tests_SRS_UWS_CLIENT_01_225: [ As a consequence of these rules, all fragments of a message are of the same type, as set by the first fragment's opcode. ]*/
TEST_FUNCTION(when_a_fragmented_frame_is_received_all_at_once_the_frame_is_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_stream[2 + 3 + 2] = { 0x02, 0x00, 0x00, 0x01, 0x00, 0x80, 0x00};

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 1))
        .IgnoreArgument_buffer();

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_stream, sizeof(test_stream));
    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_213: [ A fragmented message consists of a single frame with the FIN bit clear and an opcode other than 0, followed by zero or more frames with the FIN bit clear and the opcode set to 0, and terminated by a single frame with the FIN bit set and an opcode of 0. ]*/
/* Tests_SRS_UWS_CLIENT_01_147: [ Indicates that this is the final fragment in a message. ]*/
/* Tests_SRS_UWS_CLIENT_01_152: [* *  %x0 denotes a continuation frame *]*/
/* Tests_SRS_UWS_CLIENT_01_216: [ Message fragments MUST be delivered to the recipient in the order sent by the sender. ]*/
/* Tests_SRS_UWS_CLIENT_01_219: [ A sender MAY create fragments of any size for non-control messages. ]*/
/* Tests_SRS_UWS_CLIENT_01_225: [ As a consequence of these rules, all fragments of a message are of the same type, as set by the first fragment's opcode. ]*/
/* Tests_SRS_UWS_CLIENT_01_214: [ Control frames (see Section 5.5) MAY be injected in the middle of a fragmented message. ]*/
TEST_FUNCTION(pong_frame_can_be_injected_in_middle_of_fragmented_message)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char first_fragment[125 + 2] = { 0x01, 0x7D };
    unsigned char middle_fragment[130 + 4] = { 0x00, 0x7E, 0x00, 0x82 };
    unsigned char last_fragment[2] = { 0x80, 0x00 };
    const unsigned char ping_frame[] = { 0x89, 0x00 };
    unsigned char pong_frame[] = {0x8A, 0x00, 0x00, 0x00, 0x00, 0x00};
    BUFFER_HANDLE buffer_handle;

    unsigned char* result_payload = (unsigned char*)malloc(255);
    size_t i;

    for (i = 0; i < 255; i++)
    {
        if (i < 125)
        {
            first_fragment[2 + i] = (unsigned char)i;
        }
        else
        {
            middle_fragment[4 + (i - 125)] = (unsigned char)i;
        }
        result_payload[i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_PONG_FRAME, IGNORED_PTR_ARG, 0, true, true, 0))
        .IgnoreArgument_payload()
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(pong_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(pong_frame));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, pong_frame, sizeof(pong_frame), IGNORED_PTR_ARG, NULL))
        .ValidateArgumentBuffer(2, pong_frame, sizeof(pong_frame));
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_TEXT, IGNORED_PTR_ARG, 255))
        .ValidateArgumentBuffer(3, result_payload, 255);

    // act
    g_on_bytes_received(g_on_bytes_received_context, first_fragment, sizeof(first_fragment));
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)ping_frame, sizeof(ping_frame));
    g_on_bytes_received(g_on_bytes_received_context, middle_fragment, sizeof(middle_fragment));
    g_on_bytes_received(g_on_bytes_received_context, last_fragment, sizeof(last_fragment));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    free(result_payload);
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_215: [ Control frames themselves MUST NOT be fragmented. ]*/
TEST_FUNCTION(when_a_fragmented_control_frame_is_received_there_is_an_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char test_frame[] = {0x09, 0x00};

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_163: [ The length of the "Payload data", in bytes: ]*/
/* Tests_SRS_UWS_CLIENT_01_164: [ if 0-125, that is the payload length. ]*/
/* Tests_SRS_UWS_CLIENT_01_264: [ The "Payload data" is arbitrary binary data whose interpretation is solely up to the application layer. ]*/
/* Tests_SRS_UWS_CLIENT_01_258: [ Currently defined opcodes for data frames include 0x1 (Text), 0x2 (Binary). ]*/
TEST_FUNCTION(when_a_125_bytes_binary_frame_is_received_it_shall_be_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[125 + 2] = { 0x82, 0x7D };
    size_t i;

    for (i = 0; i < 125; i++)
    {
        test_frame[2 + i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 125))
        .ValidateArgumentBuffer(3, &test_frame[2], 125);

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_165: [ If 126, the following 2 bytes interpreted as a 16-bit unsigned integer are the payload length. ]*/
/* Tests_SRS_UWS_CLIENT_01_167: [ Multibyte length quantities are expressed in network byte order. ]*/
TEST_FUNCTION(when_a_126_bytes_binary_frame_is_received_it_shall_be_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[126 + 4] = { 0x82, 0x7E, 0x00, 0x7E };
    size_t i;

    for (i = 0; i < 126; i++)
    {
        test_frame[4 + i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 126))
        .ValidateArgumentBuffer(3, &test_frame[4], 126);

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_165: [ If 126, the following 2 bytes interpreted as a 16-bit unsigned integer are the payload length. ]*/
/* Tests_SRS_UWS_CLIENT_01_167: [ Multibyte length quantities are expressed in network byte order. ]*/
TEST_FUNCTION(when_a_127_bytes_binary_frame_is_received_it_shall_be_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[127 + 4] = { 0x82, 0x7E, 0x00, 0x7F };
    size_t i;

    for (i = 0; i < 127; i++)
    {
        test_frame[4 + i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 127))
        .ValidateArgumentBuffer(3, &test_frame[4], 127);

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_165: [ If 126, the following 2 bytes interpreted as a 16-bit unsigned integer are the payload length. ]*/
/* Tests_SRS_UWS_CLIENT_01_167: [ Multibyte length quantities are expressed in network byte order. ]*/
TEST_FUNCTION(when_a_65535_bytes_binary_frame_is_received_it_shall_be_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char* test_frame = (unsigned char*)malloc(65535 + 4);
    size_t i;
    test_frame[0] = 0x82;
    test_frame[1] = 0x7E;
    test_frame[2] = 0xFF;
    test_frame[3] = 0xFF;

    for (i = 0; i < 65535; i++)
    {
        test_frame[4 + i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 65535))
        .ValidateArgumentBuffer(3, &test_frame[4], 65535);

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, 65535 + 4);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    free(test_frame);
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_166: [ If 127, the following 8 bytes interpreted as a 64-bit unsigned integer (the most significant bit MUST be 0) are the payload length. ]*/
/* Tests_SRS_UWS_CLIENT_01_167: [ Multibyte length quantities are expressed in network byte order. ]*/
TEST_FUNCTION(when_a_65536_bytes_binary_frame_is_received_it_shall_be_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char* test_frame = (unsigned char*)malloc(65536 + 10);
    size_t i;
    test_frame[0] = 0x82;
    test_frame[1] = 0x7F;
    test_frame[2] = 0x00;
    test_frame[3] = 0x00;
    test_frame[4] = 0x00;
    test_frame[5] = 0x00;
    test_frame[6] = 0x00;
    test_frame[7] = 0x01;
    test_frame[8] = 0x00;
    test_frame[9] = 0x00;

    for (i = 0; i < 65536; i++)
    {
        test_frame[10 + i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 65536))
        .ValidateArgumentBuffer(3, &test_frame[10], 65536);

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, 65536 + 10);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    free(test_frame);
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_166: [ If 127, the following 8 bytes interpreted as a 64-bit unsigned integer (the most significant bit MUST be 0) are the payload length. ]*/
/* Tests_SRS_UWS_CLIENT_01_167: [ Multibyte length quantities are expressed in network byte order. ]*/
TEST_FUNCTION(when_a_65537_bytes_binary_frame_is_received_it_shall_be_indicated_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char* test_frame = (unsigned char*)malloc(65537 + 10);
    size_t i;
    test_frame[0] = 0x82;
    test_frame[1] = 0x7F;
    test_frame[2] = 0x00;
    test_frame[3] = 0x00;
    test_frame[4] = 0x00;
    test_frame[5] = 0x00;
    test_frame[6] = 0x00;
    test_frame[7] = 0x01;
    test_frame[8] = 0x00;
    test_frame[9] = 0x01;

    for (i = 0; i < 65537; i++)
    {
        test_frame[10 + i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 65537))
        .ValidateArgumentBuffer(3, &test_frame[10], 65537);

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, 65537 + 10);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    free(test_frame);
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_168: [ Note that in all cases, the minimal number of bytes MUST be used to encode the length, for example, the length of a 124-byte-long string can't be encoded as the sequence 126, 0, 124. ]*/
/* Tests_SRS_UWS_CLIENT_01_419: [ If there is an error decoding the WebSocket frame, an error shall be indicated by calling the on_ws_error callback with WS_ERROR_BAD_FRAME_RECEIVED. ]*/
TEST_FUNCTION(when_a_0_byte_binary_frame_is_received_with_16_bit_length_an_error_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[] = { 0x82, 0x7E, 0x00, 0x00 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_168: [ Note that in all cases, the minimal number of bytes MUST be used to encode the length, for example, the length of a 124-byte-long string can't be encoded as the sequence 126, 0, 124. ]*/
/* Tests_SRS_UWS_CLIENT_01_419: [ If there is an error decoding the WebSocket frame, an error shall be indicated by calling the on_ws_error callback with WS_ERROR_BAD_FRAME_RECEIVED. ]*/
TEST_FUNCTION(when_a_125_byte_binary_frame_is_received_with_16_bit_length_an_error_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[125 + 4] = { 0x82, 0x7E, 0x00, 0x7D };
    size_t i;

    for (i = 0; i < 125; i++)
    {
        test_frame[4 + i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_168: [ Note that in all cases, the minimal number of bytes MUST be used to encode the length, for example, the length of a 124-byte-long string can't be encoded as the sequence 126, 0, 124. ]*/
/* Tests_SRS_UWS_CLIENT_01_419: [ If there is an error decoding the WebSocket frame, an error shall be indicated by calling the on_ws_error callback with WS_ERROR_BAD_FRAME_RECEIVED. ]*/
TEST_FUNCTION(when_a_0_byte_binary_frame_is_received_with_64_bit_length_an_error_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[] = { 0x82, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_168: [ Note that in all cases, the minimal number of bytes MUST be used to encode the length, for example, the length of a 124-byte-long string can't be encoded as the sequence 126, 0, 124. ]*/
/* Tests_SRS_UWS_CLIENT_01_419: [ If there is an error decoding the WebSocket frame, an error shall be indicated by calling the on_ws_error callback with WS_ERROR_BAD_FRAME_RECEIVED. ]*/
TEST_FUNCTION(when_a_65535_byte_binary_frame_is_received_with_64_bit_length_an_error_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char* test_frame = (unsigned char*)malloc(65535 + 10);
    size_t i;
    test_frame[0] = 0x82;
    test_frame[1] = 0x7F;
    test_frame[2] = 0x00;
    test_frame[3] = 0x00;
    test_frame[4] = 0x00;
    test_frame[5] = 0x00;
    test_frame[6] = 0x00;
    test_frame[7] = 0x00;
    test_frame[8] = 0xFF;
    test_frame[9] = 0xFF;

    for (i = 0; i < 65535; i++)
    {
        test_frame[10 + i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, 65535 + 10);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
    free(test_frame);
}

/* Tests_SRS_UWS_CLIENT_01_168: [ Note that in all cases, the minimal number of bytes MUST be used to encode the length, for example, the length of a 124-byte-long string can't be encoded as the sequence 126, 0, 124. ]*/
/* Tests_SRS_UWS_CLIENT_01_419: [ If there is an error decoding the WebSocket frame, an error shall be indicated by calling the on_ws_error callback with WS_ERROR_BAD_FRAME_RECEIVED. ]*/
TEST_FUNCTION(check_for_16_bit_length_too_low_is_done_as_soon_as_length_is_received)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[] = { 0x82, 0x7E, 0x00, 0x7D };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_168: [ Note that in all cases, the minimal number of bytes MUST be used to encode the length, for example, the length of a 124-byte-long string can't be encoded as the sequence 126, 0, 124. ]*/
/* Tests_SRS_UWS_CLIENT_01_419: [ If there is an error decoding the WebSocket frame, an error shall be indicated by calling the on_ws_error callback with WS_ERROR_BAD_FRAME_RECEIVED. ]*/
TEST_FUNCTION(check_for_64_bit_length_too_low_is_done_as_soon_as_length_is_received)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[] = { 0x82, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_166: [ If 127, the following 8 bytes interpreted as a 64-bit unsigned integer (the most significant bit MUST be 0) are the payload length. ]*/
/* Tests_SRS_UWS_CLIENT_01_419: [ If there is an error decoding the WebSocket frame, an error shall be indicated by calling the on_ws_error callback with WS_ERROR_BAD_FRAME_RECEIVED. ]*/
TEST_FUNCTION(when_the_highest_bit_is_set_in_a_64_bit_length_frame_an_error_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char* test_frame = (unsigned char*)malloc(65536 + 10);
    size_t i;
    test_frame[0] = 0x82;
    test_frame[1] = 0x7F;
    test_frame[2] = 0x80;
    test_frame[3] = 0x00;
    test_frame[4] = 0x00;
    test_frame[5] = 0x00;
    test_frame[6] = 0x00;
    test_frame[7] = 0x01;
    test_frame[8] = 0x00;
    test_frame[9] = 0x00;

    for (i = 0; i < 65536; i++)
    {
        test_frame[10 + i] = (unsigned char)i;
    }

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, 65536 + 10);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
    free(test_frame);
}

/* Tests_SRS_UWS_CLIENT_01_418: [ If allocating memory for the bytes accumulated for decoding WebSocket frames fails, an error shall be indicated by calling the on_ws_error callback with WS_ERROR_NOT_ENOUGH_MEMORY. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_received_frame_bytes_fails_an_error_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[] = { 0x82, 0x00 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_NOT_ENOUGH_MEMORY));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_384: [ Any extra bytes that are left unconsumed after decoding a succesfull WebSocket upgrade response shall be used for decoding WebSocket frames ]*/
TEST_FUNCTION(when_1_byte_is_received_together_with_the_upgrade_request_and_one_byte_with_a_separate_call_decoding_frame_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char* upgrade_response_frame = (unsigned char*)malloc(sizeof(test_upgrade_response));
    unsigned char test_frame[] = { 0x00 };

    (void)memcpy(upgrade_response_frame, test_upgrade_response, sizeof(test_upgrade_response) - 1);
    upgrade_response_frame[sizeof(test_upgrade_response) - 1] = 0x82;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)upgrade_response_frame, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 0))
        .IgnoreArgument_buffer();

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
    free(upgrade_response_frame);
}

/* Tests_SRS_UWS_CLIENT_01_384: [ Any extra bytes that are left unconsumed after decoding a succesfull WebSocket upgrade response shall be used for decoding WebSocket frames ]*/
TEST_FUNCTION(when_a_complete_frame_is_received_together_with_the_upgrade_request_the_frame_is_indicated_as_received)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    size_t received_data_length = sizeof(test_upgrade_response) + 1;
    unsigned char* received_data = (unsigned char*)malloc(received_data_length);

    (void)memcpy(received_data, test_upgrade_response, sizeof(test_upgrade_response) - 1);
    received_data[received_data_length - 2] = 0x82;
    received_data[received_data_length - 1] = 0x00;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_OK));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 0))
        .IgnoreArgument_buffer();

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)received_data, received_data_length);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
    free(received_data);
}

/* Tests_SRS_UWS_CLIENT_01_384: [ Any extra bytes that are left unconsumed after decoding a succesfull WebSocket upgrade response shall be used for decoding WebSocket frames ]*/
TEST_FUNCTION(when_a_1_byte_complete_frame_is_received_together_with_the_upgrade_request_the_frame_is_indicated_as_received)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    size_t received_data_length = sizeof(test_upgrade_response) + 2;
    unsigned char* received_data = (unsigned char*)malloc(received_data_length);
    unsigned char expected_frame_payload[] = { 0x42 };

    (void)memcpy(received_data, test_upgrade_response, sizeof(test_upgrade_response) - 1);
    received_data[received_data_length - 1] = 0x42;
    received_data[received_data_length - 2] = 0x01;
    received_data[received_data_length - 3] = 0x82;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_OK));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, expected_frame_payload, sizeof(expected_frame_payload)))
        .ValidateArgumentBuffer(3, expected_frame_payload, sizeof(expected_frame_payload));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)received_data, received_data_length);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
    free(received_data);
}

/* Tests_SRS_UWS_CLIENT_01_384: [ Any extra bytes that are left unconsumed after decoding a succesfull WebSocket upgrade response shall be used for decoding WebSocket frames ]*/
TEST_FUNCTION(when_2_complete_frames_are_received_together_with_the_upgrade_request_the_frames_are_indicated_as_received)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    size_t received_data_length = sizeof(test_upgrade_response) + 4;
    unsigned char* received_data = (unsigned char*)malloc(received_data_length);

    (void)memcpy(received_data, test_upgrade_response, sizeof(test_upgrade_response) - 1);
    received_data[received_data_length - 5] = 0x81;
    received_data[received_data_length - 4] = 0x01;
    received_data[received_data_length - 3] = 'a';
    received_data[received_data_length - 2] = 0x82;
    received_data[received_data_length - 1] = 0x00;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_OK));
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_TEXT, IGNORED_PTR_ARG, 1))
        .ValidateArgumentBuffer(3, "a", 1);
    STRICT_EXPECTED_CALL(test_on_ws_frame_received((void*)0x4243, WS_FRAME_TYPE_BINARY, IGNORED_PTR_ARG, 0))
        .IgnoreArgument_buffer();

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)received_data, received_data_length);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
    free(received_data);
}

/* Tests_SRS_UWS_CLIENT_01_144: [ A client MUST close a connection if it detects a masked frame. ]*/
/* Tests_SRS_UWS_CLIENT_01_145: [ In this case, it MAY use the status code 1002 (protocol error) as defined in Section 7.4.1. (These rules might be relaxed in a future specification.) ]*/
/* Tests_SRS_UWS_CLIENT_01_160: [ Defines whether the "Payload data" is masked. ]*/
/* Tests_SRS_UWS_CLIENT_01_140: [ To avoid confusing network intermediaries (such as intercepting proxies) and for security reasons that are further discussed in Section 10.3, a client MUST mask all frames that it sends to the server (see Section 5.3 for further details). ]*/
TEST_FUNCTION(when_a_masked_frame_is_received_an_error_is_indicated_and_connection_is_closed)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[] = { 0x82, 0x80 };
    unsigned char close_frame_payload[] = { 0x03, 0xEA };
    unsigned char close_frame[] = { 0x88, 0x82, 0x00, 0x00, 0x00, 0x00, 0x03, 0xEA };
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_CLOSE_FRAME, IGNORED_PTR_ARG, sizeof(close_frame_payload), true, true, 0))
        .ValidateArgumentBuffer(2, close_frame_payload, sizeof(close_frame_payload))
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(close_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(close_frame));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, close_frame, sizeof(close_frame), IGNORED_PTR_ARG, NULL))
        .ValidateArgumentBuffer(2, close_frame, sizeof(close_frame));
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_144: [ A client MUST close a connection if it detects a masked frame. ]*/
/* Tests_SRS_UWS_CLIENT_01_145: [ In this case, it MAY use the status code 1002 (protocol error) as defined in Section 7.4.1. (These rules might be relaxed in a future specification.) ]*/
/* Tests_SRS_UWS_CLIENT_01_160: [ Defines whether the "Payload data" is masked. ]*/
/* Tests_SRS_UWS_CLIENT_01_140: [ To avoid confusing network intermediaries (such as intercepting proxies) and for security reasons that are further discussed in Section 10.3, a client MUST mask all frames that it sends to the server (see Section 5.3 for further details). ]*/
TEST_FUNCTION(when_a_masked_frame_is_received_and_encoding_the_close_frame_fails_an_error_is_indicated_anyhow)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[] = { 0x82, 0x80 };
    unsigned char close_frame_payload[] = { 0x03, 0xEA };
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);
    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_CLOSE_FRAME, IGNORED_PTR_ARG, sizeof(close_frame_payload), true, true, 0))
        .ValidateArgumentBuffer(2, close_frame_payload, sizeof(close_frame_payload))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_144: [ A client MUST close a connection if it detects a masked frame. ]*/
/* Tests_SRS_UWS_CLIENT_01_145: [ In this case, it MAY use the status code 1002 (protocol error) as defined in Section 7.4.1. (These rules might be relaxed in a future specification.) ]*/
/* Tests_SRS_UWS_CLIENT_01_160: [ Defines whether the "Payload data" is masked. ]*/
/* Tests_SRS_UWS_CLIENT_01_140: [ To avoid confusing network intermediaries (such as intercepting proxies) and for security reasons that are further discussed in Section 10.3, a client MUST mask all frames that it sends to the server (see Section 5.3 for further details). ]*/
TEST_FUNCTION(when_a_masked_frame_is_received_and_sending_the_encoded_CLOSE_frame_fails_an_error_is_indicated_anyhow)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_frame[] = { 0x82, 0x80 };
    unsigned char close_frame_payload[] = { 0x03, 0xEA };
    unsigned char close_frame[] = { 0x88, 0x82, 0x00, 0x00, 0x00, 0x00, 0x03, 0xEA };
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_CLOSE_FRAME, IGNORED_PTR_ARG, sizeof(close_frame_payload), true, true, 0))
        .ValidateArgumentBuffer(2, close_frame_payload, sizeof(close_frame_payload))
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(close_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(close_frame));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, close_frame, sizeof(close_frame), IGNORED_PTR_ARG, NULL))
        .ValidateArgumentBuffer(2, close_frame, sizeof(close_frame))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_BAD_FRAME_RECEIVED));

    // act
    g_on_bytes_received(g_on_bytes_received_context, test_frame, sizeof(test_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_460: [ When a CLOSE frame is received the callback on_ws_peer_closed passed to uws_client_open_async shall be called, while passing to it the argument on_ws_peer_closed_context. ]*/
/* Tests_SRS_UWS_CLIENT_01_156: [ *  %x8 denotes a connection close ]*/
/* Tests_SRS_UWS_CLIENT_01_234: [ The Close frame contains an opcode of 0x8. ]*/
/* Tests_SRS_UWS_CLIENT_01_235: [ The Close frame MAY contain a body (the "Application data" portion of the frame) that indicates a reason for closing, such as an endpoint shutting down, an endpoint having received a frame too large, or an endpoint having received a frame that does not conform to the format expected by the endpoint. ]*/
/* Tests_SRS_UWS_CLIENT_01_236: [ If there is a body, the first two bytes of the body MUST be a 2-byte unsigned integer (in network byte order) representing a status code with value /code/ defined in Section 7.4. ]*/
/* Tests_SRS_UWS_CLIENT_01_461: [ The argument close_code shall be set to point to the code extracted from the CLOSE frame. ]*/
/* Tests_SRS_UWS_CLIENT_01_296: [ Upon either sending or receiving a Close control frame, it is said that _The WebSocket Closing Handshake is Started_ and that the WebSocket connection is in the CLOSING state. ]*/
/* Tests_SRS_UWS_CLIENT_01_241: [ If an endpoint receives a Close frame and did not previously send a Close frame, the endpoint MUST send a Close frame in response. ]*/
/* Tests_SRS_UWS_CLIENT_01_242: [ It SHOULD do so as soon as practical. ]*/
/* Tests_SRS_UWS_CLIENT_01_239: [ Close frames sent from client to server must be masked as per Section 5.3. ]*/
/* Tests_SRS_UWS_CLIENT_01_140: [ To avoid confusing network intermediaries (such as intercepting proxies) and for security reasons that are further discussed in Section 10.3, a client MUST mask all frames that it sends to the server (see Section 5.3 for further details). ]*/
TEST_FUNCTION(when_a_CLOSE_frame_is_received_while_in_open_the_code_is_reported_to_the_user)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char close_frame[] = { 0x88, 0x02, 0x03, 0xEA };
    BUFFER_HANDLE buffer_handle;
    uint16_t expected_close_code = 1002;
    unsigned char sent_close_frame[] = { 0x88, 0x80, 0x00, 0x00, 0x00, 0x00 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_CLOSE_FRAME, NULL, 0, true, true, 0))
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sent_close_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(sent_close_frame));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, sent_close_frame, sizeof(sent_close_frame), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(2, sent_close_frame, sizeof(sent_close_frame))
        .IgnoreArgument_callback_context()
        .IgnoreArgument_on_send_complete();
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);
    STRICT_EXPECTED_CALL(test_on_ws_peer_closed((void*)0x4301, IGNORED_PTR_ARG, NULL, 0))
        .ValidateArgumentBuffer(2, &expected_close_code, sizeof(expected_close_code));

    // act
    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_460: [ When a CLOSE frame is received the callback on_ws_peer_closed passed to uws_client_open_async shall be called, while passing to it the argument on_ws_peer_closed_context. ]*/
/* Tests_SRS_UWS_CLIENT_01_156: [ *  %x8 denotes a connection close ]*/
/* Tests_SRS_UWS_CLIENT_01_234: [ The Close frame contains an opcode of 0x8. ]*/
/* Tests_SRS_UWS_CLIENT_01_462: [ If no code can be extracted then close_code shall be NULL. ]*/
/* Tests_SRS_UWS_CLIENT_01_241: [ If an endpoint receives a Close frame and did not previously send a Close frame, the endpoint MUST send a Close frame in response. ]*/
/* Tests_SRS_UWS_CLIENT_01_140: [ To avoid confusing network intermediaries (such as intercepting proxies) and for security reasons that are further discussed in Section 10.3, a client MUST mask all frames that it sends to the server (see Section 5.3 for further details). ]*/
TEST_FUNCTION(when_a_CLOSE_frame_is_received_without_a_close_code_while_in_open_the_callback_is_triggered)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char close_frame[] = { 0x88, 0x00 };
    BUFFER_HANDLE buffer_handle;
    unsigned char sent_close_frame[] = { 0x88, 0x80, 0x00, 0x00, 0x00, 0x00 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_CLOSE_FRAME, NULL, 0, true, true, 0))
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sent_close_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(sent_close_frame));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, sent_close_frame, sizeof(sent_close_frame), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(2, sent_close_frame, sizeof(sent_close_frame))
        .IgnoreArgument_callback_context()
        .IgnoreArgument_on_send_complete();
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);
    STRICT_EXPECTED_CALL(test_on_ws_peer_closed((void*)0x4301, NULL, NULL, 0));

    // act
    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_463: [ The extra bytes (besides the close code) shall be passed to the on_ws_peer_closed callback by using extra_data and extra_data_length. ]*/
/* Tests_SRS_UWS_CLIENT_01_241: [ If an endpoint receives a Close frame and did not previously send a Close frame, the endpoint MUST send a Close frame in response. ]*/
/* Tests_SRS_UWS_CLIENT_01_140: [ To avoid confusing network intermediaries (such as intercepting proxies) and for security reasons that are further discussed in Section 10.3, a client MUST mask all frames that it sends to the server (see Section 5.3 for further details). ]*/
TEST_FUNCTION(when_a_CLOSE_frame_is_received_with_extra_bytes_the_bytes_are_passed_to_the_callback)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char close_frame[] = { 0x88, 0x04, 0x03, 0xEA, 0x42, 0x43 };
    BUFFER_HANDLE buffer_handle;
    uint16_t expected_close_code = 1002;
    unsigned char expected_extra_data[] = { 0x42, 0x43 };
    unsigned char sent_close_frame[] = { 0x88, 0x80, 0x00, 0x00, 0x00, 0x00 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(utf8_checker_is_valid_utf8(IGNORED_PTR_ARG, 2))
        .ValidateArgumentBuffer(1, &close_frame[4], 2);
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_CLOSE_FRAME, NULL, 0, true, true, 0))
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sent_close_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(sent_close_frame));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, sent_close_frame, sizeof(sent_close_frame), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(2, sent_close_frame, sizeof(sent_close_frame))
        .IgnoreArgument_callback_context()
        .IgnoreArgument_on_send_complete();
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);
    STRICT_EXPECTED_CALL(test_on_ws_peer_closed((void*)0x4301, IGNORED_PTR_ARG, IGNORED_PTR_ARG, sizeof(expected_extra_data)))
        .ValidateArgumentBuffer(2, &expected_close_code, sizeof(expected_close_code))
        .ValidateArgumentBuffer(3, &expected_extra_data, sizeof(expected_extra_data));

    // act
    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_237: [ Following the 2-byte integer, the body MAY contain UTF-8-encoded data with value /reason/, the interpretation of which is not defined by this specification. ]*/
TEST_FUNCTION(when_a_CLOSE_frame_is_received_with_a_malformed_UTF8_text_the_connection_is_closed)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char close_frame[] = { 0x88, 0x03, 0x03, 0xEA, 0xDF };
    uint16_t expected_close_code = 1002;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(utf8_checker_is_valid_utf8(IGNORED_PTR_ARG, 1))
        .ValidateArgumentBuffer(1, &close_frame[4], 1)
        .SetReturn(false);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_callback_context()
        .IgnoreArgument_on_io_close_complete();
    STRICT_EXPECTED_CALL(test_on_ws_peer_closed((void*)0x4301, IGNORED_PTR_ARG, IGNORED_PTR_ARG, 0))
        .ValidateArgumentBuffer(2, &expected_close_code, sizeof(expected_close_code));

    // act
    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_288: [ To _Close the WebSocket Connection_, an endpoint closes the underlying TCP connection. ]*/
/* Tests_SRS_UWS_CLIENT_01_290: [ An endpoint MAY close the connection via any means available when necessary, such as when under attack. ]*/
/* Tests_SRS_UWS_CLIENT_01_140: [ To avoid confusing network intermediaries (such as intercepting proxies) and for security reasons that are further discussed in Section 10.3, a client MUST mask all frames that it sends to the server (see Section 5.3 for further details). ]*/
TEST_FUNCTION(when_a_CLOSE_frame_is_received_while_in_open_and_encoding_the_outgoing_CLOSE_fails_the_connection_is_closed)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char close_frame[] = { 0x88, 0x02, 0x03, 0xEA };
    uint16_t expected_close_code = 1002;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_CLOSE_FRAME, NULL, 0, true, true, 0))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_callback_context()
        .IgnoreArgument_on_io_close_complete();
    STRICT_EXPECTED_CALL(test_on_ws_peer_closed((void*)0x4301, IGNORED_PTR_ARG, NULL, 0))
        .ValidateArgumentBuffer(2, &expected_close_code, sizeof(expected_close_code));

    // act
    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_288: [ To _Close the WebSocket Connection_, an endpoint closes the underlying TCP connection. ]*/
/* Tests_SRS_UWS_CLIENT_01_290: [ An endpoint MAY close the connection via any means available when necessary, such as when under attack. ]*/
/* Tests_SRS_UWS_CLIENT_01_140: [ To avoid confusing network intermediaries (such as intercepting proxies) and for security reasons that are further discussed in Section 10.3, a client MUST mask all frames that it sends to the server (see Section 5.3 for further details). ]*/
TEST_FUNCTION(when_a_CLOSE_frame_is_received_while_in_open_and_sending_the_outgoing_CLOSE_fails_the_connection_is_closed)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char close_frame[] = { 0x88, 0x02, 0x03, 0xEA };
    BUFFER_HANDLE buffer_handle;
    uint16_t expected_close_code = 1002;
    unsigned char sent_close_frame[] = { 0x88, 0x80, 0x00, 0x00, 0x00, 0x00 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_CLOSE_FRAME, NULL, 0, true, true, 0))
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sent_close_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(sent_close_frame));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, sent_close_frame, sizeof(sent_close_frame), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(2, sent_close_frame, sizeof(sent_close_frame))
        .IgnoreArgument_callback_context()
        .IgnoreArgument_on_send_complete()
        .SetReturn(1);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_callback_context()
        .IgnoreArgument_on_io_close_complete();
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);
    STRICT_EXPECTED_CALL(test_on_ws_peer_closed((void*)0x4301, IGNORED_PTR_ARG, NULL, 0))
        .ValidateArgumentBuffer(2, &expected_close_code, sizeof(expected_close_code));

    // act
    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_240: [ The application MUST NOT send any more data frames after sending a Close frame. ] */
TEST_FUNCTION(sending_after_a_close_is_received_does_not_send_anything)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char close_frame[] = { 0x88, 0x02, 0x03, 0xEA };
    int result;
    unsigned char test_payload[] = { 0x42 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));
    umock_c_reset_all_calls();

    // act
    result = uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* uws_client_send_frame_async */

/* Tests_SRS_UWS_CLIENT_01_044: [ If any the arguments uws_client is NULL, uws_client_send_frame_async shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_client_send_frame_async_with_NULL_handle_fails)
{
    // arrange
    unsigned char test_payload[] = { 0x42 };
    int result;

    // act
    result = uws_client_send_frame_async(NULL, WS_FRAME_TYPE_BINARY, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_045: [ If size is non-zero and buffer is NULL then uws_client_send_frame_async shall fail and return a non-zero value. ]*/
TEST_FUNCTION(uws_client_send_frame_async_with_NULL_buffer_and_non_zero_size_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    // act
    result = uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, NULL, 1, true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_043: [ If the uws instance is not OPEN (open has not been called or is still in progress) then uws_client_send_frame_async shall fail and return a non-zero value. ]*/
/* Tests_SRS_UWS_CLIENT_01_146: [ A data frame MAY be transmitted by either the client or the server at any time after opening handshake completion and before that endpoint has sent a Close frame (Section 5.5.1). ]*/
/* Tests_SRS_UWS_CLIENT_01_268: [ The endpoint MUST ensure the WebSocket connection is in the OPEN state ]*/
TEST_FUNCTION(uws_client_send_frame_async_when_not_open_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    int result;
    unsigned char test_payload[] = { 0x42 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    // act
    result = uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_043: [ If the uws instance is not OPEN (open has not been called or is still in progress) then uws_client_send_frame_async shall fail and return a non-zero value. ]*/
/* Tests_SRS_UWS_CLIENT_01_146: [ A data frame MAY be transmitted by either the client or the server at any time after opening handshake completion and before that endpoint has sent a Close frame (Section 5.5.1). ]*/
TEST_FUNCTION(uws_client_send_frame_async_when_opening_underlying_io_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    int result;
    unsigned char test_payload[] = { 0x42 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    result = uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_043: [ If the uws instance is not OPEN (open has not been called or is still in progress) then uws_client_send_frame_async shall fail and return a non-zero value. ]*/
/* Tests_SRS_UWS_CLIENT_01_146: [ A data frame MAY be transmitted by either the client or the server at any time after opening handshake completion and before that endpoint has sent a Close frame (Section 5.5.1). ]*/
TEST_FUNCTION(uws_client_send_frame_async_when_waiting_for_upgrade_response_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    int result;
    unsigned char test_payload[] = { 0x42 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    // act
    result = uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_038: [ uws_client_send_frame_async shall create and queue a structure that contains: ]*/
/* Tests_SRS_UWS_CLIENT_01_039: [ - the encoded websocket frame, so that the frame can be later sent when uws_client_dowork is called ]*/
/* Tests_SRS_UWS_CLIENT_01_040: [ - the send complete callback on_ws_send_frame_complete ]*/
/* Tests_SRS_UWS_CLIENT_01_056: [ - the send_complete callback shall be the on_underlying_io_send_complete function. ]*/
/* Tests_SRS_UWS_CLIENT_01_042: [ On success, uws_client_send_frame_async shall return 0. ]*/
/* Tests_SRS_UWS_CLIENT_01_425: [ Encoding shall be done by calling uws_frame_encoder_encode and passing to it the buffer and size argument for payload, the is_final flag and setting is_masked to true. ]*/
/* Tests_SRS_UWS_CLIENT_01_428: [ The encoded frame buffer memory shall be obtained by calling BUFFER_u_char on the encode buffer. ]*/
/* Tests_SRS_UWS_CLIENT_01_429: [ The encoded frame size shall be obtained by calling BUFFER_length on the encode buffer. ]*/
/* Tests_SRS_UWS_CLIENT_01_048: [ Queueing shall be done by calling singlylinkedlist_add. ]*/
/* Tests_SRS_UWS_CLIENT_01_038: [ uws_client_send_frame_async shall create and queue a structure that contains: ]*/
/* Tests_SRS_UWS_CLIENT_01_040: [ - the send complete callback on_ws_send_frame_complete ]*/
/* Tests_SRS_UWS_CLIENT_01_041: [ - the send complete callback context on_ws_send_frame_complete_context ]*/
/* Tests_SRS_UWS_CLIENT_01_140: [ To avoid confusing network intermediaries (such as intercepting proxies) and for security reasons that are further discussed in Section 10.3, a client MUST mask all frames that it sends to the server (see Section 5.3 for further details). ]*/
/* Tests_SRS_UWS_CLIENT_01_146: [ A data frame MAY be transmitted by either the client or the server at any time after opening handshake completion and before that endpoint has sent a Close frame (Section 5.5.1). ]*/
/* Tests_SRS_UWS_CLIENT_01_270: [ An endpoint MUST encapsulate the /data/ in a WebSocket frame as defined in Section 5.2. ]*/
/* Tests_SRS_UWS_CLIENT_01_274: [ If the data is being sent by the client, the frame(s) MUST be masked as defined in Section 5.3. ]*/
/* Tests_SRS_UWS_CLIENT_01_276: [ The frame(s) that have been formed MUST be transmitted over the underlying network connection. ]*/
TEST_FUNCTION(uws_client_send_frame_async_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };
    unsigned char encoded_frame[] = { 0x82, 0x01, 0x00, 0x00, 0x00, 0x00, 0x42 };
    int result;
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_BINARY_FRAME, test_payload, sizeof(test_payload), true, true, 0))
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(encoded_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(encoded_frame));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item();
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, sizeof(encoded_frame), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_send_complete()
        .IgnoreArgument_callback_context()
        .ValidateArgumentBuffer(2, encoded_frame, sizeof(encoded_frame));
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);

    // act
    result = uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_272: [ The opcode (frame-opcode) of the first frame containing the data MUST be set to the appropriate value from Section 5.2 for data that is to be interpreted by the recipient as text or binary data. ]*/
TEST_FUNCTION(uws_send_text_frame_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 'a' };
    unsigned char encoded_frame[] = { 0x82, 0x01, 0x00, 0x00, 0x00, 0x00, 'a' };
    int result;
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_TEXT_FRAME, test_payload, sizeof(test_payload), true, true, 0))
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(encoded_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(encoded_frame));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item();
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, sizeof(encoded_frame), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_send_complete()
        .IgnoreArgument_callback_context()
        .ValidateArgumentBuffer(2, encoded_frame, sizeof(encoded_frame));
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);

    // act
    result = uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_TEXT, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_047: [ If allocating memory for the newly queued item fails, uws_client_send_frame_async shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_new_sent_item_fails_uws_client_send_frame_async_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    result = uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_426: [ If uws_frame_encoder_encode fails, uws_client_send_frame_async shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_encoding_the_frame_fails_uws_client_send_frame_async_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_BINARY_FRAME, test_payload, sizeof(test_payload), true, true, 0))
        .SetReturn(NULL);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_058: [ If xio_send fails, uws_client_send_frame_async shall fail and return a non-zero value. ]*/
/* Tests_SRS_UWS_CLIENT_09_001: [ If xio_send fails and the message is still queued, it shall be de-queued and destroyed. ] */
TEST_FUNCTION(when_xio_send_fails_uws_client_send_frame_async_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };
    unsigned char encoded_frame[] = { 0x82, 0x01, 0x00, 0x00, 0x00, 0x00, 0x42 };
    int result;
    BUFFER_HANDLE buffer_handle;
    LIST_ITEM_HANDLE new_item_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_BINARY_FRAME, test_payload, sizeof(test_payload), true, true, 0))
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(encoded_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(encoded_frame));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item()
        .CaptureReturn(&new_item_handle);
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, sizeof(encoded_frame), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_send_complete()
        .IgnoreArgument_callback_context()
        .ValidateArgumentBuffer(2, encoded_frame, sizeof(encoded_frame))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn((LIST_ITEM_HANDLE)0x1234);
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .ValidateArgumentValue_item_handle(&new_item_handle);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);

    // act
    result = uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_09_001: [ If xio_send fails and the message is still queued, it shall be de-queued and destroyed. ] */
TEST_FUNCTION(when_xio_send_fails_uws_client_send_frame_async_fails_message_removed_by_xio_send)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };
    unsigned char encoded_frame[] = { 0x82, 0x01, 0x00, 0x00, 0x00, 0x00, 0x42 };
    int result;
    BUFFER_HANDLE buffer_handle;
    LIST_ITEM_HANDLE new_item_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_BINARY_FRAME, test_payload, sizeof(test_payload), true, true, 0))
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(encoded_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(encoded_frame));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item()
        .CaptureReturn(&new_item_handle);
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, sizeof(encoded_frame), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_send_complete()
        .IgnoreArgument_callback_context()
        .ValidateArgumentBuffer(2, encoded_frame, sizeof(encoded_frame));
    STRICT_EXPECTED_CALL(singlylinkedlist_find(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);

    // section for on_io_send_complete()
    g_xio_send_result = 1;
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(test_on_ws_send_frame_complete(IGNORED_PTR_ARG, WS_SEND_FRAME_ERROR));
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    // act
    result = uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);
    g_on_io_send_complete(g_on_io_send_complete_context, IO_SEND_ERROR);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_049: [ If singlylinkedlist_add fails, uws_client_send_frame_async shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_adding_the_item_to_the_list_fails_uws_client_send_frame_async_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };
    unsigned char encoded_frame[] = { 0x82, 0x01, 0x00, 0x00, 0x00, 0x00, 0x42 };
    int result;
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_BINARY_FRAME, test_payload, sizeof(test_payload), true, true, 0))
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(encoded_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(encoded_frame));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item()
        .SetReturn(NULL);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);

    // act
    result = uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4248);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_050: [ The argument on_ws_send_frame_complete shall be optional, if NULL is passed by the caller then no send complete callback shall be triggered. ]*/
TEST_FUNCTION(uws_client_send_frame_async_with_NULL_complete_callback_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };
    unsigned char encoded_frame[] = { 0x82, 0x01, 0x00, 0x00, 0x00, 0x00, 0x42 };
    int result;
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_BINARY_FRAME, test_payload, sizeof(test_payload), true, true, 0))
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(encoded_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(encoded_frame));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item();
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, sizeof(encoded_frame), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_send_complete()
        .IgnoreArgument_callback_context()
        .ValidateArgumentBuffer(2, encoded_frame, sizeof(encoded_frame));
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);

    // act
    result = uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, test_payload, sizeof(test_payload), true, NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* on_underlying_io_send_complete */

/* Tests_SRS_UWS_CLIENT_01_389: [ When on_underlying_io_send_complete is called with IO_SEND_OK as a result of sending a WebSocket frame to the underlying IO, the send shall be indicated to the uws user by calling on_ws_send_frame_complete with WS_SEND_FRAME_OK. ]*/
/* Tests_SRS_UWS_CLIENT_01_432: [ The indicated sent frame shall be removed from the list by calling singlylinkedlist_remove. ]*/
/* Tests_SRS_UWS_CLIENT_01_434: [ The memory associated with the sent frame shall be freed. ]*/
TEST_FUNCTION(on_underlying_io_send_complete_with_OK_indicates_the_frame_as_sent_OK)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    (void)uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item_handle();
    STRICT_EXPECTED_CALL(test_on_ws_send_frame_complete((void*)0x4245, WS_SEND_FRAME_OK));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    g_on_io_send_complete(g_on_io_send_complete_context, IO_SEND_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_433: [ If singlylinkedlist_remove fails an error shall be indicated by calling the on_ws_error callback with WS_ERROR_CANNOT_REMOVE_SENT_ITEM_FROM_LIST. ]*/
TEST_FUNCTION(when_removing_the_sent_framefrom_the_list_fails_then_an_error_is_indicated)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    (void)uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item_handle()
        .SetReturn(1);
    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_CANNOT_REMOVE_SENT_ITEM_FROM_LIST));

    // act
    g_on_io_send_complete(g_on_io_send_complete_context, IO_SEND_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_390: [ When on_underlying_io_send_complete is called with IO_SEND_ERROR as a result of sending a WebSocket frame to the underlying IO, the send shall be indicated to the uws user by calling on_ws_send_frame_complete with WS_SEND_FRAME_ERROR. ]*/
TEST_FUNCTION(on_underlying_io_send_complete_with_ERROR_indicates_the_frame_with_WS_SEND_ERROR)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    (void)uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item_handle();
    STRICT_EXPECTED_CALL(test_on_ws_send_frame_complete((void*)0x4245, WS_SEND_FRAME_ERROR));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    g_on_io_send_complete(g_on_io_send_complete_context, IO_SEND_ERROR);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_391: [ When on_underlying_io_send_complete is called with IO_SEND_CANCELLED as a result of sending a WebSocket frame to the underlying IO, the send shall be indicated to the uws user by calling on_ws_send_frame_complete with WS_SEND_FRAME_CANCELLED. ]*/
TEST_FUNCTION(on_underlying_io_send_complete_with_CANCELLED_indicates_the_frame_with_WS_SEND_CANCELLED)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    (void)uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item_handle();
    STRICT_EXPECTED_CALL(test_on_ws_send_frame_complete((void*)0x4245, WS_SEND_FRAME_CANCELLED));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    g_on_io_send_complete(g_on_io_send_complete_context, IO_SEND_CANCELLED);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_435: [ When on_underlying_io_send_complete is called with a NULL context, it shall do nothing. ]*/
TEST_FUNCTION(on_underlying_io_send_complete_with_NULL_context_does_nothing)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    (void)uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    // act
    g_on_io_send_complete(NULL, IO_SEND_CANCELLED);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_436: [ When on_underlying_io_send_complete is called with any other error code, the send shall be indicated to the uws user by calling on_ws_send_frame_complete with WS_SEND_FRAME_ERROR. ]*/
TEST_FUNCTION(on_underlying_io_send_complete_with_an_unknown_result_indicates_an_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char test_payload[] = { 0x42 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response));
    (void)uws_client_send_frame_async(uws_client, WS_FRAME_TYPE_BINARY, test_payload, sizeof(test_payload), true, test_on_ws_send_frame_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE, IGNORED_PTR_ARG))
        .IgnoreArgument_item_handle();
    STRICT_EXPECTED_CALL(test_on_ws_send_frame_complete((void*)0x4245, WS_SEND_FRAME_ERROR));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    g_on_io_send_complete(g_on_io_send_complete_context, (IO_SEND_RESULT)0x42);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* uws_client_dowork */

/* Tests_SRS_UWS_CLIENT_01_059: [ If the uws_client argument is NULL, uws_client_dowork shall do nothing. ]*/
TEST_FUNCTION(uws_client_dowork_with_NULL_handle_does_nothing)
{
    // arrange

    // act
    uws_client_dowork(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_430: [ uws_client_dowork shall call xio_dowork with the IO handle argument set to the underlying IO created in uws_client_create. ]*/
TEST_FUNCTION(uws_client_dowork_calls_the_underlying_io_dowork)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_dowork(TEST_IO_HANDLE));

    // act
    uws_client_dowork(uws_client);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_060: [ If the IO is not yet open, uws_client_dowork shall do nothing. ]*/
TEST_FUNCTION(uws_client_dowork_when_closed_does_nothing)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    // act
    uws_client_dowork(uws_client);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* on_underlying_io_error */

/* Tests_SRS_UWS_CLIENT_01_375: [ When on_underlying_io_error is called while uws is OPENING, uws shall report that the open failed by calling the on_ws_open_complete callback passed to uws_client_open_async with WS_OPEN_ERROR_UNDERLYING_IO_ERROR. ]*/
/* Tests_SRS_UWS_CLIENT_01_077: [ If this fails (e.g., the server's certificate could not be verified), then the client MUST _Fail the WebSocket Connection_ and abort the connection. ]*/
TEST_FUNCTION(on_underlying_io_error_while_opening_underlying_io_indicates_an_open_complete_with_WS_OPEN_ERROR_UNDERLYING_IO_ERROR)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_UNDERLYING_IO_ERROR));

    // act
    g_on_io_error(g_on_io_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_375: [ When on_underlying_io_error is called while uws is OPENING, uws shall report that the open failed by calling the on_ws_open_complete callback passed to uws_client_open_async with WS_OPEN_ERROR_UNDERLYING_IO_ERROR. ]*/
TEST_FUNCTION(on_underlying_io_error_while_waiting_for_upgrade_response_indicates_an_open_complete_with_WS_OPEN_ERROR_UNDERLYING_IO_ERROR)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_ws_open_complete((void*)0x4242, WS_OPEN_ERROR_UNDERLYING_IO_ERROR));

    // act
    g_on_io_error(g_on_io_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_376: [ When on_underlying_io_error is called while the uws instance is OPEN, an error shall be reported to the user by calling the on_ws_error callback that was passed to uws_client_open_async with the argument WS_ERROR_UNDERLYING_IO_ERROR. ]*/
/* Tests_SRS_UWS_CLIENT_01_318: [ Servers MAY close the WebSocket connection whenever desired. ]*/
/* Tests_SRS_UWS_CLIENT_01_269: [ If at any point the state of the WebSocket connection changes, the endpoint MUST abort the following steps. ]*/
TEST_FUNCTION(on_underlying_io_error_while_OPEN_indicates_an_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_ws_error((void*)0x4244, WS_ERROR_UNDERLYING_IO_ERROR));

    // act
    g_on_io_error(g_on_io_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_377: [ When on_underlying_io_error is called while the uws instance is CLOSING the underlying IO shall be closed by calling xio_close. ]*/
TEST_FUNCTION(on_underlying_io_error_while_CLOSING_indicates_an_error)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char close_frame[] = { 0x88, 0x00 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));

    // act
    g_on_io_error(g_on_io_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_377: [ When on_underlying_io_error is called while the uws instance is CLOSING the underlying IO shall be closed by calling xio_close. ]*/
TEST_FUNCTION(open_after_error_during_sending_close_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char close_frame[] = { 0x88, 0x00 };
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));
    g_on_io_error(g_on_io_error_context);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_377: [ When on_underlying_io_error is called while the uws instance is CLOSING the underlying IO shall be closed by calling xio_close. ]*/
/* Tests_SRS_UWS_CLIENT_01_499: [ If the CLOSE was due to the peer closing, the callback on_ws_close_complete shall not be called. ]*/
TEST_FUNCTION(on_underlying_io_error_while_CLOSING_underlying_io_indicates_the_close)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char close_frame[] = { 0x88, 0x00 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));
    g_on_io_send_complete(g_on_io_send_complete_context, IO_SEND_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_callback_context()
        .IgnoreArgument_on_io_close_complete();

    // act
    g_on_io_error(g_on_io_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_377: [ When on_underlying_io_error is called while the uws instance is CLOSING the underlying IO shall be closed by calling xio_close. ]*/
TEST_FUNCTION(open_after_error_during_closing_underlying_io_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char close_frame[] = { 0x88, 0x00 };
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));
    g_on_io_send_complete(g_on_io_send_complete_context, IO_SEND_OK);
    g_on_io_error(g_on_io_error_context);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_500: [ If xio_close fails then the callback on_ws_close_complete shall be called, while passing the on_ws_close_complete_context argument to it. ]*/
/* Tests_SRS_UWS_CLIENT_01_500: [ The callback on_ws_close_complete shall be called, while passing the on_ws_close_complete_context argument to it. ]*/
TEST_FUNCTION(on_underlying_io_error_while_CLOSING_due_to_local_initiated_close)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    (void)uws_client_close_handshake_async(uws_client, 1002, "", test_on_ws_close_complete, (void*)0x6666);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_close_complete()
        .IgnoreArgument_callback_context()
        .SetReturn(1);
    STRICT_EXPECTED_CALL(test_on_ws_close_complete((void*)0x6666));

    // act
    g_on_io_error(g_on_io_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* on_underlying_io_close_sent */

/* Tests_SRS_UWS_CLIENT_01_489: [ When on_underlying_io_close_sent is called with NULL context, it shall do nothing. ]*/
TEST_FUNCTION(on_underlying_io_close_sent_with_NULL_context_does_nothing)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char close_frame[] = { 0x88, 0x00 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));
    umock_c_reset_all_calls();

    // act
    g_on_io_send_complete(NULL, IO_SEND_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_490: [ When on_underlying_io_close_sent is called while the uws client is CLOSING, on_underlying_io_close_sent shall close the underlying IO by calling xio_close. ]*/
TEST_FUNCTION(on_underlying_io_close_sent_when_a_CLOSE_was_sent_closes_the_underlying_IO)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char close_frame[] = { 0x88, 0x00 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_close_complete()
        .IgnoreArgument_callback_context();

    // act
    g_on_io_send_complete(g_on_io_send_complete_context, IO_SEND_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_493: [ If calling xio_close fails, the state of the uws client shall be considered CLOSED and the on_ws_close_complete shall be called if it was specified. ]*/
/* Tests_SRS_UWS_CLIENT_01_496: [ If the close was initiated by the peer no on_ws_close_complete shall be called. ]*/
TEST_FUNCTION(when_xio_close_fails_in_on_underlying_io_close_sent_and_CLOSE_initiated_by_peer_no_callback_is_triggered)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char close_frame[] = { 0x88, 0x00 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_close_complete()
        .IgnoreArgument_callback_context()
        .SetReturn(1);

    // act
    g_on_io_send_complete(g_on_io_send_complete_context, IO_SEND_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_493: [ If calling xio_close fails, the state of the uws client shall be considered CLOSED and the on_ws_close_complete shall be called if it was specified. ]*/
/* Tests_SRS_UWS_CLIENT_01_496: [ If the close was initiated by the peer no on_ws_close_complete shall be called. ]*/
TEST_FUNCTION(when_xio_close_fails_in_on_underlying_io_close_sent_and_CLOSE_initiated_by_peer_no_callback_is_triggered_and_next_open_succeeds)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char close_frame[] = { 0x88, 0x00 };
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_close_complete()
        .IgnoreArgument_callback_context()
        .SetReturn(1);
    g_on_io_send_complete(g_on_io_send_complete_context, IO_SEND_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Ping frame */

/* Tests_SRS_UWS_CLIENT_01_157: [ *  %x9 denotes a ping ]*/
/* Tests_SRS_UWS_CLIENT_01_249: [ Upon receipt of a Ping frame, an endpoint MUST send a Pong frame in response ]*/
/* Tests_SRS_UWS_CLIENT_01_250: [ It SHOULD respond with Pong frame as soon as is practical. ]*/
/* Tests_SRS_UWS_CLIENT_01_253: [ A Pong frame sent in response to a Ping frame must have identical "Application data" as found in the message body of the Ping frame being replied to. ]*/
/* Tests_SRS_UWS_CLIENT_01_247: [ The Ping frame contains an opcode of 0x9. ]*/
/* Tests_SRS_UWS_CLIENT_01_251: [ An endpoint MAY send a Ping frame any time after the connection is established and before the connection is closed. ]*/
TEST_FUNCTION(when_a_PING_frame_was_received_a_PONG_frame_is_sent)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char ping_frame[] = { 0x89, 0x00 };
    unsigned char pong_frame[] = { 0x8A, 0x00, 0x00, 0x00, 0x00, 0x00 };
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_PONG_FRAME, IGNORED_PTR_ARG, 0, true, true, 0))
        .IgnoreArgument_payload()
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(pong_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(pong_frame));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, pong_frame, sizeof(pong_frame), IGNORED_PTR_ARG, NULL))
        .ValidateArgumentBuffer(2, pong_frame, sizeof(pong_frame));
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)ping_frame, sizeof(ping_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_253: [ A Pong frame sent in response to a Ping frame must have identical "Application data" as found in the message body of the Ping frame being replied to. ]*/
/* Tests_SRS_UWS_CLIENT_01_248: [ A Ping frame MAY include "Application data". ]*/
TEST_FUNCTION(when_a_PING_frame_was_received_with_some_payload_a_PONG_frame_is_sent_with_the_Same_payload)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char ping_frame[] = { 0x89, 0x02, 0x42, 0x43 };
    unsigned char pong_frame_payload[] = { 0x42, 0x43 };
    unsigned char pong_frame[] = { 0x8A, 0x02, 0x00, 0x00, 0x00, 0x00, 0x42, 0x43 };
    BUFFER_HANDLE buffer_handle;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(BUFFER_new())
        .CaptureReturn(&buffer_handle);

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_PONG_FRAME, pong_frame_payload, sizeof(pong_frame_payload), true, true, 0))
        .ValidateArgumentBuffer(2, pong_frame_payload, sizeof(pong_frame_payload))
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(pong_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(pong_frame));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, pong_frame, sizeof(pong_frame), IGNORED_PTR_ARG, NULL))
        .ValidateArgumentBuffer(2, pong_frame, sizeof(pong_frame));
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)ping_frame, sizeof(ping_frame));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_251: [ An endpoint MAY send a Ping frame any time after the connection is established and before the connection is closed. ]*/
/* Tests_SRS_UWS_CLIENT_01_240: [ The application MUST NOT send any more data frames after sending a Close frame. ]*/
TEST_FUNCTION(when_a_PING_frame_is_received_after_a_close_frame_no_pong_is_sent)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    const unsigned char close_and_ping_frames[] = { 0x88, 0x00, 0x89, 0x02, 0x42, 0x43 };
    BUFFER_HANDLE buffer_handle;
    unsigned char sent_close_frame[] = { 0x88, 0x80, 0x00, 0x00, 0x00, 0x00 };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_CLOSE_FRAME, NULL, 0, true, true, 0))
        .CaptureReturn(&buffer_handle);
    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sent_close_frame);
    STRICT_EXPECTED_CALL(BUFFER_length(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle)
        .SetReturn(sizeof(sent_close_frame));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, sent_close_frame, sizeof(sent_close_frame), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(2, sent_close_frame, sizeof(sent_close_frame))
        .IgnoreArgument_callback_context()
        .IgnoreArgument_on_send_complete();
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG))
        .ValidateArgumentValue_handle(&buffer_handle);
    EXPECTED_CALL(test_on_ws_peer_closed(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, 0));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)close_and_ping_frames, sizeof(close_and_ping_frames));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* uws_setoption */

/* Tests_SRS_UWS_CLIENT_01_440: [ If any of the arguments uws_client or option_name is NULL uws_client_set_option shall return a non-zero value. ]*/
TEST_FUNCTION(uws_set_option_with_NULL_uws_handle_fails)
{
    // arrange
    int result;

    // act
    result = uws_client_set_option(NULL, "test_option", (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_440: [ If any of the arguments uws_client or option_name is NULL uws_client_set_option shall return a non-zero value. ]*/
TEST_FUNCTION(uws_set_option_with_NULL_option_name_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;
    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    // act
    result = uws_client_set_option(uws_client, NULL, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_510: [ If the option name is uWSClientOptions then uws_client_set_option shall call OptionHandler_FeedOptions and pass to it the underlying IO handle and the value argument. ]*/
/* Tests_SRS_UWS_CLIENT_01_442: [ On success, uws_client_set_option shall return 0. ]*/
TEST_FUNCTION(uws_set_option_with_uws_client_options_calls_OptionHandler_FeedOptions)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;
    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(OptionHandler_FeedOptions((OPTIONHANDLER_HANDLE)0x4242, TEST_IO_HANDLE));

    // act
    result = uws_client_set_option(uws_client, "uWSClientOptions", (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_511: [ If OptionHandler_FeedOptions fails, uws_client_set_option shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_OptionHandler_FeedOptions_fails_then_uws_set_option_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;
    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(OptionHandler_FeedOptions((OPTIONHANDLER_HANDLE)0x4242, TEST_IO_HANDLE))
        .SetReturn(OPTIONHANDLER_ERROR);

    // act
    result = uws_client_set_option(uws_client, "uWSClientOptions", (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_441: [ Otherwise all options shall be passed as they are to the underlying IO by calling xio_setoption. ]*/
/* Tests_SRS_UWS_CLIENT_01_442: [ On success, uws_client_set_option shall return 0. ]*/
TEST_FUNCTION(uws_set_option_passes_the_option_down)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;
    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_setoption(TEST_IO_HANDLE, "option1", (void*)0x4242));

    // act
    result = uws_client_set_option(uws_client, "option1", (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_443: [ If xio_setoption fails, uws_client_set_option shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_xio_setoption_fails_then_uws_set_option_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;
    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_setoption(TEST_IO_HANDLE, "option1", (void*)0x4242))
        .SetReturn(1);

    // act
    result = uws_client_set_option(uws_client, "option1", (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* uws_client_retrieve_options */

/* Tests_SRS_UWS_CLIENT_01_444: [ If parameter uws_client is NULL then uws_client_retrieve_options shall fail and return NULL. ]*/
TEST_FUNCTION(uws_retrieve_options_with_NULL_handle_fails)
{
    // arrange
    OPTIONHANDLER_HANDLE result;

    // act
    result = uws_client_retrieve_options(NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_UWS_CLIENT_01_445: [ uws_client_retrieve_options shall call OptionHandler_Create to produce an OPTIONHANDLER_HANDLE and on success return the new OPTIONHANDLER_HANDLE handle. ]*/
/* Tests_SRS_UWS_CLIENT_01_501: [ uws_client_retrieve_options shall add to the option handler one option, whose name shall be uWSClientOptions and the value shall be queried by calling xio_retrieveoptions. ]*/
/* Tests_SRS_UWS_CLIENT_01_502: [ When calling xio_retrieveoptions the underlying IO handle shall be passed to it. ]*/
/* Tests_SRS_UWS_CLIENT_01_504: [ Adding the option shall be done by calling OptionHandler_AddOption. ]*/
TEST_FUNCTION(uws_retrieve_options_calls_the_underlying_xio_retrieve_options_and_returns_the_a_new_option_handler_instance)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    OPTIONHANDLER_HANDLE result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;
    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_retrieveoptions(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(OptionHandler_AddOption(TEST_OPTIONHANDLER_HANDLE, "uWSClientOptions", TEST_IO_OPTIONHANDLER_HANDLE));
    STRICT_EXPECTED_CALL(OptionHandler_Destroy(IGNORED_PTR_ARG));

    // act
    result = uws_client_retrieve_options(uws_client);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, TEST_OPTIONHANDLER_HANDLE, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_446: [ If OptionHandler_Create fails then uws_client_retrieve_options shall fail and return NULL. ]*/
TEST_FUNCTION(when_OptionHandler_Create_fails_then_uws_retrieve_options_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    OPTIONHANDLER_HANDLE result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;
    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(NULL);

    // act
    result = uws_client_retrieve_options(uws_client);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_503: [ If xio_retrieveoptions fails, uws_client_retrieve_options shall fail and return NULL. ]*/
TEST_FUNCTION(when_xio_retrieveoptions_fails_then_uws_retrieve_options_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    OPTIONHANDLER_HANDLE result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;
    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_retrieveoptions(TEST_IO_HANDLE))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(OptionHandler_Destroy(TEST_OPTIONHANDLER_HANDLE));

    // act
    result = uws_client_retrieve_options(uws_client);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_505: [ If OptionHandler_AddOption fails, uws_client_retrieve_options shall fail and return NULL. ]*/
TEST_FUNCTION(when_OptionHandler_AddOption_fails_then_uws_retrieve_options_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    OPTIONHANDLER_HANDLE result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;
    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    umock_c_reset_all_calls();

    EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_retrieveoptions(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(OptionHandler_AddOption(TEST_OPTIONHANDLER_HANDLE, "uWSClientOptions", TEST_IO_OPTIONHANDLER_HANDLE))
        .SetReturn(OPTIONHANDLER_ERROR);
    STRICT_EXPECTED_CALL(OptionHandler_Destroy(TEST_OPTIONHANDLER_HANDLE));
    STRICT_EXPECTED_CALL(OptionHandler_Destroy(TEST_IO_OPTIONHANDLER_HANDLE));

    // act
    result = uws_client_retrieve_options(uws_client);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* uws_client_clone_option */

/* Tests_SRS_UWS_CLIENT_01_507: [ uws_client_clone_option called with name being uWSClientOptions shall return the same value. ]*/
TEST_FUNCTION(uws_client_clone_option_calls_xio_cloneoption)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    void* result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;
    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_retrieve_options(uws_client);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(OptionHandler_Clone(IGNORED_PTR_ARG));

    // act
    result = g_clone_option("uWSClientOptions", (void*)0x4243);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, (void*)0x4447, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_512: [ uws_client_clone_option called with any other option name than uWSClientOptions shall return NULL. ]*/
TEST_FUNCTION(uws_client_clone_with_an_unknown_option_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    void* result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;
    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_retrieve_options(uws_client);
    umock_c_reset_all_calls();

    // act
    result = g_clone_option("TrustedCerts", (void*)0x4243);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_506: [ If uws_client_clone_option is called with NULL name or value it shall return NULL. ]*/
TEST_FUNCTION(uws_client_clone_option_with_NULL_name_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    void* result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;
    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_retrieve_options(uws_client);
    umock_c_reset_all_calls();

    // act
    result = g_clone_option(NULL, (void*)0x4243);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_506: [ If uws_client_clone_option is called with NULL name or value it shall return NULL. ]*/
TEST_FUNCTION(uws_client_clone_option_with_NULL_value_fails)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    void* result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;
    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_retrieve_options(uws_client);
    umock_c_reset_all_calls();

    // act
    result = g_clone_option("uWSClientOptions", NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* uws_client_destroy_option */

/* Tests_SRS_UWS_CLIENT_01_509: [ If uws_client_destroy_option is called with NULL name or value it shall do nothing. ]*/
TEST_FUNCTION(uws_client_destroy_option_with_NULL_name_does_no_destroy)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;
    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_retrieve_options(uws_client);
    umock_c_reset_all_calls();

    // act
    g_destroy_option(NULL, (void*)0x4243);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_509: [ If uws_client_destroy_option is called with NULL name or value it shall do nothing. ]*/
TEST_FUNCTION(uws_client_destroy_option_with_NULL_value_does_no_destroy)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;
    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_retrieve_options(uws_client);
    umock_c_reset_all_calls();

    // act
    g_destroy_option("uWSClientOptions", NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_513: [ If uws_client_destroy_option is called with any other name it shall do nothing. ]*/
TEST_FUNCTION(uws_client_destroy_option_with_an_unknown_option_does_no_destroy)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;
    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_retrieve_options(uws_client);
    umock_c_reset_all_calls();

    // act
    g_destroy_option("TrustedCerts", (void*)0x4243);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_508: [ uws_client_destroy_option called with the option name being uWSClientOptions shall destroy the value by calling OptionHandler_Destroy. ]*/
TEST_FUNCTION(uws_client_destroy_option_with_uWSClientOptions_calls_OptionHandler_Destroy)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;
    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_retrieve_options(uws_client);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(OptionHandler_Destroy(TEST_OPTIONHANDLER_HANDLE));

    // act
    g_destroy_option("uWSClientOptions", TEST_OPTIONHANDLER_HANDLE);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* on_underlying_io_close_complete */

/* Tests_SRS_UWS_CLIENT_01_475: [ When on_underlying_io_close_complete is called while closing the underlying IO a subsequent uws_client_open_async shall succeed. ]*/
TEST_FUNCTION(underlying_io_close_after_a_send_close_frame_failed_puts_the_uws_in_closed_state_and_a_new_open_is_allowed)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char close_frame[] = { 0x88, 0x02, 0x03, 0xEA };
    uint16_t expected_close_code = 1002;
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_CLOSE_FRAME, NULL, 0, true, true, 0))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_callback_context()
        .IgnoreArgument_on_io_close_complete();
    STRICT_EXPECTED_CALL(test_on_ws_peer_closed((void*)0x4301, IGNORED_PTR_ARG, IGNORED_PTR_ARG, 0))
        .ValidateArgumentBuffer(2, &expected_close_code, sizeof(expected_close_code))
        .IgnoreArgument_extra_data();

    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));
    g_on_io_close_complete(g_on_io_close_complete_context);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_476: [ If the close is as a result of receiving a CLOSE frame, no callback shall be called. ]*/
TEST_FUNCTION(underlying_io_close_due_to_CLOSE_frame_being_received_doe_not_trigger_a_user_callback)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char close_frame[] = { 0x88, 0x02, 0x03, 0xEA };
    uint16_t expected_close_code = 1002;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_CLOSE_FRAME, NULL, 0, true, true, 0))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_callback_context()
        .IgnoreArgument_on_io_close_complete();
    STRICT_EXPECTED_CALL(test_on_ws_peer_closed((void*)0x4301, IGNORED_PTR_ARG, IGNORED_PTR_ARG, 0))
        .ValidateArgumentBuffer(2, &expected_close_code, sizeof(expected_close_code))
        .IgnoreArgument_extra_data();

    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));
    umock_c_reset_all_calls();

    // act
    g_on_io_close_complete(g_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_477: [ When on_underlying_io_close_complete is called with a NULL context, it shall do nothing. ]*/
TEST_FUNCTION(underlying_io_close_complete_with_NULL_context_does_nothing)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char close_frame[] = { 0x88, 0x02, 0x03, 0xEA };
    uint16_t expected_close_code = 1002;
    int result;

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uws_frame_encoder_encode(WS_CLOSE_FRAME, NULL, 0, true, true, 0))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_callback_context()
        .IgnoreArgument_on_io_close_complete();
    STRICT_EXPECTED_CALL(test_on_ws_peer_closed((void*)0x4301, IGNORED_PTR_ARG, IGNORED_PTR_ARG, 0))
        .ValidateArgumentBuffer(2, &expected_close_code, sizeof(expected_close_code))
        .IgnoreArgument_extra_data();

    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));
    umock_c_reset_all_calls();
    g_on_io_close_complete(NULL);

    // act
    result = uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_387: [ When on_underlying_io_close_complete is called when the uws instance is closing, the on_ws_close_complete callback passed to uws_client_close_async shall be called. ]*/
TEST_FUNCTION(when_close_complete_is_called_the_user_callback_is_triggered)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char close_frame[] = { 0x88, 0x02, 0x03, 0xEA };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    (void)uws_client_close_handshake_async(uws_client, 1002, "", test_on_ws_close_complete, (void*)0x4444);
    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_ws_close_complete((void*)0x4444));

    // act
    g_on_io_close_complete(g_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

/* Tests_SRS_UWS_CLIENT_01_387: [ When on_underlying_io_close_complete is called when the uws instance is closing, the on_ws_close_complete callback passed to uws_client_close_async shall be called. ]*/
/* Tests_SRS_UWS_CLIENT_01_469: [ The on_ws_close_complete argument shall be allowed to be NULL, in which case no callback shall be called when the close is complete. ]*/
TEST_FUNCTION(when_close_complete_is_called_and_the_user_callback_is_NULL_no_callback_is_triggered)
{
    // arrange
    TLSIO_CONFIG tlsio_config;
    UWS_CLIENT_HANDLE uws_client;
    const char test_upgrade_response[] = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    unsigned char close_frame[] = { 0x88, 0x02, 0x03, 0xEA };

    tlsio_config.hostname = "test_host";
    tlsio_config.port = 444;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));
    (void)uws_client_open_async(uws_client, test_on_ws_open_complete, (void*)0x4242, test_on_ws_frame_received, (void*)0x4243, test_on_ws_peer_closed, (void*)0x4301, test_on_ws_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)test_upgrade_response, sizeof(test_upgrade_response) - 1);
    (void)uws_client_close_handshake_async(uws_client, 1002, "", NULL, NULL);
    g_on_bytes_received(g_on_bytes_received_context, close_frame, sizeof(close_frame));
    umock_c_reset_all_calls();

    // act
    g_on_io_close_complete(g_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    uws_client_destroy(uws_client);
}

// Tests_SRS_UWS_CLIENT_09_002: [ If any of the arguments uws_client or name or value is NULL uws_client_set_request_header shall fail and return a non-zero value. ]
TEST_FUNCTION(uws_client_set_request_header_NULL_handle)
{
    // arrange
    int result;
    char* req_header1_key = "Authorization";
    char* req_header1_value = "Bearer 23420939909809283488230949";

    umock_c_reset_all_calls();

    // act
    result = uws_client_set_request_header(NULL, req_header1_key, req_header1_value);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
}

// Tests_SRS_UWS_CLIENT_09_002: [ If any of the arguments uws_client or name or value is NULL uws_client_set_request_header shall fail and return a non-zero value. ]
TEST_FUNCTION(uws_client_set_request_header_NULL_name)
{
    // arrange
    UWS_CLIENT_HANDLE uws_client;
    int result;
    char* req_header1_key = NULL;
    char* req_header1_value = "Bearer 23420939909809283488230949";

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));

    umock_c_reset_all_calls();

    // act
    result = uws_client_set_request_header(uws_client, req_header1_key, req_header1_value);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    uws_client_destroy(uws_client);
}

// Tests_SRS_UWS_CLIENT_09_002: [ If any of the arguments uws_client or name or value is NULL uws_client_set_request_header shall fail and return a non-zero value. ]
TEST_FUNCTION(uws_client_set_request_header_NULL_value)
{
    // arrange
    UWS_CLIENT_HANDLE uws_client;
    int result;
    char* req_header1_key = "Authorization";
    char* req_header1_value = NULL;

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));

    umock_c_reset_all_calls();

    // act
    result = uws_client_set_request_header(uws_client, req_header1_key, req_header1_value);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    uws_client_destroy(uws_client);
}

// Tests_SRS_UWS_CLIENT_09_004: [ If name or value fail to be stored the function shall fail and return a non-zero value. ]
TEST_FUNCTION(uws_client_set_request_header_negative_tests)
{
    // arrange
    UWS_CLIENT_HANDLE uws_client;
    char* req_header1_key = "Authorization";
    char* req_header1_value = "Bearer 23420939909809283488230949";
    size_t i;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(Map_AddOrUpdate(TEST_REQUEST_HEADERS_MAP, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    umock_c_negative_tests_snapshot();

    for (i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        // arrange
        char error_msg[64];
        int result;

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        // act
        result = uws_client_set_request_header(uws_client, req_header1_key, req_header1_value);

        sprintf(error_msg, "On failed call %lu", (unsigned long)i);
        ASSERT_ARE_NOT_EQUAL(int, 0, result, error_msg);
    }

    // cleanup
    uws_client_destroy(uws_client);
    umock_c_negative_tests_deinit();
}

// Tests_SRS_UWS_CLIENT_09_003: [ A copy of name and value shall be stored for later sending in the request message. ]
// Tests_SRS_UWS_CLIENT_09_005: [ If no failures occur the function shall return zero. ]
TEST_FUNCTION(uws_client_set_request_header_success)
{
    // arrange
    UWS_CLIENT_HANDLE uws_client;
    int result;
    char* req_header1_key = "Authorization";
    char* req_header1_value = "Bearer 23420939909809283488230949";

    uws_client = uws_client_create("test_host", 444, "/aaa", true, protocols, sizeof(protocols) / sizeof(protocols[0]));

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(Map_AddOrUpdate(TEST_REQUEST_HEADERS_MAP, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = uws_client_set_request_header(uws_client, req_header1_key, req_header1_value);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    uws_client_destroy(uws_client);
}

END_TEST_SUITE(uws_client_ut)
