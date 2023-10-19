// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#include <ctime>
#include <cstdint>
#else
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <stdint.h>
#endif

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umock_c_negative_tests.h"
#include "umock_c/umocktypes_stdint.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/agenttime.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/buffer_.h"
#include "azure_c_shared_utility/sastoken.h"
#include "azure_c_shared_utility/httpheaders.h"
#include "azure_c_shared_utility/httpapiex.h"
#include "azure_c_shared_utility/crt_abstractions.h"

#undef ENABLE_MOCKS

#include "azure_c_shared_utility/httpapiexsas.h"

TEST_DEFINE_ENUM_TYPE(HTTPAPIEX_RESULT, HTTPAPIEX_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(HTTPAPIEX_RESULT, HTTPAPIEX_RESULT_VALUES);

IMPLEMENT_UMOCK_C_ENUM_TYPE(HTTP_HEADERS_RESULT, HTTP_HEADERS_RESULT_VALUES);

#define TEST_STRING_HANDLE (STRING_HANDLE)0x46
#define TEST_NULL_STRING_HANDLE (STRING_HANDLE)0x00
#define TEST_KEYNAME_HANDLE (STRING_HANDLE)0x48
#define TEST_KEY_HANDLE (STRING_HANDLE)0x49
#define TEST_URIRESOURCE_HANDLE (STRING_HANDLE)0x50
#define TEST_CLONED_KEYNAME_HANDLE  (STRING_HANDLE)0x51
#define TEST_CLONED_URIRESOURCE_HANDLE  (STRING_HANDLE)0x52
#define TEST_CLONED_KEY_HANDLE  (STRING_HANDLE)0x53
#define TEST_HTTPAPIEX_HANDLE (HTTPAPIEX_HANDLE)0x54
#define TEST_HTTPAPI_REQUEST_TYPE (HTTPAPI_REQUEST_TYPE)0x55
#define TEST_REQUEST_HTTP_HEADERS_HANDLE (HTTP_HEADERS_HANDLE)0x56
#define TEST_REQUEST_CONTENT (BUFFER_HANDLE)0x57
#define TEST_RESPONSE_HTTP_HEADERS_HANDLE (HTTP_HEADERS_HANDLE)0x58
#define TEST_RESPONSE_CONTENT (BUFFER_HANDLE)0x59
#define TEST_CONST_CHAR_STAR_NULL (const char*)NULL
#define TEST_SASTOKEN_HANDLE (STRING_HANDLE)0x60
#define TEST_EXPIRY ((uint64_t)7200)
#define TEST_TIME_T ((time_t)-1)

static const char* TEST_KEY = "key";
static const char* TEST_SAS = "signature";
static const char* TEST_SAS_KEY = "sas=signature";
static const char* TEST_URI_RESOURCE = "test_uri";
static const char* TEST_KEY_NAME = "key_name";

static const char TEST_CHAR_ARRAY[10] = "ABCD";

static TEST_MUTEX_HANDLE g_testByTest;

static int my_mallocAndStrcpy_s(char** destination, const char* source)
{
    size_t len = strlen(source);
    *destination = (char*)my_gballoc_malloc(len+1);
    (void)strcpy(*destination, source);
    return 0;
}

static STRING_HANDLE my_STRING_construct(const char* psz)
{
    char* temp = (char*)my_gballoc_malloc(strlen(psz) + 1);
    ASSERT_IS_NOT_NULL(temp);
    (void)memcpy(temp, psz, strlen(psz) + 1);
    return (STRING_HANDLE)temp;
}

static void my_STRING_delete(STRING_HANDLE handle)
{
    my_gballoc_free(handle);
}

static STRING_HANDLE my_SASToken_CreateString(const char* key, const char* scope, const char* keyName, uint64_t expiry)
{
    (void)key, (void)scope, (void)keyName, (void)expiry;
    return (STRING_HANDLE)my_gballoc_malloc(1);
}

static void setupSASString_Create_happy_path(bool allocateKeyName)
{
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    if (allocateKeyName)
    {
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    }
}

static void setupSAS_Create_happy_path_provide_key(bool useSasKey, bool allocateKeyName)
{
    // HTTPAPIEX_SAS_Create
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)).SetReturn(useSasKey ? TEST_SAS_KEY : TEST_KEY);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)).SetReturn(TEST_URI_RESOURCE);
    if (allocateKeyName)
    {
        STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)).SetReturn(TEST_KEY_NAME);
    }
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    if (allocateKeyName)
    {
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    }
}

static void setupSAS_Create_happy_path(bool allocateKeyName)
{
    setupSAS_Create_happy_path_provide_key(false, allocateKeyName);
}

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

int umocktypes_copy_time_t(time_t* destination, const time_t* source)
{
    *destination = *source;
    return 0;
}

void umocktypes_free_time_t(time_t* value)
{
    (void)value;
}

char* umocktypes_stringify_time_t(const time_t* value)
{
    char temp_str[32];
    char* result;
    int length = snprintf(temp_str, sizeof(temp_str), "%d", (int)(*value));
    if (length <= 0)
    {
        result = NULL;
    }
    else
    {
        result = (char*)malloc(length + 1);
        (void)memcpy(result, temp_str, length + 1);
    }
    return result;
}

int umocktypes_are_equal_time_t(time_t* left, time_t* right)
{
    int result;

    if (*left == *right)
    {
        result = 1;
    }
    else
    {
        result = 0;
    }

    return result;
}

IMPLEMENT_UMOCK_C_ENUM_TYPE(HTTPAPI_REQUEST_TYPE, HTTPAPI_REQUEST_TYPE_VALUES);

BEGIN_TEST_SUITE(httpapiexsas_unittests)

TEST_SUITE_INITIALIZE(TestClassInitialize)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result, "umocktypes_stdint_register_types");

    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_TYPE(HTTPAPIEX_RESULT, HTTPAPIEX_RESULT);
    REGISTER_TYPE(HTTP_HEADERS_RESULT, HTTP_HEADERS_RESULT);
    REGISTER_UMOCK_ALIAS_TYPE(STRING_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(HTTP_HEADERS_HANDLE, void*);
    REGISTER_TYPE(time_t, time_t);
    REGISTER_UMOCK_ALIAS_TYPE(HTTPAPIEX_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(BUFFER_HANDLE, void*);
    REGISTER_TYPE(HTTPAPI_REQUEST_TYPE, HTTPAPI_REQUEST_TYPE);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_HOOK(SASToken_CreateString, my_SASToken_CreateString);
    REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_s, my_mallocAndStrcpy_s);

    REGISTER_GLOBAL_MOCK_RETURN(STRING_c_str, TEST_CONST_CHAR_STAR_NULL);
    REGISTER_GLOBAL_MOCK_RETURN(STRING_length, 0);
    REGISTER_GLOBAL_MOCK_HOOK(STRING_construct, my_STRING_construct);
    REGISTER_GLOBAL_MOCK_HOOK(STRING_delete, my_STRING_delete);

    REGISTER_GLOBAL_MOCK_RETURN(HTTPAPIEX_ExecuteRequest, HTTPAPIEX_OK);
    REGISTER_GLOBAL_MOCK_RETURN(HTTPHeaders_FindHeaderValue, TEST_CONST_CHAR_STAR_NULL);
    REGISTER_GLOBAL_MOCK_RETURN(HTTPHeaders_ReplaceHeaderNameValuePair, HTTP_HEADERS_ERROR);
    REGISTER_GLOBAL_MOCK_RETURN(get_time, TEST_TIME_T);
}

TEST_SUITE_CLEANUP(TestClassCleanup)
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
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);

    umock_c_reset_all_calls();
}

TEST_FUNCTION(HTTPAPIEX_SAS_is_zero_the_epoch)
{
    time_t epoch_candidate = 0;
    struct tm broken_down_time;
    broken_down_time = *gmtime(&epoch_candidate);
    ASSERT_ARE_EQUAL(int, broken_down_time.tm_hour, 0);
    ASSERT_ARE_EQUAL(int, broken_down_time.tm_min, 0);
    ASSERT_ARE_EQUAL(int, broken_down_time.tm_sec, 0);
    ASSERT_ARE_EQUAL(int, broken_down_time.tm_year, 70);
    ASSERT_ARE_EQUAL(int, broken_down_time.tm_mon, 0);
    ASSERT_ARE_EQUAL(int, broken_down_time.tm_mday, 1);
}

/*Tests_SRS_HTTPAPIEXSAS_01_001: [ HTTPAPIEX_SAS_Create shall create a new instance of HTTPAPIEX_SAS and return a non-NULL handle to it. ]*/
TEST_FUNCTION(HTTPAPIEX_SAS_Create_Succeeds)
{
    // arrange
    HTTPAPIEX_SAS_HANDLE handle;

    setupSAS_Create_happy_path(true);

    // act
    handle = HTTPAPIEX_SAS_Create(TEST_KEY_HANDLE, TEST_URIRESOURCE_HANDLE, TEST_KEYNAME_HANDLE);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(handle);

    // Cleanup
    HTTPAPIEX_SAS_Destroy(handle);
}

/*Tests_SRS_HTTPAPIEXSAS_06_001: [If the parameter key is NULL then HTTPAPIEX_SAS_Create shall return NULL.]*/
TEST_FUNCTION(HTTPAPIEX_SAS_Create_null_key_fails)
{
    // arrange
    HTTPAPIEX_SAS_HANDLE handle;

    // act
    handle = HTTPAPIEX_SAS_Create(NULL, TEST_STRING_HANDLE, TEST_STRING_HANDLE);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_HTTPAPIEXSAS_06_002: [If the parameter uriResource is NULL then HTTPAPIEX_SAS_Create shall return NULL.]*/
TEST_FUNCTION(HTTPAPIEX_SAS_Create_null_uriResource_fails)
{
    // arrange
    HTTPAPIEX_SAS_HANDLE handle;

    // act
    handle = HTTPAPIEX_SAS_Create(TEST_STRING_HANDLE, NULL, TEST_STRING_HANDLE);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_HTTPAPIEXSAS_06_003: [The parameter keyName for HTTPAPIEX_SAS_Create is optional and can be NULL.]*/
TEST_FUNCTION(HTTPAPIEX_SAS_Create_null_keyName_succeeds)
{
    // arrange
    HTTPAPIEX_SAS_HANDLE handle;

    setupSAS_Create_happy_path(true);

    // act
    handle = HTTPAPIEX_SAS_Create(TEST_KEY_HANDLE, TEST_URIRESOURCE_HANDLE, TEST_KEYNAME_HANDLE);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(handle);

    // Cleanup
    HTTPAPIEX_SAS_Destroy(handle);
}

/*Tests_SRS_HTTPAPIEXSAS_01_001: [ HTTPAPIEX_SAS_Create shall create a new instance of HTTPAPIEX_SAS and return a non-NULL handle to it. ]*/
TEST_FUNCTION(HTTPAPIEX_SAS_Create_From_String_Succeeds)
{
    // arrange
    HTTPAPIEX_SAS_HANDLE handle;

    setupSASString_Create_happy_path(true);

    // act
    handle = HTTPAPIEX_SAS_Create_From_String(TEST_KEY, TEST_URI_RESOURCE, TEST_KEY_NAME);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(handle);

    // Cleanup
    HTTPAPIEX_SAS_Destroy(handle);
}

/* Tests_SRS_HTTPAPIEXSAS_07_001: [ If the parameter key or uriResource is NULL then HTTPAPIEX_SAS_Create_From_String shall return NULL. ] */
TEST_FUNCTION(HTTPAPIEX_SAS_Create_From_String_null_key_fails)
{
    // arrange
    HTTPAPIEX_SAS_HANDLE handle;

    // act
    handle = HTTPAPIEX_SAS_Create_From_String(NULL, TEST_URI_RESOURCE, TEST_KEY_NAME);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_HTTPAPIEXSAS_07_001: [ If the parameter key or uriResource is NULL then HTTPAPIEX_SAS_Create_From_String shall return NULL. ] */
TEST_FUNCTION(HTTPAPIEX_SAS_Create_From_String_null_uriResource_fails)
{
    // arrange
    HTTPAPIEX_SAS_HANDLE handle;

    // act
    handle = HTTPAPIEX_SAS_Create_From_String(TEST_KEY, NULL, TEST_KEY_NAME);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_HTTPAPIEXSAS_06_003: [The parameter keyName for HTTPAPIEX_SAS_Create is optional and can be NULL.]*/
TEST_FUNCTION(HTTPAPIEX_SAS_Create_From_String_null_keyName_succeeds)
{
    // arrange
    HTTPAPIEX_SAS_HANDLE handle;

    setupSASString_Create_happy_path(false);

    // act
    handle = HTTPAPIEX_SAS_Create_From_String(TEST_KEY, TEST_URI_RESOURCE, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(handle);

    // Cleanup
    HTTPAPIEX_SAS_Destroy(handle);
}

/*Tests_SRS_HTTPAPIEXSAS_06_004: [If there are any other errors in the instantiation of this handle then HTTPAPIEX_SAS_Create shall return NULL.]*/
TEST_FUNCTION(HTTPAPIEX_SAS_Create_malloc_state_fails)
{
    // arrange
    HTTPAPIEX_SAS_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)).SetReturn(TEST_KEY);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)).SetReturn(TEST_URI_RESOURCE);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)).SetReturn(TEST_KEY_NAME);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);

    // act
    handle = HTTPAPIEX_SAS_Create(TEST_KEY_HANDLE, TEST_URIRESOURCE_HANDLE, TEST_KEYNAME_HANDLE);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_HTTPAPIEXSAS_06_004: [If there are any other errors in the instantiation of this handle then HTTPAPIEX_SAS_Create shall return NULL.]*/

TEST_FUNCTION(HTTPAPIEX_SAS_Create_first_string_copy_fails)
{
    // arrange
    HTTPAPIEX_SAS_HANDLE handle;

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(__LINE__);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    handle = HTTPAPIEX_SAS_Create_From_String(TEST_KEY, TEST_URI_RESOURCE, TEST_KEY_NAME);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_HTTPAPIEXSAS_06_004: [If there are any other errors in the instantiation of this handle then HTTPAPIEX_SAS_Create shall return NULL.]*/
TEST_FUNCTION(HTTPAPIEX_SAS_Create_second_string_copy_fails)
{
    // arrange
    HTTPAPIEX_SAS_HANDLE handle;

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(__LINE__);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    handle = HTTPAPIEX_SAS_Create_From_String(TEST_KEY, TEST_URI_RESOURCE, TEST_KEY_NAME);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_HTTPAPIEXSAS_06_004: [If there are any other errors in the instantiation of this handle then HTTPAPIEX_SAS_Create shall return NULL.]*/
TEST_FUNCTION(HTTPAPIEX_SAS_Create_third_string_copy_fails)
{
    // arrange
    HTTPAPIEX_SAS_HANDLE handle;

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(__LINE__);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    handle = HTTPAPIEX_SAS_Create_From_String(TEST_KEY, TEST_URI_RESOURCE, TEST_KEY_NAME);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_HTTPAPIEXSAS_06_006: [HTTAPIEX_SAS_Destroy shall deallocate any structures denoted by the parameter handle.]*/
TEST_FUNCTION(HTTPAPIEX_SAS_Destroy_frees_underlying_strings)
{
    // arrange
    HTTPAPIEX_SAS_HANDLE handle;

    setupSAS_Create_happy_path(true);
    handle = HTTPAPIEX_SAS_Create(TEST_KEY_HANDLE, TEST_URIRESOURCE_HANDLE, TEST_KEYNAME_HANDLE);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    HTTPAPIEX_SAS_Destroy(handle);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_HTTPAPIEXSAS_06_005: [If the parameter handle is NULL then HTTAPIEX_SAS_Destroy shall do nothing and return.]*/
TEST_FUNCTION(HTTPAPIEX_SAS_destroy_with_null_succeeds)
{
    // arrange
    // act
    HTTPAPIEX_SAS_Destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_HTTPAPIEXSAS_06_007: [If the parameter sasHandle is NULL then HTTPAPIEX_SAS_ExecuteRequest shall simply invoke HTTPAPIEX_ExecuteRequest with the remaining parameters (following sasHandle) as its arguments and shall return immediately with the result of that call as the result of HTTPAPIEX_SAS_ExecuteRequest.]*/
TEST_FUNCTION(HTTPAPIEX_SAS_invoke_executerequest_with_null_sas_handle_succeeds)
{
    HTTPAPIEX_RESULT result = HTTPAPIEX_ERROR;
    unsigned int statusCode;

    STRICT_EXPECTED_CALL(HTTPAPIEX_ExecuteRequest(TEST_HTTPAPIEX_HANDLE, TEST_HTTPAPI_REQUEST_TYPE, TEST_CHAR_ARRAY, TEST_REQUEST_HTTP_HEADERS_HANDLE, TEST_REQUEST_CONTENT, &statusCode, TEST_RESPONSE_HTTP_HEADERS_HANDLE, TEST_RESPONSE_CONTENT)).SetReturn(HTTPAPIEX_OK);
    // act
    result = HTTPAPIEX_SAS_ExecuteRequest(NULL, TEST_HTTPAPIEX_HANDLE, TEST_HTTPAPI_REQUEST_TYPE, TEST_CHAR_ARRAY, TEST_REQUEST_HTTP_HEADERS_HANDLE, TEST_REQUEST_CONTENT, &statusCode, TEST_RESPONSE_HTTP_HEADERS_HANDLE, TEST_RESPONSE_CONTENT);

    // assert
    ASSERT_ARE_EQUAL(HTTPAPIEX_RESULT, result, HTTPAPIEX_OK);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_HTTPAPIEXSAS_06_008: [if the parameter requestHttpHeadersHandle is NULL then fallthrough.]*/
TEST_FUNCTION(HTTPAPIEX_SAS_invoke_executerequest_with_null_request_http_headers_handle_succeeds)
{

    HTTPAPIEX_RESULT result;
    unsigned int statusCode;
    HTTPAPIEX_SAS_HANDLE sasHandle;

    // arrange
    setupSAS_Create_happy_path(true);
    sasHandle = HTTPAPIEX_SAS_Create(TEST_KEY_HANDLE, TEST_URIRESOURCE_HANDLE, TEST_KEYNAME_HANDLE);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(HTTPAPIEX_ExecuteRequest(TEST_HTTPAPIEX_HANDLE, TEST_HTTPAPI_REQUEST_TYPE, TEST_CHAR_ARRAY, NULL, TEST_REQUEST_CONTENT, &statusCode, TEST_RESPONSE_HTTP_HEADERS_HANDLE, TEST_RESPONSE_CONTENT)).SetReturn(HTTPAPIEX_OK);

    // act
    result = HTTPAPIEX_SAS_ExecuteRequest(sasHandle, TEST_HTTPAPIEX_HANDLE, TEST_HTTPAPI_REQUEST_TYPE, TEST_CHAR_ARRAY, NULL, TEST_REQUEST_CONTENT, &statusCode, TEST_RESPONSE_HTTP_HEADERS_HANDLE, TEST_RESPONSE_CONTENT);

    // assert
    ASSERT_ARE_EQUAL(HTTPAPIEX_RESULT, result, HTTPAPIEX_OK);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    HTTPAPIEX_SAS_Destroy(sasHandle);
}

/*Tests_SRS_HTTPAPIEXSAS_06_009: [HTTPHeaders_FindHeaderValue shall be invoked with the requestHttpHeadersHandle as its first argument and the string "Authorization" as its second argument.]*/
/*Tests_SRS_HTTPAPIEXSAS_06_010: [If the return result of the invocation of HTTPHeaders_FindHeaderValue is NULL then fallthrough.]*/
TEST_FUNCTION(HTTPAPIEX_SAS_invoke_executerequest_findheadervalues_returns_null_succeeds)
{

    HTTPAPIEX_RESULT result;
    unsigned int statusCode;
    HTTPAPIEX_SAS_HANDLE sasHandle;

    // arrange
    setupSAS_Create_happy_path(true);
    sasHandle = HTTPAPIEX_SAS_Create(TEST_KEY_HANDLE, TEST_URIRESOURCE_HANDLE, TEST_KEYNAME_HANDLE);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(HTTPHeaders_FindHeaderValue(TEST_REQUEST_HTTP_HEADERS_HANDLE, "Authorization")).SetReturn(TEST_CONST_CHAR_STAR_NULL);
    STRICT_EXPECTED_CALL(HTTPAPIEX_ExecuteRequest(TEST_HTTPAPIEX_HANDLE, TEST_HTTPAPI_REQUEST_TYPE, TEST_CHAR_ARRAY, TEST_REQUEST_HTTP_HEADERS_HANDLE, TEST_REQUEST_CONTENT, &statusCode, TEST_RESPONSE_HTTP_HEADERS_HANDLE, TEST_RESPONSE_CONTENT)).SetReturn(HTTPAPIEX_OK);

    // act
    result = HTTPAPIEX_SAS_ExecuteRequest(sasHandle, TEST_HTTPAPIEX_HANDLE, TEST_HTTPAPI_REQUEST_TYPE, TEST_CHAR_ARRAY, TEST_REQUEST_HTTP_HEADERS_HANDLE, TEST_REQUEST_CONTENT, &statusCode, TEST_RESPONSE_HTTP_HEADERS_HANDLE, TEST_RESPONSE_CONTENT);

    // assert
    ASSERT_ARE_EQUAL(HTTPAPIEX_RESULT, result, HTTPAPIEX_OK);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    HTTPAPIEX_SAS_Destroy(sasHandle);
}

/*Tests_SRS_HTTPAPIEXSAS_06_018: [A value of type time_t that shall be known as currentTime is obtained from calling get_time.]*/
/*Tests_SRS_HTTPAPIEXSAS_06_019: [If the value of currentTime is (time_t)-1 is then fallthrough.]*/
TEST_FUNCTION(HTTPAPIEX_SAS_invoke_executerequest_get_time_fails)
{

    HTTPAPIEX_RESULT result;
    unsigned int statusCode;
    HTTPAPIEX_SAS_HANDLE sasHandle;

    // arrange
    setupSAS_Create_happy_path(true);
    sasHandle = HTTPAPIEX_SAS_Create(TEST_KEY_HANDLE, TEST_URIRESOURCE_HANDLE, TEST_KEYNAME_HANDLE);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(HTTPHeaders_FindHeaderValue(TEST_REQUEST_HTTP_HEADERS_HANDLE, "Authorization")).SetReturn(TEST_CHAR_ARRAY);
    STRICT_EXPECTED_CALL(get_time(NULL)).SetReturn((time_t)-1);
    STRICT_EXPECTED_CALL(HTTPAPIEX_ExecuteRequest(TEST_HTTPAPIEX_HANDLE, TEST_HTTPAPI_REQUEST_TYPE, TEST_CHAR_ARRAY, TEST_REQUEST_HTTP_HEADERS_HANDLE, TEST_REQUEST_CONTENT, &statusCode, TEST_RESPONSE_HTTP_HEADERS_HANDLE, TEST_RESPONSE_CONTENT)).SetReturn(HTTPAPIEX_OK);

    // act
    result = HTTPAPIEX_SAS_ExecuteRequest(sasHandle, TEST_HTTPAPIEX_HANDLE, TEST_HTTPAPI_REQUEST_TYPE, TEST_CHAR_ARRAY, TEST_REQUEST_HTTP_HEADERS_HANDLE, TEST_REQUEST_CONTENT, &statusCode, TEST_RESPONSE_HTTP_HEADERS_HANDLE, TEST_RESPONSE_CONTENT);

    // assert
    ASSERT_ARE_EQUAL(HTTPAPIEX_RESULT, result, HTTPAPIEX_OK);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    HTTPAPIEX_SAS_Destroy(sasHandle);
}

/*Tests_SRS_HTTPAPIEXSAS_06_017: [If state->key is prefixed with "sas=", SharedAccessSignature will be used rather than created.  STRING_construct will be invoked.]*/
TEST_FUNCTION(HTTPAPIEX_SAS_invoke_executerequest_sas_is_provided_succeeds)
{

    HTTPAPIEX_RESULT result;
    unsigned int statusCode;
    HTTPAPIEX_SAS_HANDLE sasHandle;

    // arrange
    setupSAS_Create_happy_path_provide_key(true, true);
    sasHandle = HTTPAPIEX_SAS_Create(TEST_KEY_HANDLE, TEST_URIRESOURCE_HANDLE, TEST_KEYNAME_HANDLE);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(HTTPHeaders_FindHeaderValue(TEST_REQUEST_HTTP_HEADERS_HANDLE, "Authorization")).SetReturn(TEST_CHAR_ARRAY);
    STRICT_EXPECTED_CALL(get_time(NULL)).SetReturn(3600);
    STRICT_EXPECTED_CALL(STRING_construct(TEST_SAS));
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)).SetReturn(TEST_CHAR_ARRAY);
    STRICT_EXPECTED_CALL(HTTPHeaders_ReplaceHeaderNameValuePair(TEST_REQUEST_HTTP_HEADERS_HANDLE, "Authorization", IGNORED_PTR_ARG)).SetReturn(HTTP_HEADERS_ERROR);
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(HTTPAPIEX_ExecuteRequest(TEST_HTTPAPIEX_HANDLE, TEST_HTTPAPI_REQUEST_TYPE, TEST_CHAR_ARRAY, TEST_REQUEST_HTTP_HEADERS_HANDLE, TEST_REQUEST_CONTENT, &statusCode, TEST_RESPONSE_HTTP_HEADERS_HANDLE, TEST_RESPONSE_CONTENT)).SetReturn(HTTPAPIEX_OK);

    // act
    result = HTTPAPIEX_SAS_ExecuteRequest(sasHandle, TEST_HTTPAPIEX_HANDLE, TEST_HTTPAPI_REQUEST_TYPE, TEST_CHAR_ARRAY, TEST_REQUEST_HTTP_HEADERS_HANDLE, TEST_REQUEST_CONTENT, &statusCode, TEST_RESPONSE_HTTP_HEADERS_HANDLE, TEST_RESPONSE_CONTENT);

    // assert
    ASSERT_ARE_EQUAL(HTTPAPIEX_RESULT, result, HTTPAPIEX_OK);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    HTTPAPIEX_SAS_Destroy(sasHandle);
}

/*Tests_SRS_HTTPAPIEXSAS_06_011: [SASToken_Create shall be invoked.]*/
/*Tests_SRS_HTTPAPIEXSAS_06_012: [If the return result of SASToken_Create is NULL then fallthrough.]*/
TEST_FUNCTION(HTTPAPIEX_SAS_invoke_executerequest_sastoken_create_returns_null_succeeds)
{

    HTTPAPIEX_RESULT result;
    unsigned int statusCode;
    HTTPAPIEX_SAS_HANDLE sasHandle;

    // arrange
    setupSAS_Create_happy_path(true);
    sasHandle = HTTPAPIEX_SAS_Create(TEST_KEY_HANDLE, TEST_URIRESOURCE_HANDLE, TEST_KEYNAME_HANDLE);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(HTTPHeaders_FindHeaderValue(TEST_REQUEST_HTTP_HEADERS_HANDLE, "Authorization")).SetReturn(TEST_CHAR_ARRAY);
    STRICT_EXPECTED_CALL(get_time(NULL)).SetReturn(3600);
    STRICT_EXPECTED_CALL(get_difftime(3600, 0)).SetReturn(3600);
    STRICT_EXPECTED_CALL(SASToken_CreateString(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, TEST_EXPIRY)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(HTTPAPIEX_ExecuteRequest(TEST_HTTPAPIEX_HANDLE, TEST_HTTPAPI_REQUEST_TYPE, TEST_CHAR_ARRAY, TEST_REQUEST_HTTP_HEADERS_HANDLE, TEST_REQUEST_CONTENT, &statusCode, TEST_RESPONSE_HTTP_HEADERS_HANDLE, TEST_RESPONSE_CONTENT)).SetReturn(HTTPAPIEX_OK);

    // act
    result = HTTPAPIEX_SAS_ExecuteRequest(sasHandle, TEST_HTTPAPIEX_HANDLE, TEST_HTTPAPI_REQUEST_TYPE, TEST_CHAR_ARRAY, TEST_REQUEST_HTTP_HEADERS_HANDLE, TEST_REQUEST_CONTENT, &statusCode, TEST_RESPONSE_HTTP_HEADERS_HANDLE, TEST_RESPONSE_CONTENT);

    // assert
    ASSERT_ARE_EQUAL(HTTPAPIEX_RESULT, result, HTTPAPIEX_OK);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    HTTPAPIEX_SAS_Destroy(sasHandle);
}

/*Tests_SRS_HTTPAPIEXSAS_06_013: [HTTPHeaders_ReplaceHeaderNameValuePair shall be invoked with "Authorization" as its second argument and STRING_c_str (newSASToken) as its third argument.]*/
/*Tests_SRS_HTTPAPIEXSAS_06_014: [If the result of the invocation of HTTPHeaders_ReplaceHeaderNameValuePair is NOT HTTP_HEADERS_OK then fallthrough.]*/
/*Tests_SRS_HTTPAPIEXSAS_06_015: [STRING_delete(newSASToken) will be invoked.]*/
TEST_FUNCTION(HTTPAPIEX_SAS_invoke_executerequest_replace_header_name_value_pair_fails_succeeds)
{

    HTTPAPIEX_RESULT result;
    unsigned int statusCode;
    HTTPAPIEX_SAS_HANDLE sasHandle;

    // arrange
    setupSAS_Create_happy_path(true);
    sasHandle = HTTPAPIEX_SAS_Create(TEST_KEY_HANDLE, TEST_URIRESOURCE_HANDLE, TEST_KEYNAME_HANDLE);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(HTTPHeaders_FindHeaderValue(TEST_REQUEST_HTTP_HEADERS_HANDLE, "Authorization")).SetReturn(TEST_CHAR_ARRAY);
    STRICT_EXPECTED_CALL(get_time(NULL)).SetReturn(3600);
    STRICT_EXPECTED_CALL(get_difftime(3600, 0)).SetReturn(3600);
    STRICT_EXPECTED_CALL(SASToken_CreateString(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, TEST_EXPIRY));
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)).SetReturn(TEST_CHAR_ARRAY);
    STRICT_EXPECTED_CALL(HTTPHeaders_ReplaceHeaderNameValuePair(TEST_REQUEST_HTTP_HEADERS_HANDLE, "Authorization", IGNORED_PTR_ARG)).SetReturn(HTTP_HEADERS_ERROR);
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(HTTPAPIEX_ExecuteRequest(TEST_HTTPAPIEX_HANDLE, TEST_HTTPAPI_REQUEST_TYPE, TEST_CHAR_ARRAY, TEST_REQUEST_HTTP_HEADERS_HANDLE, TEST_REQUEST_CONTENT, &statusCode, TEST_RESPONSE_HTTP_HEADERS_HANDLE, TEST_RESPONSE_CONTENT)).SetReturn(HTTPAPIEX_OK);

    // act
    result = HTTPAPIEX_SAS_ExecuteRequest(sasHandle, TEST_HTTPAPIEX_HANDLE, TEST_HTTPAPI_REQUEST_TYPE, TEST_CHAR_ARRAY, TEST_REQUEST_HTTP_HEADERS_HANDLE, TEST_REQUEST_CONTENT, &statusCode, TEST_RESPONSE_HTTP_HEADERS_HANDLE, TEST_RESPONSE_CONTENT);

    // assert
    ASSERT_ARE_EQUAL(HTTPAPIEX_RESULT, result, HTTPAPIEX_OK);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    HTTPAPIEX_SAS_Destroy(sasHandle);
}

/*Tests_SRS_HTTPAPIEXSAS_06_016: [HTTPAPIEX_ExecuteRequest with the remaining parameters (following sasHandle) as its arguments will be invoked and the result of that call is the result of HTTPAPIEX_SAS_ExecuteRequest.]*/
TEST_FUNCTION(HTTPAPIEX_SAS_invoke_executerequest_replace_header_name_value_pair_succeeds_succeeds)
{

    HTTPAPIEX_RESULT result;
    unsigned int statusCode;
    HTTPAPIEX_SAS_HANDLE sasHandle;

    // arrange
    setupSAS_Create_happy_path(true);
    sasHandle = HTTPAPIEX_SAS_Create(TEST_KEY_HANDLE, TEST_URIRESOURCE_HANDLE, TEST_KEYNAME_HANDLE);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(HTTPHeaders_FindHeaderValue(TEST_REQUEST_HTTP_HEADERS_HANDLE, "Authorization")).SetReturn(TEST_CHAR_ARRAY);
    STRICT_EXPECTED_CALL(get_time(NULL)).SetReturn((time_t)3600);
    STRICT_EXPECTED_CALL(get_difftime(3600, 0)).SetReturn(3600);
    STRICT_EXPECTED_CALL(SASToken_CreateString(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, TEST_EXPIRY));
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)).SetReturn(TEST_CHAR_ARRAY);
    STRICT_EXPECTED_CALL(HTTPHeaders_ReplaceHeaderNameValuePair(TEST_REQUEST_HTTP_HEADERS_HANDLE, "Authorization", TEST_CHAR_ARRAY)).SetReturn(HTTP_HEADERS_OK);
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(HTTPAPIEX_ExecuteRequest(TEST_HTTPAPIEX_HANDLE, TEST_HTTPAPI_REQUEST_TYPE, TEST_CHAR_ARRAY, TEST_REQUEST_HTTP_HEADERS_HANDLE, TEST_REQUEST_CONTENT, &statusCode, TEST_RESPONSE_HTTP_HEADERS_HANDLE, TEST_RESPONSE_CONTENT));

    // act
    result = HTTPAPIEX_SAS_ExecuteRequest(sasHandle, TEST_HTTPAPIEX_HANDLE, TEST_HTTPAPI_REQUEST_TYPE, TEST_CHAR_ARRAY, TEST_REQUEST_HTTP_HEADERS_HANDLE, TEST_REQUEST_CONTENT, &statusCode, TEST_RESPONSE_HTTP_HEADERS_HANDLE, TEST_RESPONSE_CONTENT);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(HTTPAPIEX_RESULT, result, HTTPAPIEX_OK);

    // Cleanup
    HTTPAPIEX_SAS_Destroy(sasHandle);
}

END_TEST_SUITE(httpapiexsas_unittests)
