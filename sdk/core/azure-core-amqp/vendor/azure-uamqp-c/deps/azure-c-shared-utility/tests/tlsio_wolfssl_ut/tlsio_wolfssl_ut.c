// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#else
#include <stdlib.h>
#include <stddef.h>
#endif

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void* my_gballoc_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_stdint.h"
#include "umock_c/umock_c_negative_tests.h"
#include "azure_macro_utils/macro_utils.h"

#include "wolfssl/options.h"
#include "wolfssl/ssl.h"
#include "wolfssl/error-ssl.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "umock_c/umock_c_prod.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/xio.h"

MOCKABLE_FUNCTION(, void, on_bytes_recv, void*, context, const unsigned char*, buffer, size_t, size);
MOCKABLE_FUNCTION(, void, on_error, void*, context);
MOCKABLE_FUNCTION(, void, on_close_complete, void*, context);

#undef ENABLE_MOCKS

#include "azure_c_shared_utility/tlsio_wolfssl.h"

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

#define BUFFER_LEN          10
#define TEST_DEVICE_ID      11
#define WOLFSSL_READ_LIMIT  5

static WOLFSSL_METHOD* TEST_WOLFSSL_CLIENT_METHOD = (WOLFSSL_METHOD*)0x0011;
static WOLFSSL_CTX* TEST_WOLFSSL_CTX = (WOLFSSL_CTX*)0x0012;
static WOLFSSL* TEST_WOLFSSL = (WOLFSSL*)0x0013;
static const IO_INTERFACE_DESCRIPTION* TEST_SOCKETIO_INTERFACE_DESCRIPTION = (const IO_INTERFACE_DESCRIPTION*)0x0014;
static XIO_HANDLE TEST_IO_HANDLE = (XIO_HANDLE)0x0015;
static const unsigned char TEST_BUFFER[] = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA };
static const size_t TEST_BUFFER_LEN = BUFFER_LEN;
static const char* TEST_TRUSTED_CERT = "test_trusted_cert";
static const char* TEST_HOSTNAME = "hostname.test";

static HandShakeDoneCb g_handshake_done_cb = NULL;
static void* g_handshake_done_ctx = NULL;

static ON_BYTES_RECEIVED g_on_bytes_received;
static void* g_on_bytes_received_context;
static ON_IO_ERROR g_on_io_error;
static void* g_on_io_error_context;
static CallbackIORecv g_wolfssl_cb_rcv;
static void* g_wolfssl_rcv_ctx;

MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, void, wolfSSL_SetIORecv, WOLFSSL_CTX*, ctx, CallbackIORecv, cb_rcv)
    g_wolfssl_cb_rcv = cb_rcv;
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, void, wolfSSL_SetIOSend, WOLFSSL_CTX*, ctx, CallbackIORecv, cb_rcv)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, void, wolfSSL_SetIOReadCtx, WOLFSSL*, ssl, void*, ctx)
    g_wolfssl_rcv_ctx = ctx;
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, void, wolfSSL_SetIOWriteCtx, WOLFSSL*, ssl, void*, ctx)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, WOLFSSL_METHOD*, wolfTLSv1_2_client_method)
MOCK_FUNCTION_END(TEST_WOLFSSL_CLIENT_METHOD)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, WOLFSSL_CTX*, wolfSSL_CTX_new, WOLFSSL_METHOD*, method)
MOCK_FUNCTION_END(TEST_WOLFSSL_CTX)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, WOLFSSL*, wolfSSL_new, WOLFSSL_CTX*, ctx)
MOCK_FUNCTION_END(TEST_WOLFSSL)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, void, wolfSSL_set_using_nonblock, WOLFSSL*, ssl, int, opt);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_connect, WOLFSSL*, ssl)
MOCK_FUNCTION_END(SSL_SUCCESS)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_write, WOLFSSL*, ssl, const void*, data, int, len)
MOCK_FUNCTION_END(len)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_read, WOLFSSL*, ssl, void*, buff, int, len)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, void, wolfSSL_CTX_free, WOLFSSL_CTX*, ctx)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, void, wolfSSL_free, WOLFSSL*, ssl)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, void, wolfSSL_load_error_strings)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_library_init)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_CTX_load_verify_buffer, WOLFSSL_CTX*, ctx, const unsigned char*, buff, long, len, int, opt)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_use_PrivateKey_buffer, WOLFSSL*, ssl, const unsigned char*, buff, long, len, int, opt)
MOCK_FUNCTION_END(SSL_SUCCESS)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_use_certificate_chain_buffer, WOLFSSL*, ssl, const unsigned char*, chain_buff, long, len)
MOCK_FUNCTION_END(SSL_SUCCESS)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_SetHsDoneCb, WOLFSSL*, ssl, HandShakeDoneCb, hs_cb, void*, ctx)
    g_handshake_done_cb = hs_cb;
    g_handshake_done_ctx = ctx;
MOCK_FUNCTION_END(0)
#ifdef HAVE_SECURE_RENEGOTIATION
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_UseSecureRenegotiation, WOLFSSL*, ssl)
MOCK_FUNCTION_END(SSL_SUCCESS)
#endif
#ifdef INVALID_DEVID
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_SetDevId, WOLFSSL*, ssl, int, devId)
MOCK_FUNCTION_END(WOLFSSL_SUCCESS)
#endif
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_get_error, WOLFSSL*, ssl, int, ret)
MOCK_FUNCTION_END(SSL_SUCCESS)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_check_domain_name, WOLFSSL*, ssl, const char*, dn)
MOCK_FUNCTION_END(SSL_SUCCESS)

#if defined(LIBWOLFSSL_VERSION_HEX) && LIBWOLFSSL_VERSION_HEX >= 0x04000000
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_Debugging_ON)
MOCK_FUNCTION_END(SSL_SUCCESS)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_SetLoggingCb, wolfSSL_Logging_cb, log_function)
MOCK_FUNCTION_END(SSL_SUCCESS)
#endif

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static int my_mallocAndStrcpy_s(char** destination, const char* source)
{
    *destination = (char*)malloc(strlen(source) + 1);
    (void)strcpy(*destination, source);
    return 0;
}

static void execute_wolfssl_open(ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context)
{
    on_io_open_complete(on_io_open_complete_context, IO_OPEN_OK);

    if (g_handshake_done_cb != NULL)
    {
        g_handshake_done_cb(TEST_WOLFSSL, g_handshake_done_ctx);
    }
}

static void on_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    (void)context;
    (void)open_result;
}

static void on_send_complete(void* context, IO_SEND_RESULT send_result)
{
    (void)context;
    (void)send_result;
}

static int my_xio_open(XIO_HANDLE xio, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    (void)xio;
    g_on_io_error = on_io_error;
    g_on_io_error_context = on_io_error_context;
    g_on_bytes_received = on_bytes_received;
    g_on_bytes_received_context = on_bytes_received_context;

    execute_wolfssl_open(on_io_open_complete, on_io_open_complete_context);

    return 0;
}

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(tlsio_wolfssl_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    REGISTER_UMOCK_ALIAS_TYPE(CONCRETE_IO_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(XIO_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(CallbackIORecv, void*);
#if defined(LIBWOLFSSL_VERSION_HEX) && LIBWOLFSSL_VERSION_HEX >= 0x04000000
    REGISTER_UMOCK_ALIAS_TYPE(wolfSSL_Logging_cb, void*);
#endif
    REGISTER_UMOCK_ALIAS_TYPE(HandShakeDoneCb, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_OPEN_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_RECEIVED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_ERROR, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_RECEIVED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_CLOSE_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_RECEIVED, void*);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_realloc, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);

    REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_s, my_mallocAndStrcpy_s);

    REGISTER_GLOBAL_MOCK_RETURN(socketio_get_interface_description, TEST_SOCKETIO_INTERFACE_DESCRIPTION);
    REGISTER_GLOBAL_MOCK_RETURN(xio_create, TEST_IO_HANDLE);
    REGISTER_GLOBAL_MOCK_HOOK(xio_open, my_xio_open);
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

    g_on_bytes_received = NULL;
    g_on_bytes_received_context = NULL;
    g_on_io_error = NULL;
    g_on_io_error_context = NULL;
    g_wolfssl_cb_rcv = NULL;
    g_handshake_done_cb = NULL;
    g_handshake_done_ctx = NULL;

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

TEST_FUNCTION(tlsio_wolfssl_create_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;

    /*STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(wolfTLSv1_2_client_method());
    STRICT_EXPECTED_CALL(wolfSSL_CTX_new(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(IGNORED_NUM_ARG, IGNORED_NUM_ARG));*/

    //act
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);

    //assert
    ASSERT_IS_NOT_NULL(io_handle);
    //ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //clean
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_create_config_NULL_fail)
{
    //arrange

    //act
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(NULL);

    //assert
    ASSERT_IS_NULL(io_handle);

    //clean
}

TEST_FUNCTION(tlsio_wolfssl_destroy_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(wolfSSL_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(wolfSSL_CTX_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // hostname
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // tlsio

    //act
    tlsio_wolfssl_destroy(io_handle);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //clean
}

TEST_FUNCTION(tlsio_wolfssl_destroy_handle_NULL_succeeds)
{
    //arrange

    //act
    tlsio_wolfssl_destroy(NULL);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //clean
}

TEST_FUNCTION(tlsio_wolfssl_open_handle_NULL_fail)
{
    //arrange

    //act
    int test_result = tlsio_wolfssl_open(NULL, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, test_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //clean
}

TEST_FUNCTION(tlsio_wolfssl_open_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_wolfssl_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);

    //assert
    ASSERT_ARE_EQUAL(int, 0, test_result);

    //clean
    (void)tlsio_wolfssl_close(io_handle, on_close_complete, NULL);
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_open_with_cert_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    (void)tlsio_wolfssl_setoption(io_handle, SU_OPTION_X509_CERT, TEST_TRUSTED_CERT);
    (void)tlsio_wolfssl_setoption(io_handle, SU_OPTION_X509_PRIVATE_KEY, TEST_TRUSTED_CERT);
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_wolfssl_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);

    //assert
    ASSERT_ARE_EQUAL(int, 0, test_result);

    //clean
    (void)tlsio_wolfssl_close(io_handle, on_close_complete, NULL);
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_open_set_dev_id_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(wolfSSL_SetDevId(TEST_WOLFSSL, 11));
    STRICT_EXPECTED_CALL(wolfSSL_check_domain_name(TEST_WOLFSSL, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(wolfSSL_connect(TEST_WOLFSSL));

    //act
    int device_id = TEST_DEVICE_ID;
    int test_result = tlsio_wolfssl_setoption(io_handle, OPTION_WOLFSSL_SET_DEVICE_ID, &device_id);
    ASSERT_ARE_EQUAL(int, 0, test_result);

    test_result = tlsio_wolfssl_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);

    //assert
    ASSERT_ARE_EQUAL(int, 0, test_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //clean
    (void)tlsio_wolfssl_close(io_handle, on_close_complete, NULL);
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_open_set_dev_id_2_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(wolfSSL_check_domain_name(TEST_WOLFSSL, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(xio_open(TEST_IO_HANDLE, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(wolfSSL_connect(TEST_WOLFSSL));
    STRICT_EXPECTED_CALL(wolfSSL_SetDevId(TEST_WOLFSSL, 11));

    //act
    int test_result = tlsio_wolfssl_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);
    ASSERT_ARE_EQUAL(int, 0, test_result);

    int device_id = TEST_DEVICE_ID;
    test_result = tlsio_wolfssl_setoption(io_handle, OPTION_WOLFSSL_SET_DEVICE_ID, &device_id);

    //assert
    ASSERT_ARE_EQUAL(int, 0, test_result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //clean
    (void)tlsio_wolfssl_close(io_handle, on_close_complete, NULL);
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_on_handshake_done_succeed)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    (void)tlsio_wolfssl_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);
    umock_c_reset_all_calls();

    //act
    int test_result = g_handshake_done_cb(TEST_WOLFSSL, g_handshake_done_ctx);

    //assert
    ASSERT_ARE_EQUAL(int, 0, test_result);

    //clean
    (void)tlsio_wolfssl_close(io_handle, on_close_complete, NULL);
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_close_handle_NULL_fail)
{
    //arrange

    //act
    int test_result = tlsio_wolfssl_close(NULL, on_close_complete, NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, test_result);

    //clean
}

TEST_FUNCTION(tlsio_wolfssl_close_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    (void)tlsio_wolfssl_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_wolfssl_close(io_handle, on_close_complete, NULL);

    //assert
    ASSERT_ARE_EQUAL(int, 0, test_result);

    //clean
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_close_not_open_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_wolfssl_close(io_handle, on_close_complete, NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, test_result);

    //clean
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_send_handle_NULL_fail)
{
    //arrange
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_wolfssl_send(NULL, TEST_BUFFER, TEST_BUFFER_LEN, on_send_complete, NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, test_result);

    //clean
}

TEST_FUNCTION(tlsio_wolfssl_send_buffer_0_fail)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    (void)tlsio_wolfssl_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_wolfssl_send(io_handle, NULL, 0, on_send_complete, NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, test_result);

    //clean
    (void)tlsio_wolfssl_close(io_handle, on_close_complete, NULL);
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_send_not_open_fail)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_wolfssl_send(io_handle, TEST_BUFFER, TEST_BUFFER_LEN, on_send_complete, NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, test_result);

    //clean
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_send_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    (void)tlsio_wolfssl_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_wolfssl_send(io_handle, TEST_BUFFER, TEST_BUFFER_LEN, on_send_complete, NULL);

    //assert
    ASSERT_ARE_EQUAL(int, 0, test_result);

    //clean
    (void)tlsio_wolfssl_close(io_handle, on_close_complete, NULL);
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_send_write_returns_zero_fail)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    (void)tlsio_wolfssl_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(wolfSSL_write(TEST_WOLFSSL, TEST_BUFFER, TEST_BUFFER_LEN)).SetReturn(0);

    //act
    int test_result = tlsio_wolfssl_send(io_handle, TEST_BUFFER, TEST_BUFFER_LEN, on_send_complete, NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, test_result);

    //clean
    (void)tlsio_wolfssl_close(io_handle, on_close_complete, NULL);
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_dowork_handle_NULL_succeeds)
{
    //arrange

    //act
    tlsio_wolfssl_dowork(NULL);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //clean
}

TEST_FUNCTION(tlsio_wolfssl_dowork_NOT_OPEN_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    umock_c_reset_all_calls();

    //act
    tlsio_wolfssl_dowork(NULL);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //clean
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_dowork_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    (void)tlsio_wolfssl_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(wolfSSL_read(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).CopyOutArgumentBuffer_buff(&TEST_BUFFER, BUFFER_LEN).SetReturn(BUFFER_LEN);
    STRICT_EXPECTED_CALL(on_bytes_recv(NULL, IGNORED_PTR_ARG, BUFFER_LEN));
    STRICT_EXPECTED_CALL(wolfSSL_read(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_PTR_ARG));

    //act
    tlsio_wolfssl_dowork(io_handle);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //clean
    (void)tlsio_wolfssl_close(io_handle, on_close_complete, NULL);
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_get_interface_description_succeed)
{
    //arrange
    umock_c_reset_all_calls();

    //act
    const IO_INTERFACE_DESCRIPTION* interface_desc =  tlsio_wolfssl_get_interface_description();

    //assert
    ASSERT_IS_NOT_NULL(interface_desc->concrete_io_retrieveoptions);
    ASSERT_IS_NOT_NULL(interface_desc->concrete_io_create);
    ASSERT_IS_NOT_NULL(interface_desc->concrete_io_destroy);
    ASSERT_IS_NOT_NULL(interface_desc->concrete_io_open);
    ASSERT_IS_NOT_NULL(interface_desc->concrete_io_close);
    ASSERT_IS_NOT_NULL(interface_desc->concrete_io_send);
    ASSERT_IS_NOT_NULL(interface_desc->concrete_io_dowork);
    ASSERT_IS_NOT_NULL(interface_desc->concrete_io_setoption);

    //clean
}

TEST_FUNCTION(tlsio_wolfssl_setoption_tls_io_NULL_Fail)
{
    //arrange
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_wolfssl_setoption(NULL, OPTION_TRUSTED_CERT, TEST_TRUSTED_CERT);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, test_result);

    //clean
}

TEST_FUNCTION(tlsio_wolfssl_setoption_option_name_NULL_Fail)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_wolfssl_setoption(io_handle, NULL, TEST_TRUSTED_CERT);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, test_result);

    //clean
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_setoption_trusted_cert_succeed)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_wolfssl_setoption(io_handle, OPTION_TRUSTED_CERT, TEST_TRUSTED_CERT);

    //assert
    ASSERT_ARE_EQUAL(int, 0, test_result);

    //clean
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_setoption_trusted_cert_twice_succeed)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    umock_c_reset_all_calls();

    //act
    int test_result = tlsio_wolfssl_setoption(io_handle, OPTION_TRUSTED_CERT, TEST_TRUSTED_CERT);
    ASSERT_ARE_EQUAL(int, 0, test_result);

    test_result = tlsio_wolfssl_setoption(io_handle, OPTION_TRUSTED_CERT, TEST_TRUSTED_CERT);

    //assert
    ASSERT_ARE_EQUAL(int, 0, test_result);

    //clean
    tlsio_wolfssl_destroy(io_handle);
}

#ifdef INVALID_DEVID
TEST_FUNCTION(tlsio_wolfssl_setoption_device_id_succeed)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    umock_c_reset_all_calls();

    //act
    int device_id = TEST_DEVICE_ID;
    int test_result = tlsio_wolfssl_setoption(io_handle, OPTION_WOLFSSL_SET_DEVICE_ID, &device_id);

    //assert
    ASSERT_ARE_EQUAL(int, 0, test_result);

    //clean
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_setoption_device_id_fail)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(wolfSSL_SetDevId(TEST_WOLFSSL, 11)).SetReturn(0);

    //act
    int device_id = TEST_DEVICE_ID;
    int test_result = tlsio_wolfssl_setoption(io_handle, OPTION_WOLFSSL_SET_DEVICE_ID, &device_id);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, test_result);

    //clean
    tlsio_wolfssl_destroy(io_handle);
}
#endif

#if defined(LIBWOLFSSL_VERSION_HEX) && LIBWOLFSSL_VERSION_HEX >= 0x04000000
TEST_FUNCTION(tlsio_wolfssl_setoption_debug_log_succeed)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(wolfSSL_Debugging_ON()).SetReturn(1);
    STRICT_EXPECTED_CALL(wolfSSL_SetLoggingCb(IGNORED_PTR_ARG)).SetReturn(1);

    //act
    int debugLogEnable = true;
    int test_result = tlsio_wolfssl_setoption(io_handle, "debug_log", &debugLogEnable);

    //assert
    ASSERT_ARE_EQUAL(int, 0, test_result);

    //clean
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_setoption_debug_log_disable_fail)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    umock_c_reset_all_calls();

    //act
    // Unlike standard option handlers, turning debug_log off is done via NULL instead of a non-zero int.
    int* debugLogEnable = NULL;
    int test_result = tlsio_wolfssl_setoption(io_handle, "debug_log", NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, test_result);

    //clean
    tlsio_wolfssl_destroy(io_handle);
}
#endif

TEST_FUNCTION(tlsio_wolfssl_on_underlying_io_bytes_received_ctx_NULL_succeess)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    (void)tlsio_wolfssl_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);
    umock_c_reset_all_calls();

    //act
    g_on_bytes_received(NULL, TEST_BUFFER, TEST_BUFFER_LEN);

    //assert

    //clean
    (void)tlsio_wolfssl_close(io_handle, on_close_complete, NULL);
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_on_underlying_io_bytes_received_realloc_NULL_success)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    (void)tlsio_wolfssl_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(NULL);

    //act
    g_on_bytes_received(g_on_bytes_received_context, TEST_BUFFER, TEST_BUFFER_LEN);

    //assert

    //clean
    (void)tlsio_wolfssl_close(io_handle, on_close_complete, NULL);
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_on_underlying_io_bytes_received_success)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    (void)tlsio_wolfssl_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    //act
    g_on_bytes_received(g_on_bytes_received_context, TEST_BUFFER, TEST_BUFFER_LEN);

    //assert

    //clean
    (void)tlsio_wolfssl_close(io_handle, on_close_complete, NULL);
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_on_underlying_io_error_success)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    (void)tlsio_wolfssl_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);
    umock_c_reset_all_calls();

    //act
    g_on_io_error(g_on_io_error_context);

    //assert

    //clean
    (void)tlsio_wolfssl_close(io_handle, on_close_complete, NULL);
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_on_underlying_io_error_ctx_NULL_success)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    (void)tlsio_wolfssl_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);
    umock_c_reset_all_calls();

    //act
    g_on_io_error(NULL);

    //assert

    //clean
    (void)tlsio_wolfssl_close(io_handle, on_close_complete, NULL);
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_on_io_recv_on_open_success)
{
    //arrange
    char recv_buff[BUFFER_LEN];
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    (void)tlsio_wolfssl_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);
    umock_c_reset_all_calls();

    //act
    int test_result = g_wolfssl_cb_rcv(TEST_WOLFSSL, recv_buff, BUFFER_LEN, g_wolfssl_rcv_ctx);

    //assert
    ASSERT_ARE_EQUAL(int, WOLFSSL_CBIO_ERR_WANT_READ, test_result);

    //clean
    (void)tlsio_wolfssl_close(io_handle, on_close_complete, NULL);
    tlsio_wolfssl_destroy(io_handle);
}

TEST_FUNCTION(tlsio_wolfssl_on_io_recv_timeout_success)
{
    //arrange
    char recv_buff[BUFFER_LEN];
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));
    tls_io_config.hostname = TEST_HOSTNAME;
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);
    // ensure we stay in the handshake mode
    g_handshake_done_cb = NULL;
    (void)tlsio_wolfssl_open(io_handle, on_io_open_complete, NULL, on_bytes_recv, NULL, on_error, NULL);
    umock_c_reset_all_calls();

    for (size_t index = 0; index < WOLFSSL_READ_LIMIT; index++)
    {
        STRICT_EXPECTED_CALL(xio_dowork(IGNORED_PTR_ARG));
    }

    //act
    int test_result = g_wolfssl_cb_rcv(TEST_WOLFSSL, recv_buff, BUFFER_LEN, g_wolfssl_rcv_ctx);

    //assert
    ASSERT_ARE_EQUAL(int, 0, test_result);

    //clean
    (void)tlsio_wolfssl_close(io_handle, on_close_complete, NULL);
    tlsio_wolfssl_destroy(io_handle);
}

END_TEST_SUITE(tlsio_wolfssl_ut)
