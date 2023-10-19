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

#include "bearssl.h"

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
#include "azure_c_shared_utility/singlylinkedlist.h"

MOCKABLE_FUNCTION(, void, br_ssl_client_init_full, br_ssl_client_context*, sc, br_x509_minimal_context*, xc, const br_x509_trust_anchor*, tas, size_t, ta_count);
MOCKABLE_FUNCTION(, void, br_ssl_engine_set_buffer, br_ssl_engine_context*, sc_eng, void*, iobuf, size_t, iobuf_len, int, duplex);
MOCKABLE_FUNCTION(, int, br_ssl_client_reset, br_ssl_client_context*, sc, const char*, hostname, int, tryresume);
MOCKABLE_FUNCTION(, void, br_ssl_client_set_single_rsa, br_ssl_client_context*, sc, const br_x509_certificate*, certchain, size_t, certchain_len, const br_rsa_private_key*, sk, br_rsa_pkcs1_sign, irsasign);
MOCKABLE_FUNCTION(, void, br_ssl_client_set_single_ec, br_ssl_client_context*, sc, const br_x509_certificate*, certchain, size_t, certchain_len, const br_ec_private_key*, sk, unsigned, allowed_usages, unsigned, cert_issuer_key_type, const br_ec_impl*, iec, br_ecdsa_sign, iecdsa);
MOCKABLE_FUNCTION(, unsigned, br_ssl_engine_current_state, const br_ssl_engine_context*, sc);
MOCKABLE_FUNCTION(, unsigned char*, br_ssl_engine_sendrec_buf, const br_ssl_engine_context*, sc, size_t*, len);
MOCKABLE_FUNCTION(, void, br_ssl_engine_sendrec_ack, br_ssl_engine_context*, sc, size_t, len);
MOCKABLE_FUNCTION(, unsigned char*, br_ssl_engine_recvrec_buf, const br_ssl_engine_context*, sc, size_t*, len);
MOCKABLE_FUNCTION(, void, br_ssl_engine_recvrec_ack, br_ssl_engine_context*, sc, size_t, len);
MOCKABLE_FUNCTION(, unsigned char*, br_ssl_engine_sendapp_buf, const br_ssl_engine_context*, sc, size_t*, len);
MOCKABLE_FUNCTION(, void, br_ssl_engine_sendapp_ack, br_ssl_engine_context*, sc, size_t, len);
MOCKABLE_FUNCTION(, unsigned char*, br_ssl_engine_recvapp_buf, const br_ssl_engine_context*, sc, size_t*, len);
MOCKABLE_FUNCTION(, void, br_ssl_engine_recvapp_ack, br_ssl_engine_context*, sc, size_t, len);
MOCKABLE_FUNCTION(, void, br_ssl_engine_flush, br_ssl_engine_context*, cc, int, force);
MOCKABLE_FUNCTION(, br_ecdsa_sign, br_ecdsa_sign_asn1_get_default);
MOCKABLE_FUNCTION(, const br_ec_impl*, br_ec_get_default);
MOCKABLE_FUNCTION(, br_rsa_pkcs1_sign, br_rsa_pkcs1_sign_get_default);

MOCKABLE_FUNCTION(, void, br_skey_decoder_init, br_skey_decoder_context*, ctx);
MOCKABLE_FUNCTION(, void, br_skey_decoder_push, br_skey_decoder_context*, ctx, const void*, data, size_t, len);

MOCKABLE_FUNCTION(, void, br_pem_decoder_init, br_pem_decoder_context*, ctx);
MOCKABLE_FUNCTION(, size_t, br_pem_decoder_push, br_pem_decoder_context*, ctx, const void*, data, size_t, len);
MOCKABLE_FUNCTION(, int, br_pem_decoder_event, br_pem_decoder_context*, ctx);

typedef void(*f_append_dn)(void *, const void *, size_t);
MOCKABLE_FUNCTION(, void, br_x509_decoder_init, br_x509_decoder_context*, ctx, f_append_dn, append_dn, void*, append_dn_ctx);
MOCKABLE_FUNCTION(, void, br_x509_decoder_push, br_x509_decoder_context*, ctx, const void*, data, size_t, len);

MOCKABLE_FUNCTION(, void, on_io_open_complete, void*, context, IO_OPEN_RESULT, open_result);
MOCKABLE_FUNCTION(, void, on_bytes_received, void*, context, const unsigned char*, buffer, size_t, size);
MOCKABLE_FUNCTION(, void, on_io_error, void*, context);
MOCKABLE_FUNCTION(, void, on_io_close_complete, void*, context);
MOCKABLE_FUNCTION(, void, on_send_complete, void*, context, IO_SEND_RESULT, send_result);

#undef ENABLE_MOCKS

#include "azure_c_shared_utility/tlsio_bearssl.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/vector.h"

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
static SINGLYLINKEDLIST_HANDLE TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE = (SINGLYLINKEDLIST_HANDLE)0x4242;
static LIST_ITEM_HANDLE TEST_LISTITEM_HANDLE = (LIST_ITEM_HANDLE)0xdead;
static int g_decoder_state = 0;

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


// THIS NEEDS TO BE A COPIED FROM THE DECLARATIONS IN tlsio_bearssl.c

// Unfortunately there does not appear to be a simple mechanism to mock an inline function declared in a header
// This is to fake certificates being passed.

typedef enum TLSIO_STATE_ENUM_TAG
{
	TLSIO_STATE_NOT_OPEN,
	TLSIO_STATE_OPENING_UNDERLYING_IO,
	TLSIO_STATE_IN_HANDSHAKE,
	TLSIO_STATE_OPEN,
	TLSIO_STATE_CLOSING,
	TLSIO_STATE_ERROR
} TLSIO_STATE_ENUM;

typedef struct {
	int key_type;  /* BR_KEYTYPE_RSA or BR_KEYTYPE_EC */
	union {
		br_rsa_private_key rsa;
		br_ec_private_key ec;
	} key;
} private_key;

typedef struct PENDING_TLS_IO_TAG
{
	unsigned char* bytes;
	size_t size;
	ON_SEND_COMPLETE on_send_complete;
	void* callback_context;
	SINGLYLINKEDLIST_HANDLE pending_io_list;
} PENDING_TLS_IO; 

typedef struct TLS_IO_INSTANCE_TAG
{
	XIO_HANDLE socket_io;
	ON_BYTES_RECEIVED on_bytes_received;
	ON_IO_OPEN_COMPLETE on_io_open_complete;
	ON_IO_CLOSE_COMPLETE on_io_close_complete;
	ON_IO_ERROR on_io_error;
	void *on_bytes_received_context;
	void *on_io_open_complete_context;
	void *on_io_close_complete_context;
	void *on_io_error_context;
	TLSIO_STATE_ENUM tlsio_state;
	unsigned char *socket_io_read_bytes;
	size_t socket_io_read_byte_count;
	ON_SEND_COMPLETE on_send_complete;
	void *on_send_complete_callback_context;
	SINGLYLINKEDLIST_HANDLE pending_toencrypt_list;
	SINGLYLINKEDLIST_HANDLE pending_todecrypt_list;

	br_ssl_client_context sc;
	br_x509_minimal_context xc;
	br_sslio_context ioc;
	br_x509_trust_anchor *tas;
	br_x509_certificate *x509_cert;
	size_t x509_cert_len;
	private_key *x509_pk;
	size_t ta_count;
	char *trusted_certificates;
	char *x509_certificate;
	char *x509_private_key;
	unsigned char iobuf[BR_SSL_BUFSIZE_BIDI];
	char *hostname;
} TLS_IO_INSTANCE;

static void FakeoutCertOptions(CONCRETE_IO_HANDLE handle)
{
	TLS_IO_INSTANCE *tlsio = (TLS_IO_INSTANCE *)handle;

	tlsio->ta_count = 1;
}

/**
 * Tests begin here.
 *
 *   RUN_TEST_SUITE(tlsio_esp8266_ut, failedTestCount);
 *
 */
BEGIN_TEST_SUITE(tlsio_bearssl_ut)

    /**
     * This is the place where we initialize the test system. Replace the test name to associate the test
     *   suite with your test cases.
     * It is called once, before start the tests.
     */
    TEST_SUITE_INITIALIZE(suite_init)
    {
        int result;
        //TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        (void)umock_c_init(on_umock_c_error);

        result = umocktypes_charptr_register_types();
        ASSERT_ARE_EQUAL(int, 0, result);

        REGISTER_UMOCK_ALIAS_TYPE(SINGLYLINKEDLIST_HANDLE, void*);
		REGISTER_UMOCK_ALIAS_TYPE(LIST_ITEM_HANDLE, void*);
        REGISTER_UMOCK_ALIAS_TYPE(XIO_HANDLE, void*);
        REGISTER_UMOCK_ALIAS_TYPE(ON_IO_OPEN_COMPLETE, void*);
        REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_RECEIVED, void*);
        REGISTER_UMOCK_ALIAS_TYPE(ON_IO_ERROR, void*);
        REGISTER_UMOCK_ALIAS_TYPE(ON_IO_CLOSE_COMPLETE, void*);
        REGISTER_UMOCK_ALIAS_TYPE(ON_SEND_COMPLETE, void*);
		REGISTER_UMOCK_ALIAS_TYPE(VECTOR_HANDLE, void*);
		REGISTER_UMOCK_ALIAS_TYPE(unsigned, unsigned int);

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

		REGISTER_GLOBAL_MOCK_RETURN(singlylinkedlist_create, TEST_SINGLYLINKEDSINGLYLINKEDLIST_HANDLE);
		REGISTER_GLOBAL_MOCK_RETURN(singlylinkedlist_add, TEST_LISTITEM_HANDLE);
		REGISTER_GLOBAL_MOCK_RETURN(singlylinkedlist_get_head_item, NULL); // TEST_LISTITEM_HANDLE);

        REGISTER_GLOBAL_MOCK_HOOK(xio_create, my_xio_create);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(xio_create, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(xio_open, my_xio_open);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(xio_open, __LINE__);
        REGISTER_GLOBAL_MOCK_HOOK(xio_destroy, my_xio_destroy);

        REGISTER_GLOBAL_MOCK_RETURN(socketio_get_interface_description, TEST_INTERFACE_DESC);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(socketio_get_interface_description, NULL);

        REGISTER_GLOBAL_MOCK_HOOK(on_io_open_complete, my_on_io_open_complete);
        REGISTER_GLOBAL_MOCK_HOOK(on_bytes_received, my_on_bytes_received);
        REGISTER_GLOBAL_MOCK_HOOK(on_io_error, my_on_io_error);
        REGISTER_GLOBAL_MOCK_HOOK(on_io_close_complete, my_on_io_close_complete);
    }

    TEST_SUITE_CLEANUP(suite_cleanup)
    {
        umock_c_deinit();

        TEST_MUTEX_DESTROY(g_testByTest);
        //TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
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

        umock_c_reset_all_calls();
    }

    TEST_FUNCTION_CLEANUP(method_cleanup)
    {
        TEST_MUTEX_RELEASE(g_testByTest);
    }

    TEST_FUNCTION(tlsio_bearssl_create_config_NULL_fail)
    {
        //arrange (or not)

        //act
        CONCRETE_IO_HANDLE handle = tlsio_bearssl_create(NULL);

        //assert
        ASSERT_IS_NULL(handle);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(tlsio_bearssl_create_succeed)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;

        //setup_tlsio_bearssl_create_mocks(false);
		STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
		STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(xio_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(singlylinkedlist_create());
		STRICT_EXPECTED_CALL(singlylinkedlist_create());

        //act
        CONCRETE_IO_HANDLE handle = tlsio_bearssl_create(&tls_io_config);

        //assert
        ASSERT_IS_NOT_NULL(handle);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        tlsio_bearssl_destroy(handle);
    }

    TEST_FUNCTION(tlsio_bearssl_destroy_succeed)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_bearssl_create(&tls_io_config);
        umock_c_reset_all_calls();

		STRICT_EXPECTED_CALL(xio_destroy(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(singlylinkedlist_destroy(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(singlylinkedlist_destroy(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        //act
        tlsio_bearssl_destroy(handle);

        //assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

	TEST_FUNCTION(tlsio_bearssl_destroy_handle_NULL_fail)
    {
        //arrange

        //act
        tlsio_bearssl_destroy(NULL);

        //assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(tlsio_bearssl_open_handle_NULL_fail)
    {
        //arrange

        //act
        int result = tlsio_bearssl_open(NULL, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(tlsio_bearssl_open_succeed)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_bearssl_create(&tls_io_config);
		FakeoutCertOptions(handle);
		umock_c_reset_all_calls();

		STRICT_EXPECTED_CALL(br_ssl_client_init_full(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_set_buffer(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG));
		STRICT_EXPECTED_CALL(br_ssl_client_reset(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
		STRICT_EXPECTED_CALL(xio_open(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

        //act
        int result = tlsio_bearssl_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        tlsio_bearssl_close(handle, NULL, NULL);
        tlsio_bearssl_destroy(handle);
    }

    TEST_FUNCTION(tlsio_bearssl_open_multiple_calls_fail)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_bearssl_create(&tls_io_config);
		FakeoutCertOptions(handle);
		int result = tlsio_bearssl_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        umock_c_reset_all_calls();

        //act
        result = tlsio_bearssl_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        tlsio_bearssl_close(handle, NULL, NULL);
        tlsio_bearssl_destroy(handle);
    }

    TEST_FUNCTION(tlsio_bearssl_open_fail)
    {
		int result;

        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_bearssl_create(&tls_io_config);
		FakeoutCertOptions(handle);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(br_ssl_client_init_full(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_set_buffer(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG));
		STRICT_EXPECTED_CALL(br_ssl_client_reset(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(xio_open(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(__LINE__);

        //act
        result = tlsio_bearssl_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        tlsio_bearssl_destroy(handle);
    }

	TEST_FUNCTION(tlsio_bearssl_close_handle_NULL_fail)
    {
        //arrange

        //act
        int result = tlsio_bearssl_close(NULL, on_io_close_complete, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(tlsio_bearssl_close_success)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_bearssl_create(&tls_io_config);
		FakeoutCertOptions(handle);
		int result = tlsio_bearssl_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(xio_close(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

        //act
        result = tlsio_bearssl_close(handle, on_io_close_complete, NULL);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        tlsio_bearssl_destroy(handle);
    }

    TEST_FUNCTION(tlsio_bearssl_close_multiple_calls_fail)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_bearssl_create(&tls_io_config);
		FakeoutCertOptions(handle);
		int result = tlsio_bearssl_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        result = tlsio_bearssl_close(handle, on_io_close_complete, NULL);
        umock_c_reset_all_calls();

        //act
        result = tlsio_bearssl_close(handle, on_io_close_complete, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        tlsio_bearssl_destroy(handle);
    }

    TEST_FUNCTION(tlsio_bearssl_send_handle_NULL_fail)
    {
        //arrange

        //act
        int result = tlsio_bearssl_send(NULL, TEST_DATA_VALUE, TEST_DATA_SIZE, on_send_complete, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(tlsio_bearssl_send_not_open_fail)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_bearssl_create(&tls_io_config);
        umock_c_reset_all_calls();

        //act
        int result = tlsio_bearssl_send(handle, TEST_DATA_VALUE, TEST_DATA_SIZE, on_send_complete, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        tlsio_bearssl_destroy(handle);
    }

    TEST_FUNCTION(tlsio_bearssl_send_success)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_bearssl_create(&tls_io_config);
		FakeoutCertOptions(handle);
		(void)tlsio_bearssl_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        g_open_complete(g_open_complete_ctx, IO_OPEN_OK);
        umock_c_reset_all_calls();

		STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
		STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
		STRICT_EXPECTED_CALL(singlylinkedlist_add(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

        //act
        int result = tlsio_bearssl_send(handle, TEST_DATA_VALUE, TEST_DATA_SIZE, on_send_complete, NULL);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        (void)tlsio_bearssl_close(handle, on_io_close_complete, NULL);
        tlsio_bearssl_destroy(handle);
    }

    TEST_FUNCTION(tlsio_bearssl_send_failure)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_bearssl_create(&tls_io_config);
		FakeoutCertOptions(handle);
		(void)tlsio_bearssl_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        g_open_complete(g_open_complete_ctx, IO_OPEN_OK);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);

        //act
        int result = tlsio_bearssl_send(handle, TEST_DATA_VALUE, TEST_DATA_SIZE, on_send_complete, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        (void)tlsio_bearssl_close(handle, on_io_close_complete, NULL);
		tlsio_bearssl_destroy(handle);
    }

    TEST_FUNCTION(tlsio_bearssl_dowork_handle_NULL_fail)
    {
        //arrange

        //act
        tlsio_bearssl_dowork(NULL);

        //assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(tlsio_bearssl_dowork_success)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_bearssl_create(&tls_io_config);
		FakeoutCertOptions(handle);
		(void)tlsio_bearssl_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        g_open_complete(g_open_complete_ctx, IO_OPEN_OK);
        umock_c_reset_all_calls();

		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG)).SetReturn(BR_SSL_RECVREC);
		STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(xio_dowork(IGNORED_PTR_ARG));

        //act
        tlsio_bearssl_dowork(handle);

        //assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        (void)tlsio_bearssl_close(handle, on_io_close_complete, NULL);
		tlsio_bearssl_destroy(handle);
	}

    TEST_FUNCTION(tlsio_bearssl_dowork_w_data_success)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_bearssl_create(&tls_io_config);
		FakeoutCertOptions(handle);
		(void)tlsio_bearssl_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        g_open_complete(g_open_complete_ctx, IO_OPEN_OK);

		umock_c_reset_all_calls();
		
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG)).SetReturn(BR_SSL_SENDAPP);
		STRICT_EXPECTED_CALL(on_io_open_complete(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
		STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(xio_dowork(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG)).SetReturn(BR_SSL_RECVAPP);
		STRICT_EXPECTED_CALL(br_ssl_engine_recvapp_buf(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(on_bytes_received(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_recvapp_ack(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
		STRICT_EXPECTED_CALL(xio_dowork(IGNORED_PTR_ARG));

		//act
		tlsio_bearssl_dowork(handle);
		tlsio_bearssl_dowork(handle);

		//assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        (void)tlsio_bearssl_close(handle, on_io_close_complete, NULL);
        tlsio_bearssl_destroy(handle);
    }

    TEST_FUNCTION(tlsio_on_underlying_io_bytes_received_success)
    {
        //arrange
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = TEST_HOSTNAME;
        tls_io_config.port = TEST_CONNECTION_PORT;
        tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
        tls_io_config.underlying_io_parameters = NULL;
        CONCRETE_IO_HANDLE handle = tlsio_bearssl_create(&tls_io_config);
		FakeoutCertOptions(handle);
		(void)tlsio_bearssl_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
        g_open_complete(g_open_complete_ctx, IO_OPEN_OK);

		PENDING_TLS_IO *tlsio = gballoc_malloc(sizeof(PENDING_TLS_IO));

		tlsio->bytes = (unsigned char *)gballoc_malloc(4);
		memcpy(tlsio->bytes, "TST", 4);
		tlsio->size = 4;
		tlsio->callback_context = NULL;
		tlsio->pending_io_list = NULL;
		tlsio->on_send_complete = my_on_send_complete;
		unsigned char *buffer = (unsigned char *)malloc(20);
		size_t buffer_len = 8;
		
		umock_c_reset_all_calls();

		STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
		STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
		STRICT_EXPECTED_CALL(singlylinkedlist_add(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG)).SetReturn(BR_SSL_RECVREC);
		STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(IGNORED_PTR_ARG)).SetReturn(TEST_LISTITEM_HANDLE);
		STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG)).SetReturn(tlsio);
		STRICT_EXPECTED_CALL(br_ssl_engine_recvrec_buf(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).CopyOutArgumentBuffer_len(&buffer_len, sizeof(size_t)).SetReturn(buffer);
		STRICT_EXPECTED_CALL(br_ssl_engine_recvrec_ack(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
		STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(singlylinkedlist_remove(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(xio_dowork(IGNORED_PTR_ARG));

        //act
        g_on_bytes_received(g_on_bytes_received_ctx, TEST_DATA_VALUE, TEST_DATA_SIZE);
		tlsio_bearssl_dowork(handle);

        //assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
        (void)tlsio_bearssl_close(handle, on_io_close_complete, NULL);
        tlsio_bearssl_destroy(handle);
        free(buffer);
    }

	TEST_FUNCTION(tlsio_on_bytes_to_send_to_underlying_io)
	{
		//arrange
		TLSIO_CONFIG tls_io_config;
		tls_io_config.hostname = TEST_HOSTNAME;
		tls_io_config.port = TEST_CONNECTION_PORT;
		tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
		tls_io_config.underlying_io_parameters = NULL;
		CONCRETE_IO_HANDLE handle = tlsio_bearssl_create(&tls_io_config);
		FakeoutCertOptions(handle);
		(void)tlsio_bearssl_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
		g_open_complete(g_open_complete_ctx, IO_OPEN_OK);

		umock_c_reset_all_calls();

		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG)).SetReturn(BR_SSL_SENDREC);
		STRICT_EXPECTED_CALL(br_ssl_engine_sendrec_buf(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(xio_send(IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_sendrec_ack(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(xio_dowork(IGNORED_PTR_ARG));

		//act
		tlsio_bearssl_dowork(handle);

		//assert
		ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

		//cleanup
		(void)tlsio_bearssl_close(handle, on_io_close_complete, NULL);
		tlsio_bearssl_destroy(handle);
	}

	TEST_FUNCTION(tlsio_on_send_appdata_bytes_success)
	{
		//arrange
		TLSIO_CONFIG tls_io_config;
		tls_io_config.hostname = TEST_HOSTNAME;
		tls_io_config.port = TEST_CONNECTION_PORT;
		tls_io_config.underlying_io_interface = TEST_INTERFACE_DESC;
		tls_io_config.underlying_io_parameters = NULL;
		CONCRETE_IO_HANDLE handle = tlsio_bearssl_create(&tls_io_config);
		FakeoutCertOptions(handle);
		(void)tlsio_bearssl_open(handle, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
		g_open_complete(g_open_complete_ctx, IO_OPEN_OK);

		PENDING_TLS_IO *tlsio = gballoc_malloc(sizeof(PENDING_TLS_IO));

		tlsio->bytes = (unsigned char *)gballoc_malloc(4);
		memcpy(tlsio->bytes, "TST", 4);
		tlsio->size = 4;
		tlsio->callback_context = NULL;
		tlsio->pending_io_list = NULL;
		tlsio->on_send_complete = my_on_send_complete;
		//unsigned char *buffer = (unsigned char *)"TESTDATA";
        unsigned char *buffer = (unsigned char *)malloc(20);
		size_t buffer_len = 8;

		umock_c_reset_all_calls();

		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG)).SetReturn(BR_SSL_SENDAPP);
		STRICT_EXPECTED_CALL(on_io_open_complete(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
		STRICT_EXPECTED_CALL(singlylinkedlist_get_head_item(IGNORED_PTR_ARG)).SetReturn(TEST_LISTITEM_HANDLE);
		STRICT_EXPECTED_CALL(singlylinkedlist_item_get_value(IGNORED_PTR_ARG)).SetReturn(tlsio);
		STRICT_EXPECTED_CALL(br_ssl_engine_sendapp_buf(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).CopyOutArgumentBuffer_len(&buffer_len, 8).SetReturn(buffer);
		STRICT_EXPECTED_CALL(br_ssl_engine_sendapp_ack(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
		STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(singlylinkedlist_remove(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_flush(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
		STRICT_EXPECTED_CALL(br_ssl_engine_current_state(IGNORED_PTR_ARG));
		STRICT_EXPECTED_CALL(xio_dowork(IGNORED_PTR_ARG));

		//act
		tlsio_bearssl_dowork(handle);

		//assert
		ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

		//cleanup
		(void)tlsio_bearssl_close(handle, on_io_close_complete, NULL);
		tlsio_bearssl_destroy(handle);
        free(buffer);
	}

END_TEST_SUITE(tlsio_bearssl_ut)
