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

#include "tls.h"

#undef htonl
#undef htons
#undef ntohl
#undef ntohs

#if _WIN32
#undef DECLSPEC_IMPORT

#pragma warning(disable: 4273 4189)

#include <winsock2.h>
#include <mstcpip.h>
#include <ws2tcpip.h>
#endif

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#include <stdbool.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umock_c_negative_tests.h"

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/tlsio_cyclonessl_socket.h"

#define ENABLE_MOCKS

SOCKET TEST_SOCKET = (SOCKET)0x4243;
static const struct sockaddr test_sock_addr = { 0 };
static ADDRINFO TEST_ADDR_INFO = { AI_PASSIVE, AF_INET, SOCK_STREAM, IPPROTO_TCP, 128, NULL, (struct sockaddr*)&test_sock_addr, NULL };
static bool g_addrinfo_call_fail;

MOCK_FUNCTION_WITH_CODE(WSAAPI, SOCKET, socket, int, af, int, type, int, protocol)
MOCK_FUNCTION_END(TEST_SOCKET)
MOCK_FUNCTION_WITH_CODE(WSAAPI, int, closesocket, SOCKET, s)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(WSAAPI, int, connect, SOCKET, s, const struct sockaddr*, name, int, namelen)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(WSAAPI, int, WSAGetLastError)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(WSAAPI, int, getaddrinfo, PCSTR, pNodeName, PCSTR, pServiceName, const ADDRINFOA*, pHints, PADDRINFOA*, ppResult)
    int callFail;
    if (!g_addrinfo_call_fail)
    {
        *ppResult = &TEST_ADDR_INFO;
        callFail = 0;
    }
    else
    {
        *ppResult = NULL;
        callFail = MU_FAILURE;
    }
MOCK_FUNCTION_END(callFail)

#include "azure_c_shared_utility/gballoc.h"

#undef ENABLE_MOCKS

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(tlsio_cyclonessl_socket_bsd_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    result = umock_c_init(on_umock_c_error);
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_UMOCK_ALIAS_TYPE(TlsSocket, void*);
    REGISTER_UMOCK_ALIAS_TYPE(PCSTR, const char*);
    REGISTER_UMOCK_ALIAS_TYPE(SOCKET, void*);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
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

    g_addrinfo_call_fail = false;
    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* tlsio_cyclonessl_socket_create */

/* Tests_SRS_TLSIO_CYCLONESSL_SOCKET_BSD_01_001: [ tlsio_cyclonessl_socket_create shall create a new socket to be used by CycloneSSL. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_SOCKET_BSD_01_008: [ On success tlsio_cyclonessl_socket_create shall return 0 and fill in the socket handle in the new_socket out argument. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_SOCKET_BSD_01_003: [ tlsio_cyclonessl_socket_create shall call socket to create a TCP socket. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_SOCKET_BSD_01_004: [ tlsio_cyclonessl_socket_create shall call getaddrinfo to obtain a hint ADDRINFO. ]*/
/* Tests_SRS_TLSIO_CYCLONESSL_SOCKET_BSD_01_006: [ tlsio_cyclonessl_socket_create shall call connect and pass the constructed address in order to connect the socket. ]*/
TEST_FUNCTION(tlsio_cyclonessl_socket_create_succeeds)
{
    ///arrange
    TlsSocket socket;

    STRICT_EXPECTED_CALL(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    EXPECTED_CALL(getaddrinfo(IGNORED_PTR_ARG, IGNORED_PTR_ARG, &TEST_ADDR_INFO, IGNORED_PTR_ARG));
    EXPECTED_CALL(connect(TEST_SOCKET, &test_sock_addr, IGNORED_NUM_ARG))
        .ValidateArgument_s();

    ///act
    int result = tlsio_cyclonessl_socket_create("testhostname", 4242, &socket);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    ///cleanup
    tlsio_cyclonessl_socket_destroy(socket);
}

/* Tests_SRS_TLSIO_CYCLONESSL_SOCKET_BSD_01_002: [ If hostname or new_socket is NULL, then tlsio_cyclonessl_socket_create shall fail and it shall return a non-zero value. ]*/
TEST_FUNCTION(tlsio_cyclonessl_socket_create_with_NULL_hostname_fails)
{
    ///arrange
    TlsSocket socket;

    ///act
    int result = tlsio_cyclonessl_socket_create(NULL, 4242, &socket);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_TLSIO_CYCLONESSL_SOCKET_BSD_01_002: [ If hostname or new_socket is NULL, then tlsio_cyclonessl_socket_create shall fail and it shall return a non-zero value. ]*/
TEST_FUNCTION(tlsio_cyclonessl_socket_create_with_NULL_socket_fails)
{
    ///arrange

    ///act
    int result = tlsio_cyclonessl_socket_create("testhostname", 4242, NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_TLSIO_CYCLONESSL_SOCKET_BSD_01_007: [ If any of the socket calls fails, then tlsio_cyclonessl_socket_create shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_a_failure_occurs_for_tlsio_cyclonessl_socket_create_then_create_fails)
{
    ///arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);
    TlsSocket socket;

    STRICT_EXPECTED_CALL(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
        .SetFailReturn((SOCKET)-1);
    EXPECTED_CALL(getaddrinfo(IGNORED_PTR_ARG, IGNORED_PTR_ARG, &TEST_ADDR_INFO, IGNORED_PTR_ARG))
        .SetFailReturn(-1);
    EXPECTED_CALL(connect(TEST_SOCKET, &test_sock_addr, IGNORED_NUM_ARG))
        .ValidateArgument_s()
        .SetFailReturn(-1);

    umock_c_negative_tests_snapshot();

    for (size_t i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        char temp_str[128];
        (void)sprintf(temp_str, "On failed call %lu", (unsigned long)i);

        ///act
        int result = tlsio_cyclonessl_socket_create("testhostname", 4242, &socket);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result, temp_str);
    }

    ///cleanup
    umock_c_negative_tests_deinit();
}

/* Tests_SRS_TLSIO_CYCLONESSL_SOCKET_BSD_01_009: [ tlsio_cyclonessl_socket_destroy shall close the socket passed as argument by calling the function close. ]*/
TEST_FUNCTION(tlsio_cyclonessl_socket_destroy_closes_the_socket)
{
    ///arrange
    TlsSocket socket;

    STRICT_EXPECTED_CALL(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    EXPECTED_CALL(getaddrinfo(IGNORED_PTR_ARG, IGNORED_PTR_ARG, &TEST_ADDR_INFO, IGNORED_PTR_ARG));
    EXPECTED_CALL(connect(TEST_SOCKET, &test_sock_addr, IGNORED_NUM_ARG))
        .ValidateArgument_s();
    int result = tlsio_cyclonessl_socket_create("testhostname", 4242, &socket);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(closesocket(TEST_SOCKET));

    ///act
    tlsio_cyclonessl_socket_destroy(socket);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_TLSIO_CYCLONESSL_SOCKET_BSD_01_010: [ If socket is INVALID_SOCKET (-1), tlsio_cyclonessl_socket_destroy shall do nothing. ]*/
TEST_FUNCTION(tlsio_cyclonessl_socket_destroy_with_invalid_socket_does_nothing)
{
    ///arrange

    ///act
    tlsio_cyclonessl_socket_destroy((TlsSocket)-1);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(tlsio_cyclonessl_socket_bsd_unittests)
