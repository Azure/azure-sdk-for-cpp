// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#else
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umock_c_negative_tests.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_stdint.h"
#include "umock_c/umocktypes_bool.h"
#include "umock_c/umocktypes.h"
#include "umock_c/umocktypes_c.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void* real_malloc(size_t size)
    {
        return malloc(size);
    }

    void real_free(void* ptr)
    {
        free(ptr);
    }
#ifdef __cplusplus
}
#endif


#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/string_token.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#undef ENABLE_MOCKS

#include "azure_c_shared_utility/ws_url.h"

#ifdef __cplusplus
extern "C" {
#endif

    extern STRING_TOKEN_HANDLE real_StringToken_GetFirst(const char* source, size_t length, const char** delimiters, size_t n_delims);
    extern bool real_StringToken_GetNext(STRING_TOKEN_HANDLE token, const char** delimiters, size_t n_delims);
    extern const char* real_StringToken_GetValue(STRING_TOKEN_HANDLE token);
    extern size_t real_StringToken_GetLength(STRING_TOKEN_HANDLE token);
    extern const char* real_StringToken_GetDelimiter(STRING_TOKEN_HANDLE token);
    extern void real_StringToken_Destroy(STRING_TOKEN_HANDLE token);

#ifdef __cplusplus
}
#endif


static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

// Helpers
static int saved_malloc_returns_count = 0;
static void* saved_malloc_returns[20];

static void* TEST_malloc(size_t size)
{
    saved_malloc_returns[saved_malloc_returns_count] = real_malloc(size);

    return saved_malloc_returns[saved_malloc_returns_count++];
}

static void TEST_free(void* ptr)
{
    int i, j;
    for (i = 0, j = 0; j < saved_malloc_returns_count; i++, j++)
    {
        if (saved_malloc_returns[i] == ptr) j++;

        saved_malloc_returns[i] = saved_malloc_returns[j];
    }

    if (i != j) saved_malloc_returns_count--;

    real_free(ptr);
}

static int TEST_mallocAndStrcpy_s(char** destination, const char* source)
{
    *destination = (char*)real_malloc(strlen(source) + 1);
    (void)strcpy(*destination, source);
    return 0;
}

static void register_umock_alias_types()
{
    REGISTER_UMOCK_ALIAS_TYPE(STRING_TOKEN_HANDLE, void*);
}

static void register_global_mock_hooks()
{
    REGISTER_GLOBAL_MOCK_HOOK(malloc, TEST_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(free, TEST_free);
    REGISTER_GLOBAL_MOCK_HOOK(StringToken_GetFirst, real_StringToken_GetFirst);
    REGISTER_GLOBAL_MOCK_HOOK(StringToken_GetNext, real_StringToken_GetNext);
    REGISTER_GLOBAL_MOCK_HOOK(StringToken_GetDelimiter, real_StringToken_GetDelimiter);
    REGISTER_GLOBAL_MOCK_HOOK(StringToken_GetValue, real_StringToken_GetValue);
    REGISTER_GLOBAL_MOCK_HOOK(StringToken_GetLength, real_StringToken_GetLength);
    REGISTER_GLOBAL_MOCK_HOOK(StringToken_Destroy, real_StringToken_Destroy);
    REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_s, TEST_mallocAndStrcpy_s);
}

static void register_global_mock_returns()
{
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(StringToken_GetFirst, NULL);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(StringToken_GetNext, false);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(StringToken_GetDelimiter, NULL);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(StringToken_GetValue, NULL);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(StringToken_GetLength, 0);
    REGISTER_GLOBAL_MOCK_RETURN(mallocAndStrcpy_s, 0);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(mallocAndStrcpy_s, 1);
}

// Set Expected Call Helpers



BEGIN_TEST_SUITE(ws_url_ut)

    TEST_SUITE_INITIALIZE(ws_url_ut_initialize)
    {
        int result;

        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        umock_c_init(on_umock_c_error);

        result = umocktypes_charptr_register_types();
        ASSERT_ARE_EQUAL(int, 0, result);

        register_umock_alias_types();
        register_global_mock_returns();
        register_global_mock_hooks();
    }

    TEST_SUITE_CLEANUP(ws_url_ut_cleanup)
    {
        umock_c_deinit();

        TEST_MUTEX_DESTROY(g_testByTest);
    }

    TEST_FUNCTION_INITIALIZE(ws_url_ut_test_function_init)
    {
        if (TEST_MUTEX_ACQUIRE(g_testByTest))
        {
            ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
        }

        umock_c_reset_all_calls();
    }

    TEST_FUNCTION_CLEANUP(ws_url_ut_test_function_cleanup)
    {
        TEST_MUTEX_RELEASE(g_testByTest);
    }

    // Tests_SRS_WS_URL_09_001: [ If url is NULL the function shall fail and return NULL ]
    TEST_FUNCTION(ws_url_create_NULL_url)
    {
        // arrange
        WS_URL_HANDLE ws_url;

        umock_c_reset_all_calls();

        // act
        ws_url = ws_url_create(NULL);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(ws_url);

        // cleanup
    }

    // Tests_SRS_WS_URL_09_002: [ Memory shall be allocated for an instance of WS_URL (aka ws_url) ]
    // Tests_SRS_WS_URL_09_024: [ url shall be copied into ws_url->url ]
    // Tests_SRS_WS_URL_09_005: [ If url starts with "wss://" (protocol), ws_url->is_secure shall be set to false ]
    // Tests_SRS_WS_URL_09_006: [ The pointer to the token starting right after protocol (in the url string) shall be stored in ws_url->host ]
    // Tests_SRS_WS_URL_09_008: [ The length from ws_url->host up to the first occurrence of either ":" (port_delimiter), "/" (path_delimiter), "?" (query_delimiter) or \0 shall be stored in ws_url->host_length ]
    // Tests_SRS_WS_URL_09_010: [ If after ws_url->host the port_delimiter occurs (not preceeded by path_delimiter or query_delimiter) the number that follows shall be parsed and stored in ws_url->port ]
    // Tests_SRS_WS_URL_09_012: [ If after ws_url->host or the port number the path_delimiter occurs (not preceeded by query_delimiter) the following pointer address shall be stored in ws_url->path ]
    // Tests_SRS_WS_URL_09_014: [ The length from ws_url->path up to the first occurrence of either query_delimiter or \0 shall be stored in ws_url->path_length ]
    // Tests_SRS_WS_URL_09_016: [ Next if the query_delimiter occurs the following pointer address shall be stored in ws_url->query ]
    // Tests_SRS_WS_URL_09_018: [ The length from ws_url->query up to \0 shall be stored in ws_url->query_length ]
    TEST_FUNCTION(ws_url_create_wss_port_path_query_Success)
    {
        // arrange
        char* url = "wss://some.url.com:443/path/f3548245132826c6cf2fa09694bc6b93?prop1=value1";
        WS_URL_HANDLE ws_url;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        // port
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // path
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // query
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        // act
        ws_url = ws_url_create(url);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NOT_NULL(ws_url);

        // cleanup
        ws_url_destroy(ws_url);
    }

    // Tests_SRS_WS_URL_09_003: [ If ws_url failed to be allocated, the function shall return NULL ]
    // Tests_SRS_WS_URL_09_025: [ If url fails to be copied, the function shall free ws_url and return NULL ]
    // Tests_SRS_WS_URL_09_007: [ If ws_url->host ends up being NULL, the function shall fail and return NULL ]
    // Tests_SRS_WS_URL_09_009: [ If ws_url->host_length ends up being zero, the function shall fail and return NULL ]
    // Tests_SRS_WS_URL_09_011: [ If the port number fails to be parsed, the function shall fail and return NULL ]
    // Tests_SRS_WS_URL_09_013: [ If the path component is present and ws_url->path ends up being NULL, the function shall fail and return NULL ]
    // Tests_SRS_WS_URL_09_015: [ If the path component is present and ws_url->path_length ends up being zero, the function shall fail and return NULL ]
    // Tests_SRS_WS_URL_09_017: [ If the query component is present and ws_url->query ends up being NULL, the function shall fail and return NULL ]
    // Tests_SRS_WS_URL_09_019: [ If the query component is present and ws_url->query_length ends up being zero, the function shall fail and return NULL ]
    // Tests_SRS_WS_URL_09_021: [ If any failure occurs, all memory allocated by the function shall be released before returning ]
    TEST_FUNCTION(ws_url_create_negative_tests)
    {
        // arrange
        char* url = "wss://some.url.com:443/path/f3548245132826c6cf2fa09694bc6b93?prop1=value1";
        size_t i;
        size_t negative_tests_call_count = 1;

        ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

        for (i = 0; i < negative_tests_call_count; i++)
        {
            // arrange
            char error_msg[64];
            WS_URL_HANDLE ws_url;

            if (i == 5 || i == 9 || i == 13 || i == 14 || i == 17)
            {
                continue;
            }

            umock_c_reset_all_calls();
            STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
            STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

            // host
            STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3)); // 5

            // port
            STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1)); // 9

            // path
            STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1)); // 13

            // query
            STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG)); // 14
            STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
            STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1)); // 17
            umock_c_negative_tests_snapshot();

            negative_tests_call_count = umock_c_negative_tests_call_count();

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            // act
            ws_url = ws_url_create(url);

            // assert
            sprintf(error_msg, "On failed call %lu", (unsigned long)i);
            ASSERT_IS_NULL(ws_url, error_msg);
        }

        // cleanup
        umock_c_negative_tests_deinit();
    }

    // Tests_SRS_WS_URL_09_004: [ If url starts with "ws://" (protocol), ws_url->is_secure shall be set to false ]
    TEST_FUNCTION(ws_url_create_ws_port_path_query_Success)
    {
        // arrange
        char* url = "ws://some.url.com:80/path/f3548245132826c6cf2fa09694bc6b93?prop1=value1";

        bool is_secure;
        const char* host;
        size_t host_length;
        size_t port;
        const char* path;
        size_t path_length;
        const char* query;
        size_t query_length;

        WS_URL_HANDLE ws_url;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 5, strlen(url) - 5, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        // port
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // path
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // query
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        // act
        ws_url = ws_url_create(url);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NOT_NULL(ws_url);

        ASSERT_ARE_EQUAL(int, 0, ws_url_is_secure(ws_url, &is_secure));
        ASSERT_IS_FALSE(is_secure);

        ASSERT_ARE_EQUAL(int, 0, ws_url_get_host(ws_url, &host, &host_length));
        ASSERT_ARE_EQUAL(size_t, 12, host_length);
        ASSERT_ARE_EQUAL(int, 0, strncmp(url + 5, host, host_length));

        ASSERT_ARE_EQUAL(int, 0, ws_url_get_port(ws_url, &port));
        ASSERT_ARE_EQUAL(size_t, 80, port);

        ASSERT_ARE_EQUAL(int, 0, ws_url_get_path(ws_url, &path, &path_length));
        ASSERT_ARE_EQUAL(size_t, 37, path_length);
        ASSERT_ARE_EQUAL(int, 0, strncmp(url + 21, path, path_length));

        ASSERT_ARE_EQUAL(int, 0, ws_url_get_query(ws_url, &query, &query_length));
        ASSERT_ARE_EQUAL(size_t, 12, query_length);
        ASSERT_ARE_EQUAL(int, 0, strncmp(url + 59, query, query_length));

        // cleanup
        ws_url_destroy(ws_url);
    }

    // Tests_SRS_WS_URL_09_012: [ If after ws_url->host or the port number the path_delimiter occurs (not preceeded by query_delimiter) the following pointer address shall be stored in ws_url->path ]
    TEST_FUNCTION(ws_url_create_wss_path_query_Success)
    {
        // arrange
        char* url = "wss://some.url.com/path/f3548245132826c6cf2fa09694bc6b93?prop1=value1";

        bool is_secure;
        const char* host;
        size_t host_length;
        size_t port;
        const char* path;
        size_t path_length;
        const char* query;
        size_t query_length;

        WS_URL_HANDLE ws_url;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // path
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // query
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        // act
        ws_url = ws_url_create(url);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NOT_NULL(ws_url);

        ASSERT_ARE_EQUAL(int, 0, ws_url_is_secure(ws_url, &is_secure));
        ASSERT_IS_TRUE(is_secure);

        ASSERT_ARE_EQUAL(int, 0, ws_url_get_host(ws_url, &host, &host_length));
        ASSERT_ARE_EQUAL(size_t, 12, host_length);
        ASSERT_ARE_EQUAL(int, 0, strncmp(url + 6, host, host_length));

        ASSERT_ARE_EQUAL(int, 0, ws_url_get_port(ws_url, &port));
        ASSERT_ARE_EQUAL(size_t, 0, port);

        ASSERT_ARE_EQUAL(int, 0, ws_url_get_path(ws_url, &path, &path_length));
        ASSERT_ARE_EQUAL(size_t, 37, path_length);
        ASSERT_ARE_EQUAL(int, 0, strncmp(url + 19, path, path_length));

        ASSERT_ARE_EQUAL(int, 0, ws_url_get_query(ws_url, &query, &query_length));
        ASSERT_ARE_EQUAL(size_t, 12, query_length);
        ASSERT_ARE_EQUAL(int, 0, strncmp(url + 57, query, query_length));

        // cleanup
        ws_url_destroy(ws_url);
    }

    // Tests_SRS_WS_URL_09_024: [ If protocol cannot be identified in url, the function shall fail and return NULL ]
    TEST_FUNCTION(ws_url_create_unrecognized_protocol)
    {
        // arrange
        char* url = "wws://some.url.com:443/path/f3548245132826c6cf2fa09694bc6b93?prop1=value1";
        WS_URL_HANDLE ws_url;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));

        // act
        ws_url = ws_url_create(url);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(ws_url);

        // cleanup
    }

    // Tests_SRS_WS_URL_09_022: [ If url is NULL, the function shall return without further action ]
    TEST_FUNCTION(ws_url_destroy_NULL_handle)
    {
        // arrange
        umock_c_reset_all_calls();

        // act
        ws_url_destroy(NULL);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        // cleanup
    }

    // Tests_SRS_WS_URL_09_023: [ Otherwise, the memory allocated for url shall released ]
    TEST_FUNCTION(ws_url_destroy_Success)
    {
        // arrange
        char* url = "wss://some.url.com:443/path/f3548245132826c6cf2fa09694bc6b93?prop1=value1";
        WS_URL_HANDLE ws_url;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        // port
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // path
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // query
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        ws_url = ws_url_create(url);

        umock_c_reset_all_calls();

        // act
        ws_url_destroy(ws_url);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        // cleanup
    }

    // Tests_SRS_WS_URL_09_020: [ If any component cannot be parsed or is out of order, the function shall fail and return NULL ]
    TEST_FUNCTION(ws_url_create_url_out_order1)
    {
        // arrange
        char* url = "wss://some.url.com?prop1=value1:443/path/f3548245132826c6cf2fa09694bc6b93";
        WS_URL_HANDLE ws_url;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        // act
        ws_url = ws_url_create(url);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(ws_url);

        // cleanup
    }

    // Tests_SRS_WS_URL_09_020: [ If any component cannot be parsed or is out of order, the function shall fail and return NULL ]
    TEST_FUNCTION(ws_url_create_url_out_order2)
    {
        // arrange
        char* url = "wss://some.url.com:443?prop1=value1/path/f3548245132826c6cf2fa09694bc6b93";
        WS_URL_HANDLE ws_url;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        // query
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        // act
        ws_url = ws_url_create(url);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(ws_url);

        // cleanup
    }

    // Tests_SRS_WS_URL_09_020: [ If any component cannot be parsed or is out of order, the function shall fail and return NULL ]
    TEST_FUNCTION(ws_url_create_url_out_order3)
    {
        // arrange
        char* url = "wss://some.url.com?prop1=value1/path/f3548245132826c6cf2fa09694bc6b93:443";
        WS_URL_HANDLE ws_url;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        // act
        ws_url = ws_url_create(url);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(ws_url);

        // cleanup
    }

    // Tests_SRS_WS_URL_09_020: [ If any component cannot be parsed or is out of order, the function shall fail and return NULL ]
    TEST_FUNCTION(ws_url_create_url_out_order4)
    {
        // arrange
        char* url = "wss://some.url.com?prop1=value1/path/f3548245132826c6cf2fa09694bc6b93";
        WS_URL_HANDLE ws_url;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        // act
        ws_url = ws_url_create(url);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(ws_url);

        // cleanup
    }

    // Tests_SRS_WS_URL_09_026: [ If url is NULL, the function shall return a non-zero value (failure) ]
    TEST_FUNCTION(ws_url_is_NULL_handle)
    {
        // arrange
        int result;
        bool is_secure;

        umock_c_reset_all_calls();

        // act
        result = ws_url_is_secure(NULL, &is_secure);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        // cleanup
    }

    // Tests_SRS_WS_URL_09_026: [ If url is NULL, the function shall return a non-zero value (failure) ]
    TEST_FUNCTION(ws_url_is_NULL_is_secure)
    {
        // arrange
        char* url = "wss://some.url.com:443/path/f3548245132826c6cf2fa09694bc6b93?prop1=value1";
        WS_URL_HANDLE ws_url;
        int result;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        // port
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // path
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // query
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        ws_url = ws_url_create(url);

        // act
        result = ws_url_is_secure(ws_url, NULL);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        // cleanup
        ws_url_destroy(ws_url);
    }

    // Tests_SRS_WS_URL_09_027: [ Otherwize the function shall set is_secure as url->is_secure ]
    // Tests_SRS_WS_URL_09_028: [ If no errors occur function shall return zero (success) ]
    TEST_FUNCTION(ws_url_is_secure_success)
    {
        // arrange
        char* url = "wss://some.url.com:443/path/f3548245132826c6cf2fa09694bc6b93?prop1=value1";
        WS_URL_HANDLE ws_url;
        int result;
        bool is_secure;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        // port
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // path
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // query
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        ws_url = ws_url_create(url);

        // act
        result = ws_url_is_secure(ws_url, &is_secure);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_IS_TRUE(is_secure);

        // cleanup
        ws_url_destroy(ws_url);
    }

    // Tests_SRS_WS_URL_09_038: [ If url or port are NULL, the function shall return a non-zero value (failure) ]
    TEST_FUNCTION(ws_url_get_port_NULL_url)
    {
        // arrange
        int result;
        size_t port;

        umock_c_reset_all_calls();

        // act
        result = ws_url_get_port(NULL, &port);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        // cleanup
    }

    // Tests_SRS_WS_URL_09_038: [ If url or port are NULL, the function shall return a non-zero value (failure) ]
    TEST_FUNCTION(ws_url_get_port_NULL_port)
    {
        // arrange
        char* url = "wss://some.url.com:443/path/f3548245132826c6cf2fa09694bc6b93?prop1=value1";
        WS_URL_HANDLE ws_url;
        int result;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        // port
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // path
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // query
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        ws_url = ws_url_create(url);

        // act
        result = ws_url_get_port(ws_url, NULL);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        // cleanup
        ws_url_destroy(ws_url);
    }

    // Tests_SRS_WS_URL_09_039: [ Otherwize the function shall set port as url->port ]
    // Tests_SRS_WS_URL_09_040: [ If no errors occur function shall return zero (success) ]
    TEST_FUNCTION(ws_url_get_port_success)
    {
        // arrange
        char* url = "wss://some.url.com:443/path/f3548245132826c6cf2fa09694bc6b93?prop1=value1";
        WS_URL_HANDLE ws_url;
        int result;
        size_t port;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        // port
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // path
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // query
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        ws_url = ws_url_create(url);

        // act
        result = ws_url_get_port(ws_url, &port);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(size_t, 443, port);

        // cleanup
        ws_url_destroy(ws_url);
    }

    // Tests_SRS_WS_URL_09_029: [ If url or host or length are NULL, the function shall return a non-zero value (failure) ]
    TEST_FUNCTION(ws_url_get_host_NULL_url)
    {
        // arrange
        int result;
        const char* host;
        size_t host_length;

        umock_c_reset_all_calls();

        // act
        result = ws_url_get_host(NULL, &host, &host_length);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        // cleanup
    }

    // Tests_SRS_WS_URL_09_029: [ If url or host or length are NULL, the function shall return a non-zero value (failure) ]
    TEST_FUNCTION(ws_url_get_host_NULL_host)
    {
        // arrange
        char* url = "wss://some.url.com:443/path/f3548245132826c6cf2fa09694bc6b93?prop1=value1";
        WS_URL_HANDLE ws_url;
        int result;
        size_t host_length;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        // port
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // path
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // query
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        ws_url = ws_url_create(url);

        // act
        result = ws_url_get_host(ws_url, NULL, &host_length);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        // cleanup
        ws_url_destroy(ws_url);
    }

    // Tests_SRS_WS_URL_09_029: [ If url or host or length are NULL, the function shall return a non-zero value (failure) ]
    TEST_FUNCTION(ws_url_get_host_NULL_length)
    {
        // arrange
        char* url = "wss://some.url.com:443/path/f3548245132826c6cf2fa09694bc6b93?prop1=value1";
        WS_URL_HANDLE ws_url;
        int result;
        const char* host;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        // port
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // path
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // query
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        ws_url = ws_url_create(url);

        // act
        result = ws_url_get_host(ws_url, &host, NULL);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        // cleanup
        ws_url_destroy(ws_url);
    }

    // Tests_SRS_WS_URL_09_030: [ Otherwize the function shall set host to url->host and length to url->host_length ]
    // Tests_SRS_WS_URL_09_031: [ If no errors occur function shall return zero (success) ]
    TEST_FUNCTION(ws_url_get_host_success)
    {
        // arrange
        char* url = "wss://some.url.com:443/path/f3548245132826c6cf2fa09694bc6b93?prop1=value1";
        WS_URL_HANDLE ws_url;
        int result;
        const char* host;
        size_t host_length;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        // port
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // path
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // query
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        ws_url = ws_url_create(url);

        // act
        result = ws_url_get_host(ws_url, &host, &host_length);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(int, 0, strncmp(url + 6, host, host_length));

        // cleanup
        ws_url_destroy(ws_url);
    }

    // Tests_SRS_WS_URL_09_032: [ If url or path or length are NULL, the function shall return a non-zero value (failure) ]
    TEST_FUNCTION(ws_url_get_path_NULL_url)
    {
        // arrange
        int result;
        const char* path;
        size_t path_length;

        umock_c_reset_all_calls();

        // act
        result = ws_url_get_path(NULL, &path, &path_length);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        // cleanup
    }

    // Tests_SRS_WS_URL_09_032: [ If url or path or length are NULL, the function shall return a non-zero value (failure) ]
    TEST_FUNCTION(ws_url_get_path_NULL_path)
    {
        // arrange
        char* url = "wss://some.url.com:443/path/f3548245132826c6cf2fa09694bc6b93?prop1=value1";
        WS_URL_HANDLE ws_url;
        int result;
        size_t path_length;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        // port
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // path
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // query
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        ws_url = ws_url_create(url);

        // act
        result = ws_url_get_path(ws_url, NULL, &path_length);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        // cleanup
        ws_url_destroy(ws_url);
    }

    // Tests_SRS_WS_URL_09_032: [ If url or path or length are NULL, the function shall return a non-zero value (failure) ]
    TEST_FUNCTION(ws_url_get_path_NULL_length)
    {
        // arrange
        char* url = "wss://some.url.com:443/path/f3548245132826c6cf2fa09694bc6b93?prop1=value1";
        WS_URL_HANDLE ws_url;
        int result;
        const char* path;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        // port
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // path
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // query
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        ws_url = ws_url_create(url);

        // act
        result = ws_url_get_path(ws_url, &path, NULL);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        // cleanup
        ws_url_destroy(ws_url);
    }

    // Tests_SRS_WS_URL_09_033: [ Otherwize the function shall set path to url->path and length to url->path_length ]
    // Tests_SRS_WS_URL_09_034: [ If no errors occur function shall return zero (success) ]
    TEST_FUNCTION(ws_url_get_path_success)
    {
        // arrange
        char* url = "wss://some.url.com:443/path/f3548245132826c6cf2fa09694bc6b93?prop1=value1";
        WS_URL_HANDLE ws_url;
        int result;
        const char* path;
        size_t path_length;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        // port
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // path
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // query
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        ws_url = ws_url_create(url);

        // act
        result = ws_url_get_path(ws_url, &path, &path_length);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(size_t, 37, path_length);
        ASSERT_ARE_EQUAL(int, 0, strncmp(url + 23, path, path_length));

        // cleanup
        ws_url_destroy(ws_url);
    }

    // Tests_SRS_WS_URL_09_035: [ If url or query or length are NULL, the function shall return a non-zero value (failure) ]
    TEST_FUNCTION(ws_url_get_query_NULL_url)
    {
        // arrange
        int result;
        const char* query;
        size_t query_length;

        umock_c_reset_all_calls();

        // act
        result = ws_url_get_query(NULL, &query, &query_length);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        // cleanup
    }

    // Tests_SRS_WS_URL_09_035: [ If url or query or length are NULL, the function shall return a non-zero value (failure) ]
    TEST_FUNCTION(ws_url_get_query_NULL_query)
    {
        // arrange
        char* url = "wss://some.url.com:443/path/f3548245132826c6cf2fa09694bc6b93?prop1=value1";
        WS_URL_HANDLE ws_url;
        int result;
        size_t query_length;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        // port
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // path
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // query
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        ws_url = ws_url_create(url);

        // act
        result = ws_url_get_query(ws_url, NULL, &query_length);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        // cleanup
        ws_url_destroy(ws_url);
    }

    // Tests_SRS_WS_URL_09_035: [ If url or query or length are NULL, the function shall return a non-zero value (failure) ]
    TEST_FUNCTION(ws_url_get_query_NULL_length)
    {
        // arrange
        char* url = "wss://some.url.com:443/path/f3548245132826c6cf2fa09694bc6b93?prop1=value1";
        WS_URL_HANDLE ws_url;
        int result;
        const char* query;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        // port
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // path
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // query
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        ws_url = ws_url_create(url);

        // act
        result = ws_url_get_query(ws_url, &query, NULL);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        // cleanup
        ws_url_destroy(ws_url);
    }

    // Tests_SRS_WS_URL_09_036: [ Otherwize the function shall set query to url->query and length to url->query_length ]
    // Tests_SRS_WS_URL_09_037: [ If no errors occur function shall return zero (success) ]
    TEST_FUNCTION(ws_url_get_query_success)
    {
        // arrange
        char* url = "wss://some.url.com:443/path/f3548245132826c6cf2fa09694bc6b93?prop1=value1";
        WS_URL_HANDLE ws_url;
        int result;
        const char* query;
        size_t query_length;

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, url));
        STRICT_EXPECTED_CALL(StringToken_GetFirst(url + 6, strlen(url) - 6, IGNORED_PTR_ARG, 3));

        // host
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 3));

        // port
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // path
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        // query
        STRICT_EXPECTED_CALL(StringToken_GetDelimiter(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetValue(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetLength(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(StringToken_GetNext(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1));

        STRICT_EXPECTED_CALL(StringToken_Destroy(IGNORED_PTR_ARG));

        ws_url = ws_url_create(url);

        // act
        result = ws_url_get_query(ws_url, &query, &query_length);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(size_t, 12, query_length);
        ASSERT_ARE_EQUAL(int, 0, strncmp(url + 61, query, query_length));

        // cleanup
        ws_url_destroy(ws_url);
    }

END_TEST_SUITE(ws_url_ut)
