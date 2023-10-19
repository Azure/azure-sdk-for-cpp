// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#undef DECLSPEC_IMPORT

#pragma warning(disable: 4273)

#include <winsock2.h>

void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_stdint.h"
#include "umock_c/umock_c_negative_tests.h"
#include "azure_macro_utils/macro_utils.h"

#define ENABLE_MOCKS

#include "azure_c_shared_utility/gballoc.h"
#include "umock_c/umock_c_prod.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/tlsio_schannel.h"
#include "azure_c_shared_utility/httpapiex.h"
#ifdef USE_OPENSSL
#include "azure_c_shared_utility/tlsio_openssl.h"
#endif
#if USE_WOLFSSL
#include "azure_c_shared_utility/tlsio_wolfssl.h"
#endif
#undef ENABLE_MOCKS

#include "azure_c_shared_utility/platform.h"

#define ENABLE_MOCKS

MOCK_FUNCTION_WITH_CODE(WSAAPI, int, WSAStartup, WORD, wVersionRequested, LPWSADATA, lpWSAData)
MOCK_FUNCTION_END(0)

MOCK_FUNCTION_WITH_CODE(WSAAPI, int, WSACleanup)
MOCK_FUNCTION_END(0)

static const IO_INTERFACE_DESCRIPTION* TEST_IO_INTERFACE_DESCRIPTION = (const IO_INTERFACE_DESCRIPTION*)0x4444;

#ifdef __cplusplus
extern "C"
{
#endif
    STRING_HANDLE STRING_construct_sprintf(const char* format, ...);

    STRING_HANDLE STRING_construct_sprintf(const char* format, ...)
    {
        const char* psz = "STRING_construct_sprintf variable";
        char* temp = (char*)my_gballoc_malloc(strlen(psz) + 1);
        ASSERT_IS_NOT_NULL(temp);
        (void)format;
        (void)memcpy(temp, psz, strlen(psz) + 1);
        return (STRING_HANDLE)temp;
    }

    int STRING_sprintf(STRING_HANDLE handle, const char* format, ...);

    int STRING_sprintf(STRING_HANDLE handle, const char* format, ...)
    {
        (void)handle;
        (void)format;
        return 0;
    }
#ifdef __cplusplus
}
#endif

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

static STRING_HANDLE my_STRING_construct(const char* psz)
{
    char* temp = (char*)my_gballoc_malloc(strlen(psz) + 1);
    ASSERT_IS_NOT_NULL(temp);
    (void)memcpy(temp, psz, strlen(psz) + 1);
    return (STRING_HANDLE)temp;
}

const char* my_STRING_c_str(STRING_HANDLE handle)
{
    return (const char*)handle;
}

static void my_STRING_delete(STRING_HANDLE h)
{
    my_gballoc_free((void*)h);
}

static const IO_INTERFACE_DESCRIPTION* my_tlsio_schannel_get_interface_description(void)
{
    return TEST_IO_INTERFACE_DESCRIPTION;
}

static VOID my_GetSystemInfo(_Out_ LPSYSTEM_INFO lpSystemInfo)
{
    lpSystemInfo->wProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
}

BEGIN_TEST_SUITE(platform_win32_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_UMOCK_ALIAS_TYPE(STRING_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(WORD, unsigned short);
    REGISTER_UMOCK_ALIAS_TYPE(LPWSADATA, void*);
    REGISTER_UMOCK_ALIAS_TYPE(HTTPAPIEX_RESULT, int);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);

    REGISTER_GLOBAL_MOCK_HOOK(STRING_construct, my_STRING_construct);
    REGISTER_GLOBAL_MOCK_HOOK(STRING_c_str, my_STRING_c_str);
    REGISTER_GLOBAL_MOCK_HOOK(STRING_delete, my_STRING_delete);

    REGISTER_GLOBAL_MOCK_RETURN(tlsio_schannel_get_interface_description, TEST_IO_INTERFACE_DESCRIPTION);
    REGISTER_GLOBAL_MOCK_RETURNS(HTTPAPIEX_Init, HTTPAPIEX_OK, HTTPAPIEX_ERROR);

#ifdef USE_OPENSSL
    REGISTER_GLOBAL_MOCK_RETURN(tlsio_openssl_get_interface_description, TEST_IO_INTERFACE_DESCRIPTION);
    REGISTER_GLOBAL_MOCK_RETURN(tlsio_openssl_init, 0);
#endif
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

TEST_FUNCTION(platform_init_success)
{
    int result;

    //arrange
    STRICT_EXPECTED_CALL(WSAStartup(IGNORED_NUM_ARG, IGNORED_PTR_ARG));
#ifndef DONT_USE_UPLOADTOBLOB
    STRICT_EXPECTED_CALL(HTTPAPIEX_Init());
#endif /* DONT_USE_UPLOADTOBLOB */
#ifdef USE_OPENSSL
    STRICT_EXPECTED_CALL(tlsio_openssl_init());
#endif

    //act
    result = platform_init();

    //assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
}

TEST_FUNCTION(platform_init_WSAStartup_0_fail)
{
    int result;

    //arrange
    STRICT_EXPECTED_CALL(WSAStartup(IGNORED_NUM_ARG, IGNORED_PTR_ARG)).SetReturn(1);

    //act
    result = platform_init();

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
}

#ifndef DONT_USE_UPLOADTOBLOB
TEST_FUNCTION(platform_init_HTTPAPIEX_fail)
{
    int result;

    //arrange
    STRICT_EXPECTED_CALL(WSAStartup(IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(HTTPAPIEX_Init()).SetReturn(HTTPAPIEX_ERROR);

    //act
    result = platform_init();

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
}
#endif /* DONT_USE_UPLOADTOBLOB */

TEST_FUNCTION(platform_get_default_tlsio_success)
{
    const IO_INTERFACE_DESCRIPTION* io_desc;

    //arrange
#ifdef USE_OPENSSL
    STRICT_EXPECTED_CALL(tlsio_openssl_get_interface_description());
#else
    STRICT_EXPECTED_CALL(tlsio_schannel_get_interface_description());
#endif

    //act
    io_desc = platform_get_default_tlsio();

    //assert
    ASSERT_IS_NOT_NULL(io_desc);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
}

TEST_FUNCTION(platform_get_platform_info_success)
{
    //arrange

    //act
    STRING_HANDLE platform = platform_get_platform_info(PLATFORM_INFO_OPTION_RETRIEVE_SQM);

    //assert
    ASSERT_IS_NOT_NULL(platform);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    STRING_delete(platform);
}

TEST_FUNCTION(platform_deinit_success)
{
    //arrange
    STRICT_EXPECTED_CALL(WSACleanup());
#ifndef DONT_USE_UPLOADTOBLOB
    STRICT_EXPECTED_CALL(HTTPAPIEX_Deinit());
#endif /* DONT_USE_UPLOADTOBLOB */
#ifdef USE_OPENSSL
    STRICT_EXPECTED_CALL(tlsio_openssl_deinit());
#endif

    //act
    platform_deinit();

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
}

END_TEST_SUITE(platform_win32_ut)
