// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#ifdef _MSC_VER
/* CycloneSSL has a pretty big list of warnings which need to be ignored */
#pragma warning(disable: 4200 4201 4244 4100 4267 4701 4703 4389 4005 4996)
#endif

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_stdint.h"
#include "umock_c/umock_c_negative_tests.h"

/* tls and yarrow need extern "C" in case of C++ compilation */
#ifdef __cplusplus
extern "C"
{
#endif
#include "tls.h"
#include "yarrow.h"
#ifdef __cplusplus
}
#endif

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

#ifdef __cplusplus
extern "C"
{
#endif
    void* my_gballoc_malloc(size_t size)
    {
        return malloc(size);
    }

    void my_gballoc_free(void* ptr)
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
#include "azure_c_shared_utility/tlsio_cyclonessl_socket.h"
#include "azure_c_shared_utility/xio.h"

#define TlsConnectionEnd_VALUES \
    TLS_CONNECTION_END_CLIENT,  \
    TLS_CONNECTION_END_SERVER

TEST_DEFINE_ENUM_TYPE(TlsConnectionEnd, TlsConnectionEnd_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(TlsConnectionEnd, TlsConnectionEnd_VALUES);
TEST_DEFINE_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);
TEST_DEFINE_ENUM_TYPE(IO_SEND_RESULT, IO_SEND_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_SEND_RESULT, IO_SEND_RESULT_VALUES);

#define TEST_TLS_CONTEXT    (TlsContext*)0x4242
#define TEST_TLS_SOCKET     (TlsSocket)0x4243
#define TEST_OPTION_HANDLER (OPTIONHANDLER_HANDLE)0x4244

#ifdef _MSC_VER
#pragma warning(disable:4189)
#endif

MOCK_FUNCTION_WITH_CODE(, TlsContext*, tlsInit)
    TlsContext* my_result = TEST_TLS_CONTEXT;
MOCK_FUNCTION_END(my_result);
MOCK_FUNCTION_WITH_CODE(, void, tlsFree, TlsContext*, context)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, error_t, tlsSetSocket, TlsContext*, context, TlsSocket, socket)
MOCK_FUNCTION_END(NO_ERROR);
MOCK_FUNCTION_WITH_CODE(, error_t, tlsSetConnectionEnd, TlsContext*, context, TlsConnectionEnd, entity)
MOCK_FUNCTION_END(NO_ERROR);
MOCK_FUNCTION_WITH_CODE(, error_t, tlsSetPrng, TlsContext*, context, const PrngAlgo*, prngAlgo, void*, prngContext);
MOCK_FUNCTION_END(NO_ERROR);
MOCK_FUNCTION_WITH_CODE(, error_t, tlsSetTrustedCaList, TlsContext*, context, const char_t*, trustedCaList, size_t, length);
MOCK_FUNCTION_END(NO_ERROR);
MOCK_FUNCTION_WITH_CODE(, error_t, tlsConnect, TlsContext*, context);
MOCK_FUNCTION_END(NO_ERROR);
MOCK_FUNCTION_WITH_CODE(, error_t, tlsWrite, TlsContext*, context, const void*, data, size_t, length, uint_t, flags);
MOCK_FUNCTION_END(NO_ERROR);
MOCK_FUNCTION_WITH_CODE(, error_t, tlsRead, TlsContext*, context, void*, data, size_t, size, size_t*, received, uint_t, flags);
MOCK_FUNCTION_END(NO_ERROR);
MOCK_FUNCTION_WITH_CODE(, error_t, tlsShutdown, TlsContext*, context);
MOCK_FUNCTION_END(NO_ERROR);

MOCK_FUNCTION_WITH_CODE(, error_t, yarrowInit, YarrowContext*, context)
MOCK_FUNCTION_END(NO_ERROR);
MOCK_FUNCTION_WITH_CODE(, void, yarrowRelease, YarrowContext*, context)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, error_t, yarrowSeed, YarrowContext*, context, const uint8_t*, input, size_t, length);
MOCK_FUNCTION_END(NO_ERROR);
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

const PrngAlgo yarrowPrngAlgo = { 0 };

#undef ENABLE_MOCKS

#include "azure_c_shared_utility/tlsio_cyclonessl.h"
#include "azure_c_shared_utility/tlsio.h"

#ifdef __cplusplus
extern "C"
{
#endif

pfCloneOption tlsio_cyclonessl_clone_option;
pfDestroyOption tlsio_cyclonessl_destroy_option;

OPTIONHANDLER_HANDLE my_OptionHandler_Create(pfCloneOption cloneOption, pfDestroyOption destroyOption, pfSetOption setOption)
{
    (void)setOption;

    tlsio_cyclonessl_clone_option = cloneOption;
    tlsio_cyclonessl_destroy_option = destroyOption;
    return TEST_OPTION_HANDLER;
}

#ifdef __cplusplus
}
#endif

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(tlsio_cyclonessl_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    result = umock_c_init(on_umock_c_error);
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_s, real_mallocAndStrcpy_s);
    REGISTER_GLOBAL_MOCK_HOOK(OptionHandler_Create, my_OptionHandler_Create);
    REGISTER_GLOBAL_MOCK_RETURN(tlsInit, TEST_TLS_CONTEXT);
    REGISTER_GLOBAL_MOCK_RETURN(OptionHandler_Create, TEST_OPTION_HANDLER);
    REGISTER_TYPE(TlsConnectionEnd, TlsConnectionEnd);
    REGISTER_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT);
    REGISTER_TYPE(IO_SEND_RESULT, IO_SEND_RESULT);
    REGISTER_UMOCK_ALIAS_TYPE(error_t, int);
    REGISTER_UMOCK_ALIAS_TYPE(uint_t, unsigned int);
    REGISTER_UMOCK_ALIAS_TYPE(TlsSocket, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfCloneOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfDestroyOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfSetOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(OPTIONHANDLER_HANDLE, void*);
    REGISTER_UMOCKC_PAIRED_CREATE_DESTROY_CALLS(tlsInit, tlsFree);
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
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* tlsio_cyclonessl_create */

/* Tests_SRS_TLSIO_CYCLONESSL_01_001: [ tlsio_cyclonessl_create shall create a new instance of the tlsio for Cyclone SSL. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_005: [ tlsio_cyclonessl_create shall copy the hostname and port values for later use when the openof the underlying socket is needed. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_006: [ hostname shall be copied by calling mallocAndStrcpy_s. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_008: [ tlsio_cyclonessl_create shall initialize the yarrow context by calling yarrowInit. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_010: [ The yarrow context shall be seeded with 32 bytes of randomly chosen data by calling yarrowSeed. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_012: [ tlsio_cyclonessl_create shall create a TLS context by calling tlsInit. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_014: [ The TLS context shall be setup to operate as a client by calling tlsSetConnectionEnd with TLS_CONNECTION_END_CLIENT. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_016: [ The pseudo random number generator to be used shall be set by calling tlsSetPrng with the yarrow instance as argument. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_003: [ io_create_parameters shall be used as a TLSIO_CONFIG\*. ]*/
TEST_FUNCTION(tlsio_cyclonessl_create_succeeds)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test"))
        .IgnoreArgument_destination();
    EXPECTED_CALL(yarrowInit(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(yarrowSeed(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 32))
        .IgnoreArgument_context().IgnoreArgument_input();
    STRICT_EXPECTED_CALL(tlsInit());
    STRICT_EXPECTED_CALL(tlsSetConnectionEnd(TEST_TLS_CONTEXT, TLS_CONNECTION_END_CLIENT));
    STRICT_EXPECTED_CALL(tlsSetPrng(TEST_TLS_CONTEXT, YARROW_PRNG_ALGO, IGNORED_PTR_ARG))
        .IgnoreArgument_prngContext();

    ///act
    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(tlsio_handle);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_002: [ If io_create_parameters is NULL, tlsio_cyclonessl_create shall fail and return NULL. ]*/
TEST_FUNCTION(tlsio_cyclonessl_with_NULL_argument_fails)
{
    ///arrange

    ///act
    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_003: [ io_create_parameters shall be used as a TLSIO_CONFIG\*. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_004: [ If the hostname member is NULL, then tlsio_cyclonessl_create shall fail and return NULL. ]*/
TEST_FUNCTION(tlsio_cyclonessl_create_with_NULL_hostname_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = NULL;
    tlsio_config.port = 4242;

    ///act
    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_076: [ If allocating memory for the TLS IO instance fails then tlsio_cyclonessl_create shall fail and return NULL. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_007: [ If mallocAndStrcpy_s fails then tlsio_cyclonessl_create shall fail and return NULL. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_009: [ If yarrowInit fails then tlsio_cyclonessl_create shall return NULL. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_011: [ If yarrowSeed fails then tlsio_cyclonessl_create shall return NULL. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_013: [ If tlsInit fails then tlsio_cyclonessl_create shall return NULL. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_015: [ If tlsSetConnectionEnd fails then tlsio_cyclonessl_create shall return NULL. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_017: [ If tlsSetPrng fails then tlsio_cyclonessl_create shall return NULL. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_018: [ When tlsio_cyclonessl_create fails, all allocated resources up to that point shall be freed. ]*/
TEST_FUNCTION(when_a_failure_occurs_for_tlsio_cyclonessl_create_then_create_fails)
{
    ///arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "test"))
        .IgnoreArgument_destination().SetFailReturn(1);
    EXPECTED_CALL(yarrowInit(IGNORED_PTR_ARG)).SetFailReturn(ERROR_INVALID_PARAMETER);
    STRICT_EXPECTED_CALL(yarrowSeed(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 32))
        .IgnoreArgument_context().IgnoreArgument_input().SetFailReturn(ERROR_INVALID_PARAMETER);
    STRICT_EXPECTED_CALL(tlsInit()).SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(tlsSetConnectionEnd(TEST_TLS_CONTEXT, TLS_CONNECTION_END_CLIENT)).SetFailReturn(ERROR_INVALID_PARAMETER);
    STRICT_EXPECTED_CALL(tlsSetPrng(TEST_TLS_CONTEXT, YARROW_PRNG_ALGO, IGNORED_PTR_ARG))
        .IgnoreArgument_prngContext().SetFailReturn(ERROR_INVALID_PARAMETER);

    umock_c_negative_tests_snapshot();

    for (size_t i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        TLSIO_CONFIG tlsio_config;
        tlsio_config.hostname = "test";
        tlsio_config.port = 4242;

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        char temp_str[128];
        (void)sprintf(temp_str, "On failed call %lu", (unsigned long)i);

        ///act
        CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);

        ///assert
        ASSERT_IS_NULL(tlsio_handle, temp_str);
    }

    ///cleanup
    umock_c_negative_tests_deinit();
}

/* tlsio_cyclonessl_destroy */

/* Tests_SRS_TLSIO_CYCLONESSL_01_019: [ tlsio_cyclonessl_destroy shall free the tlsio CycloneSSL instance. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_021: [ tlsio_cyclonessl_destroy shall deinitialize the yarrow context by calling yarrowRelease. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_022: [ The TLS context shall be freed by calling tlsFree. ]*/
TEST_FUNCTION(tlsio_cyclonessl_destroy_frees_the_resources_allocated_by_create)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    umock_c_reset_all_calls();

    EXPECTED_CALL(tlsFree(TEST_TLS_CONTEXT));
    EXPECTED_CALL(yarrowRelease(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    ///act
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_020: [ If tls_io is NULL, tlsio_cyclonessl_destroy shall do nothing. ]*/
TEST_FUNCTION(tlsio_cyclonessl_destroy_with_NULL_does_not_free_any_resources)
{
    ///arrange

    ///act
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_077: [ All options cached via tlsio_cyclonessl_set_option shall also be freed. ]*/
TEST_FUNCTION(when_TrustedCerts_was_set_tlsio_cyclonessl_destroy_shall_free_it)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_setoption(tlsio_handle, "TrustedCerts", "x");
    umock_c_reset_all_calls();

    EXPECTED_CALL(tlsFree(TEST_TLS_CONTEXT));
    EXPECTED_CALL(yarrowRelease(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    ///act
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_081: [ tlsio_cyclonessl_destroy should close the IO if it was open before freeing all the resources. ]*/
TEST_FUNCTION(when_IO_is_open_tlsio_cyclonessl_destroy_also_closes_it)
{
    ///arrange
    TlsSocket tls_socket = TEST_TLS_SOCKET;
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(tlsio_cyclonessl_socket_create("test", 4242, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_new_socket(&tls_socket, sizeof(tls_socket));
    STRICT_EXPECTED_CALL(tlsSetSocket(TEST_TLS_CONTEXT, TEST_TLS_SOCKET));
    STRICT_EXPECTED_CALL(tlsConnect(TEST_TLS_CONTEXT));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_OK));

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(tlsShutdown(TEST_TLS_CONTEXT));
    STRICT_EXPECTED_CALL(tlsio_cyclonessl_socket_destroy(TEST_TLS_SOCKET));

    EXPECTED_CALL(tlsFree(TEST_TLS_CONTEXT));
    EXPECTED_CALL(yarrowRelease(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    ///act
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* tlsio_cyclonessl_open */

/* Tests_SRS_TLSIO_CYCLONESSL_01_023: [ tlsio_cyclonessl_open shall open the TLS io and on success it shall return 0. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_025: [ tlsio_cyclonessl_open shall create a socket by calling tlsio_cyclonessl_socket_create, while passing to it the hostname and port that were saved in the tlsio_cyclonessl_create. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_027: [ The socket created by tlsio_cyclonessl_socket_create shall be assigned to the TLS context by calling tlsSetSocket. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_031: [ tlsio_cyclonessl_open shall start the TLS handshake by calling tlsConnect. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_033: [ If tlsConnect succeeds, the callback on_io_open_complete shall be called, while passing on_io_open_complete_context and IO_OPEN_OK as arguments. ]*/
TEST_FUNCTION(tlsio_cyclonessl_open_succeeds)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;
    TlsSocket tls_socket = TEST_TLS_SOCKET;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(tlsio_cyclonessl_socket_create("test", 4242, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_new_socket(&tls_socket, sizeof(tls_socket));
    STRICT_EXPECTED_CALL(tlsSetSocket(TEST_TLS_CONTEXT, TEST_TLS_SOCKET));
    STRICT_EXPECTED_CALL(tlsConnect(TEST_TLS_CONTEXT));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_OK));

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_078: [ If certificates have been set by using tlsio_cyclonessl_set_option then a call to tlsSetTrustedCaList shall be made to pass the certificates to CycloneSSL. ]*/
TEST_FUNCTION(tlsio_cyclonessl_open_passes_certs_to_CycloneSSL)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;
    TlsSocket tls_socket = TEST_TLS_SOCKET;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_setoption(tlsio_handle, "TrustedCerts", "my_certs");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(tlsio_cyclonessl_socket_create("test", 4242, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_new_socket(&tls_socket, sizeof(tls_socket));
    STRICT_EXPECTED_CALL(tlsSetSocket(TEST_TLS_CONTEXT, TEST_TLS_SOCKET));
    STRICT_EXPECTED_CALL(tlsSetTrustedCaList(TEST_TLS_CONTEXT, "my_certs", 8))
        .ValidateArgumentBuffer(2, "my_certs", 8)
        .SetFailReturn(ERROR_INVALID_PARAMETER);
    STRICT_EXPECTED_CALL(tlsConnect(TEST_TLS_CONTEXT));
    STRICT_EXPECTED_CALL(test_on_io_open_complete((void*)0x4242, IO_OPEN_OK));

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_024: [ If any of the arguments tls_io, on_io_open_complete, on_bytes_received or on_io_error are NULL then tlsio_cyclonessl_open shall return a non-zero value. ]*/
TEST_FUNCTION(tlsio_cyclonessl_open_with_NULL_handle_fails)
{
    ///arrange

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_open(NULL, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_024: [ If any of the arguments tls_io, on_io_open_complete, on_bytes_received or on_io_error are NULL then tlsio_cyclonessl_open shall return a non-zero value. ]*/
TEST_FUNCTION(tlsio_cyclonessl_open_with_NULL_open_complete_callback_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;
    TlsSocket tls_socket = TEST_TLS_SOCKET;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    umock_c_reset_all_calls();

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, NULL, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_024: [ If any of the arguments tls_io, on_io_open_complete, on_bytes_received or on_io_error are NULL then tlsio_cyclonessl_open shall return a non-zero value. ]*/
TEST_FUNCTION(tlsio_cyclonessl_open_with_NULL_bytes_received_callback_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;
    TlsSocket tls_socket = TEST_TLS_SOCKET;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    umock_c_reset_all_calls();

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, NULL, (void*)0x4243, test_on_io_error, (void*)0x4244);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_024: [ If any of the arguments tls_io, on_io_open_complete, on_bytes_received or on_io_error are NULL then tlsio_cyclonessl_open shall return a non-zero value. ]*/
TEST_FUNCTION(tlsio_cyclonessl_open_with_NULL_io_error_callback_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;
    TlsSocket tls_socket = TEST_TLS_SOCKET;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    umock_c_reset_all_calls();

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, NULL, (void*)0x4244);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_026: [ If tlsio_cyclonessl_socket_create fails, then tlsio_cyclonessl_open shall return a non-zero value. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_028: [ If tlsSetSocket fails then tlsio_cyclonessl_open shall return a non-zero value. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_032: [ If tlsConnect fails then tlsio_cyclonessl_open shall return a non-zero value. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_079: [ If tlsSetTrustedCaList fails then tlsio_cyclonessl_open shall return a non-zero value. ]*/
TEST_FUNCTION(when_a_failure_occurs_for_tlsio_cyclonessl_open_then_create_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;
    TlsSocket tls_socket = TEST_TLS_SOCKET;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_setoption(tlsio_handle, "TrustedCerts", "certs");
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(tlsio_cyclonessl_socket_create("test", 4242, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_new_socket(&tls_socket, sizeof(tls_socket))
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(tlsSetSocket(TEST_TLS_CONTEXT, TEST_TLS_SOCKET))
        .SetFailReturn(ERROR_INVALID_PARAMETER);
    STRICT_EXPECTED_CALL(tlsSetTrustedCaList(TEST_TLS_CONTEXT, "certs", 5))
        .ValidateArgumentBuffer(2, "certs", 5)
        .SetFailReturn(ERROR_INVALID_PARAMETER);
    STRICT_EXPECTED_CALL(tlsConnect(TEST_TLS_CONTEXT))
        .SetFailReturn(ERROR_INVALID_PARAMETER);

    umock_c_negative_tests_snapshot();

    for (size_t i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        char temp_str[128];
        (void)sprintf(temp_str, "On failed call %lu", (unsigned long)i);

        ///act
        int result = tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, NULL, (void*)0x4244);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result, temp_str);
    }

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
    umock_c_negative_tests_deinit();
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_034: [ If tlsio_cyclonessl_open is called while the IO is open, tlsio_cyclonessl_open shall fail and return a non-zero value without performing any work to open the IO. ]*/
TEST_FUNCTION(tlsio_cyclonessl_open_after_open_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_034: [ If tlsio_cyclonessl_open is called while the IO is open, tlsio_cyclonessl_open shall fail and return a non-zero value without performing any work to open the IO. ]*/
TEST_FUNCTION(tlsio_cyclonessl_open_after_io_is_in_error_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(tlsRead(TEST_TLS_CONTEXT, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG, 0))
        .IgnoreArgument_data().IgnoreArgument_size().IgnoreArgument_received()
        .SetReturn(ERROR_INVALID_PARAMETER);

    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_dowork(tlsio_handle);
    umock_c_reset_all_calls();

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* tlsio_cyclonessl_close */

/* Tests_SRS_TLSIO_CYCLONESSL_01_035: [ tlsio_cyclonessl_close shall close the TLS IO and on success it shall return 0. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_037: [ tlsio_cyclonessl_close shall close the TLS connection by calling tlsShutdown. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_039: [ tlsio_cyclonessl_destroy shall destroy the underlying socket by calling tlsio_cyclonessl_socket_destroy. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_040: [ On success, on_io_close_complete shall be called while passing as argument on_io_close_complete_context. ]*/
TEST_FUNCTION(tlsio_cyclonessl_close_succeeds)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;
    TlsSocket tls_socket = TEST_TLS_SOCKET;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(tlsio_cyclonessl_socket_create("test", 4242, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_new_socket(&tls_socket, sizeof(tls_socket));

    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(tlsShutdown(TEST_TLS_CONTEXT));
    STRICT_EXPECTED_CALL(tlsio_cyclonessl_socket_destroy(TEST_TLS_SOCKET));
    STRICT_EXPECTED_CALL(test_on_io_close_complete((void*)0x4242));

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_close(tlsio_handle, test_on_io_close_complete, (void*)0x4242);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_036: [ If the argument tls_io is NULL, tlsio_cyclonessl_close shall fail and return a non-zero value. ]*/
TEST_FUNCTION(tlsio_cyclonessl_close_with_NULL_tls_io_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_close(NULL, test_on_io_close_complete, (void*)0x4242);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_038: [ If tlsShutdown fails, tlsio_cyclonessl_close shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_tlsShutdown_fails_tlsio_cyclonessl_close_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(tlsShutdown(TEST_TLS_CONTEXT))
        .SetReturn(ERROR_INVALID_PARAMETER);

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_close(tlsio_handle, test_on_io_close_complete, (void*)0x4242);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_041: [ If tlsio_cyclonessl_close is called when not open, tlsio_cyclonessl_close shall fail and return a non-zero value. ]*/
TEST_FUNCTION(tlsio_cyclonessl_close_when_IO_not_open_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    umock_c_reset_all_calls();

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_close(tlsio_handle, test_on_io_close_complete, (void*)0x4242);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_041: [ If tlsio_cyclonessl_close is called when not open, tlsio_cyclonessl_close shall fail and return a non-zero value. ]*/
TEST_FUNCTION(tlsio_cyclonessl_close_after_close_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_close(tlsio_handle, test_on_io_close_complete, (void*)0x4242);
    umock_c_reset_all_calls();

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_close(tlsio_handle, test_on_io_close_complete, (void*)0x4242);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* tlsio_cyclonessl_send */

/* Tests_SRS_TLSIO_CYCLONESSL_01_042: [ tlsio_cyclonessl_send shall send the size bytes pointed to by buffer and on success it shall return 0. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_046: [ On success, if a non-NULL value was passed for on_send_complete, then on_send_complete shall be called while passing to it the on_send_complete_context value. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_047: [ tlsio_cyclonessl_send shall send the bytes by calling tlsWrite and passing buffer and size as arguments. 0 shall be passed for the flags argument. ]*/
TEST_FUNCTION(tlsio_cyclonessl_send_succeeds)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;
    unsigned char test_buffer[] = { 0x42, 0x43 };

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(tlsWrite(TEST_TLS_CONTEXT, IGNORED_PTR_ARG, sizeof(test_buffer), 0))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer));
    STRICT_EXPECTED_CALL(test_on_send_complete((void*)0x4242, IO_SEND_OK));

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_send(tlsio_handle, test_buffer, sizeof(test_buffer), test_on_send_complete, (void*)0x4242);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_045: [ on_send_complete shall be allowed to be NULL. ]*/
TEST_FUNCTION(tlsio_cyclonessl_send_with_NULL_complete_callback_does_not_trigger_the_callback)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;
    unsigned char test_buffer[] = { 0x42, 0x43 };

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(tlsWrite(TEST_TLS_CONTEXT, IGNORED_PTR_ARG, sizeof(test_buffer), 0))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer));

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_send(tlsio_handle, test_buffer, sizeof(test_buffer), NULL, NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_043: [ If any of the arguments tls_io or buffer is NULL, tlsio_cyclonessl_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(tlsio_cyclonessl_send_with_NULL_tls_io_fails)
{
    ///arrange
    unsigned char test_buffer[] = { 0x42, 0x43 };

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_send(NULL, test_buffer, sizeof(test_buffer), NULL, NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_043: [ If any of the arguments tls_io or buffer is NULL, tlsio_cyclonessl_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(tlsio_cyclonessl_send_with_NULL_buffer_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_send(tlsio_handle, NULL, 1, NULL, NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_044: [ If size is 0, tlsio_cyclonessl_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(tlsio_cyclonessl_send_with_0_size_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;
    unsigned char test_buffer[] = { 0x42, 0x43 };

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_send(tlsio_handle, test_buffer, 0, NULL, NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_048: [ If tlsio_cyclonessl_send is called when the IO is not open, tlsio_cyclonessl_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(tlsio_cyclonessl_send_when_IO_is_closed_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;
    unsigned char test_buffer[] = { 0x42, 0x43 };

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_close(tlsio_handle, NULL, NULL);
    umock_c_reset_all_calls();

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_send(tlsio_handle, test_buffer, 0, NULL, NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_048: [ If tlsio_cyclonessl_send is called when the IO is not open, tlsio_cyclonessl_send shall fail and return a non-zero value. ]*/
TEST_FUNCTION(tlsio_cyclonessl_send_when_IO_is_not_open_yet_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;
    unsigned char test_buffer[] = { 0x42, 0x43 };

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    umock_c_reset_all_calls();

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_send(tlsio_handle, test_buffer, 0, NULL, NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_049: [ If the IO is in an error state (an error was reported through the on_io_error callback), tlsio_cyclonessl_send shall fail and return a non-zero value. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_056: [ Also the IO shall be considered in error and any subsequent calls to tlsio_cyclonessl_send shall fail. ]*/
TEST_FUNCTION(tlsio_cyclonessl_send_when_IO_is_in_error_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;
    unsigned char test_buffer[] = { 0x42, 0x43 };

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(tlsRead(TEST_TLS_CONTEXT, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG, 0))
        .IgnoreArgument_data().IgnoreArgument_size().IgnoreArgument_received()
        .SetReturn(ERROR_INVALID_PARAMETER);

    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_dowork(tlsio_handle);
    umock_c_reset_all_calls();

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_send(tlsio_handle, test_buffer, 0, NULL, NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* tlsio_cyclonessl_dowork */

/* Tests_SRS_TLSIO_CYCLONESSL_01_050: [ tlsio_cyclonessl_dowork shall check if any bytes are available to be read from the CycloneSSL library and indicate those bytes as received. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_054: [ The flags argument for tlsRead shall be 0. ]*/
TEST_FUNCTION(tlsio_cyclonessl_dowork_when_no_bytes_are_available_does_not_trigger_received_callback)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;
    size_t received = 0;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(tlsRead(TEST_TLS_CONTEXT, IGNORED_PTR_ARG, 64, IGNORED_PTR_ARG, 0))
        .IgnoreArgument_data()
        .CopyOutArgumentBuffer_received(&received, sizeof(received));

    ///act
    tlsio_cyclonessl_get_interface_description()->concrete_io_dowork(tlsio_handle);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_080: [ If any bytes are read from CycloneSSL they should be indicated via the on_bytes_received callback passed to tlsio_cyclonessl_open. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_053: [ If the IO is open, tlsio_cyclonessl_dowork shall attempt to read 64 bytes from the TLS library by calling tlsRead. ]*/
TEST_FUNCTION(tlsio_cyclonessl_dowork_when_2_bytes_are_available_they_are_indicated_as_received)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;
    unsigned char test_buffer[] = { 0x42, 0x43 };
    size_t received = sizeof(test_buffer);

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(tlsRead(TEST_TLS_CONTEXT, IGNORED_PTR_ARG, 64, IGNORED_PTR_ARG, 0))
        .CopyOutArgumentBuffer_data(test_buffer, sizeof(test_buffer))
        .CopyOutArgumentBuffer_received(&received, sizeof(received));
    STRICT_EXPECTED_CALL(test_on_bytes_received((void*)0x4243, IGNORED_PTR_ARG, sizeof(test_buffer)))
        .ValidateArgumentBuffer(2, test_buffer, sizeof(test_buffer));

    ///act
    tlsio_cyclonessl_get_interface_description()->concrete_io_dowork(tlsio_handle);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_051: [ If the tls_io argument is NULL, tlsio_cyclonessl_dowork shall do nothing. ]*/
TEST_FUNCTION(tlsio_cyclonessl_dowork_with_NULL_handle_does_nothing)
{
    ///arrange

    ///act
    tlsio_cyclonessl_get_interface_description()->concrete_io_dowork(NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_052: [ If the IO is not open (no open has been called or the IO has been closed) then tlsio_cyclonessl_dowork shall do nothing. ]*/
TEST_FUNCTION(tlsio_cyclonessl_dowork_when_the_IO_is_not_open_does_nothing)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    umock_c_reset_all_calls();

    ///act
    tlsio_cyclonessl_get_interface_description()->concrete_io_dowork(tlsio_handle);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_052: [ If the IO is not open (no open has been called or the IO has been closed) then tlsio_cyclonessl_dowork shall do nothing. ]*/
TEST_FUNCTION(tlsio_cyclonessl_dowork_when_the_IO_is_closed_does_nothing)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_close(tlsio_handle, NULL, NULL);
    umock_c_reset_all_calls();

    ///act
    tlsio_cyclonessl_get_interface_description()->concrete_io_dowork(tlsio_handle);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_055: [ If tlsRead fails, the error shall be indicated by calling the on_io_error callback passed in tlsio_cyclonessl_open, while passing the on_io_error_context to the callback. ]*/
TEST_FUNCTION(when_tlsRead_fails_then_tlsio_cyclonessl_dowork_indicates_an_error)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;
    unsigned char test_buffer[] = { 0x42, 0x43 };
    size_t received = sizeof(test_buffer);

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_open(tlsio_handle, test_on_io_open_complete, (void*)0x4242, test_on_bytes_received, (void*)0x4243, test_on_io_error, (void*)0x4244);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(tlsRead(TEST_TLS_CONTEXT, IGNORED_PTR_ARG, 64, IGNORED_PTR_ARG, 0))
        .IgnoreArgument_data()
        .IgnoreArgument_received()
        .SetReturn(ERROR_INVALID_PARAMETER);
    STRICT_EXPECTED_CALL(test_on_io_error((void*)0x4244));

    ///act
    tlsio_cyclonessl_get_interface_description()->concrete_io_dowork(tlsio_handle);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* tlsio_cyclonessl_setoption */

/* Tests_SRS_TLSIO_CYCLONESSL_01_057: [ If any of the arguments tls_io or option_name is NULL tlsio_cyclonessl_setoption shall return a non-zero value. ]*/
TEST_FUNCTION(tlsio_cyclonessl_setoption_with_NULL_argument_fails)
{
    ///arrange

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_setoption(NULL, "TrustedCerts", "xx");

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_057: [ If any of the arguments tls_io or option_name is NULL tlsio_cyclonessl_setoption shall return a non-zero value. ]*/
TEST_FUNCTION(tlsio_cyclonessl_setoption_with_NULL_option_name_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;
    unsigned char test_buffer[] = { 0x42, 0x43 };
    size_t received = sizeof(test_buffer);

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    umock_c_reset_all_calls();

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_setoption(tlsio_handle, NULL, "xx");

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_058: [ If the option_name argument indicates an option that is not handled by tlsio_cyclonessl, then tlsio_cyclonessl_setoption shall return a non-zero value. ]*/
TEST_FUNCTION(tlsio_cyclonessl_setoption_with_an_unknown_option_name_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;
    unsigned char test_buffer[] = { 0x42, 0x43 };
    size_t received = sizeof(test_buffer);

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    umock_c_reset_all_calls();

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_setoption(tlsio_handle, "nothingIknow", "xx");

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_059: [ If the option was handled by tlsio_cyclonessl, then tlsio_cyclonessl_setoption shall return 0. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_060: [ - "TrustedCerts" - a char\* that shall be saved by tlsio_cyclonessl as it shall be given to the underlying CycloneSSL TLS context when the IO is open. ]*/
TEST_FUNCTION(tlsio_cyclonessl_setoption_with_trustedCerts_clones_the_certs)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "xx"))
        .IgnoreArgument_destination();

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_setoption(tlsio_handle, "TrustedCerts", "xx");

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_061: [ If copying the char\* passed in value fails then tlsio_cyclonessl_setoption shall return a non-zero value. ]*/
TEST_FUNCTION(when_copying_the_tustedCerts_fails_tlsio_cyclonessl_setoption_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "xx"))
        .IgnoreArgument_destination()
        .SetReturn(1);

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_setoption(tlsio_handle, "TrustedCerts", "xx");

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_062: [ If a previous TrustedCerts option was saved, then the previous value shall be freed. ]*/
TEST_FUNCTION(when_copying_the_tustedCerts_the_previous_option_value_is_freed_and_new_cert_copy_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_setoption(tlsio_handle, "TrustedCerts", "xx");
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "yy"))
        .IgnoreArgument_destination()
        .SetReturn(1);

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_setoption(tlsio_handle, "TrustedCerts", "yy");

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_062: [ If a previous TrustedCerts option was saved, then the previous value shall be freed. ]*/
TEST_FUNCTION(when_copying_the_tustedCerts_the_previous_option_value_is_freed)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_setoption(tlsio_handle, "TrustedCerts", "xx");
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "yy"))
        .IgnoreArgument_destination();

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_setoption(tlsio_handle, "TrustedCerts", "yy");

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_063: [ A NULL value shall be allowed for TrustedCerts, in which case the previously stored TrustedCerts option value shall be cleared. ]*/
TEST_FUNCTION(tlsio_cyclonessl_setoption_with_NULL_TrustedCerts_frees_the_previous_certs_value)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_setoption(tlsio_handle, "TrustedCerts", "xx");
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    ///act
    int result = tlsio_cyclonessl_get_interface_description()->concrete_io_setoption(tlsio_handle, "TrustedCerts", NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* tlsio_cyclonessl_retrieve_options */

/* Tests_SRS_TLSIO_CYCLONESSL_01_064: [ If parameter handle is NULL then tlsio_cyclonessl_retrieve_options shall fail and return NULL. ]*/
TEST_FUNCTION(tlsio_cyclonessl_retrieveoptions_with_NULL_handle_fails)
{
    ///arrange

    ///act
    OPTIONHANDLER_HANDLE result = tlsio_cyclonessl_get_interface_description()->concrete_io_retrieveoptions(NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_065: [ tlsio_cyclonessl_retrieve_options shall produce an OPTIONHANDLER_HANDLE. ]*/
TEST_FUNCTION(tlsio_cyclonessl_retrieveoptions_when_no_option_was_set_gives_back_an_emptyoption_handles)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    umock_c_reset_all_calls();

    EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    ///act
    OPTIONHANDLER_HANDLE result = tlsio_cyclonessl_get_interface_description()->concrete_io_retrieveoptions(tlsio_handle);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_066: [ tlsio_cyclonessl_retrieve_options shall add to it the options: ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_067: [  - TrustedCerts ]*/
TEST_FUNCTION(tlsio_cyclonessl_retrieveoptions_when_TrustedCerts_is_set_populates_the_trustedcerts_in_the_option_handler)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_setoption(tlsio_handle, "TrustedCerts", "xx");
    umock_c_reset_all_calls();

    EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(OptionHandler_AddOption(TEST_OPTION_HANDLER, "TrustedCerts", IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, "xx", 2);

    ///act
    OPTIONHANDLER_HANDLE result = tlsio_cyclonessl_get_interface_description()->concrete_io_retrieveoptions(tlsio_handle);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_065: [ tlsio_cyclonessl_retrieve_options shall produce an OPTIONHANDLER_HANDLE. ]*/
TEST_FUNCTION(tlsio_cyclonessl_retrieveoptions_when_TrustedCerts_was_set_and_cleared_does_not_add_the_option)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_setoption(tlsio_handle, "TrustedCerts", "xx");
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_setoption(tlsio_handle, "TrustedCerts", NULL);
    umock_c_reset_all_calls();

    EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    ///act
    OPTIONHANDLER_HANDLE result = tlsio_cyclonessl_get_interface_description()->concrete_io_retrieveoptions(tlsio_handle);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_068: [ If producing the OPTIONHANDLER_HANDLE fails then tlsio_cyclonessl_retrieve_options shall fail and return NULL. ]*/
TEST_FUNCTION(when_OptionHandler_Create_fails_then_tlsio_cyclonessl_retrieveoptions_returns_NULL)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    umock_c_reset_all_calls();

    EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(NULL);

    ///act
    OPTIONHANDLER_HANDLE result = tlsio_cyclonessl_get_interface_description()->concrete_io_retrieveoptions(tlsio_handle);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* tlsio_cyclonessl_get_interface_description */

/* Tests_SRS_TLSIO_CYCLONESSL_01_069: [ tlsio_cyclonessl_get_interface_description shall return a pointer to an IO_INTERFACE_DESCRIPTION structure that contains pointers to the functions: tlsio_cyclonessl_retrieve_options, tlsio_cyclonessl_create, tlsio_cyclonessl_destroy, tlsio_cyclonessl_open, tlsio_cyclonessl_close, tlsio_cyclonessl_send and tlsio_cyclonessl_dowork.  ]*/
TEST_FUNCTION(tlsio_cyclonessl_get_interface_description_yields_a_filled_in_structure)
{
    ///arrange

    ///act
    const IO_INTERFACE_DESCRIPTION* io_interface = tlsio_cyclonessl_get_interface_description();

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(io_interface);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_close);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_create);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_destroy);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_dowork);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_open);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_retrieveoptions);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_send);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_setoption);
}

/* tlsio_cyclonessl_clone_option */

/* Tests_SRS_TLSIO_CYCLONESSL_01_070: [ If the name or value arguments are NULL, tlsio_cyclonessl_clone_option shall return NULL. ]*/
TEST_FUNCTION(tlsio_cyclonessl_clone_option_with_NULL_option_name_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_retrieveoptions(tlsio_handle);
    umock_c_reset_all_calls();

    ///act
    void* result = tlsio_cyclonessl_clone_option(NULL, "xx");

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_070: [ If the name or value arguments are NULL, tlsio_cyclonessl_clone_option shall return NULL. ]*/
TEST_FUNCTION(tlsio_cyclonessl_clone_option_with_NULL_value_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_retrieveoptions(tlsio_handle);
    umock_c_reset_all_calls();

    ///act
    void* result = tlsio_cyclonessl_clone_option("TrustedCerts", NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_071: [ tlsio_cyclonessl_clone_option shall clone the option named TrustedCerts by calling mallocAndStrcpy_s. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_01_072: [ On success it shall return a non-NULL pointer to the cloned option. ]*/
TEST_FUNCTION(tlsio_cyclonessl_clone_option_clones_TrustedCerts)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_retrieveoptions(tlsio_handle);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "xx"))
        .IgnoreArgument_destination();

    ///act
    void* result = tlsio_cyclonessl_clone_option("TrustedCerts", "xx");

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(result);

    ///cleanup
    tlsio_cyclonessl_destroy_option("TrustedCerts", result);
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_073: [ If mallocAndStrcpy_s for TrustedCerts fails, tlsio_cyclonessl_clone_option shall return NULL. ]*/
TEST_FUNCTION(when_copying_the_TrustedCerts_option_fails_tlsio_cyclonessl_clone_option_fails)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_retrieveoptions(tlsio_handle);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "xx"))
        .IgnoreArgument_destination()
        .SetReturn(1);

    ///act
    void* result = tlsio_cyclonessl_clone_option("TrustedCerts", "xx");

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_074: [ If any of the arguments is NULL, tlsio_cyclonessl_destroy_option shall do nothing. ]*/
TEST_FUNCTION(tlsio_cyclonessl_destroy_option_with_NULL_option_name_does_nothing)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_retrieveoptions(tlsio_handle);
    umock_c_reset_all_calls();

    ///act
    tlsio_cyclonessl_destroy_option(NULL, "xx");

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_074: [ If any of the arguments is NULL, tlsio_cyclonessl_destroy_option shall do nothing. ]*/
TEST_FUNCTION(tlsio_cyclonessl_destroy_option_with_NULL_value_does_nothing)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_retrieveoptions(tlsio_handle);
    umock_c_reset_all_calls();

    ///act
    tlsio_cyclonessl_destroy_option("TrustedCerts", NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

/* Tests_SRS_TLSIO_CYCLONESSL_01_075: [ If the option name is TrustedCerts, tlsio_cyclonessl_destroy_option shall free the char\* option indicated by value. ]*/
TEST_FUNCTION(tlsio_cyclonessl_destroy_option_frees_the_TrustedCerts_option)
{
    ///arrange
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = "test";
    tlsio_config.port = 4242;

    CONCRETE_IO_HANDLE tlsio_handle = tlsio_cyclonessl_get_interface_description()->concrete_io_create(&tlsio_config);
    (void)tlsio_cyclonessl_get_interface_description()->concrete_io_retrieveoptions(tlsio_handle);
    void* result = tlsio_cyclonessl_clone_option("TrustedCerts", "xx");
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    ///act
    tlsio_cyclonessl_destroy_option("TrustedCerts", result);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
    tlsio_cyclonessl_get_interface_description()->concrete_io_destroy(tlsio_handle);
}

END_TEST_SUITE(tlsio_cyclonessl_unittests)
