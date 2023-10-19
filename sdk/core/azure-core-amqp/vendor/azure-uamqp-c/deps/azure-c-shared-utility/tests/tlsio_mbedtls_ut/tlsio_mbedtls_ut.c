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

#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

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

/**
 * Include the C standards here.
 */
#include <time.h>

/**
 * Include the test tools.
 */
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umock_c_negative_tests.h"
#include "azure_macro_utils/macro_utils.h"

#include "mbedtls/config.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "mbedtls/entropy_poll.h"

/**
 * Include the mockable headers here.
 */
#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "umock_c/umock_c_prod.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/optionhandler.h"
#include "azure_c_shared_utility/threadapi.h"


typedef int(*f_rng)(void *p_rng, unsigned char *output, size_t output_len);
typedef void(*f_dbg)(void* a, int b, const char* c, int d, const char* e);
typedef int(*f_entropy)(void *, unsigned char *, size_t);

MOCKABLE_FUNCTION(, void, mbedtls_init, void*, instance, const char*, hostname);
MOCKABLE_FUNCTION(, int, mbedtls_x509_crt_parse, mbedtls_x509_crt*, crt, const unsigned char*, buf, size_t, buflen);
MOCKABLE_FUNCTION(, void, mbedtls_x509_crt_init, mbedtls_x509_crt*, crt);
MOCKABLE_FUNCTION(, void, mbedtls_x509_crt_free, mbedtls_x509_crt*, crt);
MOCKABLE_FUNCTION(, int, mbedtls_pk_parse_key, mbedtls_pk_context*, ctx, const unsigned char*, key, size_t, keylen, const unsigned char*, pwd, size_t, pwdlen);

MOCKABLE_FUNCTION(, void, mbedtls_ctr_drbg_init, mbedtls_ctr_drbg_context*, ctx);
MOCKABLE_FUNCTION(, void, mbedtls_ctr_drbg_free, mbedtls_ctr_drbg_context*, ctx)
MOCKABLE_FUNCTION(, int, mbedtls_ctr_drbg_seed_entropy_len, mbedtls_ctr_drbg_context*, ctx, f_entropy, fe, void*, p_entropy, const unsigned char*, custom, size_t, len, size_t, entropy_len);
MOCKABLE_FUNCTION(, int, mbedtls_ctr_drbg_random_with_add, void*, p_rng, unsigned char*, output, size_t, output_len, const unsigned char*, additional, size_t, add_len);
MOCKABLE_FUNCTION(, int, mbedtls_ctr_drbg_seed, mbedtls_ctr_drbg_context*, ctx, f_entropy, fe, void*, p_entropy, const unsigned char*, custom, size_t, len);
MOCKABLE_FUNCTION(, int, mbedtls_ctr_drbg_random, void*, p_rng, unsigned char*, output, size_t, output_len);

MOCKABLE_FUNCTION(, void, mbedtls_ssl_init, mbedtls_ssl_context*, ssl)
MOCKABLE_FUNCTION(, void, mbedtls_ssl_free, mbedtls_ssl_context*, ssl)
MOCKABLE_FUNCTION(, void, mbedtls_ssl_config_free, mbedtls_ssl_config*, conf)
MOCKABLE_FUNCTION(, int, mbedtls_ssl_handshake_step, mbedtls_ssl_context*, ssl)
MOCKABLE_FUNCTION(, int, mbedtls_ssl_setup, mbedtls_ssl_context*, ssl, const mbedtls_ssl_config*, conf)
MOCKABLE_FUNCTION(, int, mbedtls_ssl_set_session, mbedtls_ssl_context*, ssl, const mbedtls_ssl_session*, session)
MOCKABLE_FUNCTION(, int, mbedtls_ssl_read, mbedtls_ssl_context*, ssl, unsigned char*, buf, size_t, len)
MOCKABLE_FUNCTION(, size_t, mbedtls_ssl_get_max_frag_len, const mbedtls_ssl_context*, ssl)

MOCKABLE_FUNCTION(, void, mbedtls_ssl_conf_authmode, mbedtls_ssl_config*, conf, int, authmode)
MOCKABLE_FUNCTION(, void, mbedtls_ssl_conf_rng, mbedtls_ssl_config*, conf, f_rng, fr, void*, p_rng);
MOCKABLE_FUNCTION(, void, mbedtls_ssl_conf_dbg, mbedtls_ssl_config*, conf, f_dbg, fd, void*, p_dbg);
MOCKABLE_FUNCTION(, void, mbedtls_ssl_set_bio, mbedtls_ssl_context*, ssl, void*, p_bio, mbedtls_ssl_send_t*, f_send, mbedtls_ssl_recv_t*, f_recv, mbedtls_ssl_recv_timeout_t*, f_recv_timeout);
MOCKABLE_FUNCTION(, void, mbedtls_ssl_conf_ca_chain, mbedtls_ssl_config*, conf, mbedtls_x509_crt*, ca_chain, mbedtls_x509_crl*, ca_crl);
MOCKABLE_FUNCTION(, void, mbedtls_ssl_conf_min_version, mbedtls_ssl_config*, conf, int, major, int, minor);

MOCKABLE_FUNCTION(, int, mbedtls_ssl_set_hostname, mbedtls_ssl_context*, ssl, const char*, hostname);
MOCKABLE_FUNCTION(, int, mbedtls_ssl_handshake, mbedtls_ssl_context*, ssl);
MOCKABLE_FUNCTION(, int, mbedtls_ssl_write, mbedtls_ssl_context*, ssl, const unsigned char*, buf, size_t, len);
MOCKABLE_FUNCTION(, int, mbedtls_ssl_close_notify, mbedtls_ssl_context*, ssl);
MOCKABLE_FUNCTION(, int, mbedtls_ssl_config_defaults, mbedtls_ssl_config*, conf, int, endpoint, int, transport, int, preset);
MOCKABLE_FUNCTION(, void, mbedtls_ssl_config_init, mbedtls_ssl_config*, conf);
MOCKABLE_FUNCTION(, void, mbedtls_ssl_session_init, mbedtls_ssl_session*, session);
MOCKABLE_FUNCTION(, int, mbedtls_ssl_session_reset, mbedtls_ssl_context*, ssl);
MOCKABLE_FUNCTION(, void, mbedtls_ssl_session_free, mbedtls_ssl_session*, ssl);
MOCKABLE_FUNCTION(, int, mbedtls_ssl_conf_own_cert, mbedtls_ssl_config*, conf, mbedtls_x509_crt*, own_cert, mbedtls_pk_context*, pk_key);
MOCKABLE_FUNCTION(, void, mbedtls_ssl_conf_renegotiation, mbedtls_ssl_config*, conf, int, renegotiation);

MOCKABLE_FUNCTION(, void, mbedtls_debug_set_threshold, int, threshold);

MOCKABLE_FUNCTION(, void, mbedtls_entropy_init, mbedtls_entropy_context*, ctx);
MOCKABLE_FUNCTION(, int, mbedtls_entropy_add_source, mbedtls_entropy_context*, ctx, mbedtls_entropy_f_source_ptr, f_source, void*, p_source, size_t, threshold, int, strong);
MOCKABLE_FUNCTION(, int, mbedtls_entropy_func, void*, data, unsigned char*, output, size_t, len);
MOCKABLE_FUNCTION(, void, mbedtls_entropy_free, mbedtls_entropy_context*, ctx)

MOCKABLE_FUNCTION(, void, mbedtls_pk_init, mbedtls_pk_context*, ctx);
MOCKABLE_FUNCTION(, mbedtls_pk_type_t, mbedtls_pk_get_type, const mbedtls_pk_context*, ctx);
MOCKABLE_FUNCTION(, void, mbedtls_pk_free, mbedtls_pk_context*, ctx);

MOCKABLE_FUNCTION(, void, on_io_open_complete, void*, context, IO_OPEN_RESULT, open_result);
MOCKABLE_FUNCTION(, void, on_bytes_received, void*, context, const unsigned char*, buffer, size_t, size);
MOCKABLE_FUNCTION(, void, on_io_error, void*, context);
MOCKABLE_FUNCTION(, void, on_io_close_complete, void*, context);
MOCKABLE_FUNCTION(, void, on_send_complete, void*, context, IO_SEND_RESULT, send_result);

#undef ENABLE_MOCKS

#include "azure_c_shared_utility/tlsio_mbedtls.h"

static const char* const TEST_X509_CERTIFICATE = "test certificate";
static const char* const TEST_X509_KEY = "test certificate key";

static const char* const TEST_HOSTNAME = "test.azure-devices.net";
static int TEST_CONNECTION_PORT = 443;
static const IO_INTERFACE_DESCRIPTION* TEST_INTERFACE_DESC = (IO_INTERFACE_DESCRIPTION*)0x6543;
static const unsigned char TEST_DATA_VALUE[] = { 0x02, 0x34, 0x03 };
static size_t TEST_DATA_SIZE = sizeof(TEST_DATA_VALUE) / sizeof(TEST_DATA_VALUE[0]);

static ON_IO_OPEN_COMPLETE g_open_complete = NULL;
static void* g_open_complete_ctx = NULL;
static ON_BYTES_RECEIVED g_on_bytes_received = NULL;
static void* g_on_bytes_received_ctx = NULL;
static ON_IO_ERROR g_on_io_error = NULL;
static void* g_on_io_error_ctx = NULL;
static size_t g_max_send_fragment_size = 0;
static int g_failed_fragment_index = -1;

static mbedtls_ssl_send_t* mbed_f_send = NULL;
static mbedtls_ssl_recv_t* mbed_f_recv = NULL;
static mbedtls_ssl_recv_timeout_t* mbed_f_recv_timeout = NULL;
static void* g_mbedtls_ctx = NULL;

static mbedtls_entropy_f_source_ptr g_entropy_f_source;

#define MAX_RETRY           20
#define RECEIVE_BUFFER_SIZE 1024
#define ENTROPY_LENGTH      16

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    const IO_INTERFACE_DESCRIPTION* tlsio_openssl_get_interface_description(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

static int my_mallocAndStrcpy_s(char** destination, const char* source)
{
    (void)source;
    size_t src_len = strlen(source);
    *destination = (char*)my_gballoc_malloc(src_len + 1);
    strcpy(*destination, source);
    return 0;
}

static XIO_HANDLE my_xio_create(const IO_INTERFACE_DESCRIPTION* io_interface_description, const void* xio_create_parameters)
{
    (void)io_interface_description;
    (void)xio_create_parameters;
    return (XIO_HANDLE)my_gballoc_malloc(1);
}

static int my_xio_open(XIO_HANDLE xio, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    (void)xio;

    g_open_complete = on_io_open_complete;
    g_open_complete_ctx = on_io_open_complete_context;
    g_on_bytes_received = on_bytes_received;
    g_on_bytes_received_ctx = on_bytes_received_context;
    g_on_io_error = on_io_error;
    g_on_io_error_ctx = on_io_error_context;
    return 0;
}

static void my_xio_destroy(XIO_HANDLE xio)
{
    my_gballoc_free(xio);
}

static int my_xio_send(XIO_HANDLE xio, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    (void)xio;
    (void)buffer;
    (void)size;
    (void)on_send_complete;

    if (on_send_complete != NULL)
    {
        if (g_failed_fragment_index == 0)
        {
            on_send_complete(callback_context, IO_SEND_ERROR);
        }
        else
        {
            on_send_complete(callback_context, IO_SEND_OK);
        }

        g_failed_fragment_index >= 0 ? g_failed_fragment_index-- : g_failed_fragment_index;
    }

    return 0;
}

static void my_mbedtls_ssl_set_bio(mbedtls_ssl_context* ssl, void* p_bio, mbedtls_ssl_send_t* f_send, mbedtls_ssl_recv_t* f_recv, mbedtls_ssl_recv_timeout_t* f_recv_timeout)
{
    (void)ssl;
    g_mbedtls_ctx = p_bio;
    mbed_f_send = f_send;
    mbed_f_recv = f_recv;
    mbed_f_recv_timeout = f_recv_timeout;
}

static int my_mbedtls_entropy_add_source(mbedtls_entropy_context* ctx, mbedtls_entropy_f_source_ptr f_source, void* p_source, size_t threshold, int strong)
{
    (void)ctx;
    (void)p_source;
    (void)threshold;
    (void)strong;
    g_entropy_f_source = f_source;
    return 0;
}

static int my_mbedtls_ssl_write(mbedtls_ssl_context* ssl, const unsigned char *buf, size_t len)
{
    int ret;
    (void)buf;

    if (mbed_f_send != NULL)
    {
        // send tls app data
        ssl->out_msgtype = MBEDTLS_SSL_MSG_APPLICATION_DATA;
        mbed_f_send(g_mbedtls_ctx, buf, len);
    }

    if (g_max_send_fragment_size > 0)
    {
        ret = (int)(g_max_send_fragment_size > len ? len : g_max_send_fragment_size);
    }
    else
    {
        ret = (int)len;
    }

    return ret;
}

static void my_os_delay_us(int us)
{
    (void)(us);
}

static void my_on_bytes_received(void* context, const unsigned char* buffer, size_t size)
{
    (void)context;
    (void)buffer;
    (void)size;
}

static void my_on_send_complete(void* context, IO_SEND_RESULT send_result)
{
    (void)context;
    (void)send_result;

}

static void my_on_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    (void)context;
    (void)open_result;
}

static void my_on_io_close_complete(void* context)
{
    (void)context;
}

static void my_on_io_error(void* context)
{
    (void)context;
}

IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_SEND_RESULT, IO_SEND_RESULT_VALUES);

/**
  * Umock error will helps you to identify errors in the test suite or in the way that you are
  *    using it, just keep it as is.
  */
MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

/**
 * This is necessary for the test suite, just keep as is.
 */
static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

/**
 * Tests begin here.
 *
 *   RUN_TEST_SUITE(tlsio_esp8266_ut, failedTestCount);
 *
 */
BEGIN_TEST_SUITE(tlsio_mbedtls_ut)

    /**
     * This is the place where we initialize the test system. Replace the test name to associate the test
     *   suite with your test cases.
     * It is called once, before start the tests.
     */
    TEST_SUITE_INITIALIZE(suite_init)
    {
        int result;
        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        (void)umock_c_init(on_umock_c_error);

        result = umocktypes_charptr_register_types();
        ASSERT_ARE_EQUAL(int, 0, result);

        REGISTER_UMOCK_ALIAS_TYPE(mbedtls_entropy_f_source_ptr, void*);
        REGISTER_UMOCK_ALIAS_TYPE(f_entropy, void*);
        REGISTER_UMOCK_ALIAS_TYPE(f_rng, void*);
        REGISTER_UMOCK_ALIAS_TYPE(f_dbg, void*);
        REGISTER_UMOCK_ALIAS_TYPE(XIO_HANDLE, void*);
        REGISTER_UMOCK_ALIAS_TYPE(ON_IO_OPEN_COMPLETE, void*);
        REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_RECEIVED, void*);
        REGISTER_UMOCK_ALIAS_TYPE(ON_IO_ERROR, void*);
        REGISTER_UMOCK_ALIAS_TYPE(ON_IO_CLOSE_COMPLETE, void*);
        REGISTER_UMOCK_ALIAS_TYPE(ON_SEND_COMPLETE, void*);
        REGISTER_UMOCK_ALIAS_TYPE(mbedtls_pk_type_t, int);

        REGISTER_TYPE(IO_SEND_RESULT, IO_SEND_RESULT);
        REGISTER_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT);

        REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, my_gballoc_calloc);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_calloc, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_realloc, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);

        REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_s, my_mallocAndStrcpy_s);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(mallocAndStrcpy_s, __LINE__);

        REGISTER_GLOBAL_MOCK_HOOK(xio_create, my_xio_create);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(xio_create, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(xio_open, my_xio_open);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(xio_open, __LINE__);
        REGISTER_GLOBAL_MOCK_HOOK(xio_destroy, my_xio_destroy);
        REGISTER_GLOBAL_MOCK_HOOK(xio_send, my_xio_send);

        REGISTER_GLOBAL_MOCK_RETURN(socketio_get_interface_description, TEST_INTERFACE_DESC);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(socketio_get_interface_description, NULL);

        REGISTER_GLOBAL_MOCK_RETURN(mbedtls_ssl_read, 0);
        REGISTER_GLOBAL_MOCK_HOOK(mbedtls_ssl_set_bio, my_mbedtls_ssl_set_bio);
        REGISTER_GLOBAL_MOCK_HOOK(mbedtls_entropy_add_source, my_mbedtls_entropy_add_source);
        REGISTER_GLOBAL_MOCK_HOOK(mbedtls_ssl_write, my_mbedtls_ssl_write);

        REGISTER_GLOBAL_MOCK_HOOK(on_io_open_complete, my_on_io_open_complete);
        REGISTER_GLOBAL_MOCK_HOOK(on_bytes_received, my_on_bytes_received);
        REGISTER_GLOBAL_MOCK_HOOK(on_io_error, my_on_io_error);
        REGISTER_GLOBAL_MOCK_HOOK(on_io_close_complete, my_on_io_close_complete);
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
        g_open_complete = NULL;
        g_open_complete_ctx = NULL;
        g_on_bytes_received = NULL;
        g_on_bytes_received_ctx = NULL;
        g_on_io_error = NULL;
        g_on_io_error_ctx = NULL;

        mbed_f_send = NULL;
        mbed_f_recv = NULL;
        mbed_f_recv_timeout = NULL;

        g_max_send_fragment_size = 0;
        g_failed_fragment_index = -1;

        umock_c_reset_all_calls();
    }

    TEST_FUNCTION_CLEANUP(method_cleanup)
    {
        TEST_MUTEX_RELEASE(g_testByTest);
    }

    static int should_skip_index(size_t current_index, const size_t skip_array[], size_t length)
    {
        int result = 0;
        for (size_t index = 0; index < length; index++)
        {
            if (current_index == skip_array[index])
            {
                result = __LINE__;
                break;
            }
        }
        return result;
    }

    static void setup_tlsio_mbedtls_create_mocks(bool call_iface_desc)
    {
        STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
        if (call_iface_desc)
        {
            STRICT_EXPECTED_CALL(socketio_get_interface_description());
        }
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(xio_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(mbedtls_x509_crt_init(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(mbedtls_x509_crt_init(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(mbedtls_pk_init(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(mbedtls_entropy_init(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(mbedtls_entropy_add_source(IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL, IGNORED_NUM_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(mbedtls_ctr_drbg_init(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(mbedtls_ctr_drbg_seed(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(mbedtls_ssl_config_init(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(mbedtls_ssl_config_defaults(IGNORED_PTR_ARG, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT));
        STRICT_EXPECTED_CALL(mbedtls_ssl_conf_rng(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(mbedtls_ssl_conf_authmode(IGNORED_PTR_ARG, MBEDTLS_SSL_VERIFY_REQUIRED));
        STRICT_EXPECTED_CALL(mbedtls_ssl_conf_min_version(IGNORED_PTR_ARG, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3));

        STRICT_EXPECTED_CALL(mbedtls_ssl_init(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(mbedtls_ssl_set_bio(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL));
        STRICT_EXPECTED_CALL(mbedtls_ssl_set_hostname(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(mbedtls_ssl_session_init(IGNORED_PTR_ARG));

        STRICT_EXPECTED_CALL(mbedtls_ssl_set_session(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(mbedtls_ssl_setup(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    }

    TEST_FUNCTION(tlsio_mbedtls_create_config_NULL_fail)
    {
        //arrange

        //act
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(NULL);

        //assert
        ASSERT_IS_NULL(handle);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(tlsio_mbedtls_create_succeed)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;

        setup_tlsio_mbedtls_create_mocks(false);

        //act
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);

        //assert
        ASSERT_IS_NOT_NULL(handle);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_mbedtls_create_fail)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;

        int negativeTestsInitResult = umock_c_negative_tests_init();
        ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

        setup_tlsio_mbedtls_create_mocks(false);

        umock_c_negative_tests_snapshot();

        size_t count = umock_c_negative_tests_call_count();
        // Only the first 2 calls can fail
        for (size_t index = 0; index < 2; index++)
        {
            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(index);

            //act
            CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);

            //assert
            ASSERT_IS_NULL(handle, "tlsio_mbedtls_create failure in test %lu/%lu", (unsigned long)index, (unsigned long)count);
        }

        //cleanup
        umock_c_negative_tests_deinit();
    }

    TEST_FUNCTION(tlsio_mbedtls_destroy_succeed)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(mbedtls_ssl_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(mbedtls_ssl_session_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(mbedtls_ssl_config_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(mbedtls_x509_crt_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(mbedtls_x509_crt_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(mbedtls_pk_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(mbedtls_ctr_drbg_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(mbedtls_entropy_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(xio_destroy(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));

        //act
        tlsio_mbedtls_destroy(handle);

        //assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(tlsio_mbedtls_destroy_handle_NULL_fail)
    {
        //arrange

        //act
        tlsio_mbedtls_destroy(NULL);

        //assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(tlsio_mbedtls_open_handle_NULL_fail)
    {
        //arrange

        //act
        int result = tlsio_mbedtls_open(NULL, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(tlsio_mbedtls_open_succeed)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(mbedtls_ssl_session_reset(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(xio_open(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

        //act
        int result = tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        tlsio_mbedtls_close(handle, NULL, NULL);
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_mbedtls_open_multiple_calls_fail)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        int result = tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        umock_c_reset_all_calls();

        //act
        result = tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        tlsio_mbedtls_close(handle, NULL, NULL);
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_mbedtls_open_fail)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(mbedtls_ssl_session_reset(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(xio_open(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(__LINE__);

        //act
        int result = tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_entropy_poll_success)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        umock_c_reset_all_calls();

        unsigned char output[ENTROPY_LENGTH];
        size_t len = ENTROPY_LENGTH;
        size_t olen;

        //act
        g_entropy_f_source(NULL, output, len, &olen);

        //assert
        ASSERT_ARE_EQUAL(int, ENTROPY_LENGTH, olen);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_mbedtls_close_handle_NULL_fail)
    {
        //arrange

        //act
        int result = tlsio_mbedtls_close(NULL, on_io_close_complete, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(tlsio_mbedtls_close_success)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        int result = tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(mbedtls_ssl_close_notify(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(xio_close(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

        //act
        result = tlsio_mbedtls_close(handle, on_io_close_complete, NULL);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_mbedtls_close_multiple_calls_fail)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        int result = tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        result = tlsio_mbedtls_close(handle, on_io_close_complete, NULL);
        umock_c_reset_all_calls();

        //act
        result = tlsio_mbedtls_close(handle, on_io_close_complete, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_mbedtls_send_handle_NULL_fail)
    {
        //arrange

        //act
        int result = tlsio_mbedtls_send(NULL, TEST_DATA_VALUE, TEST_DATA_SIZE, on_send_complete, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(tlsio_mbedtls_send_not_open_fail)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        umock_c_reset_all_calls();

        //act
        int result = tlsio_mbedtls_send(handle, TEST_DATA_VALUE, TEST_DATA_SIZE, on_send_complete, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_mbedtls_send_success)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        (void)tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        g_open_complete(g_open_complete_ctx, IO_OPEN_OK);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(mbedtls_ssl_get_max_frag_len(IGNORED_PTR_ARG)).SetReturn(TEST_DATA_SIZE);
        STRICT_EXPECTED_CALL(mbedtls_ssl_write(IGNORED_PTR_ARG, TEST_DATA_VALUE, TEST_DATA_SIZE)).SetReturn(TEST_DATA_SIZE);

        //act
        int result = tlsio_mbedtls_send(handle, TEST_DATA_VALUE, TEST_DATA_SIZE, on_send_complete, NULL);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        (void)tlsio_mbedtls_close(handle, on_io_close_complete, NULL);
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_mbedtls_send_failure)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        (void)tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        g_open_complete(g_open_complete_ctx, IO_OPEN_OK);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(mbedtls_ssl_get_max_frag_len(IGNORED_PTR_ARG)).SetReturn(TEST_DATA_SIZE);
        STRICT_EXPECTED_CALL(mbedtls_ssl_write(IGNORED_PTR_ARG, TEST_DATA_VALUE, TEST_DATA_SIZE)).SetReturn(-1);

        //act
        int result = tlsio_mbedtls_send(handle, TEST_DATA_VALUE, TEST_DATA_SIZE, on_send_complete, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        (void)tlsio_mbedtls_close(handle, on_io_close_complete, NULL);
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_mbedtls_send_large_payload_success)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        tls_io_config.invoke_on_send_complete_callback_for_fragments = false;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        (void)tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        g_open_complete(g_open_complete_ctx, IO_OPEN_OK);
        umock_c_reset_all_calls();

        size_t MAX_FRAGMENT_SIZE = 1;
        int rounds = 3;
        size_t total_data = rounds * MAX_FRAGMENT_SIZE;
        const unsigned char* dummy_data = (const unsigned char*) 0x51;
        g_max_send_fragment_size = MAX_FRAGMENT_SIZE;

        for (int index = 0; index < rounds; index++)
        {
            size_t data_left = total_data - index * MAX_FRAGMENT_SIZE;
            size_t data_processed = data_left > MAX_FRAGMENT_SIZE ? MAX_FRAGMENT_SIZE : data_left;
            const unsigned char* data_ptr = dummy_data + index * MAX_FRAGMENT_SIZE;
            STRICT_EXPECTED_CALL(mbedtls_ssl_get_max_frag_len(IGNORED_PTR_ARG)).SetReturn(MAX_FRAGMENT_SIZE);
            STRICT_EXPECTED_CALL(mbedtls_ssl_write(IGNORED_PTR_ARG, data_ptr, data_left));
            STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        }

        STRICT_EXPECTED_CALL(on_send_complete(IGNORED_PTR_ARG, IO_SEND_OK));

        //act
        int result = tlsio_mbedtls_send(handle, dummy_data, total_data, on_send_complete, NULL);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        (void)tlsio_mbedtls_close(handle, on_io_close_complete, NULL);
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_mbedtls_send_large_payload_failure)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        tls_io_config.invoke_on_send_complete_callback_for_fragments = false;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        (void)tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        g_open_complete(g_open_complete_ctx, IO_OPEN_OK);
        umock_c_reset_all_calls();

        size_t MAX_FRAGMENT_SIZE = 1;
        int rounds = 3;
        size_t total_data = rounds * MAX_FRAGMENT_SIZE;
        const unsigned char* dummy_data = (const unsigned char*) 0x51;
        g_failed_fragment_index = 1; // second fragment to fail
        g_max_send_fragment_size = MAX_FRAGMENT_SIZE;

        for (int index = 0; index <= g_failed_fragment_index; index++)
        {
            size_t data_left = total_data - index * MAX_FRAGMENT_SIZE;
            size_t data_processed = data_left > MAX_FRAGMENT_SIZE ? MAX_FRAGMENT_SIZE : data_left;
            const unsigned char* data_ptr = dummy_data + index * MAX_FRAGMENT_SIZE;
            STRICT_EXPECTED_CALL(mbedtls_ssl_get_max_frag_len(IGNORED_PTR_ARG)).SetReturn(MAX_FRAGMENT_SIZE);
            STRICT_EXPECTED_CALL(mbedtls_ssl_write(IGNORED_PTR_ARG, data_ptr, data_left));
            STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        }

        STRICT_EXPECTED_CALL(on_send_complete(IGNORED_PTR_ARG, IO_SEND_ERROR));

        //act
        int result = tlsio_mbedtls_send(handle, dummy_data, total_data, on_send_complete, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        (void)tlsio_mbedtls_close(handle, on_io_close_complete, NULL);
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_mbedtls_dowork_handle_NULL_fail)
    {
        //arrange

        //act
        tlsio_mbedtls_dowork(NULL);

        //assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(tlsio_mbedtls_dowork_success)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        (void)tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        g_open_complete(g_open_complete_ctx, IO_OPEN_OK);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(mbedtls_ssl_read(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(xio_dowork(IGNORED_PTR_ARG));

        //act
        tlsio_mbedtls_dowork(handle);

        //assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        (void)tlsio_mbedtls_close(handle, on_io_close_complete, NULL);
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_mbedtls_dowork_w_data_success)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        (void)tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        g_open_complete(g_open_complete_ctx, IO_OPEN_OK);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(mbedtls_ssl_read(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG))
            .CopyOutArgumentBuffer_buf(TEST_DATA_VALUE, sizeof(unsigned char**) )
            .SetReturn(TEST_DATA_SIZE);
        STRICT_EXPECTED_CALL(on_bytes_received(IGNORED_PTR_ARG, IGNORED_PTR_ARG, TEST_DATA_SIZE));
        STRICT_EXPECTED_CALL(mbedtls_ssl_read(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(xio_dowork(IGNORED_PTR_ARG));

        //act
        tlsio_mbedtls_dowork(handle);

        //assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        (void)tlsio_mbedtls_close(handle, on_io_close_complete, NULL);
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_on_underlying_io_bytes_received_success)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        (void)tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        g_open_complete(g_open_complete_ctx, IO_OPEN_OK);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

        //act
        g_on_bytes_received(g_on_bytes_received_ctx, TEST_DATA_VALUE, TEST_DATA_SIZE);

        //assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        (void)tlsio_mbedtls_close(handle, on_io_close_complete, NULL);
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_on_io_recv_timeout_success)
    {
        unsigned char* read_buff[32];
        size_t buff_len = 32;

        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        (void)tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        g_open_complete(g_open_complete_ctx, IO_OPEN_OK);

        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(xio_dowork(IGNORED_PTR_ARG));

        //act
        mbed_f_recv(g_mbedtls_ctx, (unsigned char*)read_buff, buff_len);

        //assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        (void)tlsio_mbedtls_close(handle, on_io_close_complete, NULL);
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_on_io_recv_success)
    {
        unsigned char* read_buff[32];
        size_t buff_len = 32;

        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        (void)tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        g_on_bytes_received(g_on_bytes_received_ctx, TEST_DATA_VALUE, TEST_DATA_SIZE);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        //act
        mbed_f_recv(g_mbedtls_ctx, (unsigned char*)read_buff, buff_len);

        //assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        (void)tlsio_mbedtls_close(handle, on_io_close_complete, NULL);
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_on_io_recv_context_NULL_success)
    {
        unsigned char* read_buff[32];
        size_t buff_len = 32;

        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        (void)tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        g_on_bytes_received(g_on_bytes_received_ctx, TEST_DATA_VALUE, TEST_DATA_SIZE);
        umock_c_reset_all_calls();

        //act
        mbed_f_recv(NULL, (unsigned char*)read_buff, buff_len);

        //assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        (void)tlsio_mbedtls_close(handle, on_io_close_complete, NULL);
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_mbedtls_setoption_certificate_success)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        (void)tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        g_open_complete(g_open_complete_ctx, IO_OPEN_OK);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_X509_CERTIFICATE));
        STRICT_EXPECTED_CALL(mbedtls_x509_crt_parse(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(mbedtls_pk_get_type(IGNORED_PTR_ARG)).SetReturn(MBEDTLS_PK_NONE);

        //act
        tlsio_mbedtls_setoption(handle, SU_OPTION_X509_CERT, TEST_X509_CERTIFICATE);

        //assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        (void)tlsio_mbedtls_close(handle, on_io_close_complete, NULL);
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_mbedtls_setoption_certificate_key_success)
    {
        //arrange
        mbedtls_pk_info_t* pk_info = (mbedtls_pk_info_t*)0x12345;

        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        (void)tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        g_open_complete(g_open_complete_ctx, IO_OPEN_OK);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, TEST_X509_KEY));
        STRICT_EXPECTED_CALL(mbedtls_pk_parse_key(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, NULL, 0))
            .CopyOutArgumentBuffer_ctx(&pk_info, sizeof(pk_info));

        //act
        tlsio_mbedtls_setoption(handle, SU_OPTION_X509_PRIVATE_KEY, TEST_X509_KEY);

        //assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        (void)tlsio_mbedtls_close(handle, on_io_close_complete, NULL);
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_mbedtls_setoption_renegotiation_success)
    {
        //arrange
        mbedtls_pk_info_t* pk_info = (mbedtls_pk_info_t*)0x12345;
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        (void)tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        g_open_complete(g_open_complete_ctx, IO_OPEN_OK);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(mbedtls_ssl_conf_renegotiation(IGNORED_PTR_ARG, 1));

        //act
        bool set_renegotiation = true;
        int result = tlsio_mbedtls_setoption(handle, OPTION_SET_TLS_RENEGOTIATION, &set_renegotiation);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        (void)tlsio_mbedtls_close(handle, on_io_close_complete, NULL);
        tlsio_mbedtls_destroy(handle);
    }

    TEST_FUNCTION(tlsio_mbedtls_setoption_renegotiation_value_NULL_fail)
    {
        //arrange
        mbedtls_pk_info_t* pk_info = (mbedtls_pk_info_t*)0x12345;
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_mbedtls_create(&tls_io_config);
        (void)tlsio_mbedtls_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        g_open_complete(g_open_complete_ctx, IO_OPEN_OK);
        umock_c_reset_all_calls();

        //act
        bool set_renegotiation = true;
        int result = tlsio_mbedtls_setoption(handle, OPTION_SET_TLS_RENEGOTIATION, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        (void)tlsio_mbedtls_close(handle, on_io_close_complete, NULL);
        tlsio_mbedtls_destroy(handle);
    }

END_TEST_SUITE(tlsio_mbedtls_ut)
