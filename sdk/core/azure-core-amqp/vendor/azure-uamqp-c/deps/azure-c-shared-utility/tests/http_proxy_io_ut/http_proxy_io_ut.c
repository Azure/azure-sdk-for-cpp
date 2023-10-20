// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#else
#include <stdlib.h>
#include <stddef.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umock_c_negative_tests.h"

static TEST_MUTEX_HANDLE g_testByTest;

#if defined _MSC_VER
#pragma warning(disable: 4054) /* MSC incorrectly fires this */
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    void* real_malloc(size_t size)
    {
        return malloc(size);
    }

    void* real_calloc(size_t nmemb, size_t size)
    {
        return calloc(nmemb, size);
    }

    void* real_realloc(void* ptr, size_t size)
    {
        return realloc(ptr, size);
    }

    void real_free(void* ptr)
    {
        free(ptr);
    }

    int real_mallocAndStrcpy_s(char** destination, const char* source);

#ifdef __cplusplus
}
#endif

#define ENABLE_MOCKS

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/optionhandler.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/azure_base64.h"

IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_SEND_RESULT, IO_SEND_RESULT_VALUES);

#define TEST_OPTION_HANDLER                     (OPTIONHANDLER_HANDLE)0x4244
#define TEST_SOCKETIO_INTERFACE_DESCRIPTION     (const IO_INTERFACE_DESCRIPTION*)0x4242
#define TEST_IO_HANDLE                          (XIO_HANDLE)0x4243
#define TEST_STRING_HANDLE                      (STRING_HANDLE)0x4244
#define OPTION_UNDERLYING_IO_OPTIONS            "underlying_io_options"

MOCK_FUNCTION_WITH_CODE(, void, test_on_io_open_complete, void*, context, IO_OPEN_RESULT, open_result)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_on_bytes_received, void*, context, const unsigned char*, buffer, size_t, size)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_on_io_error, void*, context)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_on_io_close_complete, void*, context)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, void, test_on_send_complete, void*, context, IO_SEND_RESULT, send_result)
MOCK_FUNCTION_END();

#undef ENABLE_MOCKS

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
        result = (char*)real_malloc(strlen(temp_buffer) + 1);
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
        result = (*left)->port == (*right)->port;
        result = result && ((*left)->accepted_socket == (*right)->accepted_socket);
        if ((*right)->hostname == NULL)
        {
            result = result && ((*left)->hostname == (*right)->hostname);
        }
        else
        {
            result = result && (strcmp((*left)->hostname, (*right)->hostname) == 0);
        }
    }

    return result;
}

static int copy_string(char** destination, const char* source)
{
    int result;

    if (source == NULL)
    {
        *destination = NULL;
        result = 0;
    }
    else
    {
        size_t length = strlen(source);
        *destination = (char*)real_malloc(length + 1);
        if (*destination == NULL)
        {
            result = __LINE__;
        }
        else
        {
            (void)memcpy(*destination, source, length + 1);
            result = 0;
        }
    }

    return result;
}

static int umocktypes_copy_const_SOCKETIO_CONFIG_ptr(SOCKETIO_CONFIG** destination, const SOCKETIO_CONFIG** source)
{
    int result;

    *destination = (SOCKETIO_CONFIG*)real_malloc(sizeof(SOCKETIO_CONFIG));
    if (*destination == NULL)
    {
        result = __LINE__;
    }
    else
    {
        if (copy_string((char**)&(*destination)->hostname, (*source)->hostname) != 0)
        {
            result = __LINE__;
        }
        else
        {
            (*destination)->port = (*source)->port;
            (*destination)->accepted_socket = (*source)->accepted_socket;

            result = 0;
        }
    }

    return result;
}

static void umocktypes_free_const_SOCKETIO_CONFIG_ptr(SOCKETIO_CONFIG** value)
{
    real_free((void*)(*value)->hostname);
    real_free(*value);
}

#include "azure_c_shared_utility/http_proxy_io.h"

#ifdef __cplusplus
extern "C"
{
#endif

pfCloneOption tlsio_clone_option;
pfDestroyOption tlsio_destroy_option;

OPTIONHANDLER_HANDLE my_OptionHandler_Create(pfCloneOption cloneOption, pfDestroyOption destroyOption, pfSetOption setOption)
{
    (void)setOption;

    tlsio_clone_option = cloneOption;
    tlsio_destroy_option = destroyOption;
    return TEST_OPTION_HANDLER;
}

static ON_IO_OPEN_COMPLETE g_on_io_open_complete;
static void* g_on_io_open_complete_context;
static ON_SEND_COMPLETE g_on_io_send_complete;
static void* g_on_io_send_complete_context;
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

static const char connect_response[] = "HTTP/1.1 200\r\n\r\n";
static const HTTP_PROXY_IO_CONFIG default_http_proxy_io_config = {
    "test_host",
    443,
    "a_proxy",
    4444,
    "test_user",
    "shhhh"
};

static const HTTP_PROXY_IO_CONFIG http_proxy_io_config_no_username = {
    "test_host",
    443,
    "a_proxy",
    4444,
    NULL,
    NULL
    };

static const HTTP_PROXY_IO_CONFIG http_proxy_io_config_with_username = {
    "another_test_host",
    445,
    "another_proxy",
    8888,
    "le_user",
    "le_password"
    };

static const HTTP_PROXY_IO_CONFIG http_proxy_io_config_with_username_cased = {
    "another_test_host",
    445,
    "another_proxy",
    8888,
    "lE_uSeR",
    "lE_pAsSwOrD"
    };

static const SOCKETIO_CONFIG socketio_config =
{
    "a_proxy",
    4444,
    NULL
};

#ifdef __cplusplus
}
#endif

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(http_proxy_io_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    result = umock_c_init(on_umock_c_error);
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, real_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, real_calloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, real_realloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, real_free);
    REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_s, real_mallocAndStrcpy_s);
    REGISTER_GLOBAL_MOCK_HOOK(OptionHandler_Create, my_OptionHandler_Create);
    REGISTER_GLOBAL_MOCK_HOOK(xio_open, my_xio_open);
    REGISTER_GLOBAL_MOCK_HOOK(xio_close, my_xio_close);
    REGISTER_GLOBAL_MOCK_RETURN(OptionHandler_Create, TEST_OPTION_HANDLER);
    REGISTER_GLOBAL_MOCK_RETURN(socketio_get_interface_description, TEST_SOCKETIO_INTERFACE_DESCRIPTION);
    REGISTER_GLOBAL_MOCK_RETURN(xio_create, TEST_IO_HANDLE);
    REGISTER_GLOBAL_MOCK_RETURN(xio_retrieveoptions, TEST_OPTION_HANDLER);
    REGISTER_GLOBAL_MOCK_RETURN(Azure_Base64_Encode_Bytes, TEST_STRING_HANDLE);
    REGISTER_GLOBAL_MOCK_RETURN(STRING_c_str, "test_str");
    REGISTER_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT);
    REGISTER_TYPE(IO_SEND_RESULT, IO_SEND_RESULT);
    REGISTER_TYPE(const SOCKETIO_CONFIG*, const_SOCKETIO_CONFIG_ptr);
    REGISTER_UMOCK_ALIAS_TYPE(pfCloneOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfDestroyOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfSetOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(OPTIONHANDLER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(SOCKETIO_CONFIG*, const SOCKETIO_CONFIG*);
    REGISTER_UMOCK_ALIAS_TYPE(XIO_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_OPEN_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_RECEIVED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_ERROR, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_CLOSE_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_SEND_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(STRING_HANDLE, void*);
    REGISTER_UMOCKC_PAIRED_CREATE_DESTROY_CALLS(xio_create, xio_destroy);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    umock_c_negative_tests_deinit();

    TEST_MUTEX_RELEASE(g_testByTest);
}

/* http_proxy_io_create */

/* Tests_SRS_HTTP_PROXY_IO_01_001: [ http_proxy_io_create shall create a new instance of the HTTP proxy IO. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_003: [ io_create_parameters shall be used as an HTTP_PROXY_IO_CONFIG*. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_005: [ http_proxy_io_create shall copy the hostname, port, username and password values for later use when the actual CONNECT is performed. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_006: [ hostname and proxy_hostname, username and password shall be copied by calling mallocAndStrcpy_s. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_009: [ http_proxy_io_create shall create a new socket IO by calling xio_create with the arguments: ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_010: [ - io_interface_description shall be set to the result of socketio_get_interface_description. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_011: [ - xio_create_parameters shall be set to a SOCKETIO_CONFIG* where hostname is set to the proxy_hostname member of io_create_parameters and port is set to the proxy_port member of io_create_parameters. ]*/
TEST_FUNCTION(http_proxy_io_create_succeeds)
{
    // arrange
    HTTP_PROXY_IO_CONFIG http_proxy_io_config;
    CONCRETE_IO_HANDLE http_io;

    http_proxy_io_config.hostname = "test_host";
    http_proxy_io_config.port = 443;
    http_proxy_io_config.proxy_hostname = "a_proxy";
    http_proxy_io_config.proxy_port = 4444;
    http_proxy_io_config.username = "test_user";
    http_proxy_io_config.password = "shhhh";

    EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "a_proxy"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_user"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "shhhh"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKETIO_INTERFACE_DESCRIPTION, &socketio_config))
        .ValidateArgumentValue_io_create_parameters_AsType(UMOCK_TYPE(SOCKETIO_CONFIG*));

    // act
    http_io = http_proxy_io_get_interface_description()->concrete_io_create(&http_proxy_io_config);

    // assert
    ASSERT_IS_NOT_NULL(http_io);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_094: [ username and password shall be optional. ]*/
TEST_FUNCTION(http_proxy_io_create_with_NULL_username_and_password_succeeds)
{
    // arrange
    HTTP_PROXY_IO_CONFIG http_proxy_io_config;
    CONCRETE_IO_HANDLE http_io;

    http_proxy_io_config.hostname = "test_host";
    http_proxy_io_config.port = 443;
    http_proxy_io_config.proxy_hostname = "a_proxy";
    http_proxy_io_config.proxy_port = 4444;
    http_proxy_io_config.username = NULL;
    http_proxy_io_config.password = NULL;

    EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "a_proxy"))
        .IgnoreArgument_destination();
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKETIO_INTERFACE_DESCRIPTION, &socketio_config))
        .ValidateArgumentValue_io_create_parameters_AsType(UMOCK_TYPE(SOCKETIO_CONFIG*));

    // act
    http_io = http_proxy_io_get_interface_description()->concrete_io_create(&http_proxy_io_config);

    // assert
    ASSERT_IS_NOT_NULL(http_io);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_095: [ If one of the fields username and password is non-NULL, then the other has to be also non-NULL, otherwise http_proxy_io_create shall fail and return NULL. ]*/
TEST_FUNCTION(http_proxy_io_create_with_NULL_username_and_non_NULL_password_fails)
{
    // arrange
    HTTP_PROXY_IO_CONFIG http_proxy_io_config;
    CONCRETE_IO_HANDLE http_io;

    http_proxy_io_config.hostname = "test_host";
    http_proxy_io_config.port = 443;
    http_proxy_io_config.proxy_hostname = "a_proxy";
    http_proxy_io_config.proxy_port = 4444;
    http_proxy_io_config.username = NULL;
    http_proxy_io_config.password = "a";

    // act
    http_io = http_proxy_io_get_interface_description()->concrete_io_create(&http_proxy_io_config);

    // assert
    ASSERT_IS_NULL(http_io);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HTTP_PROXY_IO_01_095: [ If one of the fields username and password is non-NULL, then the other has to be also non-NULL, otherwise http_proxy_io_create shall fail and return NULL. ]*/
TEST_FUNCTION(http_proxy_io_create_with_non_NULL_username_and_NULL_password_fails)
{
    // arrange
    HTTP_PROXY_IO_CONFIG http_proxy_io_config;
    CONCRETE_IO_HANDLE http_io;

    http_proxy_io_config.hostname = "test_host";
    http_proxy_io_config.port = 443;
    http_proxy_io_config.proxy_hostname = "a_proxy";
    http_proxy_io_config.proxy_port = 4444;
    http_proxy_io_config.username = "a";
    http_proxy_io_config.password = NULL;

    // act
    http_io = http_proxy_io_get_interface_description()->concrete_io_create(&http_proxy_io_config);

    // assert
    ASSERT_IS_NULL(http_io);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HTTP_PROXY_IO_01_002: [ If io_create_parameters is NULL, http_proxy_io_create shall fail and return NULL. ]*/
TEST_FUNCTION(http_proxy_io_create_with_NULL_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    // act
    http_io = http_proxy_io_get_interface_description()->concrete_io_create(NULL);

    // assert
    ASSERT_IS_NULL(http_io);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HTTP_PROXY_IO_01_004: [ If the hostname or proxy_hostname member is NULL, then http_proxy_io_create shall fail and return NULL. ]*/
TEST_FUNCTION(http_proxy_io_create_with_NULL_hostname_fails)
{
    // arrange
    HTTP_PROXY_IO_CONFIG http_proxy_io_config;
    CONCRETE_IO_HANDLE http_io;

    http_proxy_io_config.hostname = NULL;
    http_proxy_io_config.port = 443;
    http_proxy_io_config.proxy_hostname = "a_proxy";
    http_proxy_io_config.proxy_port = 4444;
    http_proxy_io_config.username = "test_user";
    http_proxy_io_config.password = "shhhh";

    // act
    http_io = http_proxy_io_get_interface_description()->concrete_io_create(&http_proxy_io_config);

    // assert
    ASSERT_IS_NULL(http_io);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HTTP_PROXY_IO_01_004: [ If the hostname or proxy_hostname member is NULL, then http_proxy_io_create shall fail and return NULL. ]*/
TEST_FUNCTION(http_proxy_io_create_with_NULL_proxy_hostname_fails)
{
    // arrange
    HTTP_PROXY_IO_CONFIG http_proxy_io_config;
    CONCRETE_IO_HANDLE http_io;

    http_proxy_io_config.hostname = "a_hostname";
    http_proxy_io_config.port = 443;
    http_proxy_io_config.proxy_hostname = NULL;
    http_proxy_io_config.proxy_port = 4444;
    http_proxy_io_config.username = "test_user";
    http_proxy_io_config.password = "shhhh";

    // act
    http_io = http_proxy_io_get_interface_description()->concrete_io_create(&http_proxy_io_config);

    // assert
    ASSERT_IS_NULL(http_io);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HTTP_PROXY_IO_01_051: [ If allocating memory for the new instance fails, http_proxy_io_create shall fail and return NULL. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_007: [ If mallocAndStrcpy_s fails then http_proxy_io_create shall fail and return NULL. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_050: [ If socketio_get_interface_description fails, http_proxy_io_create shall fail and return NULL. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_012: [ If xio_create fails, http_proxy_io_create shall fail and return NULL. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_008: [ When http_proxy_io_create fails, all allocated resources up to that point shall be freed. ]*/
TEST_FUNCTION(when_a_call_made_by_http_proxy_io_create_fails_then_http_proxy_io_create_fails)
{
    // arrange
    size_t i;
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreAllArguments()
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_host"))
        .IgnoreArgument_destination()
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "a_proxy"))
        .IgnoreArgument_destination()
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test_user"))
        .IgnoreArgument_destination()
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "shhhh"))
        .IgnoreArgument_destination()
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(socketio_get_interface_description())
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(xio_create(TEST_SOCKETIO_INTERFACE_DESCRIPTION, &socketio_config))
        .ValidateArgumentValue_io_create_parameters_AsType(UMOCK_TYPE(SOCKETIO_CONFIG*))
        .SetFailReturn(NULL);

    umock_c_negative_tests_snapshot();

    for (i = 0; i < umock_c_negative_tests_call_count() - 1; i++)
    {
        char temp_str[128];
        CONCRETE_IO_HANDLE http_io;

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        (void)sprintf(temp_str, "On failed call %lu", (unsigned long)i);

        // act
        http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);

        // assert
        ASSERT_IS_NULL(http_io, temp_str);
    }
}

/* http_proxy_io_destroy */

/* Tests_SRS_HTTP_PROXY_IO_01_013: [ http_proxy_io_destroy shall free the HTTP proxy IO instance indicated by http_proxy_io. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_016: [ http_proxy_io_destroy shall destroy the underlying IO created in http_proxy_io_create by calling xio_destroy. ]*/
TEST_FUNCTION(http_proxy_io_destroy_frees_the_resources)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_destroy(TEST_IO_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HTTP_PROXY_IO_01_014: [ If http_proxy_io is NULL, http_proxy_io_destroy shall do nothing. ]*/
TEST_FUNCTION(http_proxy_io_destroy_with_NULL_does_nothing)
{
    // arrange

    // act
    http_proxy_io_get_interface_description()->concrete_io_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* http_proxy_io_open */

/* Tests_SRS_HTTP_PROXY_IO_01_017: [ http_proxy_io_open shall open the HTTP proxy IO and on success it shall return 0. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_019: [ http_proxy_io_open shall open the underlying IO by calling xio_open on the underlying IO handle created in http_proxy_io_create, while passing to it the callbacks on_underlying_io_open_complete, on_underlying_io_bytes_received and on_underlying_io_error. ]*/
TEST_FUNCTION(http_proxy_io_open_opens_the_underlying_IO)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, http_io, test_on_bytes_received, http_io, test_on_io_error, http_io);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_020: [ If xio_open fails, then http_proxy_io_open shall return a non-zero value. ]*/
TEST_FUNCTION(when_the_underlying_xio_open_fails_http_proxy_io_open_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
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
    result = http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, http_io, test_on_bytes_received, http_io, test_on_io_error, http_io);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_018: [ If any of the arguments http_proxy_io, on_io_open_complete, on_bytes_received or on_io_error are NULL then http_proxy_io_open shall return a non-zero value. ]*/
TEST_FUNCTION(http_proxy_io_open_with_NULL_open_complete_callback_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    umock_c_reset_all_calls();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_open(http_io, NULL, http_io, test_on_bytes_received, http_io, test_on_io_error, http_io);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_018: [ If any of the arguments http_proxy_io, on_io_open_complete, on_bytes_received or on_io_error are NULL then http_proxy_io_open shall return a non-zero value. ]*/
TEST_FUNCTION(http_proxy_io_open_with_NULL_bytes_received_callback_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    umock_c_reset_all_calls();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, http_io, NULL, http_io, test_on_io_error, http_io);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_018: [ If any of the arguments http_proxy_io, on_io_open_complete, on_bytes_received or on_io_error are NULL then http_proxy_io_open shall return a non-zero value. ]*/
TEST_FUNCTION(http_proxy_io_open_with_NULL_on_io_error_callback_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    umock_c_reset_all_calls();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, http_io, test_on_bytes_received, http_io, NULL, http_io);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_018: [ If any of the arguments http_proxy_io, on_io_open_complete, on_bytes_received or on_io_error are NULL then http_proxy_io_open shall return a non-zero value. ]*/
TEST_FUNCTION(http_proxy_io_open_with_NULL_handle_fails)
{
    // arrange
    int result;

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_open(NULL, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4242, test_on_io_error, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HTTP_PROXY_IO_01_021: [ If http_proxy_io_open is called while the IO was already open, http_proxy_io_open shall fail and return a non-zero value. ]*/
TEST_FUNCTION(http_proxy_io_open_after_open_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_051: [ The arguments on_io_open_complete_context, on_bytes_received_context and on_io_error_context shall be allowed to be NULL. ]*/
TEST_FUNCTION(http_proxy_io_open_with_NULL_contexts_is_allowed)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, NULL, test_on_bytes_received, NULL, test_on_io_error, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* http_proxy_io_close */

/* Tests_SRS_HTTP_PROXY_IO_01_022: [ http_proxy_io_close shall close the HTTP proxy IO and on success it shall return 0. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_024: [ http_proxy_io_close shall close the underlying IO by calling xio_close on the IO handle create in http_proxy_io_create, while passing to it the on_underlying_io_close_complete callback. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_026: [ The on_io_close_complete and on_io_close_complete_context arguments shall be saved for later use. ]*/
TEST_FUNCTION(http_proxy_io_close_closes_the_IO)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_io_open_complete_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_close_complete()
        .IgnoreArgument_callback_context();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_close(http_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_023: [ If the argument http_proxy_io is NULL, http_proxy_io_close shall fail and return a non-zero value. ]*/
TEST_FUNCTION(http_proxy_io_close_with_NULL_handle_fails)
{
    // arrange
    int result;

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_close(NULL, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HTTP_PROXY_IO_01_025: [ If xio_close fails, http_proxy_io_close shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_xio_close_fails_http_proxy_io_close_also_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_io_open_complete_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_close_complete()
        .IgnoreArgument_callback_context()
        .SetReturn(1);

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_close(http_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_027: [ If http_proxy_io_close is called when not open, http_proxy_io_close shall fail and return a non-zero value. ]*/
TEST_FUNCTION(http_proxy_io_close_when_not_open_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    umock_c_reset_all_calls();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_close(http_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_027: [ If http_proxy_io_close is called when not open, http_proxy_io_close shall fail and return a non-zero value. ]*/
TEST_FUNCTION(http_proxy_io_close_when_already_closed_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_io_open_complete_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    (void)http_proxy_io_get_interface_description()->concrete_io_close(http_io, test_on_io_close_complete, (void*)0x4245);
    g_on_io_close_complete(g_on_io_close_complete_context);
    umock_c_reset_all_calls();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_close(http_io, test_on_io_close_complete, (void*)0x4245);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_028: [ on_io_close_complete shall be allowed to be NULL. ]*/
TEST_FUNCTION(http_proxy_io_close_with_NULL_close_complete_callback_is_allowed)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_io_open_complete_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_close_complete()
        .IgnoreArgument_callback_context();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_close(http_io, NULL, (void*)0x4245);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_052: [ on_io_close_complete_context shall be allowed to be NULL. ]*/
TEST_FUNCTION(http_proxy_io_close_with_NULL_close_complete_callback_context_is_allowed)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_io_open_complete_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_close_complete()
        .IgnoreArgument_callback_context();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_close(http_io, test_on_io_close_complete, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_053: [ http_proxy_io_close while OPENING shall trigger the on_io_open_complete callback with IO_OPEN_CANCELLED. ]*/
TEST_FUNCTION(http_proxy_io_close_while_opening_indicates_open_as_cancelled)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_CANCELLED));

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_close(http_io, test_on_io_close_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_053: [ http_proxy_io_close while OPENING shall trigger the on_io_open_complete callback with IO_OPEN_CANCELLED. ]*/
TEST_FUNCTION(http_proxy_io_close_while_opening_waiting_for_reply_indicates_open_as_cancelled)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_CANCELLED));

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_close(http_io, test_on_io_close_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_054: [ http_proxy_io_close while OPENING shall fail and return a non-zero value. ]*/
TEST_FUNCTION(http_proxy_io_close_while_closing_indicates_open_as_cancelled)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    (void)http_proxy_io_get_interface_description()->concrete_io_close(http_io, test_on_io_close_complete, (void*)0x4242);
    umock_c_reset_all_calls();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_close(http_io, test_on_io_close_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* http_proxy_io_send */

/* Tests_SRS_HTTP_PROXY_IO_01_029: [ http_proxy_io_send shall send the size bytes pointed to by buffer and on success it shall return 0. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_033: [ http_proxy_io_send shall send the bytes by calling xio_send on the underlying IO created in http_proxy_io_create and passing buffer and size as arguments. ]*/
TEST_FUNCTION(http_proxy_io_send_calls_send_on_the_underlying_IO)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;
    unsigned char test_buffer[] = { 0x42 };

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_io_open_complete_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, sizeof(test_buffer), test_on_send_complete, (void*)0x4247))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer));

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_send(http_io, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4247);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_030: [ If any of the arguments http_proxy_io or buffer is NULL, http_proxy_io_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(http_proxy_io_send_with_NULL_handle_fails)
{
    // arrange
    int result;
    unsigned char test_buffer[] = { 0x42 };

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_send(NULL, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4247);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HTTP_PROXY_IO_01_030: [ If any of the arguments http_proxy_io or buffer is NULL, http_proxy_io_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(http_proxy_io_send_with_NULL_buffer_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_io_open_complete_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    umock_c_reset_all_calls();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_send(http_io, NULL, 1, test_on_send_complete, (void*)0x4247);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_031: [ If size is 0, http_proxy_io_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(http_proxy_io_send_with_zero_size_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;
    unsigned char test_buffer[] = { 0x42 };

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_io_open_complete_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    umock_c_reset_all_calls();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_send(http_io, test_buffer, 0, test_on_send_complete, (void*)0x4247);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_032: [ on_send_complete shall be allowed to be NULL. ]*/
TEST_FUNCTION(http_proxy_io_send_with_NULL_send_complete_callback_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;
    unsigned char test_buffer[] = { 0x42 };

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_io_open_complete_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, sizeof(test_buffer), NULL, (void*)0x4247))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer));

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_send(http_io, test_buffer, sizeof(test_buffer), NULL, (void*)0x4247);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_034: [ If http_proxy_io_send is called when the IO is not open, http_proxy_io_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(http_proxy_io_send_when_waiting_for_connect_reply_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;
    unsigned char test_buffer[] = { 0x42 };

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_send(http_io, test_buffer, sizeof(test_buffer), NULL, (void*)0x4247);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_034: [ If http_proxy_io_send is called when the IO is not open, http_proxy_io_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(http_proxy_io_send_when_opening_underlying_IO_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;
    unsigned char test_buffer[] = { 0x42 };

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_send(http_io, test_buffer, sizeof(test_buffer), NULL, (void*)0x4247);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_034: [ If http_proxy_io_send is called when the IO is not open, http_proxy_io_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(http_proxy_io_send_when_open_not_called_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;
    unsigned char test_buffer[] = { 0x42 };

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    umock_c_reset_all_calls();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_send(http_io, test_buffer, sizeof(test_buffer), NULL, (void*)0x4247);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_034: [ If http_proxy_io_send is called when the IO is not open, http_proxy_io_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(http_proxy_io_send_when_closing_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;
    unsigned char test_buffer[] = { 0x42 };

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_io_open_complete_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    (void)http_proxy_io_get_interface_description()->concrete_io_close(http_io, test_on_io_close_complete, (void*)0x4247);
    umock_c_reset_all_calls();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_send(http_io, test_buffer, sizeof(test_buffer), NULL, (void*)0x4247);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_035: [ If the IO is in an error state (an error was reported through the on_io_error callback), http_proxy_io_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(http_proxy_io_send_when_IO_is_in_error_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;
    unsigned char test_buffer[] = { 0x42 };

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_io_open_complete_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    g_on_io_error(g_on_io_error_context);
    umock_c_reset_all_calls();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_send(http_io, test_buffer, sizeof(test_buffer), NULL, (void*)0x4247);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_055: [ If xio_send fails, http_proxy_io_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_xio_send_fails_http_proxy_io_send_also_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;
    unsigned char test_buffer[] = { 0x42 };

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_io_open_complete_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, sizeof(test_buffer), NULL, (void*)0x4247))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer))
        .SetReturn(1);

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_send(http_io, test_buffer, sizeof(test_buffer), NULL, (void*)0x4247);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* http_proxy_io_dowork */

/* Tests_SRS_HTTP_PROXY_IO_01_037: [ http_proxy_io_dowork shall call xio_dowork on the underlying IO created in http_proxy_io_create. ]*/
TEST_FUNCTION(http_proxy_io_dowork_calls_the_underlying_IO_dowork)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_io_open_complete_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_dowork(TEST_IO_HANDLE));

    // act
    http_proxy_io_get_interface_description()->concrete_io_dowork(http_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_038: [ If the http_proxy_io argument is NULL, http_proxy_io_dowork shall do nothing. ]*/
TEST_FUNCTION(http_proxy_io_dowork_with_NULL_handle_does_nothing)
{
    // arrange

    // act
    http_proxy_io_get_interface_description()->concrete_io_dowork(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HTTP_PROXY_IO_01_039: [ If the IO is not open (no open has been called or the IO has been closed) then http_proxy_io_dowork shall do nothing. ]*/
TEST_FUNCTION(http_proxy_io_dowork_when_not_open_does_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    umock_c_reset_all_calls();

    // act
    http_proxy_io_get_interface_description()->concrete_io_dowork(http_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_039: [ If the IO is not open (no open has been called or the IO has been closed) then http_proxy_io_dowork shall do nothing. ]*/
TEST_FUNCTION(http_proxy_io_dowork_when_closed_does_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_io_open_complete_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    (void)http_proxy_io_get_interface_description()->concrete_io_close(http_io, NULL, NULL);
    g_on_io_close_complete(g_on_io_close_complete_context);
    umock_c_reset_all_calls();

    // act
    http_proxy_io_get_interface_description()->concrete_io_dowork(http_io);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* http_proxy_io_set_option */

/* Tests_SRS_HTTP_PROXY_IO_01_040: [ If any of the arguments http_proxy_io or option_name is NULL, http_proxy_io_set_option shall return a non-zero value. ]*/
TEST_FUNCTION(http_proxy_io_set_option_with_NULL_option_name_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    umock_c_reset_all_calls();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_setoption(http_io, NULL, "test");

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_040: [ If any of the arguments http_proxy_io or option_name is NULL, http_proxy_io_set_option shall return a non-zero value. ]*/
TEST_FUNCTION(http_proxy_io_set_option_with_NULL_handle_fails)
{
    // arrange
    int result;

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_setoption(NULL, "option_1", "test");

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HTTP_PROXY_IO_01_042: [ If the option was handled by http_proxy_io_set_option or the underlying IO, then http_proxy_io_set_option shall return 0. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_043: [ If the option_name argument indicates an option that is not handled by http_proxy_io_set_option, then xio_setoption shall be called on the underlying IO created in http_proxy_io_create, passing the option name and value to it. ]*/
TEST_FUNCTION(when_the_underlying_IO_handles_the_option_http_proxy_io_set_option_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_setoption(TEST_IO_HANDLE, "option_1", "test"))
        .ValidateArgumentValue_value_AsType(UMOCK_TYPE(char*));

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_setoption(http_io, "option_1", "test");

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_044: [ if xio_setoption fails, http_proxy_io_set_option shall return a non-zero value. ]*/
TEST_FUNCTION(when_the_underlying_xio_setoption_fails_http_proxy_io_set_option_also_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_setoption(TEST_IO_HANDLE, "option_1", "test"))
        .ValidateArgumentValue_value_AsType(UMOCK_TYPE(char*))
        .SetReturn(1);

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_setoption(http_io, "option_1", "test");

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_056: [ The value argument shall be allowed to be NULL. ]*/
TEST_FUNCTION(http_proxy_io_set_option_with_NULL_value_is_allowed)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_setoption(TEST_IO_HANDLE, "option_2", NULL))
        .ValidateArgumentValue_value_AsType(UMOCK_TYPE(char*));

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_setoption(http_io, "option_2", NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* http_proxy_io_retrieve_options */

/* Tests_SRS_HTTP_PROXY_IO_01_046: [ http_proxy_io_retrieve_options shall return an OPTIONHANDLER_HANDLE obtained by calling xio_retrieveoptions on the underlying IO created in http_proxy_io_create. ]*/
TEST_FUNCTION(http_proxy_io_retrieve_options_calls_the_underlying_retrieve_options)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    OPTIONHANDLER_HANDLE result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_retrieveoptions(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(OptionHandler_AddOption(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(OptionHandler_Destroy(IGNORED_PTR_ARG));

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_retrieveoptions(http_io);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, TEST_OPTION_HANDLER, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* GitHub PR 564 */
TEST_FUNCTION(http_proxy_io_clone_option_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_retrieveoptions(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(OptionHandler_AddOption(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    http_proxy_io_get_interface_description()->concrete_io_retrieveoptions(http_io);

    ASSERT_IS_NOT_NULL(tlsio_clone_option);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(OptionHandler_Clone(TEST_OPTION_HANDLER))
        .SetReturn(TEST_OPTION_HANDLER);

    // act
    void* cloned_value = tlsio_clone_option(OPTION_UNDERLYING_IO_OPTIONS, TEST_OPTION_HANDLER);

    // assert
    ASSERT_IS_NOT_NULL(cloned_value);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* GitHub PR 564 */
TEST_FUNCTION(http_proxy_io_destroy_option_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_retrieveoptions(TEST_IO_HANDLE));
    STRICT_EXPECTED_CALL(OptionHandler_AddOption(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    http_proxy_io_get_interface_description()->concrete_io_retrieveoptions(http_io);

    ASSERT_IS_NOT_NULL(tlsio_destroy_option);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(OptionHandler_Destroy(TEST_OPTION_HANDLER));

    // act
    tlsio_destroy_option(OPTION_UNDERLYING_IO_OPTIONS, TEST_OPTION_HANDLER);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_047: [ If the parameter http_proxy_io is NULL then http_proxy_io_retrieve_options shall fail and return NULL. ]*/
TEST_FUNCTION(http_proxy_io_retrieve_options_with_NULL_handle_fails)
{
    // arrange
    OPTIONHANDLER_HANDLE result;

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_retrieveoptions(NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HTTP_PROXY_IO_01_048: [ If xio_retrieveoptions fails, http_proxy_io_retrieve_options shall return NULL. ]*/
TEST_FUNCTION(when_xio_retrieveoptions_fails_then_http_proxy_io_retrieve_options_fails)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    OPTIONHANDLER_HANDLE result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&default_http_proxy_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_retrieveoptions(TEST_IO_HANDLE))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(OptionHandler_Destroy(IGNORED_PTR_ARG));

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_retrieveoptions(http_io);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* http_proxy_io_get_interface_description */

/* Tests_SRS_HTTP_PROXY_IO_01_049: [ http_proxy_io_get_interface_description shall return a pointer to an IO_INTERFACE_DESCRIPTION structure that contains pointers to the functions: http_proxy_io_retrieve_options, http_proxy_io_retrieve_create, http_proxy_io_destroy, http_proxy_io_open, http_proxy_io_close, http_proxy_io_send and http_proxy_io_dowork. ]*/
TEST_FUNCTION(http_proxy_io_get_interface_description_returns_a_structure_with_non_NULL_members)
{
    // arrange
    const IO_INTERFACE_DESCRIPTION* io_interface;

    // act
    io_interface = http_proxy_io_get_interface_description();

    // assert
    ASSERT_IS_NOT_NULL(io_interface);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_create);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_destroy);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_open);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_close);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_send);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_setoption);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_retrieveoptions);
}

/* on_underlying_io_open_complete */

/* Tests_SRS_HTTP_PROXY_IO_01_081: [ on_underlying_io_open_complete called with NULL context shall do nothing. ]*/
TEST_FUNCTION(underlying_io_open_complete_with_NULL_does_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_no_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    // act
    g_on_io_open_complete(NULL, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_057: [ When on_underlying_io_open_complete is called, the http_proxy_io shall send the CONNECT request constructed per RFC 2817: ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_075: [ The Request-URI portion of the Request-Line is always an 'authority' as defined by URI Generic Syntax [2], which is to say the host name and port number destination of the requested connection separated by a colon: ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_063: [ The request shall be sent by calling xio_send and passing NULL as on_send_complete callback. ]*/
TEST_FUNCTION(when_the_underlying_io_open_complete_is_called_the_CONNECT_request_is_sent)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    const char connect_request[] = "CONNECT test_host:443 HTTP/1.1\r\nHost:test_host:443\r\n\r\n";

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_no_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, sizeof(connect_request) - 1, IGNORED_PTR_ARG, NULL))
        .ValidateArgumentBuffer(2, connect_request, sizeof(connect_request) - 1);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_064: [ If xio_send fails, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(when_xio_send_fails_on_open_complete_is_triggered_with_IO_OPEN_ERROR)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    const char connect_request[] = "CONNECT test_host:443 HTTP/1.1\r\nHost:test_host:443\r\n\r\n";

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_no_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, sizeof(connect_request) - 1, IGNORED_PTR_ARG, NULL))
        .ValidateArgumentBuffer(2, connect_request, sizeof(connect_request) - 1)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_017: [ http_proxy_io_open shall open the HTTP proxy IO and on success it shall return 0. ]*/
TEST_FUNCTION(http_proxy_io_open_after_CONNECT_request_send_error_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    const char connect_request[] = "CONNECT test_host:443 HTTP/1.1\r\nHost:test_host:443\r\n\r\n";
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_no_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, sizeof(connect_request) - 1, IGNORED_PTR_ARG, NULL))
        .ValidateArgumentBuffer(2, connect_request, sizeof(connect_request) - 1)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

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
    result = http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_062: [ If any failure is encountered while constructing the request, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_connect_request_fails_on_open_complete_is_triggered_with_IO_OPEN_ERROR)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_no_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_017: [ http_proxy_io_open shall open the HTTP proxy IO and on success it shall return 0. ]*/
TEST_FUNCTION(http_proxy_io_open_after_CONNECT_request_allocation_error_error_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_no_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

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
    result = http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_059: [ - If username and password have been specified in the arguments passed to http_proxy_io_create, then the header Proxy-Authorization shall be added to the request. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_061: [ Encoding to Base64 shall be done by calling Azure_Base64_Encode_Bytes. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_060: [ - The value of Proxy-Authorization shall be the constructed according to RFC 2617. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_091: [ To receive authorization, the client sends the userid and password, separated by a single colon (":") character, within a base64 [7] encoded string in the credentials. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_092: [ A client MAY preemptively send the corresponding Authorization header with requests for resources in that space without receipt of another challenge from the server. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_093: [ Userids might be case sensitive. ]*/
TEST_FUNCTION(when_the_underlying_io_open_complete_is_called_the_CONNECT_request_with_auth_is_sent)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    const char connect_request[] = "CONNECT another_test_host:445 HTTP/1.1\r\nHost:another_test_host:445\r\nProxy-authorization: Basic __encoded_base64__\r\n\r\n";
    const char plain_auth_string[] = "le_user:le_password";
    const char base64encoded[] = "__encoded_base64__";

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)); // auth
    STRICT_EXPECTED_CALL(Azure_Base64_Encode_Bytes(IGNORED_NUM_ARG, sizeof(plain_auth_string) - 1))
        .ValidateArgumentBuffer(1, plain_auth_string, sizeof(plain_auth_string) - 1);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // auth
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_STRING_HANDLE))
        .SetReturn(base64encoded);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, sizeof(connect_request) - 1, IGNORED_PTR_ARG, NULL))
        .ValidateArgumentBuffer(2, connect_request, sizeof(connect_request) - 1);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_STRING_HANDLE));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_059: [ - If username and password have been specified in the arguments passed to http_proxy_io_create, then the header Proxy-Authorization shall be added to the request. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_061: [ Encoding to Base64 shall be done by calling Azure_Base64_Encode_Bytes. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_060: [ - The value of Proxy-Authorization shall be the constructed according to RFC 2617. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_091: [ To receive authorization, the client sends the userid and password, separated by a single colon (":") character, within a base64 [7] encoded string in the credentials. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_092: [ A client MAY preemptively send the corresponding Authorization header with requests for resources in that space without receipt of another challenge from the server. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_093: [ Userids might be case sensitive. ]*/
TEST_FUNCTION(when_the_underlying_io_open_complete_is_called_the_CONNECT_request_with_auth_is_sent_cased)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    const char connect_request[] = "CONNECT another_test_host:445 HTTP/1.1\r\nHost:another_test_host:445\r\nProxy-authorization: Basic __encoded_base64__\r\n\r\n";
    const char plain_auth_string[] = "lE_uSeR:lE_pAsSwOrD";
    const char base64encoded[] = "__encoded_base64__";

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username_cased);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)); // auth
    STRICT_EXPECTED_CALL(Azure_Base64_Encode_Bytes(IGNORED_NUM_ARG, sizeof(plain_auth_string) - 1))
        .ValidateArgumentBuffer(1, plain_auth_string, sizeof(plain_auth_string) - 1);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // auth
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_STRING_HANDLE))
        .SetReturn(base64encoded);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(TEST_IO_HANDLE, IGNORED_PTR_ARG, sizeof(connect_request) - 1, IGNORED_PTR_ARG, NULL))
        .ValidateArgumentBuffer(2, connect_request, sizeof(connect_request) - 1);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_STRING_HANDLE));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_062: [ If any failure is encountered while constructing the request, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(when_Base64_Encode_Bytes_fails_on_open_complete_is_triggered_with_IO_OPEN_ERROR)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    const char plain_auth_string[] = "le_user:le_password";

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)); // auth
    STRICT_EXPECTED_CALL(Azure_Base64_Encode_Bytes(IGNORED_NUM_ARG, sizeof(plain_auth_string) - 1))
        .ValidateArgumentBuffer(1, plain_auth_string, sizeof(plain_auth_string) - 1)
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // auth

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_062: [ If any failure is encountered while constructing the request, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(after_Base64_Encode_Bytes_fails_http_proxy_io_open_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    const char plain_auth_string[] = "le_user:le_password";
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)); // auth
    STRICT_EXPECTED_CALL(Azure_Base64_Encode_Bytes(IGNORED_NUM_ARG, sizeof(plain_auth_string) - 1))
        .ValidateArgumentBuffer(1, plain_auth_string, sizeof(plain_auth_string) - 1)
        .SetReturn(NULL);

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
    result = http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_062: [ If any failure is encountered while constructing the request, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_plain_auth_string_fails_on_open_complete_is_triggered_with_IO_OPEN_ERROR)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL); // auth
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_062: [ If any failure is encountered while constructing the request, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(after_allocating_memory_for_the_plain_auth_string_fails_http_proxy_io_open_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL); // auth

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
    result = http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_078: [ When on_underlying_io_open_complete is called with IO_OPEN_ERROR, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(on_underlying_io_open_complete_with_error_yields_an_error)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_ERROR);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_078: [ When on_underlying_io_open_complete is called with IO_OPEN_ERROR, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(after_on_underlying_io_open_complete_with_error_http_proxy_io_open_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
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
    result = http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_079: [ When on_underlying_io_open_complete is called with IO_OPEN_CANCELLED, the on_open_complete callback shall be triggered with IO_OPEN_CANCELLED, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(on_underlying_io_open_complete_with_cancelled_yields_an_error)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_CANCELLED));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_CANCELLED);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_079: [ When on_underlying_io_open_complete is called with IO_OPEN_CANCELLED, the on_open_complete callback shall be triggered with IO_OPEN_CANCELLED, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(after_on_underlying_io_open_complete_with_cancelled_http_proxy_io_open_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
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
    result = http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_076: [ When on_underlying_io_open_complete is called while waiting for the CONNECT reply, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(when_on_underlying_io_open_complete_is_called_when_waiting_for_connect_reply_an_error_is_indicated)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_076: [ When on_underlying_io_open_complete is called while waiting for the CONNECT reply, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(after_on_underlying_io_open_complete_is_called_when_waiting_for_connect_reply_http_proxy_io_open_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
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
    result = http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_077: [ When on_underlying_io_open_complete is called in after OPEN has completed, the on_io_error callback shall be triggered passing the on_io_error_context argument as context. ]*/
TEST_FUNCTION(on_underlying_io_open_complete_in_OPEN_indicates_an_error)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_io_open_complete_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_077: [ When on_underlying_io_open_complete is called in after OPEN has completed, the on_io_error callback shall be triggered passing the on_io_error_context argument as context. ]*/
TEST_FUNCTION(on_underlying_io_open_complete_in_CLOSING_indicates_an_error)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_io_open_complete_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    (void)http_proxy_io_get_interface_description()->concrete_io_close(http_io, test_on_io_close_complete, (void*)0x4246);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* on_underlying_io_bytes_received */

/* Tests_SRS_HTTP_PROXY_IO_01_065: [ When bytes are received and the response to the CONNECT request was not yet received, the bytes shall be accumulated until a double new-line is detected. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_1_byte_buffers_the_received_bytes)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response, 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_065: [ When bytes are received and the response to the CONNECT request was not yet received, the bytes shall be accumulated until a double new-line is detected. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_2_times_1_byte_buffers_the_received_bytes)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response, 1);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response + 1, 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_066: [ When a double new-line is detected the response shall be parsed in order to extract the status code. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_069: [ Any successful (2xx) response to a CONNECT request indicates that the proxy has established a connection to the requested host and port, and has switched to tunneling the current connection to that server connection. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_070: [ When a success status code is parsed, the on_open_complete callback shall be triggered with IO_OPEN_OK, passing also the on_open_complete_context argument as context. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_073: [ Once a success status code was parsed, the IO shall be OPEN. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_a_good_reply_indicates_OPEN_OK)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_OK));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_066: [ When a double new-line is detected the response shall be parsed in order to extract the status code. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_a_good_reply_in_2_chunks_indicates_OPEN_OK)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response, sizeof(connect_response) - 2);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_OK));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response + sizeof(connect_response) - 2, 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_067: [ If allocating memory for the buffered bytes fails, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(when_allocating_memory_for_cached_data_in_on_underlying_io_bytes_an_error_is_triggered)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_066: [ When a double new-line is detected the response shall be parsed in order to extract the status code. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_069: [ Any successful (2xx) response to a CONNECT request indicates that the proxy has established a connection to the requested host and port, and has switched to tunneling the current connection to that server connection. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_070: [ When a success status code is parsed, the on_open_complete callback shall be triggered with IO_OPEN_OK, passing also the on_open_complete_context argument as context. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_073: [ Once a success status code was parsed, the IO shall be OPEN. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_a_good_reply_status_code_201_indicates_OPEN_OK)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    static const char connect_response_201[] = "HTTP/1.1 201\r\n\r\n";

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_OK));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response_201, sizeof(connect_response_201) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_066: [ When a double new-line is detected the response shall be parsed in order to extract the status code. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_069: [ Any successful (2xx) response to a CONNECT request indicates that the proxy has established a connection to the requested host and port, and has switched to tunneling the current connection to that server connection. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_070: [ When a success status code is parsed, the on_open_complete callback shall be triggered with IO_OPEN_OK, passing also the on_open_complete_context argument as context. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_073: [ Once a success status code was parsed, the IO shall be OPEN. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_a_good_reply_status_code_299_indicates_OPEN_OK)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    static const char connect_response_299[] = "HTTP/1.1 299\r\n\r\n";

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_OK));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response_299, sizeof(connect_response_299) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_066: [ When a double new-line is detected the response shall be parsed in order to extract the status code. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_069: [ Any successful (2xx) response to a CONNECT request indicates that the proxy has established a connection to the requested host and port, and has switched to tunneling the current connection to that server connection. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_070: [ When a success status code is parsed, the on_open_complete callback shall be triggered with IO_OPEN_OK, passing also the on_open_complete_context argument as context. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_073: [ Once a success status code was parsed, the IO shall be OPEN. ]*/
/* Tests_SRS_HTTP_PROXY_IO_01_090: [ Any successful (2xx) response to a CONNECT request indicates that the proxy has established a connection to the requested host and port, and has switched to tunneling the current connection to that server connection. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_a_good_reply_status_code_200_and_some_text_indicates_OPEN_OK)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    static const char connect_response_200[] = "HTTP/1.1 200 Blah blah\r\n\r\n";

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_OK));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response_200, sizeof(connect_response_200) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_071: [ If the status code is not successful, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_a_199_code_indicates_an_error)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    static const char connect_response_199[] = "HTTP/1.1 199\r\n\r\n";

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response_199, sizeof(connect_response_199) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_071: [ If the status code is not successful, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_a_300_code_indicates_an_error)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    static const char connect_response_300[] = "HTTP/1.1 300\r\n\r\n";

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response_300, sizeof(connect_response_300) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_071: [ If the status code is not successful, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(after_a_bad_status_code_http_proxy_io_open_succeeds)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    static const char connect_response_300[] = "HTTP/1.1 300\r\n\r\n";
    int result;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response_300, sizeof(connect_response_300) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument_on_io_open_complete()
        .IgnoreArgument_on_io_open_complete_context()
        .IgnoreArgument_on_bytes_received()
        .IgnoreArgument_on_bytes_received_context()
        .IgnoreArgument_on_io_error()
        .IgnoreArgument_on_io_error_context();

    // act
    result = http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_072: [ Any bytes that are extra (not consumed by the CONNECT response), shall be indicated as received by calling the on_bytes_received callback and passing the on_bytes_received_context as context argument. ]*/
TEST_FUNCTION(one_extra_byte_gets_indicated_as_received)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    static const char connect_response_with_byte[] = "HTTP/1.1 200\r\n\r\nA";
    static const unsigned char expected_bytes[] = { 'A' };

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_OK));
    STRICT_EXPECTED_CALL(test_on_bytes_received((void*)0x4243, IGNORED_PTR_ARG, sizeof(expected_bytes)))
        .ValidateArgumentBuffer(2, expected_bytes, sizeof(expected_bytes));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response_with_byte, sizeof(connect_response_with_byte) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_072: [ Any bytes that are extra (not consumed by the CONNECT response), shall be indicated as received by calling the on_bytes_received callback and passing the on_bytes_received_context as context argument. ]*/
TEST_FUNCTION(three_extra_byte_get_indicated_as_received)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    static const char connect_response_with_byte[] = "HTTP/1.1 200\r\n\r\nABC";
    static const unsigned char expected_bytes[] = { 'A', 'B', 'C' };

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_OK));
    STRICT_EXPECTED_CALL(test_on_bytes_received((void*)0x4243, IGNORED_PTR_ARG, sizeof(expected_bytes)))
        .ValidateArgumentBuffer(2, expected_bytes, sizeof(expected_bytes));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response_with_byte, sizeof(connect_response_with_byte) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_074: [ If on_underlying_io_bytes_received is called while OPEN, all bytes shall be indicated as received by calling the on_bytes_received callback and passing the on_bytes_received_context as context argument. ]*/
TEST_FUNCTION(bytes_indicated_as_received_in_OPEN_get_bubbled_up)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    static const unsigned char expected_bytes[] = { 'A', 'B', 'C' };

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_bytes_received((void*)0x4243, IGNORED_PTR_ARG, sizeof(expected_bytes)))
        .ValidateArgumentBuffer(2, expected_bytes, sizeof(expected_bytes));

    // act
    g_on_bytes_received(g_on_bytes_received_context, expected_bytes, sizeof(expected_bytes));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_080: [ If on_underlying_io_bytes_received is called while the underlying IO is being opened, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(if_bytes_are_indicated_as_received_while_opening_the_underlying_IO_an_error_is_indicated_in_the_open_complete_callback)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_068: [ If parsing the CONNECT response fails, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(a_bad_reply_triggers_an_error_in_open_complete_callback)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    static const char bad_reply[] = "HTTP/1.1 \r\n\r\n";

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)bad_reply, sizeof(bad_reply) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_068: [ If parsing the CONNECT response fails, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(a_bad_reply_malformed_char_triggers_an_error_in_open_complete_callback)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    static const char bad_reply[] = "HYTP/1.1 200\r\n\r\n";

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)bad_reply, sizeof(bad_reply) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_068: [ If parsing the CONNECT response fails, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(a_bad_reply_only_one_char_triggers_an_error_in_open_complete_callback)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;
    static const char bad_reply[] = "H\r\n\r\n";

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)bad_reply, sizeof(bad_reply) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_082: [ on_underlying_io_bytes_received called with NULL context shall do nothing. ]*/
TEST_FUNCTION(on_underlying_io_bytes_received_with_NULL_does_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    // act
    g_on_bytes_received(NULL, (const unsigned char*)connect_response, sizeof(connect_response) - 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* on_underlying_io_close_complete */

/* Tests_SRS_HTTP_PROXY_IO_01_083: [ on_underlying_io_close_complete while CLOSING shall call the on_io_close_complete callback, passing to it the on_io_close_complete_context as context argument. ]*/
TEST_FUNCTION(on_underlying_io_close_complete_in_CLOSING_triggers_the_close_complete_callback)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    (void)http_proxy_io_get_interface_description()->concrete_io_close(http_io, test_on_io_close_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_close_complete((void*)0x4245));

    // act
    g_on_io_close_complete(g_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_084: [ on_underlying_io_close_complete called with NULL context shall do nothing. ]*/
TEST_FUNCTION(on_underlying_io_close_complete_in_OPEN_does_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    (void)http_proxy_io_get_interface_description()->concrete_io_close(http_io, test_on_io_close_complete, (void*)0x4245);
    umock_c_reset_all_calls();

    // act
    g_on_io_close_complete(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_086: [ If the on_io_close_complete callback passed to http_proxy_io_close was NULL, no callback shall be triggered. ]*/
TEST_FUNCTION(on_underlying_io_close_complete_in_CLOSING_with_NULL_callback_does_not_trigger_any_callback)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    (void)http_proxy_io_get_interface_description()->concrete_io_close(http_io, NULL, (void*)0x4245);
    umock_c_reset_all_calls();

    // act
    g_on_io_close_complete(g_on_io_close_complete_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* on_underlying_io_error */

/* Tests_SRS_HTTP_PROXY_IO_01_088: [ on_underlying_io_error called with NULL context shall do nothing. ]*/
TEST_FUNCTION(on_underlying_io_error_with_NULL_handle_does_nothing)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    umock_c_reset_all_calls();

    // act
    g_on_io_error(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_089: [ If the on_underlying_io_error callback is called while the IO is OPEN, the on_io_error callback shall be called with the on_io_error_context argument as context. ]*/
TEST_FUNCTION(when_on_underlying_io_error_is_called_in_OPEN_the_error_is_indicated_up)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    g_on_bytes_received(g_on_bytes_received_context, (const unsigned char*)connect_response, sizeof(connect_response) - 1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    // act
    g_on_io_error(g_on_io_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_087: [ If the on_underlying_io_error callback is called while OPENING, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(when_on_underlying_io_error_is_called_while_waiting_for_CONNECT_response_the_error_is_indicated_via_open_complete)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    g_on_io_open_complete(g_on_io_open_complete_context, IO_OPEN_OK);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    g_on_io_error(g_on_io_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

/* Tests_SRS_HTTP_PROXY_IO_01_087: [ If the on_underlying_io_error callback is called while OPENING, the on_open_complete callback shall be triggered with IO_OPEN_ERROR, passing also the on_open_complete_context argument as context. ]*/
TEST_FUNCTION(when_on_underlying_io_error_is_called_while_waiting_for_underlying_IO_to_open_the_error_is_indicated_via_open_complete)
{
    // arrange
    CONCRETE_IO_HANDLE http_io;

    http_io = http_proxy_io_get_interface_description()->concrete_io_create((void*)&http_proxy_io_config_with_username);
    (void)http_proxy_io_get_interface_description()->concrete_io_open(http_io, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(xio_close(TEST_IO_HANDLE, NULL, NULL));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_ERROR));

    // act
    g_on_io_error(g_on_io_error_context);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    http_proxy_io_get_interface_description()->concrete_io_destroy(http_io);
}

END_TEST_SUITE(http_proxy_io_unittests)
