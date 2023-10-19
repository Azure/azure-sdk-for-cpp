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

/**
* The gballoc.h will replace the malloc, free, and realloc by the my_gballoc functions, in this case,
*    if you define these mock functions after include the gballoc.h, you will create an infinity recursion,
*    so, places the my_gballoc functions before the #include "azure_c_shared_utility/gballoc.h"
*/
#include "gballoc_ut_impl_1.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#undef ENABLE_MOCKS

#include "azure_c_shared_utility/tlsio_options.h"
#include "azure_c_shared_utility/shared_util_options.h"

#include "testrunnerswitcher.h"
#include "umock_c/umock_c_negative_tests.h"


static TEST_MUTEX_HANDLE g_testByTest;

#include "gballoc_ut_impl_2.h"

const char* fake_trusted_cert = "Fake trusted cert";
const char* fake_x509_cert = "Fake x509 cert";
const char* fake_x509_key = "Fake x509 key";

#define SET_PV_COUNT 3
#define RETRIEVE_PV_COUNT 4
#define SET_INCONSISTENT_x509_COUNT 8
#define SET_NOT_SUPPORTED_COUNT 5

void ASSERT_COPIED_STRING(const char* target, const char* source)
{
    ASSERT_IS_NOT_NULL(target, "Target string is NULL");
    ASSERT_IS_NOT_NULL(target, "Source string is NULL");
    ASSERT_ARE_NOT_EQUAL(void_ptr, (void*)target, (void*)source, "Strings are duplicates instead of copies");
    ASSERT_ARE_EQUAL(char_ptr, target, source, "Strings do not match");
}

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

static void use_negative_mocks()
{
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);
}


void* pfCloneOption_impl(const char* name, const void* value)
{
    void* result;
    tlsio_options_clone_option(name, value, &result);
    return result;
}

void pfDestroyOption_impl(const char* name, const void* value)
{
    (void)tlsio_options_destroy_option(name, value);
}

int pfSetOption_impl(void* handle, const char* name, const void* value)
{
    TLSIO_OPTIONS* options = (TLSIO_OPTIONS*)handle;
    TLSIO_OPTIONS_RESULT sr = tlsio_options_set(options, name, value);
    return sr == TLSIO_OPTIONS_RESULT_SUCCESS ? 0 : MU_FAILURE;
}

BEGIN_TEST_SUITE(tlsio_options_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    REGISTER_UMOCK_ALIAS_TYPE(TLSIO_OPTIONS_x509_TYPE, int);

    /**
    * Or you can combine, for example, in the success case malloc will call my_gballoc_malloc, and for
    *    the failed cases, it will return NULL.
    */
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
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
    init_gballoc_checks();
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

TEST_FUNCTION(tlsio_options_initialize__succeeds)
{
    ///arrange
    TLSIO_OPTIONS options;
    memset(&options, 0xff, sizeof(options));

    ///act
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT | TLSIO_OPTION_BIT_x509_ECC_CERT);

    ///assert
    ASSERT_IS_NULL(options.trusted_certs);
    ASSERT_IS_NULL(options.x509_cert);
    ASSERT_IS_NULL(options.x509_key);
    ASSERT_ARE_EQUAL(int, options.supported_options, (int)(TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT | TLSIO_OPTION_BIT_x509_ECC_CERT));
    ASSERT_ARE_EQUAL(int, (int)options.x509_type, (int)TLSIO_OPTIONS_x509_TYPE_UNSPECIFIED);

    ///clean
}

TEST_FUNCTION(tlsio_options__set_trusted_certs__succeeds)
{
    ///arrange
    TLSIO_OPTIONS_RESULT result;
    TLSIO_OPTIONS options;
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS);

    ///act
    result = tlsio_options_set(&options, OPTION_TRUSTED_CERT, fake_trusted_cert);

    ///assert
    ASSERT_COPIED_STRING(options.trusted_certs, fake_trusted_cert);
    ASSERT_IS_NULL(options.x509_cert);
    ASSERT_IS_NULL(options.x509_key);
    ASSERT_ARE_EQUAL(int, options.supported_options, (int)(TLSIO_OPTION_BIT_TRUSTED_CERTS));
    ASSERT_ARE_EQUAL(int, (int)options.x509_type, (int)TLSIO_OPTIONS_x509_TYPE_UNSPECIFIED);
    ASSERT_ARE_EQUAL(int, (int)result, 0);

    ///clean
    tlsio_options_release_resources(&options);
    assert_gballoc_checks();
}

TEST_FUNCTION(tlsio_options__set_x509_certs__succeeds)
{
    ///arrange
    TLSIO_OPTIONS_RESULT result;
    TLSIO_OPTIONS options;
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_x509_RSA_CERT);

    ///act
    result = tlsio_options_set(&options, SU_OPTION_X509_CERT, fake_x509_cert);

    ///assert
    ASSERT_IS_NULL(options.trusted_certs);
    ASSERT_COPIED_STRING(options.x509_cert, fake_x509_cert);
    ASSERT_IS_NULL(options.x509_key);
    ASSERT_ARE_EQUAL(int, options.supported_options, (int)(TLSIO_OPTION_BIT_x509_RSA_CERT));
    ASSERT_ARE_EQUAL(int, (int)options.x509_type, (int)TLSIO_OPTIONS_x509_TYPE_RSA);
    ASSERT_ARE_EQUAL(int, (int)result, 0);

    ///clean
    tlsio_options_release_resources(&options);
    assert_gballoc_checks();
}

TEST_FUNCTION(tlsio_options__set_x509_ECC_certs__succeeds)
{
    ///arrange
    TLSIO_OPTIONS_RESULT result;
    TLSIO_OPTIONS options;
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_x509_ECC_CERT);

    ///act
    result = tlsio_options_set(&options, OPTION_X509_ECC_CERT, fake_x509_cert);

    ///assert
    ASSERT_IS_NULL(options.trusted_certs);
    ASSERT_COPIED_STRING(options.x509_cert, fake_x509_cert);
    ASSERT_IS_NULL(options.x509_key);
    ASSERT_ARE_EQUAL(int, options.supported_options, (int)(TLSIO_OPTION_BIT_x509_ECC_CERT));
    ASSERT_ARE_EQUAL(int, (int)options.x509_type, (int)TLSIO_OPTIONS_x509_TYPE_ECC);
    ASSERT_ARE_EQUAL(int, (int)result, 0);

    ///clean
    tlsio_options_release_resources(&options);
    assert_gballoc_checks();
}

TEST_FUNCTION(tlsio_options__set_x509_key__succeeds)
{
    ///arrange
    TLSIO_OPTIONS_RESULT result;
    TLSIO_OPTIONS options;
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_x509_RSA_CERT);

    ///act
    result = tlsio_options_set(&options, SU_OPTION_X509_PRIVATE_KEY, fake_x509_key);

    ///assert
    ASSERT_IS_NULL(options.trusted_certs);
    ASSERT_IS_NULL(options.x509_cert);
    ASSERT_COPIED_STRING(options.x509_key, fake_x509_key);
    ASSERT_ARE_EQUAL(int, options.supported_options, (int)(TLSIO_OPTION_BIT_x509_RSA_CERT));
    ASSERT_ARE_EQUAL(int, (int)options.x509_type, (int)TLSIO_OPTIONS_x509_TYPE_RSA);
    ASSERT_ARE_EQUAL(int, (int)result, 0);

    ///clean
    tlsio_options_release_resources(&options);
    assert_gballoc_checks();
}

TEST_FUNCTION(tlsio_options__set_x509_ECC_key__succeeds)
{
    ///arrange
    TLSIO_OPTIONS_RESULT result;
    TLSIO_OPTIONS options;
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_x509_ECC_CERT);

    ///act
    result = tlsio_options_set(&options, OPTION_X509_ECC_KEY, fake_x509_key);

    ///assert
    ASSERT_IS_NULL(options.trusted_certs);
    ASSERT_IS_NULL(options.x509_cert);
    ASSERT_COPIED_STRING(options.x509_key, fake_x509_key);
    ASSERT_ARE_EQUAL(int, options.supported_options, (int)(TLSIO_OPTION_BIT_x509_ECC_CERT));
    ASSERT_ARE_EQUAL(int, (int)options.x509_type, (int)TLSIO_OPTIONS_x509_TYPE_ECC);
    ASSERT_ARE_EQUAL(int, (int)result, 0);

    ///clean
    tlsio_options_release_resources(&options);
    assert_gballoc_checks();
}

TEST_FUNCTION(tlsio_options__set_unhandled__succeeds)
{
    ///arrange
    TLSIO_OPTIONS_RESULT result;
    TLSIO_OPTIONS options;
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_x509_ECC_CERT);

    ///act
    result = tlsio_options_set(&options, OPTION_HTTP_PROXY, fake_x509_key);

    ///assert
    ASSERT_IS_NULL(options.trusted_certs);
    ASSERT_IS_NULL(options.x509_cert);
    ASSERT_IS_NULL(options.x509_key);
    ASSERT_ARE_EQUAL(int, options.supported_options, (int)(TLSIO_OPTION_BIT_x509_ECC_CERT));
    ASSERT_ARE_EQUAL(int, (int)options.x509_type, (int)TLSIO_OPTIONS_x509_TYPE_UNSPECIFIED);
    ASSERT_ARE_EQUAL(int, (int)result, (int)TLSIO_OPTIONS_RESULT_NOT_HANDLED);

    ///clean
    tlsio_options_release_resources(&options);
    assert_gballoc_checks();
}

TEST_FUNCTION(tlsio_options__set_parameter_validation__fails)
{
    int i;
    int k = 0;
    TLSIO_OPTIONS* p0[SET_PV_COUNT];
    const char* p1[SET_PV_COUNT];
    const char* p2[SET_PV_COUNT];
    const char* fm[SET_PV_COUNT];

    TLSIO_OPTIONS options;
    TLSIO_OPTIONS_RESULT result;

    p0[k] = NULL;     p1[k] = OPTION_TRUSTED_CERT; p2[k] = fake_x509_key; fm[k] = "Unexpected tlsio_options_initialize success when options is NULL"; /* */  k++;
    p0[k] = &options; p1[k] = NULL; /*          */ p2[k] = fake_x509_key; fm[k] = "Unexpected tlsio_options_initialize success when option_name is NULL"; k++;
    p0[k] = &options; p1[k] = OPTION_TRUSTED_CERT; p2[k] = NULL;          fm[k] = "Unexpected tlsio_options_initialize success when option_value is NULL"; k++;



    // Cycle through each failing combo of parameters
    for (i = 0; i < SET_PV_COUNT; i++)
    {
        ///arrange
        tlsio_options_initialize(&options, TLSIO_OPTION_BIT_x509_ECC_CERT);

        ///act
        result = tlsio_options_set(p0[i], p1[i], p2[i]);

        ///assert
        ASSERT_IS_NULL(options.trusted_certs);
        ASSERT_IS_NULL(options.x509_cert);
        ASSERT_IS_NULL(options.x509_key);
        ASSERT_ARE_EQUAL(int, options.supported_options, (int)(TLSIO_OPTION_BIT_x509_ECC_CERT));
        ASSERT_ARE_EQUAL(int, (int)options.x509_type, (int)TLSIO_OPTIONS_x509_TYPE_UNSPECIFIED);
        ASSERT_ARE_EQUAL(int, (int)result, (int)TLSIO_OPTIONS_RESULT_ERROR, fm[i]);

        ///clean
        tlsio_options_release_resources(&options);
        assert_gballoc_checks();
    }
}

TEST_FUNCTION(tlsio_options__set_x509_bad_combos__fails)
{
    int i;
    int k = 0;
    const char* p0[SET_INCONSISTENT_x509_COUNT];
    const char* p1[SET_INCONSISTENT_x509_COUNT];

    TLSIO_OPTIONS options;
    TLSIO_OPTIONS_RESULT result;

    p0[k] = SU_OPTION_X509_CERT; /*  */ p1[k] = OPTION_X509_ECC_CERT; k++;
    p0[k] = SU_OPTION_X509_CERT; /*  */ p1[k] = OPTION_X509_ECC_KEY; k++;
    p0[k] = SU_OPTION_X509_PRIVATE_KEY; p1[k] = OPTION_X509_ECC_CERT; k++;
    p0[k] = SU_OPTION_X509_PRIVATE_KEY; p1[k] = OPTION_X509_ECC_KEY; k++;
    p0[k] = OPTION_X509_ECC_CERT; /* */ p1[k] = SU_OPTION_X509_CERT; k++;
    p0[k] = OPTION_X509_ECC_CERT; /* */ p1[k] = SU_OPTION_X509_PRIVATE_KEY; k++;
    p0[k] = OPTION_X509_ECC_KEY; /*  */ p1[k] = SU_OPTION_X509_CERT; k++;
    p0[k] = OPTION_X509_ECC_KEY; /*  */ p1[k] = SU_OPTION_X509_PRIVATE_KEY; k++;



    // Cycle through each failing combo of parameters
    for (i = 0; i < SET_INCONSISTENT_x509_COUNT; i++)
    {
        ///arrange
        tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT | TLSIO_OPTION_BIT_x509_ECC_CERT);
        result = tlsio_options_set(&options, p0[i], fake_x509_key);
        ASSERT_ARE_EQUAL(int, (int)result, (int)TLSIO_OPTIONS_RESULT_SUCCESS);

        ///act
        result = tlsio_options_set(&options, p1[i], fake_x509_key);

        ///assert
        ASSERT_ARE_EQUAL(int, (int)result, (int)TLSIO_OPTIONS_RESULT_ERROR, "Unexpected success with inconsistent x509 settings");

        ///clean
        tlsio_options_release_resources(&options);
        assert_gballoc_checks();
    }
}

TEST_FUNCTION(tlsio_options__set_not_supported__fails)
{
    int i;
    int k = 0;
    const char* p0[SET_NOT_SUPPORTED_COUNT];

    TLSIO_OPTIONS options;
    TLSIO_OPTIONS_RESULT result;

    p0[k] = OPTION_TRUSTED_CERT; /*  */ k++;
    p0[k] = SU_OPTION_X509_CERT; /*  */ k++;
    p0[k] = SU_OPTION_X509_PRIVATE_KEY; k++;
    p0[k] = OPTION_X509_ECC_CERT; /* */ k++;
    p0[k] = OPTION_X509_ECC_KEY; /*  */ k++;

    // Cycle through each failing combo of parameters
    for (i = 0; i < SET_NOT_SUPPORTED_COUNT; i++)
    {
        ///arrange
        umock_c_reset_all_calls();
        tlsio_options_initialize(&options, TLSIO_OPTION_BIT_NONE);

        ///act
        result = tlsio_options_set(&options, p0[i], fake_x509_key);

        ///assert
        ASSERT_ARE_EQUAL(int, (int)result, (int)TLSIO_OPTIONS_RESULT_ERROR, "Unexpected success with unsupported option");

        ///clean
        tlsio_options_release_resources(&options);
        assert_gballoc_checks();
    }
}

TEST_FUNCTION(tlsio_options__set_malloc_fail__fails)
{
    int i;
    int k = 0;
    const char* p0[SET_NOT_SUPPORTED_COUNT];

    TLSIO_OPTIONS options;
    TLSIO_OPTIONS_RESULT result;

    p0[k] = OPTION_TRUSTED_CERT; /*  */ k++;
    p0[k] = SU_OPTION_X509_CERT; /*  */ k++;
    p0[k] = SU_OPTION_X509_PRIVATE_KEY; k++;
    p0[k] = OPTION_X509_ECC_CERT; /* */ k++;
    p0[k] = OPTION_X509_ECC_KEY; /*  */ k++;
    use_negative_mocks();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));  // concrete_io struct
    umock_c_negative_tests_snapshot();

    // Cycle through each failing combo of parameters
    for (i = 0; i < SET_NOT_SUPPORTED_COUNT; i++)
    {
        ///arrange
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(0);

        tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT | TLSIO_OPTION_BIT_x509_ECC_CERT);

        ///act
        result = tlsio_options_set(&options, p0[i], fake_x509_key);

        ///assert
        ASSERT_ARE_EQUAL(int, (int)result, (int)TLSIO_OPTIONS_RESULT_ERROR, "Unexpected success with malloc failure");

        ///clean
        tlsio_options_release_resources(&options);
        assert_gballoc_checks();
    }
    umock_c_negative_tests_deinit();
}

TEST_FUNCTION(tlsio_options__release_resources__succeeds)
{
    ///arrange
    TLSIO_OPTIONS_RESULT result;
    TLSIO_OPTIONS options;
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT);
    result = tlsio_options_set(&options, OPTION_TRUSTED_CERT, fake_trusted_cert);
    result = tlsio_options_set(&options, SU_OPTION_X509_CERT, fake_x509_cert);
    result = tlsio_options_set(&options, SU_OPTION_X509_PRIVATE_KEY, fake_x509_key);

    ///act
    tlsio_options_release_resources(&options);

    ///assert
    assert_gballoc_checks(); // checks for un-freed memory

    ///clean
}

TEST_FUNCTION(tlsio_options__clone_option_OPTION_TRUSTED_CERT__succeeds)
{
    ///arrange
    void* out_result;
    TLSIO_OPTIONS_RESULT result;

    ///act
    result = tlsio_options_clone_option(OPTION_TRUSTED_CERT, fake_trusted_cert, &out_result);

    ///assert
    ASSERT_COPIED_STRING((const char*)out_result, fake_trusted_cert);
    ASSERT_ARE_EQUAL(int, (int)result, (int)TLSIO_OPTIONS_RESULT_SUCCESS);

    ///clean
    free(out_result);
}

TEST_FUNCTION(tlsio_options__clone_option_SU_OPTION_X509_CERT__succeeds)
{
    ///arrange
    void* out_result;
    TLSIO_OPTIONS_RESULT result;

    ///act
    result = tlsio_options_clone_option(SU_OPTION_X509_CERT, fake_trusted_cert, &out_result);

    ///assert
    ASSERT_COPIED_STRING((const char*)out_result, fake_trusted_cert);
    ASSERT_ARE_EQUAL(int, (int)result, (int)TLSIO_OPTIONS_RESULT_SUCCESS);

    ///clean
    free(out_result);
}

TEST_FUNCTION(tlsio_options__clone_option_SU_OPTION_X509_PRIVATE_KEY__succeeds)
{
    ///arrange
    void* out_result;
    TLSIO_OPTIONS_RESULT result;

    ///act
    result = tlsio_options_clone_option(SU_OPTION_X509_PRIVATE_KEY, fake_trusted_cert, &out_result);

    ///assert
    ASSERT_COPIED_STRING((const char*)out_result, fake_trusted_cert);
    ASSERT_ARE_EQUAL(int, (int)result, (int)TLSIO_OPTIONS_RESULT_SUCCESS);

    ///clean
    free(out_result);
}

TEST_FUNCTION(tlsio_options__clone_option_OPTION_X509_ECC_CERT__succeeds)
{
    ///arrange
    void* out_result;
    TLSIO_OPTIONS_RESULT result;

    ///act
    result = tlsio_options_clone_option(OPTION_X509_ECC_CERT, fake_trusted_cert, &out_result);

    ///assert
    ASSERT_COPIED_STRING((const char*)out_result, fake_trusted_cert);
    ASSERT_ARE_EQUAL(int, (int)result, (int)TLSIO_OPTIONS_RESULT_SUCCESS);

    ///clean
    free(out_result);
}

TEST_FUNCTION(tlsio_options__clone_option_OPTION_X509_ECC_KEY__succeeds)
{
    ///arrange
    void* out_result;
    TLSIO_OPTIONS_RESULT result;

    ///act
    result = tlsio_options_clone_option(OPTION_X509_ECC_KEY, fake_trusted_cert, &out_result);

    ///assert
    ASSERT_COPIED_STRING((const char*)out_result, fake_trusted_cert);
    ASSERT_ARE_EQUAL(int, (int)result, (int)TLSIO_OPTIONS_RESULT_SUCCESS);

    ///clean
    free(out_result);
}

TEST_FUNCTION(tlsio_options__clone_option_malloc_fail__fails)
{
    int i;
    int k = 0;
    const char* p0[SET_NOT_SUPPORTED_COUNT];

    p0[k] = OPTION_TRUSTED_CERT; /*  */ k++;
    p0[k] = SU_OPTION_X509_CERT; /*  */ k++;
    p0[k] = SU_OPTION_X509_PRIVATE_KEY; k++;
    p0[k] = OPTION_X509_ECC_CERT; /* */ k++;
    p0[k] = OPTION_X509_ECC_KEY; /*  */ k++;
    use_negative_mocks();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));  // concrete_io struct
    umock_c_negative_tests_snapshot();

    // Cycle through each failing combo of parameters
    for (i = 0; i < SET_NOT_SUPPORTED_COUNT; i++)
    {
        void* out_result = NULL;
        TLSIO_OPTIONS_RESULT result;
        ///arrange
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(0);


        ///act
        result = tlsio_options_clone_option(p0[i], fake_trusted_cert, &out_result);

        ///assert
        ASSERT_IS_NULL(out_result);
        ASSERT_ARE_EQUAL(int, (int)result, (int)TLSIO_OPTIONS_RESULT_ERROR, "Unexpected success with malloc failure");

        ///clean
    }
    umock_c_negative_tests_deinit();
}

TEST_FUNCTION(tlsio_options__clone_parameter_validation__fails)
{
    int i;
    int k = 0;
    const char* p0[SET_PV_COUNT];
    const char* p1[SET_PV_COUNT];
    void** p2[SET_PV_COUNT];
    const char* fm[SET_PV_COUNT];

    TLSIO_OPTIONS_RESULT result;
    void* out_result = NULL;

    p0[k] = NULL; /*          */ p1[k] = fake_x509_key; p2[k] = &out_result; fm[k] = "Unexpected clone_option success when name is NULL"; /* */  k++;
    p0[k] = OPTION_TRUSTED_CERT; p1[k] = NULL; /*    */ p2[k] = &out_result; fm[k] = "Unexpected clone_option success when option value is NULL"; k++;
    p0[k] = OPTION_TRUSTED_CERT; p1[k] = fake_x509_key; p2[k] = NULL; /*  */ fm[k] = "Unexpected clone_option success when out_status is NULL"; k++;

    // Cycle through each failing combo of parameters
    for (i = 0; i < SET_PV_COUNT; i++)
    {
        ///arrange

        ///act
        result = tlsio_options_clone_option(p0[i], p1[i], p2[i]);

        ///assert
        ASSERT_IS_NULL(out_result);
        ASSERT_ARE_EQUAL(int, (int)result, (int)TLSIO_OPTIONS_RESULT_ERROR, "Unexpected success with bad clone parameter");

        ///clean
    }
}

TEST_FUNCTION(tlsio_options__clone_malloc_fail__fails)
{
    int i;
    int k = 0;
    const char* p0[SET_NOT_SUPPORTED_COUNT];

    void* out_result = NULL;
    TLSIO_OPTIONS_RESULT result;

    p0[k] = OPTION_TRUSTED_CERT; /*  */ k++;
    p0[k] = SU_OPTION_X509_CERT; /*  */ k++;
    p0[k] = SU_OPTION_X509_PRIVATE_KEY; k++;
    p0[k] = OPTION_X509_ECC_CERT; /* */ k++;
    p0[k] = OPTION_X509_ECC_KEY; /*  */ k++;
    use_negative_mocks();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));  // concrete_io struct
    umock_c_negative_tests_snapshot();

    // Cycle through each failing combo of parameters
    for (i = 0; i < SET_NOT_SUPPORTED_COUNT; i++)
    {
        ///arrange
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(0);

        ///act
        result = tlsio_options_clone_option(p0[i], fake_x509_key, &out_result);

        ///assert
        ASSERT_IS_NULL(out_result);
        ASSERT_ARE_EQUAL(int, (int)result, (int)TLSIO_OPTIONS_RESULT_ERROR, "Unexpected success with malloc failure");

        ///clean
    }
    umock_c_negative_tests_deinit();
}

TEST_FUNCTION(tlsio_options__retrieve_ex_OPTION_TRUSTED_CERT__succeeds)
{
    ///arrange
    TLSIO_OPTIONS options;
    TLSIO_OPTIONS_RESULT set_result;
    OPTIONHANDLER_RESULT option_handler_result;
    OPTIONHANDLER_HANDLE result = NULL;
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT | TLSIO_OPTION_BIT_x509_ECC_CERT);
    set_result = tlsio_options_set(&options, OPTION_TRUSTED_CERT, fake_trusted_cert);
    ASSERT_ARE_EQUAL(int, (int)set_result, (int)TLSIO_OPTIONS_RESULT_SUCCESS);

    ///act
    result = tlsio_options_retrieve_options_ex(&options, pfCloneOption_impl, pfDestroyOption_impl, pfSetOption_impl);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    tlsio_options_release_resources(&options);
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT | TLSIO_OPTION_BIT_x509_ECC_CERT);

    option_handler_result = OptionHandler_FeedOptions(result, &options);
    ASSERT_ARE_EQUAL(int, (int)option_handler_result, (int)OPTIONHANDLER_OK);

    ASSERT_COPIED_STRING(options.trusted_certs, fake_trusted_cert);
    ASSERT_ARE_EQUAL(int, (int)options.x509_type, (int)TLSIO_OPTIONS_x509_TYPE_UNSPECIFIED);

    ///clean
    tlsio_options_release_resources(&options);
    OptionHandler_Destroy(result);
    assert_gballoc_checks();
}

TEST_FUNCTION(tlsio_options__retrieve_ex_SU_OPTION_X509_CERT__succeeds)
{
    ///arrange
    TLSIO_OPTIONS options;
    TLSIO_OPTIONS_RESULT set_result;
    OPTIONHANDLER_RESULT option_handler_result;
    OPTIONHANDLER_HANDLE result = NULL;
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT | TLSIO_OPTION_BIT_x509_ECC_CERT);
    set_result = tlsio_options_set(&options, SU_OPTION_X509_CERT, fake_trusted_cert);
    ASSERT_ARE_EQUAL(int, (int)set_result, (int)TLSIO_OPTIONS_RESULT_SUCCESS);

    ///act
    result = tlsio_options_retrieve_options_ex(&options, pfCloneOption_impl, pfDestroyOption_impl, pfSetOption_impl);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    tlsio_options_release_resources(&options);
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT | TLSIO_OPTION_BIT_x509_ECC_CERT);

    option_handler_result = OptionHandler_FeedOptions(result, &options);
    ASSERT_ARE_EQUAL(int, (int)option_handler_result, (int)OPTIONHANDLER_OK);

    ASSERT_COPIED_STRING(options.x509_cert, fake_trusted_cert);
    ASSERT_ARE_EQUAL(int, (int)options.x509_type, (int)TLSIO_OPTIONS_x509_TYPE_RSA);

    ///clean
    tlsio_options_release_resources(&options);
    OptionHandler_Destroy(result);
    assert_gballoc_checks();
}

TEST_FUNCTION(tlsio_options__retrieve_ex_SU_OPTION_X509_PRIVATE_KEY__succeeds)
{
    ///arrange
    TLSIO_OPTIONS options;
    TLSIO_OPTIONS_RESULT set_result;
    OPTIONHANDLER_RESULT option_handler_result;
    OPTIONHANDLER_HANDLE result = NULL;
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT | TLSIO_OPTION_BIT_x509_ECC_CERT);
    set_result = tlsio_options_set(&options, SU_OPTION_X509_PRIVATE_KEY, fake_trusted_cert);
    ASSERT_ARE_EQUAL(int, (int)set_result, (int)TLSIO_OPTIONS_RESULT_SUCCESS);

    ///act
    result = tlsio_options_retrieve_options_ex(&options, pfCloneOption_impl, pfDestroyOption_impl, pfSetOption_impl);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    tlsio_options_release_resources(&options);
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT | TLSIO_OPTION_BIT_x509_ECC_CERT);

    option_handler_result = OptionHandler_FeedOptions(result, &options);
    ASSERT_ARE_EQUAL(int, (int)option_handler_result, (int)OPTIONHANDLER_OK);

    ASSERT_COPIED_STRING(options.x509_key, fake_trusted_cert);
    ASSERT_ARE_EQUAL(int, (int)options.x509_type, (int)TLSIO_OPTIONS_x509_TYPE_RSA);

    ///clean
    tlsio_options_release_resources(&options);
    OptionHandler_Destroy(result);
    assert_gballoc_checks();
}

TEST_FUNCTION(tlsio_options__retrieve_ex_OPTION_X509_ECC_CERT__succeeds)
{
    ///arrange
    TLSIO_OPTIONS options;
    TLSIO_OPTIONS_RESULT set_result;
    OPTIONHANDLER_RESULT option_handler_result;
    OPTIONHANDLER_HANDLE result = NULL;
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT | TLSIO_OPTION_BIT_x509_ECC_CERT);
    set_result = tlsio_options_set(&options, OPTION_X509_ECC_CERT, fake_trusted_cert);
    ASSERT_ARE_EQUAL(int, (int)set_result, (int)TLSIO_OPTIONS_RESULT_SUCCESS);

    ///act
    result = tlsio_options_retrieve_options_ex(&options, pfCloneOption_impl, pfDestroyOption_impl, pfSetOption_impl);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    tlsio_options_release_resources(&options);
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT | TLSIO_OPTION_BIT_x509_ECC_CERT);

    option_handler_result = OptionHandler_FeedOptions(result, &options);
    ASSERT_ARE_EQUAL(int, (int)option_handler_result, (int)OPTIONHANDLER_OK);

    ASSERT_COPIED_STRING(options.x509_cert, fake_trusted_cert);
    ASSERT_ARE_EQUAL(int, (int)options.x509_type, (int)TLSIO_OPTIONS_x509_TYPE_ECC);

    ///clean
    tlsio_options_release_resources(&options);
    OptionHandler_Destroy(result);
    assert_gballoc_checks();
}

TEST_FUNCTION(tlsio_options__retrieve_ex_OPTION_X509_ECC_KEY__succeeds)
{
    ///arrange
    TLSIO_OPTIONS options;
    TLSIO_OPTIONS_RESULT set_result;
    OPTIONHANDLER_RESULT option_handler_result;
    OPTIONHANDLER_HANDLE result = NULL;
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT | TLSIO_OPTION_BIT_x509_ECC_CERT);
    set_result = tlsio_options_set(&options, OPTION_X509_ECC_KEY, fake_trusted_cert);
    ASSERT_ARE_EQUAL(int, (int)set_result, (int)TLSIO_OPTIONS_RESULT_SUCCESS);

    ///act
    result = tlsio_options_retrieve_options_ex(&options, pfCloneOption_impl, pfDestroyOption_impl, pfSetOption_impl);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    tlsio_options_release_resources(&options);
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT | TLSIO_OPTION_BIT_x509_ECC_CERT);

    option_handler_result = OptionHandler_FeedOptions(result, &options);
    ASSERT_ARE_EQUAL(int, (int)option_handler_result, (int)OPTIONHANDLER_OK);

    ASSERT_COPIED_STRING(options.x509_key, fake_trusted_cert);
    ASSERT_ARE_EQUAL(int, (int)options.x509_type, (int)TLSIO_OPTIONS_x509_TYPE_ECC);

    ///clean
    tlsio_options_release_resources(&options);
    OptionHandler_Destroy(result);
    assert_gballoc_checks();
}

TEST_FUNCTION(tlsio_options__retrieve_ex_parameter_validation__fails)
{
    int i;
    int k = 0;
    TLSIO_OPTIONS options;
    TLSIO_OPTIONS* p0[RETRIEVE_PV_COUNT];
    pfCloneOption p1[RETRIEVE_PV_COUNT];
    pfDestroyOption p2[RETRIEVE_PV_COUNT];
    pfSetOption p3[RETRIEVE_PV_COUNT];

    p0[k] = NULL;     p1[k] = pfCloneOption_impl; p2[k] = pfDestroyOption_impl; p3[k] = pfSetOption_impl; k++;
    p0[k] = &options; p1[k] = NULL; /*         */ p2[k] = pfDestroyOption_impl; p3[k] = pfSetOption_impl; k++;
    p0[k] = &options; p1[k] = pfCloneOption_impl; p2[k] = NULL; /*           */ p3[k] = pfSetOption_impl; k++;
    p0[k] = &options; p1[k] = pfCloneOption_impl; p2[k] = pfDestroyOption_impl; p3[k] = NULL; k++;

    // Cycle through each failing combo of parameters
    for (i = 0; i < RETRIEVE_PV_COUNT; i++)
    {
        ///arrange
        TLSIO_OPTIONS_RESULT set_result;
        OPTIONHANDLER_HANDLE result = NULL;
        tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT | TLSIO_OPTION_BIT_x509_ECC_CERT);
        set_result = tlsio_options_set(&options, OPTION_X509_ECC_KEY, fake_trusted_cert);
        ASSERT_ARE_EQUAL(int, (int)set_result, (int)TLSIO_OPTIONS_RESULT_SUCCESS);

        ///act
        result = tlsio_options_retrieve_options_ex(p0[i], p1[i], p2[i], p3[i]);

        ///assert
        ASSERT_IS_NULL(result, "Unexpected success with bad retrieve parameter");

        ///clean
        tlsio_options_release_resources(&options);
    }
}

TEST_FUNCTION(tlsio_options__retrieve_ex_OptionHandler_Create_fail__fails)
{
    int i;
    int k = 0;
    const char* p0[SET_NOT_SUPPORTED_COUNT];

    TLSIO_OPTIONS options;
    TLSIO_OPTIONS_RESULT set_result;
    OPTIONHANDLER_HANDLE result;

    p0[k] = OPTION_TRUSTED_CERT; /*  */ k++;
    p0[k] = SU_OPTION_X509_CERT; /*  */ k++;
    p0[k] = SU_OPTION_X509_PRIVATE_KEY; k++;
    p0[k] = OPTION_X509_ECC_CERT; /* */ k++;
    p0[k] = OPTION_X509_ECC_KEY; /*  */ k++;
    use_negative_mocks();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    umock_c_negative_tests_snapshot();

    // Cycle through each option
    for (i = 0; i < SET_NOT_SUPPORTED_COUNT; i++)
    {
        ///arrange
        tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT | TLSIO_OPTION_BIT_x509_ECC_CERT);
        set_result = tlsio_options_set(&options, p0[i], fake_x509_key);
        ASSERT_ARE_EQUAL(int, (int)set_result, (int)TLSIO_OPTIONS_RESULT_SUCCESS);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(0);

        ///act
        result = tlsio_options_retrieve_options_ex(&options, pfCloneOption_impl, pfDestroyOption_impl, pfSetOption_impl);

        ///assert
        ASSERT_IS_NULL(result, "Unexpected success with OptionHandler_Create failure");

        ///clean
        tlsio_options_release_resources(&options);
        assert_gballoc_checks();
    }
    umock_c_negative_tests_deinit();
}

TEST_FUNCTION(tlsio_options__retrieve_ex_OptionHandler_AddOption_fail__fails)
{
    int i;
    int k = 0;
    const char* p0[SET_NOT_SUPPORTED_COUNT];

    TLSIO_OPTIONS options;
    TLSIO_OPTIONS_RESULT set_result;
    OPTIONHANDLER_HANDLE result;

    p0[k] = OPTION_TRUSTED_CERT; /*  */ k++;
    p0[k] = SU_OPTION_X509_CERT; /*  */ k++;
    p0[k] = SU_OPTION_X509_PRIVATE_KEY; k++;
    p0[k] = OPTION_X509_ECC_CERT; /* */ k++;
    p0[k] = OPTION_X509_ECC_KEY; /*  */ k++;
    use_negative_mocks();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    umock_c_negative_tests_snapshot();

    // Cycle through each option
    for (i = 0; i < SET_NOT_SUPPORTED_COUNT; i++)
    {
        ///arrange
        tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT | TLSIO_OPTION_BIT_x509_ECC_CERT);
        set_result = tlsio_options_set(&options, p0[i], fake_x509_key);
        ASSERT_ARE_EQUAL(int, (int)set_result, (int)TLSIO_OPTIONS_RESULT_SUCCESS);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(2);

        ///act
        result = tlsio_options_retrieve_options_ex(&options, pfCloneOption_impl, pfDestroyOption_impl, pfSetOption_impl);

        ///assert
        ASSERT_IS_NULL(result, "Unexpected success with OptionHandler_Create failure");

        ///clean
        tlsio_options_release_resources(&options);
        assert_gballoc_checks();
    }
    umock_c_negative_tests_deinit();
}

// The tlsio_options_retrieve_options is a simple pass-through to tlsio_options_retrieve_options_ex,
// so a single success case is sufficient for validation
TEST_FUNCTION(tlsio_options__retrieve__succeeds)
{
    ///arrange
    TLSIO_OPTIONS options;
    TLSIO_OPTIONS_RESULT set_result;
    OPTIONHANDLER_RESULT option_handler_result;
    OPTIONHANDLER_HANDLE result = NULL;
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT | TLSIO_OPTION_BIT_x509_ECC_CERT);
    set_result = tlsio_options_set(&options, OPTION_X509_ECC_KEY, fake_trusted_cert);
    ASSERT_ARE_EQUAL(int, (int)set_result, (int)TLSIO_OPTIONS_RESULT_SUCCESS);

    ///act
    result = tlsio_options_retrieve_options(&options, pfSetOption_impl);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    tlsio_options_release_resources(&options);
    tlsio_options_initialize(&options, TLSIO_OPTION_BIT_TRUSTED_CERTS | TLSIO_OPTION_BIT_x509_RSA_CERT | TLSIO_OPTION_BIT_x509_ECC_CERT);

    option_handler_result = OptionHandler_FeedOptions(result, &options);
    ASSERT_ARE_EQUAL(int, (int)option_handler_result, (int)OPTIONHANDLER_OK);

    ASSERT_COPIED_STRING(options.x509_key, fake_trusted_cert);
    ASSERT_ARE_EQUAL(int, (int)options.x509_type, (int)TLSIO_OPTIONS_x509_TYPE_ECC);

    ///clean
    tlsio_options_release_resources(&options);
    OptionHandler_Destroy(result);
    assert_gballoc_checks();
}

END_TEST_SUITE(tlsio_options_unittests)
