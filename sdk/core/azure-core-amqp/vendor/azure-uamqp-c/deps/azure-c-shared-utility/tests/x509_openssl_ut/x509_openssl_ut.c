// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void my_gballoc_free(void* s)
{
    free(s);
}

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"

#include "openssl/ssl.h"
#include "openssl/x509.h"
#include "openssl/err.h"
#include "openssl/opensslv.h"
#include "openssl/pem.h"
#include "openssl/bio.h"
#include "openssl/rsa.h"
#include "openssl/evp.h"
#include "openssl/engine.h"

#include "azure_c_shared_utility/x509_openssl.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umock_c_negative_tests.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"

#include "umock_c/umock_c_prod.h"

#ifndef VALIDATED_PTR_ARG
#define VALIDATED_PTR_ARG NULL
#endif

#ifndef VALIDATED_NUM_ARG
#define VALIDATED_NUM_ARG 0
#endif

/*from openssl/bio.h*/
/*the below function has different signatures on different versions of OPENSSL*/
/*
|openssl version (number) | openssl string                   | BIO_new_mem_buf prototype                         | Observations                    |
|-------------------------|----------------------------------|---------------------------------------------------|---------------------------------|
| 0x1000106fL             | OpenSSL 1.0.1f 6 Jan 2014        | BIO *BIO_new_mem_buf(void *buf, int len);         | Ubuntu 14.04                    |
| 0x1000204fL             | OpenSSL 1.0.2d 9 Jul 2015        | BIO *BIO_new_mem_buf(void *buf, int len);         | Ubuntu 15.10                    |
| 0x1000207fL             | OpenSSL 1.0.2g-fips  1 Mar 2016  | BIO *BIO_new_mem_buf(const void *buf, int len);   | Ubuntu 16.10                    |
| 0x1010007fL             | OpenSSL 1.0.2g-fips  1 Mar 2016  | BIO *BIO_new(const BIO_METHOD *type);             | Ubuntu 18.04                    |
*/

MOCKABLE_FUNCTION(,int, BIO_free, BIO *,a);
#if OPENSSL_VERSION_NUMBER >= 0x1010007fL
MOCKABLE_FUNCTION(, BIO *, BIO_new, const BIO_METHOD *, type);
MOCKABLE_FUNCTION(, const BIO_METHOD *, BIO_s_mem);
#else
MOCKABLE_FUNCTION(, BIO *, BIO_new, BIO_METHOD *, type);
MOCKABLE_FUNCTION(, BIO_METHOD *, BIO_s_mem);
#endif

MOCKABLE_FUNCTION(, BIO *, BIO_mem, BIO_METHOD *, type);
MOCKABLE_FUNCTION(, int, BIO_puts, BIO *, bp, const char *, buf);

#if OPENSSL_VERSION_NUMBER >= 0x1000207fL
MOCKABLE_FUNCTION(,BIO *,BIO_new_mem_buf, const void *,buf, int, len);
#else
MOCKABLE_FUNCTION(, BIO *, BIO_new_mem_buf, void *, buf, int, len);
#endif

/*from openssl/rsa.h*/
MOCKABLE_FUNCTION(, void, RSA_free, RSA *,rsa);

/*from openssl/x509.h*/
MOCKABLE_FUNCTION(, void, X509_free, X509 *, a);

/*from  openssl/pem.h*/
MOCKABLE_FUNCTION(, X509 *, PEM_read_bio_X509, BIO *, bp, X509 **, x, pem_password_cb *, cb, void *, u);
MOCKABLE_FUNCTION(, RSA *, PEM_read_bio_RSAPrivateKey, BIO *,bp, RSA **,x, pem_password_cb *,cb, void *,u);

MOCKABLE_FUNCTION(, RSA*, EVP_PKEY_get1_RSA, EVP_PKEY*, pkey);

/*from openssl/ssl.h*/
MOCKABLE_FUNCTION(,int, SSL_CTX_use_RSAPrivateKey, SSL_CTX *,ctx, RSA *,rsa);
MOCKABLE_FUNCTION(,int, SSL_CTX_use_certificate, SSL_CTX *,ctx, X509*, x);
MOCKABLE_FUNCTION(, X509_STORE *, SSL_CTX_get_cert_store, const SSL_CTX *, ssl_ctx);

/*from openssl/err.h*/
MOCKABLE_FUNCTION(,unsigned long, ERR_get_error);
MOCKABLE_FUNCTION(, char *,ERR_error_string, unsigned long, e, char *,buf);
MOCKABLE_FUNCTION(, unsigned long, ERR_peek_error);

/*from openssl/x509_vfy.h*/
MOCKABLE_FUNCTION(,int, X509_STORE_add_cert, X509_STORE *, ctx, X509 *, x);

typedef void (*x509_FREE_FUNC)(void*);
MOCKABLE_FUNCTION(, void, sk_pop_free, _STACK*, st, x509_FREE_FUNC, free_func);
MOCKABLE_FUNCTION(, void, EVP_PKEY_free, EVP_PKEY*, pkey);
MOCKABLE_FUNCTION(, X509*, PEM_read_bio_X509_AUX, BIO*, bp, X509**, x, pem_password_cb*, cb, void*, u);
MOCKABLE_FUNCTION(, EVP_PKEY*, PEM_read_bio_PrivateKey, BIO*, bp, EVP_PKEY**, x, pem_password_cb*, cb, void*, u);
MOCKABLE_FUNCTION(, int, SSL_CTX_use_PrivateKey, SSL_CTX*, ctx, EVP_PKEY*, pkey);
MOCKABLE_FUNCTION(, long, SSL_CTX_ctrl, SSL_CTX*, ctx, int, cmd, long, larg, void*, parg);
MOCKABLE_FUNCTION(, unsigned long, ERR_peek_last_error);
MOCKABLE_FUNCTION(, void, ERR_clear_error);

MOCKABLE_FUNCTION(, int, ENGINE_init, ENGINE*, e);
MOCKABLE_FUNCTION(, int, ENGINE_set_default, ENGINE*, e, unsigned int, flags);
MOCKABLE_FUNCTION(, EVP_PKEY*, ENGINE_load_private_key, ENGINE*, e, const char*, key_id, UI_METHOD*, ui_method, void*, callback_data);
MOCKABLE_FUNCTION(, int, ENGINE_finish, ENGINE*, e);

#ifndef __APPLE__
MOCKABLE_FUNCTION(, int, EVP_PKEY_id, const EVP_PKEY*, pkey);
#endif

#undef ENABLE_MOCKS

/*the below function has different signatures on different versions of OPENSSL*/
#if OPENSSL_VERSION_NUMBER >= 0x1000207fL
static BIO* my_BIO_new_mem_buf(const void * buf, int len)
#else
static BIO* my_BIO_new_mem_buf(void * buf, int len)
#endif
{
    (void)len, (void)buf;
    return (BIO*)my_gballoc_malloc(1);
}

static int my_BIO_free(BIO * a)
{
    my_gballoc_free(a);
    return 0;
}

#if OPENSSL_VERSION_NUMBER >= 0x1010007fL
static BIO *my_BIO_new(const BIO_METHOD *type)
#else
static BIO *my_BIO_new(BIO_METHOD *type)
#endif
{
    (void)type;
    return (BIO*)my_gballoc_malloc(1);
}

static void my_RSA_free(RSA * rsa)
{
    my_gballoc_free(rsa);
}

static void my_X509_free(X509 * a)
{
    my_gballoc_free(a);
}

static X509* my_PEM_read_bio_X509_AUX(BIO* bp, X509** x, pem_password_cb* cb, void* u)
{
    (void)u, (void)cb, (void)x, (void)bp;
    return (X509*)my_gballoc_malloc(1);
}

static long my_SSL_CTX_ctrl(SSL_CTX* ctx, int cmd, long larg, void* parg)
{
    (void)ctx;
    (void)cmd;
    (void)larg;
    my_gballoc_free(parg);
    return 1;
}

static RSA* my_EVP_PKEY_get1_RSA(EVP_PKEY* pkey)
{
    (void)pkey;
    return (RSA*)my_gballoc_malloc(1);
}

static X509 * my_PEM_read_bio_X509(BIO * bp, X509 ** x, pem_password_cb * cb, void * u)
{
    (void)u, (void)cb, (void)x, (void)bp;
    return (X509*)my_gballoc_malloc(1);
}

static RSA * my_PEM_read_bio_RSAPrivateKey(BIO * bp, RSA ** x, pem_password_cb * cb, void * u)
{
    (void)u, (void)cb, (void)x, (void)bp;
    return (RSA*)my_gballoc_malloc(1);
}

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    (void)error_code;
    ASSERT_FAIL("umock_c reported error");
}

typedef struct SSL_TEST_CTX_tag
{
    void* extra_certs;
} SSL_TEST_CTX;

typedef struct replace_evp_pkey_st_tag
{
    int type;
} replace_evp_pkey_st;

#define TEST_SSL_CTX ((SSL_CTX*)(0x42))
#define TEST_CERTIFICATE_1 "one certificate"
#define TEST_X509_STORE (X509_STORE *)"le store"
#define TEST_BIO_METHOD (BIO_METHOD*)"le method"
#define TEST_BIO (BIO*)"le bio"
#define TEST_ENGINE (ENGINE*)"the engine"
#define TEST_KEY_ID "the key id"

static const char* TEST_PUBLIC_CERTIFICATE = "PUBLIC CERTIFICATE";
static const char* TEST_PRIVATE_CERTIFICATE = "PRIVATE KEY";
static BIO* TEST_BIO_CERT = (BIO*)0x11;
static X509* TEST_X509 = (X509*)0x13;
static SSL_CTX* TEST_SSL_CTX_STRUCTURE;
static SSL_TEST_CTX g_replace_ctx;
static EVP_PKEY* g_evp_pkey;
static replace_evp_pkey_st g_replace_evp_key;

BEGIN_TEST_SUITE(x509_openssl_unittests)

    TEST_SUITE_INITIALIZE(a)
    {
        g_evp_pkey = (EVP_PKEY*)&g_replace_evp_key;

        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        (void)umock_c_init(on_umock_c_error);

        (void)umocktypes_charptr_register_types();

        REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);

        REGISTER_GLOBAL_MOCK_HOOK(BIO_new_mem_buf, my_BIO_new_mem_buf);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(BIO_new_mem_buf, NULL);

        REGISTER_GLOBAL_MOCK_HOOK(PEM_read_bio_X509, my_PEM_read_bio_X509);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(PEM_read_bio_X509, NULL);

        REGISTER_GLOBAL_MOCK_HOOK(PEM_read_bio_RSAPrivateKey, my_PEM_read_bio_RSAPrivateKey);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(PEM_read_bio_X509, NULL);

        REGISTER_GLOBAL_MOCK_RETURNS(SSL_CTX_use_certificate, 1, 0);
        REGISTER_GLOBAL_MOCK_RETURNS(BIO_s_mem, TEST_BIO_METHOD, NULL);

        REGISTER_GLOBAL_MOCK_HOOK(BIO_new, my_BIO_new);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(BIO_new, NULL);
        REGISTER_GLOBAL_MOCK_RETURNS(BIO_puts, strlen(TEST_CERTIFICATE_1), strlen(TEST_CERTIFICATE_1)-1);

        REGISTER_GLOBAL_MOCK_RETURNS(SSL_CTX_get_cert_store, TEST_X509_STORE, NULL);
        REGISTER_GLOBAL_MOCK_RETURNS(X509_STORE_add_cert, __LINE__, 0);

        REGISTER_GLOBAL_MOCK_RETURNS(SSL_CTX_use_RSAPrivateKey, 1, 0);

        REGISTER_GLOBAL_MOCK_HOOK(BIO_free, my_BIO_free);
        REGISTER_GLOBAL_MOCK_HOOK(RSA_free, my_RSA_free);
        REGISTER_GLOBAL_MOCK_HOOK(X509_free, my_X509_free);
        REGISTER_GLOBAL_MOCK_HOOK(EVP_PKEY_get1_RSA, my_EVP_PKEY_get1_RSA);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(EVP_PKEY_get1_RSA, NULL);

        REGISTER_GLOBAL_MOCK_RETURNS(PEM_read_bio_PrivateKey, g_evp_pkey, NULL);

        REGISTER_GLOBAL_MOCK_RETURNS(BIO_new_mem_buf, TEST_BIO_CERT, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(PEM_read_bio_X509_AUX, my_PEM_read_bio_X509_AUX);
        REGISTER_GLOBAL_MOCK_RETURNS(SSL_CTX_use_PrivateKey, 1, 0);
        REGISTER_GLOBAL_MOCK_HOOK(SSL_CTX_ctrl, my_SSL_CTX_ctrl);

        REGISTER_GLOBAL_MOCK_RETURNS(ENGINE_init, 1, 0);
        REGISTER_GLOBAL_MOCK_RETURNS(ENGINE_set_default, 1, 0);
        REGISTER_GLOBAL_MOCK_RETURNS(ENGINE_load_private_key, g_evp_pkey, NULL);
        REGISTER_GLOBAL_MOCK_RETURNS(ENGINE_finish, 1, 0);
    }

    TEST_SUITE_CLEANUP(TestClassCleanup)
    {
        umock_c_deinit();

        TEST_MUTEX_DESTROY(g_testByTest);
    }

    TEST_FUNCTION_INITIALIZE(initialize)
    {
        umock_c_reset_all_calls();

        memset(&g_replace_ctx, 0, sizeof(SSL_TEST_CTX) );
        TEST_SSL_CTX_STRUCTURE = (SSL_CTX*)&g_replace_ctx;

        memset(&g_replace_evp_key, 0, sizeof(replace_evp_pkey_st));
        g_evp_pkey = (EVP_PKEY*)&g_replace_evp_key;
    }

    TEST_FUNCTION_CLEANUP(cleans)
    {
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

    static void setup_load_alias_key_cert_mocks(bool is_rsa_cert)
    {
        if (is_rsa_cert)
        {
            g_replace_evp_key.type = EVP_PKEY_RSA;
            STRICT_EXPECTED_CALL(EVP_PKEY_get1_RSA(g_evp_pkey));
            STRICT_EXPECTED_CALL(SSL_CTX_use_RSAPrivateKey(TEST_SSL_CTX_STRUCTURE, IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(RSA_free(IGNORED_PTR_ARG) );
        }
        else
        {
            g_replace_evp_key.type = EVP_PKEY_EC;
            STRICT_EXPECTED_CALL(SSL_CTX_use_PrivateKey(TEST_SSL_CTX_STRUCTURE, g_evp_pkey));
        }
    }

    static void setup_load_certificate_chain_mocks()
    {
        STRICT_EXPECTED_CALL(BIO_new_mem_buf((void*)TEST_PUBLIC_CERTIFICATE, -1));
        STRICT_EXPECTED_CALL(PEM_read_bio_X509_AUX(IGNORED_PTR_ARG, NULL, NULL, NULL));
        STRICT_EXPECTED_CALL(SSL_CTX_use_certificate(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L) || defined(LIBRESSL_VERSION_NUMBER)
        // Actual macro name: SSL_CTX_clear_extra_chain_certs:
        STRICT_EXPECTED_CALL(SSL_CTX_ctrl(TEST_SSL_CTX_STRUCTURE, SSL_CTRL_CLEAR_EXTRA_CHAIN_CERTS, 0, NULL));
#endif
        STRICT_EXPECTED_CALL(PEM_read_bio_X509(IGNORED_PTR_ARG, NULL, NULL, NULL));
        // Actual macro name: SSL_CTX_add_extra_chain_cert:
        STRICT_EXPECTED_CALL(SSL_CTX_ctrl(TEST_SSL_CTX_STRUCTURE, SSL_CTRL_EXTRA_CHAIN_CERT, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(PEM_read_bio_X509(IGNORED_PTR_ARG, NULL, NULL, NULL))
            .SetReturn(NULL); // Needed because the x509 needs not to be free
        STRICT_EXPECTED_CALL(X509_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(BIO_free(IGNORED_PTR_ARG));
    }

    static void setup_add_credentials_pem_file(bool is_rsa_cert)
    {
        // x509_openssl_add_pem_file_key
        STRICT_EXPECTED_CALL(BIO_new_mem_buf((char*)TEST_PRIVATE_CERTIFICATE, -1));
        STRICT_EXPECTED_CALL(PEM_read_bio_PrivateKey(IGNORED_PTR_ARG, NULL, NULL, NULL));
#ifndef __APPLE__
        STRICT_EXPECTED_CALL(EVP_PKEY_id(IGNORED_PTR_ARG)).SetReturn(is_rsa_cert ? EVP_PKEY_RSA : EVP_PKEY_EC);
#endif
        setup_load_alias_key_cert_mocks(is_rsa_cert);
        STRICT_EXPECTED_CALL(EVP_PKEY_free(g_evp_pkey));
        STRICT_EXPECTED_CALL(BIO_free(IGNORED_PTR_ARG));

        setup_load_certificate_chain_mocks();
    }

    static void setup_add_credentials_engine()
    {
        // x509_openssl_add_pem_file_key
        STRICT_EXPECTED_CALL(ENGINE_init(TEST_ENGINE));
        STRICT_EXPECTED_CALL(ENGINE_set_default(TEST_ENGINE, ENGINE_METHOD_ALL));
        STRICT_EXPECTED_CALL(ENGINE_load_private_key(TEST_ENGINE, TEST_KEY_ID, NULL, NULL));
        setup_load_alias_key_cert_mocks(false);
        STRICT_EXPECTED_CALL(EVP_PKEY_free(g_evp_pkey));
        STRICT_EXPECTED_CALL(ENGINE_finish(TEST_ENGINE));
        setup_load_certificate_chain_mocks();
    }

    /*Tests_SRS_X509_OPENSSL_02_001: [ If any argument is NULL then x509_openssl_add_credentials shall fail and return a non-zero value. ]*/
    TEST_FUNCTION(x509_openssl_add_credentials_with_NULL_SSL_CTX_fails)
    {
        //arrange

        //act
        int result = x509_openssl_add_credentials(NULL, TEST_PUBLIC_CERTIFICATE, "privatekey", KEY_TYPE_DEFAULT, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        //cleanup
    }

    /*Tests_SRS_X509_OPENSSL_02_001: [ If any argument is NULL then x509_openssl_add_credentials shall fail and return a non-zero value. ]*/
    TEST_FUNCTION(x509_openssl_add_credentials_with_NULL_certificate_fails)
    {
        //arrange

        //act
        int result = x509_openssl_add_credentials(TEST_SSL_CTX, NULL, "privatekey", KEY_TYPE_DEFAULT, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        //cleanup
    }

    /*Tests_SRS_X509_OPENSSL_02_001: [ If any argument is NULL then x509_openssl_add_credentials shall fail and return a non-zero value. ]*/
    TEST_FUNCTION(x509_openssl_add_credentials_with_NULL_privatekey_fails)
    {
        //arrange

        //act
        int result = x509_openssl_add_credentials(TEST_SSL_CTX, TEST_PUBLIC_CERTIFICATE, NULL, KEY_TYPE_DEFAULT, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        //cleanup
    }

    TEST_FUNCTION(x509_openssl_engine_add_credentials_with_NULL_certificate_fails)
    {
        //arrange

        //act
        int result = x509_openssl_add_credentials(TEST_SSL_CTX, NULL, "privatekey", KEY_TYPE_ENGINE, TEST_ENGINE);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        //cleanup
    }

    TEST_FUNCTION(x509_openssl_engine_add_credentials_with_NULL_privatekey_fails)
    {
        //arrange

        //act
        int result = x509_openssl_add_credentials(TEST_SSL_CTX, TEST_PUBLIC_CERTIFICATE, NULL, KEY_TYPE_ENGINE, TEST_ENGINE);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        //cleanup
    }

    TEST_FUNCTION(x509_openssl_engine_add_credentials_with_NULL_engine_fails)
    {
        //arrange

        //act
        int result = x509_openssl_add_credentials(TEST_SSL_CTX, TEST_PUBLIC_CERTIFICATE, "privatekey", KEY_TYPE_ENGINE, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        //cleanup
    }

    /*Tests_SRS_X509_OPENSSL_02_002: [ x509_openssl_add_credentials shall use BIO_new_mem_buf to create a memory BIO from the x509 certificate. ] */
    /*Tests_SRS_X509_OPENSSL_02_003: [ x509_openssl_add_credentials shall use PEM_read_bio_X509 to read the x509 certificate. ] */
    /*Tests_SRS_X509_OPENSSL_02_004: [ x509_openssl_add_credentials shall use BIO_new_mem_buf to create a memory BIO from the x509 privatekey. ] */
    /*Tests_SRS_X509_OPENSSL_02_005: [ x509_openssl_add_credentials shall use PEM_read_bio_RSAPrivateKey to read the x509 private key. ] */
    /*Tests_SRS_X509_OPENSSL_02_006: [ x509_openssl_add_credentials shall use SSL_CTX_use_certificate to load the certicate into the SSL context. ] */
    /*Tests_SRS_X509_OPENSSL_02_007: [ x509_openssl_add_credentials shall use SSL_CTX_use_RSAPrivateKey to load the private key into the SSL context. ]*/
    /*Tests_SRS_X509_OPENSSL_02_008: [ If no error occurs, then x509_openssl_add_credentials shall succeed and return 0. ] */
    TEST_FUNCTION(x509_openssl_add_credentials_rsa_happy_path)
    {
        setup_add_credentials_pem_file(true);

        //act
        int result = x509_openssl_add_credentials(TEST_SSL_CTX_STRUCTURE, TEST_PUBLIC_CERTIFICATE, TEST_PRIVATE_CERTIFICATE, KEY_TYPE_DEFAULT, NULL);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(x509_openssl_add_credentials_ecc_happy_path)
    {
        setup_add_credentials_pem_file(false);

        //act
        int result = x509_openssl_add_credentials(TEST_SSL_CTX_STRUCTURE, TEST_PUBLIC_CERTIFICATE, TEST_PRIVATE_CERTIFICATE, KEY_TYPE_DEFAULT, NULL);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(x509_openssl_engine_add_credentials_happy_path)
    {
        setup_add_credentials_engine();

        //act
        int result = x509_openssl_add_credentials(TEST_SSL_CTX_STRUCTURE, TEST_PUBLIC_CERTIFICATE, TEST_KEY_ID, KEY_TYPE_ENGINE, TEST_ENGINE);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    void x509_openssl_add_credentials_fails(bool is_rsa, bool use_engine)
    {
        //arrange
        umock_c_reset_all_calls();

        int negativeTestsInitResult = umock_c_negative_tests_init();
        ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

        if (!use_engine)
        {
            setup_add_credentials_pem_file(is_rsa);
        }
        else
        {
            setup_add_credentials_engine();
        }

        umock_c_negative_tests_snapshot();

#if (OPENSSL_VERSION_NUMBER >= 0x10100000L) || defined(LIBRESSL_VERSION_NUMBER)
    #ifdef __APPLE__
        size_t calls_cannot_fail_rsa[] = { 4, 5, 6, 10, 12, 13, 14 };
        size_t calls_cannot_fail_ecc[] = { 3, 4, 8, 10, 11, 12} ;
        size_t calls_cannot_fail_engine[] = { 4, 9, 11, 12, 13 };
#else
        size_t calls_cannot_fail_rsa[] = { 2, 5, 6, 7, 11, 12, 14, 15, 16 };
        size_t calls_cannot_fail_ecc[] = { 2, 4, 5, 9, 10, 12, 13, 14 };
        size_t calls_cannot_fail_engine[] = { 4, 9, 10, 12, 13, 14 };
    #endif
#else
    #ifdef __APPLE__
        size_t calls_cannot_fail_rsa[] = { 4, 5, 6, 10, 12, 13, 14 };
        size_t calls_cannot_fail_ecc[] = { 3, 4, 8, 10, 11, 12 };
        size_t calls_cannot_fail_engine[] = {4, 9, 11, 12, 13};
    #else
        size_t calls_cannot_fail_rsa[] = { 2, 5, 6, 7, 11, 13, 14, 15, 16 };
        size_t calls_cannot_fail_ecc[] = { 2, 4, 5, 9, 11, 12, 13, 14 };
        size_t calls_cannot_fail_engine[] = { 4, 9, 11, 12, 13, 14 };
    #endif
#endif
        size_t *calls_cannot_fail;
        size_t calls_cannot_fail_size;

        if (!use_engine)
        {
            calls_cannot_fail = is_rsa ? calls_cannot_fail_rsa : calls_cannot_fail_ecc;
            calls_cannot_fail_size = is_rsa ? sizeof(calls_cannot_fail_rsa) / sizeof(calls_cannot_fail_rsa[0]) : sizeof(calls_cannot_fail_ecc) / sizeof(calls_cannot_fail_ecc[0]);
        }
        else
        {
            calls_cannot_fail = calls_cannot_fail_engine;
            calls_cannot_fail_size = sizeof(calls_cannot_fail_engine) / sizeof(calls_cannot_fail_engine[0]);
        }

        //act
        int result;
        size_t count = umock_c_negative_tests_call_count();
        for (size_t index = 0; index < count; index++)
        {
            if (should_skip_index(index, calls_cannot_fail, calls_cannot_fail_size) != 0)
            {
                continue;
            }

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(index);

            char tmp_msg[128];
            sprintf(tmp_msg, "x509_openssl_add_credentials failure in test %lu/%lu", (unsigned long)index, (unsigned long)count);

            g_replace_ctx.extra_certs = NULL;

            if (!use_engine)
            {
                result = x509_openssl_add_credentials(TEST_SSL_CTX_STRUCTURE, TEST_PUBLIC_CERTIFICATE, TEST_PRIVATE_CERTIFICATE, KEY_TYPE_DEFAULT, NULL);
            }
            else
            {
                result = x509_openssl_add_credentials(TEST_SSL_CTX_STRUCTURE, TEST_PUBLIC_CERTIFICATE, TEST_KEY_ID, KEY_TYPE_ENGINE, TEST_ENGINE);
            }

            //assert
            ASSERT_ARE_NOT_EQUAL(int, 0, result, tmp_msg);
        }

        //cleanup
        umock_c_negative_tests_deinit();
    }

    /*Tests_SRS_X509_OPENSSL_02_009: [ Otherwise x509_openssl_add_credentials shall fail and return a non-zero number. ]*/
    TEST_FUNCTION(x509_openssl_add_rsa_credentials_fails)
    {
        x509_openssl_add_credentials_fails(/* is_rsa: */ true, /* use_engine: */ false);
    }

    /*Tests_SRS_X509_OPENSSL_02_009: [ Otherwise x509_openssl_add_credentials shall fail and return a non-zero number. ]*/
    TEST_FUNCTION(x509_openssl_add_ecc_credentials_fails)
    {
        x509_openssl_add_credentials_fails(/* is_rsa: */ false, /* use_engine: */ false);
    }

    TEST_FUNCTION(x509_openssl_add_engine_credentials_fails)
    {
        x509_openssl_add_credentials_fails(/* is_rsa: */ false, /* use_engine: */ true);
    }

    /*Tests_SRS_X509_OPENSSL_02_010: [ If ssl_ctx is NULL then x509_openssl_add_certificates shall fail and return a non-zero value. ]*/
    TEST_FUNCTION(x509_openssl_add_certificates_with_NULL_ssl_ctx_fails)
    {
        //arrange

        //act
        int result = x509_openssl_add_certificates(NULL, "a");

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        //clean
    }

    /*Tests_SRS_X509_OPENSSL_02_011: [ If certificates is NULL then x509_openssl_add_certificates shall fail and return a non-zero value. ]*/
    TEST_FUNCTION(x509_openssl_add_certificates_with_NULL_certificates_fails)
    {
        //arrange

        //act
        int result = x509_openssl_add_certificates(TEST_SSL_CTX, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        //clean
    }

    /*Tests_SRS_X509_OPENSSL_02_012: [ x509_openssl_add_certificates shall get the memory BIO method function by calling BIO_s_mem. ]*/
    /*Tests_SRS_X509_OPENSSL_02_013: [ x509_openssl_add_certificates shall create a new memory BIO by calling BIO_new. ]*/
    /*Tests_SRS_X509_OPENSSL_02_014: [ x509_openssl_add_certificates shall load certificates into the memory BIO by a call to BIO_puts. ]*/
    /*Tests_SRS_X509_OPENSSL_02_015: [ x509_openssl_add_certificates shall retrieve each certificate by a call to PEM_read_bio_X509. ]*/
    /*Tests_SRS_X509_OPENSSL_02_016: [ x509_openssl_add_certificates shall add the certificate to the store by a call to X509_STORE_add_cert. ]*/
    /*Tests_SRS_X509_OPENSSL_02_019: [ Otherwise, x509_openssl_add_certificates shall succeed and return 0. ]*/
    TEST_FUNCTION(x509_openssl_add_certificates_1_certificate_happy_path)
    {
        //arrange
        int result;

        STRICT_EXPECTED_CALL(SSL_CTX_get_cert_store(TEST_SSL_CTX));
        STRICT_EXPECTED_CALL(BIO_s_mem());
        STRICT_EXPECTED_CALL(BIO_new(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(BIO_puts(IGNORED_PTR_ARG, TEST_CERTIFICATE_1));
        STRICT_EXPECTED_CALL(PEM_read_bio_X509(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(X509_STORE_add_cert(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(X509_free(IGNORED_PTR_ARG));

        STRICT_EXPECTED_CALL(PEM_read_bio_X509(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
            .SetReturn(NULL);

        //act
        result = x509_openssl_add_certificates(TEST_SSL_CTX, TEST_CERTIFICATE_1);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);

        //clean
    }

    void x509_openssl_add_certificates_1_certificate_which_exists_inert_path(void)
    {
        STRICT_EXPECTED_CALL(SSL_CTX_get_cert_store(TEST_SSL_CTX));
        STRICT_EXPECTED_CALL(BIO_s_mem());
        STRICT_EXPECTED_CALL(BIO_new(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(BIO_puts(IGNORED_PTR_ARG, TEST_CERTIFICATE_1));
        STRICT_EXPECTED_CALL(PEM_read_bio_X509(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(X509_STORE_add_cert(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
            .SetReturn(0);
        STRICT_EXPECTED_CALL(ERR_peek_error())
            .SetReturn(X509_R_CERT_ALREADY_IN_HASH_TABLE);
        STRICT_EXPECTED_CALL(X509_free(IGNORED_PTR_ARG));

        STRICT_EXPECTED_CALL(PEM_read_bio_X509(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
            .SetReturn(NULL);
    }

    /*Tests_SRS_X509_OPENSSL_02_017: [ If X509_STORE_add_cert returns with error and that error is X509_R_CERT_ALREADY_IN_HASH_TABLE then x509_openssl_add_certificates shall ignore it as the certificate is already in the store. ]*/
    TEST_FUNCTION(x509_openssl_add_certificates_1_certificate_which_exists_happy_path)
    {
        //arrange
        int result;

        x509_openssl_add_certificates_1_certificate_which_exists_inert_path();

        //act
        result = x509_openssl_add_certificates(TEST_SSL_CTX, TEST_CERTIFICATE_1);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);

        //clean
    }

    /*Tests_SRS_X509_OPENSSL_02_018: [ In case of any failure x509_openssl_add_certificates shall fail and return a non-zero value. ]*/
    TEST_FUNCTION(x509_openssl_add_certificates_1_certificate_which_exists_unhappy_paths)
    {
        //arrange
        umock_c_reset_all_calls();

        int negativeTestsInitResult = umock_c_negative_tests_init();
        ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

        x509_openssl_add_certificates_1_certificate_which_exists_inert_path();
        umock_c_negative_tests_snapshot();

        size_t calls_cannot_fail[] = { 4, 5, 7, 8 };

        int result;
        size_t count = umock_c_negative_tests_call_count();
        for (size_t index = 0; index < count; index++)
        {
            if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
            {
                continue;
            }

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(index);

            char tmp_msg[128];
            sprintf(tmp_msg, "x509_openssl_add_credentials failure in test %lu/%lu", (unsigned long)index, (unsigned long)count);

            //act
            result = x509_openssl_add_certificates(TEST_SSL_CTX, TEST_CERTIFICATE_1);

            //assert
            ASSERT_ARE_NOT_EQUAL(int, 0, result, tmp_msg);
        }

        //clean
        umock_c_negative_tests_deinit();
    }

END_TEST_SUITE(x509_openssl_unittests)
