// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#else
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#endif
#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umockalloc.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_stdint.h"

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

#define ENABLE_MOCKS

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/singlylinkedlist.h"

#undef ENABLE_MOCKS

#include "azure_uamqp_c/header_detect_io.h"
#include "azure_uamqp_c/server_protocol_io.h"

static XIO_HANDLE test_underlying_amqp_io = (XIO_HANDLE)0x4242;
static const IO_INTERFACE_DESCRIPTION* test_detected_io_interface_description_1 = (const IO_INTERFACE_DESCRIPTION*)0x4243;
static XIO_HANDLE test_detected_io_1 = (XIO_HANDLE)0x4244;
static XIO_HANDLE test_detected_io_2 = (XIO_HANDLE)0x4245;
static OPTIONHANDLER_HANDLE test_option_handler = (OPTIONHANDLER_HANDLE)0x4246;
static SINGLYLINKEDLIST_HANDLE test_singlylinked_list = (SINGLYLINKEDLIST_HANDLE)0x4247;

static ON_BYTES_RECEIVED saved_on_bytes_received;
static void* saved_on_bytes_received_context;
static ON_IO_OPEN_COMPLETE saved_on_io_open_complete;
static void* saved_on_io_open_complete_context;
static ON_IO_ERROR saved_on_io_error;
static void* saved_on_io_error_context;
static ON_IO_CLOSE_COMPLETE saved_on_io_close_complete;
static void* saved_on_io_close_complete_context;
static ON_SEND_COMPLETE saved_on_send_complete;
static void* saved_on_send_complete_context;
static XIO_HANDLE xio_create_return;

static TEST_MUTEX_HANDLE g_testByTest;

TEST_DEFINE_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);

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

static char* umocktypes_stringify_const_SERVER_PROTOCOL_IO_CONFIG_ptr(const SERVER_PROTOCOL_IO_CONFIG** value)
{
    char* result = NULL;
    char temp_buffer[1024];
    int length;
    length = sprintf(temp_buffer, "{ underlying_io = %p }",
        (*value)->underlying_io);

    if (length > 0)
    {
        result = (char*)umockalloc_malloc(strlen(temp_buffer) + 1);
        if (result != NULL)
        {
            (void)memcpy(result, temp_buffer, strlen(temp_buffer) + 1);
        }
    }

    return result;
}

static int umocktypes_are_equal_const_SERVER_PROTOCOL_IO_CONFIG_ptr(const SERVER_PROTOCOL_IO_CONFIG** left, const SERVER_PROTOCOL_IO_CONFIG** right)
{
    int result;

    if ((left == NULL) ||
        (right == NULL))
    {
        result = -1;
    }
    else
    {
        result = ((*left)->underlying_io == (*right)->underlying_io);
    }

    return result;
}

static int umocktypes_copy_const_SERVER_PROTOCOL_IO_CONFIG_ptr(SERVER_PROTOCOL_IO_CONFIG** destination, const SERVER_PROTOCOL_IO_CONFIG** source)
{
    int result;

    *destination = (SERVER_PROTOCOL_IO_CONFIG*)umockalloc_malloc(sizeof(SERVER_PROTOCOL_IO_CONFIG));
    if (*destination == NULL)
    {
        result = MU_FAILURE;
    }
    else
    {
        (*destination)->underlying_io = (*source)->underlying_io;

        result = 0;
    }

    return result;
}

static void umocktypes_free_const_SERVER_PROTOCOL_IO_CONFIG_ptr(SERVER_PROTOCOL_IO_CONFIG** value)
{
    umockalloc_free(*value);
}

MOCK_FUNCTION_WITH_CODE(, void, test_on_io_open_complete, void*, context, IO_OPEN_RESULT, open_result)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_on_bytes_received, void*, context, const unsigned char*, buffer, size_t, size)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_detected_io_1_on_bytes_received, void*, context, const unsigned char*, buffer, size_t, size)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_detected_io_2_on_bytes_received, void*, context, const unsigned char*, buffer, size_t, size)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_on_io_error, void*, context)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_on_io_close_complete, void*, context)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_on_send_complete, void*, context, IO_SEND_RESULT, send_result)
MOCK_FUNCTION_END();

static SERVER_PROTOCOL_IO_CONFIG* server_protocol_io_config_detected_io;
static void* test_detected_io_1_on_bytes_received_context = (void*)0x5000;
static void* test_detected_io_2_on_bytes_received_context = (void*)0x5000;

XIO_HANDLE my_xio_create(const IO_INTERFACE_DESCRIPTION* io_interface_description, const void* xio_create_parameters)
{
    (void)io_interface_description;
    server_protocol_io_config_detected_io = (SERVER_PROTOCOL_IO_CONFIG*)xio_create_parameters;
    return xio_create_return;
}

static int my_xio_open(XIO_HANDLE io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    if (io == test_detected_io_1)
    {
        *server_protocol_io_config_detected_io->on_bytes_received = test_detected_io_1_on_bytes_received;
        *server_protocol_io_config_detected_io->on_bytes_received_context = test_detected_io_1_on_bytes_received_context;
    }

    if (io == test_detected_io_2)
    {
        *server_protocol_io_config_detected_io->on_bytes_received = test_detected_io_2_on_bytes_received;
        *server_protocol_io_config_detected_io->on_bytes_received_context = test_detected_io_2_on_bytes_received_context;
    }

    saved_on_bytes_received = on_bytes_received;
    saved_on_bytes_received_context = on_bytes_received_context;
    saved_on_io_open_complete = on_io_open_complete;
    saved_on_io_open_complete_context = on_io_open_complete_context;
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
    (void)xio;
    (void)buffer;
    (void)size;
    saved_on_send_complete = on_send_complete;
    saved_on_send_complete_context = callback_context;
    return 0;
}

static const void** list_items = NULL;
static size_t list_item_count = 0;
static int singlylinkedlist_remove_result;

static LIST_ITEM_HANDLE add_to_list(const void* item)
{
    const void** items = (const void**)umockalloc_realloc((void*)list_items, (list_item_count + 1) * sizeof(item));
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
        umockalloc_free((void*)list_items);
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

static LIST_ITEM_HANDLE my_singlylinkedlist_get_next_item(LIST_ITEM_HANDLE item)
{
    LIST_ITEM_HANDLE result = NULL;
    if ((size_t)item < list_item_count)
    {
        result = (LIST_ITEM_HANDLE)((size_t)item + 1);
    }
    else
    {
        result = NULL;
    }
    return result;
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

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(header_detect_io_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, my_gballoc_calloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_HOOK(xio_open, my_xio_open);
    REGISTER_GLOBAL_MOCK_HOOK(xio_close, my_xio_close);
    REGISTER_GLOBAL_MOCK_HOOK(xio_send, my_xio_send);
    REGISTER_GLOBAL_MOCK_HOOK(xio_create, my_xio_create);
    REGISTER_GLOBAL_MOCK_RETURN(OptionHandler_Create, test_option_handler);
    REGISTER_GLOBAL_MOCK_RETURN(singlylinkedlist_create, test_singlylinked_list);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_add, my_singlylinkedlist_add);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_remove, my_singlylinkedlist_remove);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_get_head_item, my_singlylinkedlist_get_head_item);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_get_next_item, my_singlylinkedlist_get_next_item);
    REGISTER_GLOBAL_MOCK_HOOK(singlylinkedlist_item_get_value, my_singlylinkedlist_item_get_value);

    REGISTER_UMOCK_ALIAS_TYPE(XIO_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_OPEN_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_RECEIVED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_ERROR, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_CLOSE_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_SEND_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfCloneOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfDestroyOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfSetOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(OPTIONHANDLER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SINGLYLINKEDLIST_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_ITEM_HANDLE, void*);

    REGISTER_TYPE(const SERVER_PROTOCOL_IO_CONFIG*, const_SERVER_PROTOCOL_IO_CONFIG_ptr);
    REGISTER_UMOCK_ALIAS_TYPE(SERVER_PROTOCOL_IO_CONFIG*, const SERVER_PROTOCOL_IO_CONFIG*);

    REGISTER_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT);
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

    xio_create_return = test_detected_io_1;
    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* header_detect_io_create */

/* Tests_SRS_HEADER_DETECT_IO_01_001: [ `header_detect_io_create` shall create a new header detect IO instance and on success it shall return a non-NULL handle to the newly created instance. ] */
/* Tests_SRS_HEADER_DETECT_IO_01_004: [ `io_create_parameters` shall be used as `HEADER_DETECT_IO_CONFIG*`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_009: [ The `header_detect_entries` array shall be copied so that it can be later used when detecting which header was received. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_014: [ For each entry in `header_detect_entries` the `header` field shall also be copied. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_009: [ The `header_detect_entries` array shall be copied so that it can be later used when detecting which header was received. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_060: [ `header_detect_io_create` shall create a singly linked list by calling `singlylinkedlist_create` where the chained detected IOs shall be stored. ]*/
TEST_FUNCTION(header_detect_io_create_with_valid_args_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes);
    header_detect_entries[0].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_create());

    // act
    result = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(result);
}

/* Tests_SRS_HEADER_DETECT_IO_01_001: [ `header_detect_io_create` shall create a new header detect IO instance and on success it shall return a non-NULL handle to the newly created instance. ] */
/* Tests_SRS_HEADER_DETECT_IO_01_004: [ `io_create_parameters` shall be used as `HEADER_DETECT_IO_CONFIG*`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_009: [ The `header_detect_entries` array shall be copied so that it can be later used when detecting which header was received. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_014: [ For each entry in `header_detect_entries` the `header` field shall also be copied. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_009: [ The `header_detect_entries` array shall be copied so that it can be later used when detecting which header was received. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_060: [ `header_detect_io_create` shall create a singly linked list by calling `singlylinkedlist_create` where the chained detected IOs shall be stored. ]*/
TEST_FUNCTION(header_detect_io_create_with_2_header_detect_entries_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[2];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG)); // array
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)); // first entry
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)); // second entry
    STRICT_EXPECTED_CALL(singlylinkedlist_create());

    // act
    result = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(result);
}

/* Tests_SRS_HEADER_DETECT_IO_01_002: [ If allocating memory for the header detect IO instance fails, `header_detect_io_create` shall fail and return NULL. ]*/
TEST_FUNCTION(when_allocating_memory_fails_header_detect_io_create_fails)
{
    // arrange
    CONCRETE_IO_HANDLE result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    result = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HEADER_DETECT_IO_01_003: [ If `io_create_parameters` is NULL, `header_detect_io_create` shall fail and return NULL. ]*/
TEST_FUNCTION(header_detect_io_create_with_NULL_io_create_parameters_fails)
{
    // arrange
    CONCRETE_IO_HANDLE result;

    // act
    result = header_detect_io_get_interface_description()->concrete_io_create(NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HEADER_DETECT_IO_01_005: [ If the member `header_detect_entry_count` of `HEADER_DETECT_IO_CONFIG` is 0 then `header_detect_io_create` shall fail and return NULL. ]*/
TEST_FUNCTION(header_detect_io_create_with_0_header_detect_entries_fails)
{
    // arrange
    CONCRETE_IO_HANDLE result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.underlying_io = test_underlying_amqp_io;
    header_detect_io_config.header_detect_entry_count = 0;
    header_detect_io_config.header_detect_entries = header_detect_entries;

    // act
    result = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HEADER_DETECT_IO_01_006: [ If the member `header_detect_entries` is NULL then `header_detect_io_create` shall fail and return NULL. ]*/
TEST_FUNCTION(header_detect_io_create_with_NULL_header_detect_entries_fails)
{
    // arrange
    CONCRETE_IO_HANDLE result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;

    header_detect_io_config.underlying_io = test_underlying_amqp_io;
    header_detect_io_config.header_detect_entries = NULL;
    header_detect_io_config.header_detect_entry_count = 1;

    // act
    result = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HEADER_DETECT_IO_01_007: [ If the member `underlying_io` is NULL then `header_detect_io_create` shall fail and return NULL. ]*/
TEST_FUNCTION(header_detect_io_create_with_NULL_underlying_io_fails)
{
    // arrange
    CONCRETE_IO_HANDLE result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;

    header_detect_io_config.underlying_io = NULL;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.header_detect_entry_count = 1;

    // act
    result = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HEADER_DETECT_IO_01_008: [ If the `header` member in the `header_detect_entries` is NULL then `header_detect_io_create` shall fail and return NULL. ]*/
TEST_FUNCTION(header_detect_io_create_with_NULL_header_in_a_header_entry_fails)
{
    // arrange
    CONCRETE_IO_HANDLE result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = NULL;
    header_detect_entries[0].header.header_size = 1;
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;

    header_detect_io_config.underlying_io = test_underlying_amqp_io;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.header_detect_entry_count = 1;

    // act
    result = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HEADER_DETECT_IO_01_010: [ If allocating memory for the `header_detect_entries` or its constituents fails then `header_detect_io_create` shall fail and return NULL. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_header_detect_entries_array_fails_header_detect_io_create_fails)
{
    // arrange
    CONCRETE_IO_HANDLE result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HEADER_DETECT_IO_01_010: [ If allocating memory for the `header_detect_entries` or its constituents fails then `header_detect_io_create` shall fail and return NULL. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_header_detect_entry_fails_header_detect_io_create_fails)
{
    // arrange
    CONCRETE_IO_HANDLE result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HEADER_DETECT_IO_01_010: [ If allocating memory for the `header_detect_entries` or its constituents fails then `header_detect_io_create` shall fail and return NULL. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_second_header_detect_entry_fails_header_detect_io_create_fails)
{
    // arrange
    CONCRETE_IO_HANDLE result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x40, 0x41 };
    HEADER_DETECT_ENTRY header_detect_entries[2];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HEADER_DETECT_IO_01_065: [ If `singlylinkedlist_create` fails then `header_detect_io_create` shall fail and return NULL. ]*/
TEST_FUNCTION(when_singlylinkedlist_create_fails_header_detect_io_create_fails)
{
    // arrange
    CONCRETE_IO_HANDLE result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x40, 0x41 };
    HEADER_DETECT_ENTRY header_detect_entries[2];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_create())
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HEADER_DETECT_IO_01_054: [ At least one entry in `header_detect_entries` shall have IO set to NULL, otherwise `header_detect_io_create` shall fail and return NULL. ]*/
TEST_FUNCTION(when_no_header_entry_has_NULL_header_detect_io_create_fails)
{
    // arrange
    CONCRETE_IO_HANDLE result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x40, 0x41 };
    HEADER_DETECT_ENTRY header_detect_entries[2];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description =  test_detected_io_interface_description_1;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    // act
    result = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HEADER_DETECT_IO_01_054: [ At least one entry in `header_detect_entries` shall have IO set to NULL, otherwise `header_detect_io_create` shall fail and return NULL. ]*/
TEST_FUNCTION(two_NULL_IO_entries_are_OK_for_header_detect_io_create)
{
    // arrange
    CONCRETE_IO_HANDLE result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x40, 0x41 };
    HEADER_DETECT_ENTRY header_detect_entries[2];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG)); // array
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)); // first entry
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)); // second entry
    STRICT_EXPECTED_CALL(singlylinkedlist_create());

    // act
    result = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(result);
}

/* header_detect_io_destroy */

/* Tests_SRS_HEADER_DETECT_IO_01_011: [ `header_detect_io_destroy` shall free all resources associated with the `header_detect_io` handle. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_013: [ `header_detect_io_destroy` shall free the memory allocated for the `header_detect_entries`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_061: [ `header_detect_io_destroy` shall destroy the chained IO list by calling `singlylinkedlist_destroy`. ]*/
TEST_FUNCTION(header_detect_io_destroy_frees_associated_resources)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x40, 0x41 };
    HEADER_DETECT_ENTRY header_detect_entries[2];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(test_singlylinked_list));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HEADER_DETECT_IO_01_012: [ If `header_detect_io` is NULL, `header_detect_io_destroy` shall do nothing. ]*/
TEST_FUNCTION(header_detect_io_destroy_with_NULL_handle_does_not_free_anything)
{
    // arrange

    // act
    header_detect_io_get_interface_description()->concrete_io_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HEADER_DETECT_IO_01_062: [ If the IO is still open when `header_detect_io_destroy` is called, all actions normally executed when closing the IO shall also be executed. ]*/
TEST_FUNCTION(header_detect_io_destroy_also_closes_the_underlying_io_when_no_other_detected_IOs_were_open)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    // close items
    STRICT_EXPECTED_CALL(xio_close(test_underlying_amqp_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));

    // destroy
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(test_singlylinked_list));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HEADER_DETECT_IO_01_062: [ If the IO is still open when `header_detect_io_destroy` is called, all actions normally executed when closing the IO shall also be executed. ]*/
TEST_FUNCTION(header_detect_io_destroy_also_closes_the_underlying_IO_and_the_other_detected_IOs)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[2];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_2, sizeof(amqp_header_bytes_2));
    umock_c_reset_all_calls();

    // close items
    STRICT_EXPECTED_CALL(xio_close(test_detected_io_1, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinked_list, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_destroy(test_detected_io_1));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));

    // destroy items
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(test_singlylinked_list));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HEADER_DETECT_IO_01_062: [ If the IO is still open when `header_detect_io_destroy` is called, all actions normally executed when closing the IO shall also be executed. ]*/
TEST_FUNCTION(header_detect_io_destroy_also_closes_the_underlying_IO_and_the_other_2_detected_IOs)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[2];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    xio_create_return = test_detected_io_2;
    STRICT_EXPECTED_CALL(xio_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // setup for second detected IO
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_2, sizeof(amqp_header_bytes_2));
    umock_c_reset_all_calls();

    // close items
    STRICT_EXPECTED_CALL(xio_close(test_detected_io_2, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinked_list, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_destroy(test_detected_io_1));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinked_list, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_destroy(test_detected_io_2));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));

    // destroy items
    STRICT_EXPECTED_CALL(singlylinkedlist_destroy(test_singlylinked_list));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* header_detect_io_open_async */

/* Tests_SRS_HEADER_DETECT_IO_01_015: [ `header_detect_io_open_async` shall open the underlying IO by calling `xio_open` and passing to it: ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_016: [ - `xio` shall be the `underlying_io` member of the `io_create_parameters` passed to `header_detect_io_create`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_017: [ - `on_io_open_complete`, `on_io_open_complete_context`, `on_bytes_received`, `on_bytes_received_context`, `on_error` and `on_error_context` shall be set to implementation specific values of `header_detect_io`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_018: [ On success `header_detect_io_open_async` shall return 0. ]*/
TEST_FUNCTION(header_detect_io_open_async_opens_the_underlying_IO)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(test_underlying_amqp_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_021: [ If `header_detect_io`, `on_io_open_complete`, `on_bytes_received` or `on_io_error` is NULL, `header_detect_io_open_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(header_detect_io_open_async_with_NULL_io_fails)
{
    // arrange
    int result;

    // act
    result = header_detect_io_get_interface_description()->concrete_io_open(NULL, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_HEADER_DETECT_IO_01_021: [ If `header_detect_io`, `on_io_open_complete`, `on_bytes_received` or `on_io_error` is NULL, `header_detect_io_open_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(header_detect_io_open_async_with_NULL_on_io_open_complete_fails)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  test_detected_io_interface_description_1;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);
    umock_c_reset_all_calls();

    // act
    result = header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, NULL, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_021: [ If `header_detect_io`, `on_io_open_complete`, `on_bytes_received` or `on_io_error` is NULL, `header_detect_io_open_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(header_detect_io_open_async_with_NULL_on_bytes_received_fails)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  test_detected_io_interface_description_1;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);
    umock_c_reset_all_calls();

    // act
    result = header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, NULL, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_021: [ If `header_detect_io`, `on_io_open_complete`, `on_bytes_received` or `on_io_error` is NULL, `header_detect_io_open_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(header_detect_io_open_async_with_NULL_on_io_error_fails)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  test_detected_io_interface_description_1;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);
    umock_c_reset_all_calls();

    // act
    result = header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, NULL, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_019: [ If `xio_open` fails, `header_detect_io_open_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_xio_open_fails_header_detect_io_open_async_opens_the_underlying_IO)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(test_underlying_amqp_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1);

    // act
    result = header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_020: [ If the IO is already OPEN or OPENING then `header_detect_io_open_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(header_detect_io_open_async_when_the_IO_is_OPENING_but_not_yet_OPEN_fails)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    result = header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_020: [ If the IO is already OPEN or OPENING then `header_detect_io_open_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(header_detect_io_open_async_when_the_IO_is_already_open_fails)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    // act
    result = header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* header_detect_io_close_async */

/* Tests_SRS_HEADER_DETECT_IO_01_022: [ `header_detect_io_close_async` shall close the underlying IO by calling `xio_close` and passing to it: ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_023: [ - `xio` shall be the `underlying_io` member of the `io_create_parameters` passed to `header_detect_io_create`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_024: [ - `on_io_close_complete` shall be set to implementation specific values of `header_detect_io`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_025: [ On success `header_detect_io_close_async` shall return 0. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_070: [ If no detected IO was created then `header_detect_io_close_async` shall close the `underlying_io` passed in `header_detect_io_create`. ]*/
TEST_FUNCTION(header_detect_io_close_async_closes_the_underlying_io_when_no_other_detected_IOs_were_open)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_amqp_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_close(header_detect_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_022: [ `header_detect_io_close_async` shall close the underlying IO by calling `xio_close` and passing to it: ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_023: [ - `xio` shall be the `underlying_io` member of the `io_create_parameters` passed to `header_detect_io_create`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_024: [ - `on_io_close_complete` shall be set to implementation specific values of `header_detect_io`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_025: [ On success `header_detect_io_close_async` shall return 0. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_063: [ `header_detect_io_close_async` shall close the last detected IO that was created as a result of matching a header. ]*/
TEST_FUNCTION(header_detect_io_close_async_closes_the_underlying_IO_and_the_other_detected_IOs)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[2];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_2, sizeof(amqp_header_bytes_2));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_detected_io_1, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_close(header_detect_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_092: [ If `xio_close` fails `header_detect_io_close_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_xio_close_fails_header_detect_io_close_async_fails)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[2];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_2, sizeof(amqp_header_bytes_2));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_detected_io_1, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1);

    // act
    result = header_detect_io_get_interface_description()->concrete_io_close(header_detect_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_022: [ `header_detect_io_close_async` shall close the underlying IO by calling `xio_close` and passing to it: ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_023: [ - `xio` shall be the `underlying_io` member of the `io_create_parameters` passed to `header_detect_io_create`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_024: [ - `on_io_close_complete` shall be set to implementation specific values of `header_detect_io`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_025: [ On success `header_detect_io_close_async` shall return 0. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_063: [ `header_detect_io_close_async` shall close the last detected IO that was created as a result of matching a header. ]*/
TEST_FUNCTION(header_detect_io_close_async_closes_the_underlying_IO)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[2];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    xio_create_return = test_detected_io_2;
    STRICT_EXPECTED_CALL(xio_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // setup for second detected IO
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_2, sizeof(amqp_header_bytes_2));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_detected_io_2, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_close(header_detect_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_026: [ If `header_detect_io` is NULL, `header_detect_io_close_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(header_detect_io_close_async_with_NULL_io_handle_fails)
{
    // arrange
    int result;

    // act
    result = header_detect_io_get_interface_description()->concrete_io_close(NULL, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_HEADER_DETECT_IO_01_094: [ `on_io_close_complete` shall be allowed to be NULL, in which case no close complete callback shall be triggered. ]*/
TEST_FUNCTION(header_detect_io_close_async_with_NULL_on_io_close_complete_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_amqp_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_close(header_detect_io, NULL, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_027: [ If the IO is not OPEN (open has not been called or close has been completely carried out) `header_detect_io_close_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(header_detect_io_close_async_when_IO_is_NOT_OPEN_fails)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);
    umock_c_reset_all_calls();

    // act
    result = header_detect_io_get_interface_description()->concrete_io_close(header_detect_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_027: [ If the IO is not OPEN (open has not been called or close has been completely carried out) `header_detect_io_close_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(header_detect_io_close_async_when_IO_is_CLOSED_fails)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));

    // close
    header_detect_io_get_interface_description()->concrete_io_close(header_detect_io, test_on_io_close_complete, (void*)0x4245);
    saved_on_io_close_complete(saved_on_io_open_complete_context);

    umock_c_reset_all_calls();

    // act
    result = header_detect_io_get_interface_description()->concrete_io_close(header_detect_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_053: [ If the IO is CLOSING then `header_detect_io_close_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(header_detect_io_close_async_when_IO_is_CLOSING_fails)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));

    // close
    header_detect_io_get_interface_description()->concrete_io_close(header_detect_io, test_on_io_close_complete, (void*)0x4245);

    umock_c_reset_all_calls();

    // act
    result = header_detect_io_get_interface_description()->concrete_io_close(header_detect_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_028: [ If the IO is OPENING (`header_detect_io_open_async` has been called, but no header has been detected yet), `header_detect_io_close_async` shall close the underlying IO and call `on_io_open_complete` with `IO_OPEN_CANCELLED`. ]*/
TEST_FUNCTION(header_detect_io_close_async_when_IO_is_OPENING_indicates_io_open_complete_with_IO_OPEN_CANCELLED)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_amqp_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_CANCELLED));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_close(header_detect_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_028: [ If the IO is OPENING (`header_detect_io_open_async` has been called, but no header has been detected yet), `header_detect_io_close_async` shall close the underlying IO and call `on_io_open_complete` with `IO_OPEN_CANCELLED`. ]*/
TEST_FUNCTION(header_detect_io_close_async_when_IO_is_OPENING_and_underlying_io_is_open_indicates_io_open_complete_with_IO_OPEN_CANCELLED)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);

    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_amqp_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_CANCELLED));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_close(header_detect_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_028: [ If the IO is OPENING (`header_detect_io_open_async` has been called, but no header has been detected yet), `header_detect_io_close_async` shall close the underlying IO and call `on_io_open_complete` with `IO_OPEN_CANCELLED`. ]*/
TEST_FUNCTION(header_detect_io_close_async_when_IO_is_OPENING_and_underlying_io_is_open_and_one_byte_has_been_received_indicates_io_open_complete_with_IO_OPEN_CANCELLED)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42, 0x43 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, 1);

    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_amqp_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_CANCELLED));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_close(header_detect_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_028: [ If the IO is OPENING (`header_detect_io_open_async` has been called, but no header has been detected yet), `header_detect_io_close_async` shall close the underlying IO and call `on_io_open_complete` with `IO_OPEN_CANCELLED`. ]*/
TEST_FUNCTION(header_detect_io_close_async_when_IO_is_OPENING_and_the_detected_io_is_opening_indicates_io_open_complete_with_IO_OPEN_CANCELLED)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[2];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_detected_io_1, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinked_list, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_destroy(test_detected_io_1));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_CANCELLED));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_close(header_detect_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* on_underlying_io_close_complete */

/* Tests_SRS_HEADER_DETECT_IO_01_095: [ When `on_underlying_io_open_complete` is called when the IO is closing, it shall destroy all the detected IOs that were created. ]*/
TEST_FUNCTION(on_underlying_io_close_complete_destroys_the_created_IO)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[2];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_2, sizeof(amqp_header_bytes_2));
    (void)header_detect_io_get_interface_description()->concrete_io_close(header_detect_io, test_on_io_close_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinked_list, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_destroy(test_detected_io_1));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(test_on_io_close_complete((void*)0x4245));

    // act
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_095: [ When `on_underlying_io_open_complete` is called when the IO is closing, it shall destroy all the detected IOs that were created. ]*/
TEST_FUNCTION(on_underlying_io_close_complete_destroys_the_2_created_IOs)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[2];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    xio_create_return = test_detected_io_2;
    STRICT_EXPECTED_CALL(xio_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // setup for second detected IO
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_2, sizeof(amqp_header_bytes_2));
    (void)header_detect_io_get_interface_description()->concrete_io_close(header_detect_io, test_on_io_close_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinked_list, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_destroy(test_detected_io_1));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinked_list, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_destroy(test_detected_io_2));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(test_on_io_close_complete((void*)0x4245));

    // act
    saved_on_io_close_complete(saved_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* header_detect_io_send_async */

/* Tests_SRS_HEADER_DETECT_IO_01_029: [ If no detected IO was created, `header_detect_io_send_async` shall send the bytes to the underlying IO passed via `header_detect_io_create`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_030: [ The `buffer`, `size`, `on_send_complete` and `callback_context` shall be passed as is to `xio_send`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_031: [ On success `header_detect_io_send_async` shall return 0. ]*/
TEST_FUNCTION(header_detect_io_send_async_calls_send_on_the_underlying_IO)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char send_payload[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_amqp_io, IGNORED_PTR_ARG, sizeof(send_payload), test_on_send_complete, (void*)0x4247))
        .ValidateArgumentBuffer(2, send_payload, sizeof(send_payload));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_send(header_detect_io, send_payload, sizeof(send_payload), test_on_send_complete, (void*)0x4247);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_071: [ If the header IO is open `header_detect_io_send_async` shall send the bytes to the last detected IO by calling `xio_send` that was created as result of matching a header. ]*/
TEST_FUNCTION(header_detect_io_send_async_calls_send_on_the_last_detected_io)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x43, 0x44 };
    unsigned char send_payload[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[2];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_2, sizeof(amqp_header_bytes_2));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_detected_io_1, IGNORED_PTR_ARG, sizeof(send_payload), test_on_send_complete, (void*)0x4247))
        .ValidateArgumentBuffer(2, send_payload, sizeof(send_payload));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_send(header_detect_io, send_payload, sizeof(send_payload), test_on_send_complete, (void*)0x4247);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_071: [ If the header IO is open `header_detect_io_send_async` shall send the bytes to the last detected IO by calling `xio_send` that was created as result of matching a header. ]*/
TEST_FUNCTION(header_detect_io_send_async_calls_send_on_the_last_of_2_detected_ios)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x43, 0x44 };
    unsigned char send_payload[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[2];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    xio_create_return = test_detected_io_2;
    STRICT_EXPECTED_CALL(xio_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // setup for second detected IO
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_2, sizeof(amqp_header_bytes_2));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_detected_io_2, IGNORED_PTR_ARG, sizeof(send_payload), test_on_send_complete, (void*)0x4247))
        .ValidateArgumentBuffer(2, send_payload, sizeof(send_payload));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_send(header_detect_io, send_payload, sizeof(send_payload), test_on_send_complete, (void*)0x4247);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_033: [ If `header_detect_io` or `buffer` is NULL, `header_detect_io_send_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(header_detect_io_send_async_with_NULL_io_handle_fails)
{
    // arrange
    unsigned char send_payload[] = { 0x43, 0x44 };
    int result;

    // act
    result = header_detect_io_get_interface_description()->concrete_io_send(NULL, send_payload, sizeof(send_payload), test_on_send_complete, (void*)0x4247);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_HEADER_DETECT_IO_01_033: [ If `header_detect_io` or `buffer` is NULL, `header_detect_io_send_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(header_detect_io_send_async_with_NULL_buffer_fails)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char send_payload[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    // act
    result = header_detect_io_get_interface_description()->concrete_io_send(header_detect_io, NULL, sizeof(send_payload), test_on_send_complete, (void*)0x4247);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_055: [ `on_send_complete` and `callback_context` shall be allowed to be NULL. ]*/
TEST_FUNCTION(header_detect_io_send_async_with_NULL_on_send_complete_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char send_payload[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_amqp_io, IGNORED_PTR_ARG, sizeof(send_payload), NULL, (void*)0x4247))
        .ValidateArgumentBuffer(2, send_payload, sizeof(send_payload));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_send(header_detect_io, send_payload, sizeof(send_payload), NULL, (void*)0x4247);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_055: [ `on_send_complete` and `callback_context` shall be allowed to be NULL. ]*/
TEST_FUNCTION(header_detect_io_send_async_with_NULL_callback_context_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char send_payload[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_amqp_io, IGNORED_PTR_ARG, sizeof(send_payload), test_on_send_complete, NULL))
        .ValidateArgumentBuffer(2, send_payload, sizeof(send_payload));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_send(header_detect_io, send_payload, sizeof(send_payload), test_on_send_complete, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_034: [ If `size` is 0, `header_detect_io_send_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(header_detect_io_send_async_with_0_size_fails)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char send_payload[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    // act
    result = header_detect_io_get_interface_description()->concrete_io_send(header_detect_io, send_payload, 0, test_on_send_complete, (void*)0x4247);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_032: [ If `xio_send` fails, `header_detect_io_send_async` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_the_underlying_send_fails_header_detect_io_send_async_fails)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char send_payload[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_amqp_io, IGNORED_PTR_ARG, sizeof(send_payload), test_on_send_complete, (void*)0x4247))
        .ValidateArgumentBuffer(2, send_payload, sizeof(send_payload))
        .SetReturn(1);

    // act
    result = header_detect_io_get_interface_description()->concrete_io_send(header_detect_io, send_payload, sizeof(send_payload), test_on_send_complete, (void*)0x4247);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_093: [ `header_detect_io_send_async` when the IO is not open shall fail and return a non-zero value. ]*/
TEST_FUNCTION(header_detect_io_send_async_on_a_not_open_IO_fails)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char send_payload[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);
    umock_c_reset_all_calls();

    // act
    result = header_detect_io_get_interface_description()->concrete_io_send(header_detect_io, send_payload, sizeof(send_payload), test_on_send_complete, (void*)0x4247);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_093: [ `header_detect_io_send_async` when the IO is not open shall fail and return a non-zero value. ]*/
TEST_FUNCTION(header_detect_io_send_async_on_an_IO_that_was_open_and_closed_fails)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char send_payload[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    int result;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));

    // close
    (void)header_detect_io_get_interface_description()->concrete_io_close(header_detect_io, test_on_io_close_complete, (void*)0x4245);
    saved_on_io_close_complete(saved_on_io_close_complete_context);
    umock_c_reset_all_calls();

    // act
    result = header_detect_io_get_interface_description()->concrete_io_send(header_detect_io, send_payload, sizeof(send_payload), test_on_send_complete, (void*)0x4247);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* header_detect_io_dowork */

/* Tests_SRS_HEADER_DETECT_IO_01_035: [ `header_detect_io_dowork` shall schedule work for the underlying IO associated with `header_detect_io` by calling `xio_dowork` and passing as argument the `underlying_io` member of the `io_create_parameters` passed to `header_detect_io_create`. ]*/
TEST_FUNCTION(header_detect_io_dowork_calls_the_underlying_io_dowork)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(xio_dowork(test_underlying_amqp_io));

    // act
    header_detect_io_get_interface_description()->concrete_io_dowork(header_detect_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_036: [ If `header_detect_io` is NULL, `header_detect_io_dowork` shall do nothing. ]*/
TEST_FUNCTION(header_detect_io_dowork_with_NULL_IO_does_nothing)
{
    // arrange

    // act
    header_detect_io_get_interface_description()->concrete_io_dowork(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HEADER_DETECT_IO_01_037: [ No work shall be scheduled if `header_detect_io` is not OPEN or in ERROR (an error has been indicated to the user). ]*/
TEST_FUNCTION(header_detect_io_dowork_does_nothing_when_not_open)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);
    umock_c_reset_all_calls();

    // act
    header_detect_io_get_interface_description()->concrete_io_dowork(header_detect_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_037: [ No work shall be scheduled if `header_detect_io` is not OPEN or in ERROR (an error has been indicated to the user). ]*/
TEST_FUNCTION(header_detect_io_dowork_does_nothing_when_already_closed)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));

    //close
    (void)header_detect_io_get_interface_description()->concrete_io_close(header_detect_io, test_on_io_close_complete, (void*)0x4244);
    saved_on_io_close_complete(saved_on_io_close_complete_context);
    umock_c_reset_all_calls();

    // act
    header_detect_io_get_interface_description()->concrete_io_dowork(header_detect_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_037: [ No work shall be scheduled if `header_detect_io` is not OPEN or in ERROR (an error has been indicated to the user). ]*/
TEST_FUNCTION(header_detect_io_dowork_schedules_work_when_OPENING)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(xio_dowork(test_underlying_amqp_io));

    // act
    header_detect_io_get_interface_description()->concrete_io_dowork(header_detect_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_037: [ No work shall be scheduled if `header_detect_io` is not OPEN or in ERROR (an error has been indicated to the user). ]*/
TEST_FUNCTION(header_detect_io_dowork_schedules_work_when_CLOSING)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));

    //close
    (void)header_detect_io_get_interface_description()->concrete_io_close(header_detect_io, test_on_io_close_complete, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(xio_dowork(test_underlying_amqp_io));

    // act
    header_detect_io_get_interface_description()->concrete_io_dowork(header_detect_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_056: [ `header_detect_io_dowork` shall call `xio_dowork` for all detected IOs created as a result of matching headers. ]*/
TEST_FUNCTION(header_detect_io_dowork_schedules_work_for_the_detected_IO)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[2];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_dowork(test_detected_io_1));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_next_item(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_dowork(test_underlying_amqp_io));

    // act
    header_detect_io_get_interface_description()->concrete_io_dowork(header_detect_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_056: [ `header_detect_io_dowork` shall call `xio_dowork` for all detected IOs created as a result of matching headers. ]*/
TEST_FUNCTION(header_detect_io_dowork_schedules_work_for_the_2_detected_IOs)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[2];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    xio_create_return = test_detected_io_2;
    STRICT_EXPECTED_CALL(xio_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // setup for second detected IO
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_dowork(test_detected_io_1));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_next_item(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_dowork(test_detected_io_2));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_next_item(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_dowork(test_underlying_amqp_io));

    // act
    header_detect_io_get_interface_description()->concrete_io_dowork(header_detect_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* header_detect_io_set_option */

/* Tests_SRS_HEADER_DETECT_IO_01_042: [ If no detected IO was created `header_detect_io_set_option` shall pass any option to the underlying IO by calling `xio_setoption` and passing as IO handle the `underlying_io` member of the `io_create_parameters` passed to `header_detect_io_create`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_043: [ On success, `header_detect_io_set_option` shall return 0. ]*/
TEST_FUNCTION(header_detect_io_set_option_calls_the_underlying_io_set_option)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    int result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_setoption(test_underlying_amqp_io, "option_1", (void*)0x4242));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_setoption(header_detect_io, "option_1", (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_072: [ If any detected IO was created, `header_detect_io_set_option` shall pass any option to the last detected IO by calling `xio_setoption` and passing as IO handle the `underlying_io` member of the `io_create_parameters` passed to `header_detect_io_create`. ]*/
TEST_FUNCTION(header_detect_io_set_option_calls_the_last_detected_io_set_option)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    int result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[2];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_setoption(test_detected_io_1, "option_1", (void*)0x4242));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_setoption(header_detect_io, "option_1", (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_072: [ If any detected IO was created, `header_detect_io_set_option` shall pass any option to the last detected IO by calling `xio_setoption` and passing as IO handle the `underlying_io` member of the `io_create_parameters` passed to `header_detect_io_create`. ]*/
TEST_FUNCTION(header_detect_io_set_option_calls_the_last_of_2_detected_io_set_option)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    int result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[2];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    // open
    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    xio_create_return = test_detected_io_2;
    STRICT_EXPECTED_CALL(xio_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // setup for second detected IO
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_setoption(test_detected_io_2, "option_1", (void*)0x4242));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_setoption(header_detect_io, "option_1", (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_042: [ If no detected IO was created `header_detect_io_set_option` shall pass any option to the underlying IO by calling `xio_setoption` and passing as IO handle the `underlying_io` member of the `io_create_parameters` passed to `header_detect_io_create`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_043: [ On success, `header_detect_io_set_option` shall return 0. ]*/
TEST_FUNCTION(header_detect_io_set_option_calls_the_underlying_io_set_option_with_NULL_value)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    int result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_setoption(test_underlying_amqp_io, "option_1", NULL));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_setoption(header_detect_io, "option_1", NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_044: [ If `header_detect_io` or `option_name` is NULL, `header_detect_io_set_option` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(header_detect_io_set_option_with_NULL_IO_handle_fails)
{
    // arrange
    int result;

    // act
    result = header_detect_io_get_interface_description()->concrete_io_setoption(NULL, "option", (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_HEADER_DETECT_IO_01_044: [ If `header_detect_io` or `option_name` is NULL, `header_detect_io_set_option` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(header_detect_io_set_option_with_NULL_option_fails)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    int result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);
    umock_c_reset_all_calls();

    // act
    result = header_detect_io_get_interface_description()->concrete_io_setoption(header_detect_io, NULL, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_045: [ If `xio_setoption` fails, `header_detect_io_set_option` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_the_underlying_setoption_fails_header_detect_io_set_option_fails)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    int result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_setoption(test_underlying_amqp_io, "option_1", (void*)0x4242))
        .SetReturn(1);

    // act
    result = header_detect_io_get_interface_description()->concrete_io_setoption(header_detect_io, "option_1", (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* header_detect_io_retrieve_options */

/* Tests_SRS_HEADER_DETECT_IO_01_038: [ `header_detect_io_retrieve_options` shall create a new `OPTIONHANDLER_HANDLE` by calling `OptionHandler_Create` and on success it shall return a non-NULL handle to the newly created option handler. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_039: [ No options shall be added to the newly created option handler. ]*/
TEST_FUNCTION(header_detect_io_retrieve_options_creates_an_OPTION_HANDLER)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    OPTIONHANDLER_HANDLE result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    result = header_detect_io_get_interface_description()->concrete_io_retrieveoptions(header_detect_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_040: [ If `OptionHandler_Create` fails, `header_detect_io_retrieve_options` shall return NULL. ]*/
TEST_FUNCTION(when_creating_the_option_handler_fails_header_detect_io_retrieve_options_fails)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    OPTIONHANDLER_HANDLE result;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(NULL);

    // act
    result = header_detect_io_get_interface_description()->concrete_io_retrieveoptions(header_detect_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_041: [ If `header_detect_io` is NULL, `header_detect_io_retrieve_options` shall return NULL. ]*/
TEST_FUNCTION(header_detect_io_retrieve_options_with_NULL_handle_fails)
{
    // arrange
    OPTIONHANDLER_HANDLE result;

    // act
    result = header_detect_io_get_interface_description()->concrete_io_retrieveoptions(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* on_underlying_io_open_complete */

/* Tests_SRS_HEADER_DETECT_IO_01_046: [ When `on_underlying_io_open_complete` is called with `open_result` being `IO_OPEN_OK` while OPENING, the IO shall start monitoring received bytes in order to detect headers. ]*/
TEST_FUNCTION(on_underlying_io_open_complete_with_IO_OPEN_OK_starts_waiting_for_bytes)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_047: [ When `on_underlying_io_open_complete` is called with `open_result` being `IO_OPEN_ERROR` while OPENING, the `on_io_open_complete` callback passed to `header_detect_io_open` shall be called with `IO_OPEN_ERROR`. ]*/
TEST_FUNCTION(on_underlying_io_open_complete_with_IO_OPEN_ERROR_indicates_on_io_open_complete_with_IO_OPEN_ERROR)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_amqp_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_ERROR);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_048: [ If `context` is NULL, `on_underlying_io_open_complete` shall do nothing. ]*/
TEST_FUNCTION(on_underlying_io_open_complete_with_NULL_does_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    saved_on_io_open_complete(NULL, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* on_underlying_io_error */

/* Tests_SRS_HEADER_DETECT_IO_01_058: [ If `context` is NULL, `on_underlying_io_error` shall do nothing. ]*/
TEST_FUNCTION(on_underlying_io_error_with_NULL_context_does_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    saved_on_io_error(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_057: [ When `on_underlying_io_error` is called while OPENING, the IO shall indicate an error by calling `on_io_open_complete` with `IO_OPEN_ERROR` and it shall close the underlying IOs. ]*/
TEST_FUNCTION(on_underlying_io_error_in_OPENING_indicates_an_io_open_complete_with_IO_OPEN_ERROR)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_amqp_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_io_error(saved_on_io_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_057: [ When `on_underlying_io_error` is called while OPENING, the IO shall indicate an error by calling `on_io_open_complete` with `IO_OPEN_ERROR` and it shall close the underlying IOs. ]*/
TEST_FUNCTION(on_underlying_io_error_in_OPENING_and_waiting_for_bytes_indicates_an_io_open_complete_with_IO_OPEN_ERROR)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_underlying_amqp_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_io_error(saved_on_io_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_057: [ When `on_underlying_io_error` is called while OPENING, the IO shall indicate an error by calling `on_io_open_complete` with `IO_OPEN_ERROR` and it shall close the underlying IOs. ]*/
TEST_FUNCTION(on_underlying_io_error_in_OPENING_and_waiting_for_detected_io_open_to_complete_indicates_an_io_open_complete_with_IO_OPEN_ERROR)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    unsigned char amqp_header_bytes_2[] = { 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[2];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(test_detected_io_1, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinked_list, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_destroy(test_detected_io_1));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_io_error(saved_on_io_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_059: [ When `on_underlying_io_error` is called while OPEN, the error should be indicated to the consumer by calling `on_io_error` and passing the `on_io_error_context` to it. ]*/
TEST_FUNCTION(on_underlying_io_error_when_OPEN_indicates_the_error_up)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    saved_on_io_error(saved_on_io_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* on_underlying_io_bytes_received */

/* Tests_SRS_HEADER_DETECT_IO_01_050: [ If `context` is NULL, `on_underlying_io_bytes_received` shall do nothing. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_NULL_context_does_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    unsigned char received_bytes[] = { 0x42 };

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    // act
    saved_on_bytes_received(NULL, received_bytes, sizeof(received_bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_051: [ If `buffer` is NULL or `size` is 0 while the IO is OPEN an error shall be indicated by calling `on_io_error`. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_NULL_bytes_in_OPEN_indicates_an_error)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, NULL, 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_051: [ If `buffer` is NULL or `size` is 0 while the IO is OPEN an error shall be indicated by calling `on_io_error`. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_0_size_in_OPEN_indicates_an_error)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    unsigned char received_bytes[] = { 0x42 };

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, received_bytes, 0);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_049: [ When `on_underlying_io_bytes_received` is called while opening the underlying IO (before the underlying open complete is received), an error shall be indicated by calling `on_io_open_complete` with `IO_OPEN_ERROR`. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_when_underlying_IO_is_not_yet_open_indicates_open_complete_with_error)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    unsigned char received_bytes[] = { 0x42 };

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, received_bytes, sizeof(received_bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_066: [ If the bytes received since matching started do not match any of the headers in the `header_detect_entries` field, then the IO shall be considered not open and an open complete with `IO_OPEN_ERROR` shall be indicated. ]*/
TEST_FUNCTION(when_the_first_byte_does_not_match_any_headers_then_on_io_open_complete_is_called_with_IO_OPEN_ERROR)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    unsigned char received_bytes[] = { 0x43 };

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_amqp_io, amqp_header_bytes_1, sizeof(amqp_header_bytes_1), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(2, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    STRICT_EXPECTED_CALL(xio_close(test_underlying_amqp_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, received_bytes, sizeof(received_bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_066: [ If the bytes received since matching started do not match any of the headers in the `header_detect_entries` field, then the IO shall be considered not open and an open complete with `IO_OPEN_ERROR` shall be indicated. ]*/
TEST_FUNCTION(when_the_last_byte_does_not_match_any_headers_then_on_io_open_complete_is_called_with_IO_OPEN_ERROR)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42, 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[1];
    unsigned char received_bytes[] = { 0x42, 0x43, 0x45 };

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_amqp_io, amqp_header_bytes_1, sizeof(amqp_header_bytes_1), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(2, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    STRICT_EXPECTED_CALL(xio_close(test_underlying_amqp_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, received_bytes, sizeof(received_bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_066: [ If the bytes received since matching started do not match any of the headers in the `header_detect_entries` field, then the IO shall be considered not open and an open complete with `IO_OPEN_ERROR` shall be indicated. ]*/
TEST_FUNCTION(when_the_last_byte_does_not_match_any_of_the_2_headers_then_on_io_open_complete_is_called_with_IO_OPEN_ERROR)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42, 0x43, 0x44 };
    unsigned char amqp_header_bytes_2[] = { 0x42, 0x43, 0x45 };
    HEADER_DETECT_ENTRY header_detect_entries[2];
    unsigned char received_bytes[] = { 0x42, 0x43, 0x46 };

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_amqp_io, amqp_header_bytes_1, sizeof(amqp_header_bytes_1), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(2, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    STRICT_EXPECTED_CALL(xio_close(test_underlying_amqp_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, received_bytes, sizeof(received_bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_067: [ When `on_underlying_io_bytes_received` is called while waiting for header bytes (after the underlying IO was open), the bytes shall be matched against the entries provided in the configuration passed to `header_detect_io_create`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_068: [ Header bytes shall be accepted in multiple `on_underlying_io_bytes_received` calls. ]*/
TEST_FUNCTION(header_bytes_can_be_parsed_in_multiple_on_underlying_bytes_received_calls)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42, 0x43, 0x44 };
    HEADER_DETECT_ENTRY header_detect_entries[1];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_amqp_io, amqp_header_bytes_1, sizeof(amqp_header_bytes_1), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(2, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_OK));

    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, 1);

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1 + 1, sizeof(amqp_header_bytes_1) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_069: [ If a header match was detected on an entry with a non-NULL io handle, a new IO associated shall be created by calling `xio_create`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_073: [ The interface description passed to `xio_create` shall be the interface description associated with the detected header. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_074: [ The IO create parameters shall be a `SERVER_PROTOCOL_IO_CONFIG` structure. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_076: [ If no detected IO was created then the underlying IO in the `SERVER_PROTOCOL_IO_CONFIG` structure shall be set to the `underlying_io` passed in the create arguments. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_078: [ The newly create IO shall be open by calling `xio_open`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_086: [ The newly created IO shall be added to the chain of IOs by calling `singlylinkedlist_add`. ]*/
TEST_FUNCTION(when_a_header_is_detected_and_it_specifies_an_IO_then_the_IO_is_created)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42, 0x43, 0x44 };
    unsigned char amqp_header_bytes_2[] = { 0x42, 0x43, 0x45 };
    HEADER_DETECT_ENTRY header_detect_entries[2];
    SERVER_PROTOCOL_IO_CONFIG server_protocol_io_config;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description =  test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description =  NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_amqp_io, amqp_header_bytes_1, sizeof(amqp_header_bytes_1), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(2, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));

    server_protocol_io_config.underlying_io = test_underlying_amqp_io;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_create(test_detected_io_interface_description_1, &server_protocol_io_config))
        .ValidateArgumentValue_io_create_parameters_AsType(UMOCK_TYPE(SERVER_PROTOCOL_IO_CONFIG*));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinked_list, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_open(test_detected_io_1, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_069: [ If a header match was detected on an entry with a non-NULL io handle, a new IO associated shall be created by calling `xio_create`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_075: [ The underlying IO in the `SERVER_PROTOCOL_IO_CONFIG` structure shall be set to the last detected IO that was created if any. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_078: [ The newly create IO shall be open by calling `xio_open`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_079: [ The `on_io_open_complete` callback passed to `xio_open` shall be `on_underlying_io_open_complete`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_080: [ The `on_bytes_received` callback passed to `xio_open` shall be `on_underlying_io_bytes_received`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_081: [ The `on_io_error` callback passed to `xio_open` shall be `on_underlying_io_error`. ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_083: [ The header detect IO shall wait for opening of the detected IO (signaled by the `on_underlying_io_open_complete`). ]*/
/* Tests_SRS_HEADER_DETECT_IO_01_086: [ The newly created IO shall be added to the chain of IOs by calling `singlylinkedlist_add`. ]*/
TEST_FUNCTION(when_a_header_is_detected_again_and_it_specifies_an_IO_then_the_IO_is_created_again_and_added_to_the_chain)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42, 0x43, 0x44 };
    unsigned char amqp_header_bytes_2[] = { 0x42, 0x43, 0x45 };
    HEADER_DETECT_ENTRY header_detect_entries[2];
    SERVER_PROTOCOL_IO_CONFIG server_protocol_io_config;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_detected_io_1, amqp_header_bytes_1, sizeof(amqp_header_bytes_1), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(2, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));

    server_protocol_io_config.underlying_io = test_detected_io_1;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    xio_create_return = test_detected_io_2;
    STRICT_EXPECTED_CALL(xio_create(test_detected_io_interface_description_1, &server_protocol_io_config))
        .ValidateArgumentValue_io_create_parameters_AsType(UMOCK_TYPE(SERVER_PROTOCOL_IO_CONFIG*));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinked_list, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_open(test_detected_io_2, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_077: [ If `xio_create` fails the header detect IO shall be closed and an error shall be indicated by calling `on_io_open_complete` with `IO_OPEN_ERROR`. ]*/
TEST_FUNCTION(when_xio_create_fails_on_io_complete_is_called_with_IO_OPEN_ERROR)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42, 0x43, 0x44 };
    unsigned char amqp_header_bytes_2[] = { 0x42, 0x43, 0x45 };
    HEADER_DETECT_ENTRY header_detect_entries[2];
    SERVER_PROTOCOL_IO_CONFIG server_protocol_io_config;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_amqp_io, amqp_header_bytes_1, sizeof(amqp_header_bytes_1), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(2, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    server_protocol_io_config.underlying_io = test_underlying_amqp_io;
    STRICT_EXPECTED_CALL(xio_create(test_detected_io_interface_description_1, &server_protocol_io_config))
        .ValidateArgumentValue_io_create_parameters_AsType(UMOCK_TYPE(SERVER_PROTOCOL_IO_CONFIG*))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_close(test_underlying_amqp_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_082: [ If `xio_open` fails the header detect IO shall be closed and an error shall be indicated by calling `on_io_open_complete` with `IO_OPEN_ERROR`. ]*/
TEST_FUNCTION(when_xio_open_fails_on_io_complete_is_called_with_IO_OPEN_ERROR)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42, 0x43, 0x44 };
    unsigned char amqp_header_bytes_2[] = { 0x42, 0x43, 0x45 };
    HEADER_DETECT_ENTRY header_detect_entries[2];
    SERVER_PROTOCOL_IO_CONFIG server_protocol_io_config;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_amqp_io, amqp_header_bytes_1, sizeof(amqp_header_bytes_1), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(2, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));

    server_protocol_io_config.underlying_io = test_underlying_amqp_io;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_create(test_detected_io_interface_description_1, &server_protocol_io_config))
        .ValidateArgumentValue_io_create_parameters_AsType(UMOCK_TYPE(SERVER_PROTOCOL_IO_CONFIG*));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinked_list, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_open(test_detected_io_1, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(singlylinkedlist_remove(test_singlylinked_list, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_destroy(test_detected_io_1));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_close(test_underlying_amqp_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_084: [ If `singlylinkedlist_add` fails the newly created IO shall be destroyed and an error shall be indicated by calling `on_io_open_complete` with `IO_OPEN_ERROR`. ]*/
TEST_FUNCTION(when_adding_the_newly_created_IO_to_the_list_fails_on_io_complete_is_called_with_IO_OPEN_ERROR)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42, 0x43, 0x44 };
    unsigned char amqp_header_bytes_2[] = { 0x42, 0x43, 0x45 };
    HEADER_DETECT_ENTRY header_detect_entries[2];
    SERVER_PROTOCOL_IO_CONFIG server_protocol_io_config;

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(test_underlying_amqp_io, amqp_header_bytes_1, sizeof(amqp_header_bytes_1), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(2, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));

    server_protocol_io_config.underlying_io = test_underlying_amqp_io;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_create(test_detected_io_interface_description_1, &server_protocol_io_config))
        .ValidateArgumentValue_io_create_parameters_AsType(UMOCK_TYPE(SERVER_PROTOCOL_IO_CONFIG*));
    STRICT_EXPECTED_CALL(singlylinkedlist_add(test_singlylinked_list, IGNORED_PTR_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(xio_destroy(test_detected_io_1));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_close(test_underlying_amqp_io, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(test_singlylinked_list));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_087: [ If `on_underlying_io_bytes_received` is called while waiting for the detected IO to complete its open, the bytes shall be given to the last created IO by calling its `on_bytes_received` callback that was filled into the `on_bytes_received` member of `SERVER_PROTOCOL_IO_CONFIG`. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_when_opening_a_detected_IO_passes_the_bytes_to_it)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42, 0x43, 0x44 };
    unsigned char amqp_header_bytes_2[] = { 0x42, 0x43, 0x45 };
    unsigned char received_payload[] = { 0x42, 0x43 };
    HEADER_DETECT_ENTRY header_detect_entries[2];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_detected_io_1_on_bytes_received(test_detected_io_1_on_bytes_received_context, received_payload, sizeof(received_payload)));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, received_payload, sizeof(received_payload));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_089: [ If `on_underlying_io_bytes_received` is called while header detect IO is OPEN the bytes shall be given to the user via the `on_bytes_received` callback that was the `on_bytes_received` callback passed to `header_detect_io_open_async`. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_when_open_gives_the_bytes_to_the_proper_IO)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42, 0x43, 0x44 };
    unsigned char amqp_header_bytes_2[] = { 0x42, 0x43, 0x45 };
    unsigned char received_payload[] = { 0x42, 0x43 };
    HEADER_DETECT_ENTRY header_detect_entries[2];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_1, sizeof(amqp_header_bytes_1));
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_2, sizeof(amqp_header_bytes_2));
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_bytes_received((void*)0x4243, received_payload, sizeof(received_payload)));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, received_payload, sizeof(received_payload));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* Tests_SRS_HEADER_DETECT_IO_01_090: [ If no detected IOs were created and `on_underlying_io_bytes_received` is called while header detect IO is OPEN, the `on_bytes_received` callback passed to `header_detect_io_open_async` shall be called to indicate the bytes as received. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_when_ono_detected_IOs_were_created_gives_the_bytes_to_the_user)
{
    // arrange
    CONCRETE_IO_HANDLE header_detect_io;
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    unsigned char amqp_header_bytes_1[] = { 0x42, 0x43, 0x44 };
    unsigned char amqp_header_bytes_2[] = { 0x42, 0x43, 0x45 };
    unsigned char received_payload[] = { 0x42, 0x43 };
    HEADER_DETECT_ENTRY header_detect_entries[2];

    header_detect_entries[0].header.header_bytes = amqp_header_bytes_1;
    header_detect_entries[0].header.header_size = sizeof(amqp_header_bytes_1);
    header_detect_entries[0].io_interface_description = test_detected_io_interface_description_1;
    header_detect_entries[1].header.header_bytes = amqp_header_bytes_2;
    header_detect_entries[1].header.header_size = sizeof(amqp_header_bytes_2);
    header_detect_entries[1].io_interface_description = NULL;

    header_detect_io_config.header_detect_entry_count = 2;
    header_detect_io_config.header_detect_entries = header_detect_entries;
    header_detect_io_config.underlying_io = test_underlying_amqp_io;

    header_detect_io = header_detect_io_get_interface_description()->concrete_io_create(&header_detect_io_config);

    (void)header_detect_io_get_interface_description()->concrete_io_open(header_detect_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    saved_on_bytes_received(saved_on_bytes_received_context, amqp_header_bytes_2, sizeof(amqp_header_bytes_2));
    saved_on_io_open_complete(saved_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_bytes_received((void*)0x4243, received_payload, sizeof(received_payload)));

    // act
    saved_on_bytes_received(saved_on_bytes_received_context, received_payload, sizeof(received_payload));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    header_detect_io_get_interface_description()->concrete_io_destroy(header_detect_io);
}

/* header_detect_io_get_amqp_header */

/* Tests_SRS_HEADER_DETECT_IO_01_091: [ `header_detect_io_get_amqp_header` shall return a structure that should point to a buffer that contains the bytes { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 }. ]*/
TEST_FUNCTION(header_detect_io_get_amqp_header_returns_the_AMQP_header)
{
    // arrange
    AMQP_HEADER amqp_header;
    char expected_header[256];
    char actual_header[256];
    unsigned char expected_header_bytes[] = { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 };

    // act
    amqp_header = header_detect_io_get_amqp_header();

    // assert
    stringify_bytes(amqp_header.header_bytes, amqp_header.header_size, actual_header, sizeof(actual_header));
    stringify_bytes(expected_header_bytes, sizeof(expected_header_bytes), expected_header, sizeof(expected_header));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(char_ptr, expected_header, actual_header);
}

/* header_detect_io_get_sasl_amqp_header */

/* Tests_SRS_HEADER_DETECT_IO_01_091: [ `header_detect_io_get_sasl_amqp_header` shall return a structure that should point to a buffer that contains the bytes { 'A', 'M', 'Q', 'P', 3, 1, 0, 0 }. ]*/
TEST_FUNCTION(header_detect_io_get_sasl_header_returns_the_AMQP_header)
{
    // arrange
    AMQP_HEADER amqp_header;
    char expected_header[256];
    char actual_header[256];
    unsigned char expected_header_bytes[] = { 'A', 'M', 'Q', 'P', 3, 1, 0, 0 };

    // act
    amqp_header = header_detect_io_get_sasl_amqp_header();

    // assert
    stringify_bytes(amqp_header.header_bytes, amqp_header.header_size, actual_header, sizeof(actual_header));
    stringify_bytes(expected_header_bytes, sizeof(expected_header_bytes), expected_header, sizeof(expected_header));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(char_ptr, expected_header, actual_header);
}

END_TEST_SUITE(header_detect_io_ut)
