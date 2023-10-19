// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#else
#include <stdlib.h>
#include <stddef.h>
#endif

void* real_malloc(size_t size)
{
    return malloc(size);
}

void* real_realloc(void* block, size_t size)
{
    return realloc(block, size);
}

void real_free(void* ptr)
{
    free(ptr);
}

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umock_c_negative_tests.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_stdint.h"
#include "umock_c/umocktypes_bool.h"
#include "umock_c/umocktypes.h"
#include "umock_c/umocktypes_c.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#undef ENABLE_MOCKS

#include "azure_c_shared_utility/string_token.h"

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
    return real_malloc(size);
}

static int saved_realloc_returns_count = 0;
static void* saved_realloc_returns[20];

static void* TEST_realloc(void* block, size_t size)
{
    return real_realloc(block, size);
}

static void TEST_free(void* ptr)
{
    real_free(ptr);
}

static void register_global_mock_hooks()
{
    REGISTER_GLOBAL_MOCK_HOOK(malloc, TEST_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(realloc, TEST_realloc);
    REGISTER_GLOBAL_MOCK_HOOK(free, TEST_free);
}

// Set Expected Call Helpers
static void set_expected_calls_for_get_delimiters_lengths()
{
    STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
}

static void set_expected_calls_for_StringToken_GetFirst()
{
    STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
    set_expected_calls_for_get_delimiters_lengths();
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));
}

static void set_expected_calls_for_StringToken_GetNext()
{
    set_expected_calls_for_get_delimiters_lengths();
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG)); // delimiters lengths
}

BEGIN_TEST_SUITE(string_token_ut)

    TEST_SUITE_INITIALIZE(string_token_ut_initialize)
    {
        int result;

        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        umock_c_init(on_umock_c_error);

        result = umocktypes_charptr_register_types();
        ASSERT_ARE_EQUAL(int, 0, result);

        register_global_mock_hooks();
    }

    TEST_SUITE_CLEANUP(string_token_ut_cleanup)
    {
        umock_c_deinit();

        TEST_MUTEX_DESTROY(g_testByTest);
    }

    TEST_FUNCTION_INITIALIZE(string_token_ut_test_function_init)
    {
        if (TEST_MUTEX_ACQUIRE(g_testByTest))
        {
            ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
        }

        umock_c_reset_all_calls();

        //saved_realloc_returns_count = 0;
    }

    TEST_FUNCTION_CLEANUP(string_token_ut_test_function_cleanup)
    {
        TEST_MUTEX_RELEASE(g_testByTest);
    }

    // Tests_SRS_STRING_TOKENIZER_09_001: [ If source or delimiters are NULL, or n_delims is zero, the function shall return NULL ]
    TEST_FUNCTION(StringToken_GetFirst_NULL_source)
    {
        ///arrange
        size_t length = 10;
        STRING_TOKEN_HANDLE handle;
        const char* delimiters[1];

        delimiters[0] = "?";

        umock_c_reset_all_calls();

        // act
        handle = StringToken_GetFirst(NULL, length, delimiters, 1);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(handle);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_001: [ If source or delimiters are NULL, or n_delims is zero, the function shall return NULL ]
    TEST_FUNCTION(StringToken_GetFirst_NULL_delimiters)
    {
        ///arrange
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length;
        STRING_TOKEN_HANDLE handle;

        length = strlen(string);

        umock_c_reset_all_calls();

        // act
        handle = StringToken_GetFirst(string, length, NULL, 4);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(handle);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_001: [ If source or delimiters are NULL, or n_delims is zero, the function shall return NULL ]
    TEST_FUNCTION(StringToken_GetFirst_ZERO_delimiters)
    {
        ///arrange
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length;
        const char* delimiters[1];
        STRING_TOKEN_HANDLE handle;

        length = strlen(string);
        delimiters[0] = "?";

        umock_c_reset_all_calls();

        // act
        handle = StringToken_GetFirst(string, length, delimiters, 0);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(handle);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_002: [ If any of the strings in delimiters are NULL, the function shall return NULL ]
    // Tests_SRS_STRING_TOKENIZER_09_007: [ If any failure occurs, all memory allocated by this function shall be released ]
    TEST_FUNCTION(StringToken_GetFirst_NULL_delimiter)
    {
        ///arrange
        STRING_TOKEN_HANDLE handle;
        const char* delimiters[4];
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        delimiters[0] = "http://";
        delimiters[1] = NULL;
        delimiters[2] = "/";
        delimiters[3] = "?";

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
        set_expected_calls_for_get_delimiters_lengths();
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

        // act
        handle = StringToken_GetFirst(string, length, delimiters, 4);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(handle);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_004: [ If the STRING_TOKEN structure fails to be allocated, the function shall return NULL ]
    TEST_FUNCTION(StringToken_GetFirst_negative_tests)
    {
        ///arrange
        size_t i;
        const char* delimiters[1];
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

        delimiters[0] = "?";

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
        set_expected_calls_for_get_delimiters_lengths();
        umock_c_negative_tests_snapshot();

        // act
        for (i = 0; i < umock_c_negative_tests_call_count(); i++)
        {
            // arrange
            char error_msg[64];
            STRING_TOKEN_HANDLE handle;

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            // act
            handle = StringToken_GetFirst(string, length, delimiters, 1);

            sprintf(error_msg, "On failed call %lu", (unsigned long)i);
            ASSERT_IS_NULL(handle, error_msg);
        }

        // cleanup
        umock_c_negative_tests_deinit();
    }

    // Tests_SRS_STRING_TOKENIZER_09_003: [ A STRING_TOKEN structure shall be allocated to hold the token parameters ]
    // Tests_SRS_STRING_TOKENIZER_09_005: [ The source string shall be split in a token starting from the beginning of source up to occurrence of any one of the demiliters, whichever occurs first in the order provided ]
    TEST_FUNCTION(StringToken_GetFirst_Success)
    {
        ///arrange
        STRING_TOKEN_HANDLE handle;
        const char* delimiters[1];
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        delimiters[0] = "?";

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
        set_expected_calls_for_get_delimiters_lengths();
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

        // act
        handle = StringToken_GetFirst(string, length, delimiters, 1);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NOT_NULL(handle);

        // cleanup
        StringToken_Destroy(handle);
    }

    TEST_FUNCTION(StringToken_GetFirst_Empty_String_Success)
    {
        ///arrange
        STRING_TOKEN_HANDLE handle;
        const char* delimiters[1];
        char* string = "";
        size_t length = 0;

        delimiters[0] = "?";

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
        set_expected_calls_for_get_delimiters_lengths();
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

        // act
        handle = StringToken_GetFirst(string, length, delimiters, 1);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NOT_NULL(handle);
        ASSERT_IS_NULL(StringToken_GetValue(handle));
        ASSERT_ARE_EQUAL(size_t, 0, StringToken_GetLength(handle));
        ASSERT_IS_NULL(StringToken_GetDelimiter(handle));
        ASSERT_IS_FALSE(StringToken_GetNext(handle, delimiters, 1));

        // cleanup
        StringToken_Destroy(handle);
    }

    // Tests_SRS_STRING_TOKENIZER_09_006: [ If the source string does not have any of the demiliters, the resulting token shall be the entire source string ]
    TEST_FUNCTION(StringToken_GetFirst_delimiter_not_found)
    {
        ///arrange
        STRING_TOKEN_HANDLE handle;
        const char* delimiters[1];
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        delimiters[0] = "#";

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
        set_expected_calls_for_get_delimiters_lengths();
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG)); // delimiters lengths

        // act
        handle = StringToken_GetFirst(string, length, delimiters, 1);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NOT_NULL(handle);
        ASSERT_ARE_EQUAL(void_ptr, (void_ptr)string, (void_ptr)StringToken_GetValue(handle));
        ASSERT_ARE_EQUAL(size_t, length, StringToken_GetLength(handle));

        // cleanup
        StringToken_Destroy(handle);
    }

    // Tests_SRS_STRING_TOKENIZER_09_008: [ If token or delimiters are NULL, or n_delims is zero, the function shall return false ]
    TEST_FUNCTION(StringToken_GetNext_NULL_token)
    {
        ///arrange
        bool result;
        const char* delimiters[2];

        delimiters[0] = "https://";
        delimiters[1] = "/path";

        umock_c_reset_all_calls();

        // act
        result = StringToken_GetNext(NULL, delimiters, 2);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_FALSE(result);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_008: [ If token or delimiters are NULL, or n_delims is zero, the function shall return false ]
    TEST_FUNCTION(StringToken_GetNext_NULL_delimiters)
    {
        ///arrange
        const char* delimiters[2];
        STRING_TOKEN_HANDLE handle;
        bool result;
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        delimiters[0] = "https://";
        delimiters[1] = "/path";

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();

        handle = StringToken_GetFirst(string, length, delimiters, 2);

        umock_c_reset_all_calls();

        // act
        result = StringToken_GetNext(handle, NULL, 2);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_FALSE(result);

        // cleanup
        StringToken_Destroy(handle);
    }

    // Tests_SRS_STRING_TOKENIZER_09_008: [ If token or delimiters are NULL, or n_delims is zero, the function shall return false ]
    TEST_FUNCTION(StringToken_GetNext_zero_delimiters)
    {
        ///arrange
        bool result;
        STRING_TOKEN_HANDLE handle;
        const char* delimiters[2];
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        delimiters[0] = "https://";
        delimiters[1] = "/path";

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();

        handle = StringToken_GetFirst(string, length, delimiters, 2);

        umock_c_reset_all_calls();

        // act
        result = StringToken_GetNext(handle, delimiters, 0);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_FALSE(result);

        // cleanup
        StringToken_Destroy(handle);
    }

    // Tests_SRS_STRING_TOKENIZER_09_010: [ The next token shall be selected starting from the position in source right after the previous delimiter up to occurrence of any one of demiliters, whichever occurs first in the order provided ]
    TEST_FUNCTION(StringToken_GetNext_Success)
    {
        ///arrange
        int result;
        STRING_TOKEN_HANDLE handle;
        const char* delimiters[2];
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);


        delimiters[0] = "https://";
        delimiters[1] = "/path";

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();

        handle = StringToken_GetFirst(string, length, delimiters, 2);

        umock_c_reset_all_calls();
        set_expected_calls_for_get_delimiters_lengths();
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG)); // delimiters lengths

        // act
        result = StringToken_GetNext(handle, delimiters, 2);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 1, result);
        ASSERT_ARE_EQUAL(int, 0, strncmp(StringToken_GetValue(handle), "some.site.com", StringToken_GetLength(handle)));

        // cleanup
        StringToken_Destroy(handle);
    }

    // Tests_SRS_STRING_TOKENIZER_09_012: [ If a token was identified, the function shall return true ]
    TEST_FUNCTION(StringToken_GetNext_malloc_fails)
    {
        ///arrange
        bool result;
        STRING_TOKEN_HANDLE handle;
        const char* delimiters[2];
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

        delimiters[0] = "https://";
        delimiters[1] = "/path"; // this causes the failure

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();

        handle = StringToken_GetFirst(string, length, delimiters, 2);

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG)); // delimiters lengths
        umock_c_negative_tests_snapshot();

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(0);

        // act
        result = StringToken_GetNext(handle, delimiters, 2);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_FALSE(result);

        // cleanup
        umock_c_negative_tests_deinit();
        StringToken_Destroy(handle);
    }

    // Tests_SRS_STRING_TOKENIZER_09_009: [ If the previous token already extended to the end of source, the function shall return false ]
    TEST_FUNCTION(StringToken_GetNext_no_more_tokens)
    {
        ///arrange
        const char* delimiters[1];
        bool result;
        STRING_TOKEN_HANDLE handle;
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        delimiters[0] = "?";

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();
        handle = StringToken_GetFirst(string, length, delimiters, 1);
        ASSERT_IS_NOT_NULL(handle);

        set_expected_calls_for_StringToken_GetNext();
        result = StringToken_GetNext(handle, delimiters, 1);
        ASSERT_IS_TRUE(result);

        umock_c_reset_all_calls();

        // act
        result = StringToken_GetNext(handle, delimiters, 1);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_FALSE(result);

        // cleanup
        StringToken_Destroy(handle);
    }

    // Tests_SRS_STRING_TOKENIZER_09_011: [ If the source string, starting right after the position of the last delimiter found, does not have any of the demiliters, the resulting token shall be the entire remaining of the source string ]
    TEST_FUNCTION(StringToken_GetNext_delimiter_not_found)
    {
        ///arrange
        bool result;
        const char* delimiters[1];
        STRING_TOKEN_HANDLE handle;
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        delimiters[0] = "?";

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();
        handle = StringToken_GetFirst(string, length, delimiters, 1);

        umock_c_reset_all_calls();
        set_expected_calls_for_get_delimiters_lengths();
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG)); // delimiters lengths

        // act
        result = StringToken_GetNext(handle, delimiters, 1);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_TRUE(result);
        ASSERT_ARE_EQUAL(int, 0, strncmp(StringToken_GetValue(handle), "prop1=site.com&prop2=/prop2/abc", StringToken_GetLength(handle)));

        // cleanup
        StringToken_Destroy(handle);
    }

    // Tests_SRS_STRING_TOKENIZER_09_013: [ If token is NULL the function shall return NULL ]
    TEST_FUNCTION(StringToken_GetValue_NULL_handle)
    {
        ///arrange
        const char* value;

        umock_c_reset_all_calls();

        // act
        value = StringToken_GetValue(NULL);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(value);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_015: [ If token is NULL the function shall return zero ]
    TEST_FUNCTION(StringToken_GetLength_NULL_handle)
    {
        ///arrange
        size_t length;

        umock_c_reset_all_calls();

        // act
        length = StringToken_GetLength(NULL);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, length);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_017: [ If token is NULL the function shall return NULL ]
    TEST_FUNCTION(StringToken_GetDelimiter_NULL_handle)
    {
        ///arrange
        const char* value;

        umock_c_reset_all_calls();

        // act
        value = StringToken_GetDelimiter(NULL);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(value);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_020: [ If token is NULL the function shall return ]
    TEST_FUNCTION(StringToken_Destroy_NULL_handle)
    {
        ///arrange
        umock_c_reset_all_calls();

        // act
        StringToken_Destroy(NULL);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_021: [ Otherwise the memory allocated for STRING_TOKEN shall be released ]
    TEST_FUNCTION(StringToken_Destroy_success)
    {
        ///arrange
        STRING_TOKEN_HANDLE handle;
        const char* delimiters[1];
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);


        delimiters[0] = "?";

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();
        handle = StringToken_GetFirst(string, length, delimiters, 1);

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG)); // STRING_TOKEN

        // act
        StringToken_Destroy(handle);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_014: [ The function shall return the pointer to the position in source where the current token starts. ]
    // Tests_SRS_STRING_TOKENIZER_09_016: [ The function shall return the length of the current token ]
    // Tests_SRS_STRING_TOKENIZER_09_018: [ The function shall return a pointer to the delimiter that defined the current token, as passed to the previous call to StringToken_GetNext() or StringToken_GetFirst() ]
    // Tests_SRS_STRING_TOKENIZER_09_019: [ If the current token extends to the end of source, the function shall return NULL ]
    TEST_FUNCTION(StringToken_tokenize_HTTP_URL)
    {
        ///arrange
        STRING_TOKEN_HANDLE handle;
        bool result;
        char* host = "some.site.com";
        char* relative_path = "path/morepath/";
        char* property1 = "prop1=site.com";
        char* property2 = "prop2=/prop2/abc";
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";

        const char* delimiters1[4];
        const char* delimiters2[1];

        delimiters1[0] = "?";
        delimiters1[1] = "http://";
        delimiters1[2] = "https://";
        delimiters1[3] = "/";

        delimiters2[0] = "&";

        // act
        // assert
        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();
        handle = StringToken_GetFirst(string, strlen(string), delimiters1, 4);
        ASSERT_IS_NOT_NULL(handle);
        ASSERT_ARE_EQUAL(void_ptr, (void*)delimiters1[2], (void*)StringToken_GetDelimiter(handle));
        ASSERT_IS_NULL(StringToken_GetValue(handle));
        ASSERT_ARE_EQUAL(int, 0, StringToken_GetLength(handle));

        set_expected_calls_for_StringToken_GetNext();
        result = StringToken_GetNext(handle, delimiters1, 4);
        ASSERT_IS_TRUE(result);
        ASSERT_ARE_EQUAL(void_ptr, (void*)delimiters1[3], (void*)StringToken_GetDelimiter(handle));
        ASSERT_IS_TRUE(strncmp(host, StringToken_GetValue(handle), StringToken_GetLength(handle)) == 0);

        set_expected_calls_for_StringToken_GetNext();
        result = StringToken_GetNext(handle, delimiters1, 1); // intentionally restricting to "?" only
        ASSERT_IS_TRUE(result);
        ASSERT_ARE_EQUAL(void_ptr, (void*)delimiters1[0], (void*)StringToken_GetDelimiter(handle));
        ASSERT_IS_TRUE(strncmp(relative_path, StringToken_GetValue(handle), StringToken_GetLength(handle)) == 0);

        set_expected_calls_for_StringToken_GetNext();
        result = StringToken_GetNext(handle, delimiters2, 1);
        ASSERT_IS_TRUE(result);
        ASSERT_ARE_EQUAL(void_ptr, (void*)delimiters2[0], (void*)StringToken_GetDelimiter(handle));
        ASSERT_IS_TRUE(strncmp(property1, StringToken_GetValue(handle), StringToken_GetLength(handle)) == 0);

        set_expected_calls_for_StringToken_GetNext();
        result = StringToken_GetNext(handle, delimiters2, 1);
        ASSERT_IS_TRUE(result);
        // SRS_STRING_TOKENIZER_09_019:
        ASSERT_IS_NULL(StringToken_GetDelimiter(handle));
        ASSERT_IS_TRUE(strncmp(property2, StringToken_GetValue(handle), StringToken_GetLength(handle)) == 0);

        result = StringToken_GetNext(handle, delimiters2, 1);
        ASSERT_IS_FALSE(result);

        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        // cleanup
        StringToken_Destroy(handle);
    }

    TEST_FUNCTION(StringToken_string_ends_with_delimiter)
    {
        ///arrange
        bool result;
        const char* delimiters[1];
        STRING_TOKEN_HANDLE handle;
        char* string = "abcde";

        delimiters[0] = "de";

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();

        // act
        handle = StringToken_GetFirst(string, strlen(string), delimiters, 1);

        // assert
        ASSERT_IS_NOT_NULL(handle);
        ASSERT_ARE_EQUAL(void_ptr, (void*)delimiters[0], (void*)StringToken_GetDelimiter(handle));
        ASSERT_IS_TRUE(strncmp(string, StringToken_GetValue(handle), StringToken_GetLength(handle)) == 0);
        ASSERT_ARE_EQUAL(int, 3, StringToken_GetLength(handle));

        ///arrange
        set_expected_calls_for_StringToken_GetNext();

        // act
        result = StringToken_GetNext(handle, delimiters, 1);

        // assert
        ASSERT_IS_TRUE(result);
        ASSERT_IS_NULL(StringToken_GetDelimiter(handle));
        ASSERT_IS_NULL(StringToken_GetValue(handle));
        ASSERT_ARE_EQUAL(int, 0, StringToken_GetLength(handle));

        ///arrange
        // act
        result = StringToken_GetNext(handle, delimiters, 1);

        // assert
        ASSERT_IS_FALSE(result);

        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        // cleanup
        StringToken_Destroy(handle);
    }

    // Tests_SRS_STRING_TOKENIZER_09_022: [ If source, delimiters, token or token_count are NULL, or n_delims is zero the function shall return a non-zero value ]
    TEST_FUNCTION(StringToken_Split_NULL_source)
    {
        ///arrange
        int result;

        umock_c_reset_all_calls();

        result = StringToken_Split(NULL, 30, (const char**)0x4444, 2, false, (char***)0x4445, (size_t*)0x4446);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_022: [ If source, delimiters, token or token_count are NULL, or n_delims is zero the function shall return a non-zero value ]
    TEST_FUNCTION(StringToken_Split_NULL_delimiters)
    {
        ///arrange
        int result;

        umock_c_reset_all_calls();

        result = StringToken_Split((char*)0x4443, 30, (const char**)NULL, 2, false, (char***)0x4445, (size_t*)0x4446);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_022: [ If source, delimiters, token or token_count are NULL, or n_delims is zero the function shall return a non-zero value ]
    TEST_FUNCTION(StringToken_Split_NULL_token)
    {
        ///arrange
        int result;

        umock_c_reset_all_calls();

        result = StringToken_Split((char*)0x4443, 30, (const char**)0x4444, 2, false, (char***)NULL, (size_t*)0x4446);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_022: [ If source, delimiters, token or token_count are NULL, or n_delims is zero the function shall return a non-zero value ]
    TEST_FUNCTION(StringToken_Split_NULL_token_count)
    {
        ///arrange
        int result;

        umock_c_reset_all_calls();

        result = StringToken_Split((char*)0x4443, 30, (const char**)0x4444, 2, false, (char***)0x4445, (size_t*)NULL);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_022: [ If source, delimiters, token or token_count are NULL, or n_delims is zero the function shall return a non-zero value ]
    TEST_FUNCTION(StringToken_Split_zero_n_delims)
    {
        ///arrange
        int result;

        umock_c_reset_all_calls();

        result = StringToken_Split((char*)0x4443, 30, (const char**)0x4444, 0, false, (char***)0x4445, (size_t*)0x4446);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_NOT_EQUAL(int, 0, result);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_023: [ source (up to length) shall be split into individual tokens separated by any of delimiters ]
    // Tests_SRS_STRING_TOKENIZER_09_024: [ All NULL tokens shall be ommited if include_empty is not TRUE ]
    // Tests_SRS_STRING_TOKENIZER_09_025: [ The tokens shall be stored in tokens, and their count stored in token_count ]
    // Tests_SRS_STRING_TOKENIZER_09_027: [ If no failures occur the function shall return zero ]
    TEST_FUNCTION(StringToken_Split_Success)
    {
        ///arrange
        int result;
        const char* delimiters[2];
        char* string = "abc/def&ghi/jkl";
        char** tokens;
        size_t token_count;
        size_t length = strlen(string);

        delimiters[0] = "/";
        delimiters[1] = "&";

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));

        set_expected_calls_for_StringToken_GetNext();
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));

        set_expected_calls_for_StringToken_GetNext();
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));

        set_expected_calls_for_StringToken_GetNext();
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));

        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

        // act
        result = StringToken_Split(string, length, delimiters, 2, false, &tokens, &token_count);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(size_t, 4, token_count);
        ASSERT_ARE_EQUAL(int, 0, strcmp("abc", tokens[0]));
        ASSERT_ARE_EQUAL(int, 0, strcmp("def", tokens[1]));
        ASSERT_ARE_EQUAL(int, 0, strcmp("ghi", tokens[2]));
        ASSERT_ARE_EQUAL(int, 0, strcmp("jkl", tokens[3]));

        // cleanup
        while (token_count > 0)
        {
            free(tokens[--token_count]);
        }
        free(tokens);
    }

    TEST_FUNCTION(StringToken_Split_Zero_Length_Success)
    {
        ///arrange
        int result;
        const char* delimiters[2];
        char* string = "";
        size_t length = 0;
        char** tokens = NULL;
        size_t token_count;

        delimiters[0] = "/";
        delimiters[1] = "&";

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

        // act
        result = StringToken_Split(string, length, delimiters, 2, false, &tokens, &token_count);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(size_t, 0, token_count);
        ASSERT_IS_NULL(tokens);

        // cleanup
        free(tokens);
    }

    // Tests_SRS_STRING_TOKENIZER_09_024: [ All NULL tokens shall be ommited if include_empty is not TRUE ]
    TEST_FUNCTION(StringToken_Split_include_NULL_Success)
    {
        ///arrange
        int result;
        const char* delimiters[2];
        char* string = "&abc/&def&ghi/jkl//";
        char** tokens;
        size_t token_count;
        size_t length = strlen(string);

        delimiters[0] = "/";
        delimiters[1] = "&";

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

        set_expected_calls_for_StringToken_GetNext();
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));

        set_expected_calls_for_StringToken_GetNext();
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

        set_expected_calls_for_StringToken_GetNext();
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));

        set_expected_calls_for_StringToken_GetNext();
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));

        set_expected_calls_for_StringToken_GetNext();
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));

        set_expected_calls_for_StringToken_GetNext();
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

        set_expected_calls_for_StringToken_GetNext();
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

        // act
        result = StringToken_Split(string, length, delimiters, 2, true, &tokens, &token_count);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(size_t, 8, token_count);
        ASSERT_IS_NULL(tokens[0]);
        ASSERT_ARE_EQUAL(int, 0, strcmp("abc", tokens[1]));
        ASSERT_IS_NULL(tokens[2]);
        ASSERT_ARE_EQUAL(int, 0, strcmp("def", tokens[3]));
        ASSERT_ARE_EQUAL(int, 0, strcmp("ghi", tokens[4]));
        ASSERT_ARE_EQUAL(int, 0, strcmp("jkl", tokens[5]));
        ASSERT_IS_NULL(tokens[6]);
        ASSERT_IS_NULL(tokens[7]);

        // cleanup
        while (token_count > 0)
        {
            if (tokens[--token_count] != NULL)
            {
                free(tokens[token_count]);
            }
        }
        free(tokens);
    }

    // Tests_SRS_STRING_TOKENIZER_09_026: [ If any failures splitting or storing the tokens occur the function shall return a non-zero value ]
    TEST_FUNCTION(StringToken_Split_negative_tests)
    {
        ///arrange
        const char* delimiters[2];
        char* string = "abc/def&ghi/jkl";
        char** tokens;
        size_t token_count;
        size_t i;
        size_t length = strlen(string);

        ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

        delimiters[0] = "/";
        delimiters[1] = "&";

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst(); // 0, 1, 2
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));

        set_expected_calls_for_StringToken_GetNext(); // 5, 6
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));

        set_expected_calls_for_StringToken_GetNext(); // 9, 10
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));

        set_expected_calls_for_StringToken_GetNext(); // 13, 14
        STRICT_EXPECTED_CALL(realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));

        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG)); // 17
        umock_c_negative_tests_snapshot();

        for (i = 0; i < umock_c_negative_tests_call_count(); i++)
        {
            // arrange
            char error_msg[64];
            int result;

            if (i == 0 || i == 1 || i == 2 || i == 5 || i == 6 || i == 9 || i == 10 || i == 13 || i == 14 || i == 17)
            {
                continue;
            }

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            // act
            result = StringToken_Split(string, length, delimiters, 2, false, &tokens, &token_count);

            sprintf(error_msg, "On failed call %lu", (unsigned long)i);
            ASSERT_ARE_NOT_EQUAL(int, 0, result, error_msg);
        }

        // cleanup
        umock_c_negative_tests_deinit();
    }

END_TEST_SUITE(string_token_ut)
