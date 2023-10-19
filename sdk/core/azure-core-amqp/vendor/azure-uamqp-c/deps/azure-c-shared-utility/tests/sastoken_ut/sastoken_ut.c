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

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#ifdef __cplusplus
#include <cstdio>
#include <ctime>
#else
#include <stdio.h>
#include <time.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_stdint.h"

#define ENABLE_MOCKS

#include "azure_c_shared_utility/gballoc.h"

#include "azure_c_shared_utility/hmacsha256.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/buffer_.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/urlencode.h"
#include "azure_c_shared_utility/azure_base64.h"
#include "azure_c_shared_utility/agenttime.h"

#undef ENABLE_MOCKS

IMPLEMENT_UMOCK_C_ENUM_TYPE(HMACSHA256_RESULT, HMACSHA256_RESULT_VALUES);

double my_get_difftime(time_t stopTime, time_t startTime)
{
    return (double)(stopTime - startTime);
}

STRING_HANDLE my_STRING_new(void)
{
    return (STRING_HANDLE)malloc(1);
}

BUFFER_HANDLE my_BUFFER_new(void)
{
    return (BUFFER_HANDLE)malloc(1);
}

STRING_HANDLE my_Base64_Encode(BUFFER_HANDLE input)
{
    (void)input;
    return (STRING_HANDLE)malloc(1);
}

BUFFER_HANDLE my_Azure_Base64_Decode(const char* source)
{
    (void)source;
    return (BUFFER_HANDLE)malloc(1);
}

STRING_HANDLE my_URL_Encode(STRING_HANDLE input)
{
    (void)input;
    return (STRING_HANDLE)malloc(1);
}

#include "azure_c_shared_utility/sastoken.h"

#define TEST_STRING_HANDLE (STRING_HANDLE)0x46
#define TEST_NULL_STRING_HANDLE (STRING_HANDLE)0x00
#define TEST_BUFFER_HANDLE (BUFFER_HANDLE)0x47
#define TEST_NULL_BUFFER_HANDLE (BUFFER_HANDLE)0x00
#define TEST_SCOPE_HANDLE (STRING_HANDLE)0x48
#define TEST_KEY_HANDLE (STRING_HANDLE)0x49
#define TEST_KEYNAME_HANDLE (STRING_HANDLE)0x50
#define TEST_HASH_HANDLE (BUFFER_HANDLE)0x51
#define TEST_TOBEHASHED_HANDLE (STRING_HANDLE)0x52
#define TEST_RESULT_HANDLE (STRING_HANDLE)0x53
#define TEST_BASE64SIGNATURE_HANDLE (STRING_HANDLE)0x54
#define TEST_URLENCODEDSIGNATURE_HANDLE (STRING_HANDLE)0x55
#define TEST_DECODEDKEY_HANDLE (BUFFER_HANDLE)0x56
#define TEST_TIME_T ((time_t)3600)
#define TEST_PTR_DECODEDKEY (unsigned char*)0x123
#define TEST_LENGTH_DECODEDKEY (size_t)32
#define TEST_PTR_TOBEHASHED (const char*)0x456
#define TEST_LENGTH_TOBEHASHED (size_t)456
#define TEST_EXPIRY ((uint64_t)7200)
#define TEST_EXPIRY_LARGE 18446744073709551615ul
#define TEST_EXPIRY_LARGE_STRING "18446744073709551615"
#define TEST_LATER_TIME (time_t) 11
#define TEST_EARLY_TIME (time_t) 10

static const char* TEST_STRING_VALUE = "Test string value";
static const char* TEST_NULL_STRING_VALUE = 0x00;
static char TEST_CHAR_ARRAY[10] = "ABCD";
static unsigned char TEST_UNSIGNED_CHAR_ARRAY[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };
static char TEST_TOKEN_EXPIRATION_TIME[32] = "7200";
static char TEST_TOKEN_EXPIRATION_TIME_LARGE[32] = TEST_EXPIRY_LARGE_STRING;

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

#ifdef __cplusplus
extern "C"
{
#endif

    static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
    {
        ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
    }

#ifdef __cplusplus
}
#endif

BEGIN_TEST_SUITE(sastoken_unittests)

TEST_SUITE_INITIALIZE(TestClassInitialize)
{
    int result;
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    REGISTER_UMOCK_ALIAS_TYPE(time_t, long long);
    REGISTER_UMOCK_ALIAS_TYPE(time_t*, void*);
    REGISTER_UMOCK_ALIAS_TYPE(STRING_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(BUFFER_HANDLE, void*);

    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result, "umocktypes_stdint_register_types");

    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);
    REGISTER_TYPE(HMACSHA256_RESULT, HMACSHA256_RESULT);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);

    REGISTER_GLOBAL_MOCK_HOOK(STRING_new, my_STRING_new);
    REGISTER_GLOBAL_MOCK_RETURN(STRING_concat, 0);
    REGISTER_GLOBAL_MOCK_RETURN(STRING_concat_with_STRING, 0);
    REGISTER_GLOBAL_MOCK_RETURN(STRING_c_str, &TEST_CHAR_ARRAY[0]);
    REGISTER_GLOBAL_MOCK_RETURN(STRING_length, 1);
    REGISTER_GLOBAL_MOCK_RETURN(STRING_copy, 0);

    REGISTER_GLOBAL_MOCK_HOOK(BUFFER_new, my_BUFFER_new);
    REGISTER_GLOBAL_MOCK_RETURN(BUFFER_u_char, &TEST_UNSIGNED_CHAR_ARRAY[0]);
    REGISTER_GLOBAL_MOCK_RETURN(BUFFER_length, 1);

    REGISTER_GLOBAL_MOCK_HOOK(Azure_Base64_Encode, my_Base64_Encode);
    REGISTER_GLOBAL_MOCK_HOOK(Azure_Base64_Decode, my_Azure_Base64_Decode);
    REGISTER_GLOBAL_MOCK_HOOK(URL_Encode, my_URL_Encode);
    REGISTER_GLOBAL_MOCK_RETURN(HMACSHA256_ComputeHash, HMACSHA256_OK);
    REGISTER_GLOBAL_MOCK_RETURN(uint64_tToString, 0);

    REGISTER_GLOBAL_MOCK_RETURN(get_time, TEST_TIME_T);
    REGISTER_GLOBAL_MOCK_HOOK(get_difftime, my_get_difftime);

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

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/*Tests_SRS_SASTOKEN_25_025: [**SASToken_Validate shall get the SASToken value by invoking STRING_c_str on the handle.**]***/
TEST_FUNCTION(SASToken_validate_null_handle_fails)
{
    // arrange
    STRING_HANDLE handle = NULL;
    bool result;
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_NULL_STRING_HANDLE)).SetReturn(NULL);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_25_025: [**SASToken_Validate shall get the SASToken value by invoking STRING_c_str on the handle.**]***/
TEST_FUNCTION(SASToken_validate_null_string_valid_handle_fails)
{
    // arrange
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_NULL_STRING_VALUE);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_25_025: [**SASToken_Validate shall get the SASToken value by invoking STRING_c_str on the handle.**]***/
TEST_FUNCTION(SASToken_validate_se_improper_format_1_fails)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sr=TESTSR&sig=TESTSIG&se0123456789";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}
/*Tests_SRS_SASTOKEN_25_027: [**If SASTOKEN does not obey the SASToken format then SASToken_Validate shall return false.**]*/
TEST_FUNCTION(SASToken_validate_se_improper_format_2_fails)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sr=TESTSR&sig=TESTSIG&se";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_se_improper_format_3_fails)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sr=TESTSR&sig=TESTSIG&se=";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_se_improper_format_4_fails)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sr=TESTSR&sig=TESTSIGse=0123456789";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_se_improper_format_5_fails)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sr=TESTSR&se0123456789&sig=TESTSIG";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_25_028: [**SASToken_validate shall check for the presence of sr, se and sig from the token and return false if not found**]*/
TEST_FUNCTION(SASToken_validate_improper_format_no_se_fails)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sr=TESTSR&sig=TESTSIG";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_improper_format_no_sr_fails)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature se=0123456789&sig=TESTSIG";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_improper_format_no_sig_fails)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature se=0123456789&sr=TESTSR";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_sr_improper_format_1_fails)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature srTESTSR&sig=TESTSIG&se=0123456789";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_sr_improper_format_2_fails)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sr&sig=TESTSIG&se=0123456789";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_sr_improper_format_3_fails)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sr=&sig=TESTSIG&se=0123456789";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_sr_improper_format_4_fails)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignaturesr=TESTSR&sig=TESTSIGse=0123456789";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_sr_improper_format_5_fails)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature srTESTSR&se=0123456789&sig=TESTSIG";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_sig_improper_format_1_fails)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sr=TESTSR&sigTESTSIG&se=0123456789";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_sig_improper_format_2_fails)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sr=TESTSR&sig&se=0123456789";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_sig_improper_format_3_fails)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sr=TESTSR&sig=&se=0123456789";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_sig_improper_format_4_fails)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sr=TESTSRsig=TESTSIGse=0123456789";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_sig_improper_format_5_fails)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sr=TESTSR&se0123456789&sig=TESTSIG";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_25_030: [**SASToken_validate shall return true only if the format is obeyed and the token has not yet expired **]*/
TEST_FUNCTION(SASToken_validate_proper_format_1_pass)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature se=0123456789&sr=TESTSR&sig=TESTSIG";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(get_time(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(get_difftime(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).IgnoreAllArguments().SetReturn(TEST_TIME_T);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)).IgnoreAllArguments();

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_25_030: [**SASToken_validate shall return true only if the format is obeyed and the token has not yet expired **]*/
TEST_FUNCTION(SASToken_validate_proper_format_with_skn_1_pass)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature sr=devices.net/devices/tmp_device&sig=TESTSIG&se=0123456789&skn=";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(get_time(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(get_difftime(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).IgnoreAllArguments().SetReturn(TEST_TIME_T);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)).IgnoreAllArguments();

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_proper_format_with_skn_expired_se_fail)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature sr=devices.net/devices/tmp_device&sig=TESTSIG&se=011&skn=";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(get_time(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(get_difftime(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).IgnoreAllArguments().SetReturn(TEST_TIME_T);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)).IgnoreAllArguments();

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_proper_format_2_pass)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sr=TESTSR&se=0123456789&sig=TESTSIG";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(get_time(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(get_difftime(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).IgnoreAllArguments().SetReturn(TEST_TIME_T);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)).IgnoreAllArguments();

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_proper_format_3_pass)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sig=TESTSIG&sr=TESTSR&se=0123456789";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(get_time(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(get_difftime(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).IgnoreAllArguments().SetReturn(TEST_TIME_T);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)).IgnoreAllArguments();

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_not_expired_pass)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sig=TESTSIG&sr=TESTSR&se=11";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(get_time(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(get_difftime(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).IgnoreAllArguments().SetReturn(TEST_EARLY_TIME);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)).IgnoreAllArguments();

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

// Testing for Y2K38.
TEST_FUNCTION(SASToken_validate_not_expired_large_int_pass)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sig=TESTSIG&sr=TESTSR&se=" TEST_EXPIRY_LARGE_STRING;
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(get_time(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(get_difftime(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).IgnoreAllArguments().SetReturn(TEST_EARLY_TIME);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)).IgnoreAllArguments();

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_25_029: [**SASToken_validate shall check for expiry time from token and if token has expired then would return false **]*/
TEST_FUNCTION(SASToken_validate_expired_fail)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sig=TESTSIG&sr=TESTSR&se=10";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(get_time(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(get_difftime(TEST_TIME_T, IGNORED_NUM_ARG)).IgnoreAllArguments().SetReturn(TEST_LATER_TIME);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)).IgnoreAllArguments();

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_invalid_expiry_1_fail)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sig=TESTSIG&sr=TESTSR&se=10A";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)).IgnoreAllArguments();

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_invalid_expiry_2_fail)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sig=TESTSIG&sr=TESTSR&se=-10";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)).IgnoreAllArguments();

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_invalid_expiry_3_fail)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sig=TESTSIG&sr=TESTSR&se=0";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)).IgnoreAllArguments();

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_invalid_expiry_4_fail)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sig=TESTSIG&sr=TESTSR&se=A0";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)).IgnoreAllArguments();

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_validate_invalid_expiry_5_fail)
{
    // arrange
    const char* TEST_INVALID_SE = "SharedAccessSignature=SharedAccessSignature sig=TESTSIG&sr=TESTSR&se=1A0";
    size_t TEST_INVALID_SE_LENGTH = strlen(TEST_INVALID_SE);
    STRING_HANDLE handle = TEST_STRING_HANDLE;
    bool result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(STRING_c_str(handle)).SetReturn(TEST_INVALID_SE);
    STRICT_EXPECTED_CALL(STRING_length(handle)).SetReturn(TEST_INVALID_SE_LENGTH);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)).IgnoreAllArguments();

    // act
    result = SASToken_Validate(handle);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_06_001: [If key is NULL then SASToken_Create shall return NULL.]*/
TEST_FUNCTION(SASToken_Create_null_key_fails)
{
    // arrange
    STRING_HANDLE handle;

    // act
    handle = SASToken_Create(TEST_NULL_STRING_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_06_003: [If scope is NULL then SASToken_Create shall return NULL.]*/
TEST_FUNCTION(SASToken_Create_null_scope_fails)
{
    // arrange
    STRING_HANDLE handle;

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_NULL_STRING_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_06_007: [keyName is optional and can be set to NULL.]*/
TEST_FUNCTION(SASToken_Create_null_keyName_succeeds)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(NULL)).SetReturn(NULL);

    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(BUFFER_u_char(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(Azure_Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&sig="));
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&se="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));


    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_NULL_STRING_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NOT_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_06_030: [If there is an error in the decoding then SASToken_Create shall return NULL.]*/
TEST_FUNCTION(SASToken_Create_decoded_key_fails)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_NULL_BUFFER_HANDLE);

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_06_029: [The key parameter is decoded from base64.]*/
/*Tests_SRS_SASTOKEN_06_026: [If the conversion to string form fails for any reason then SASToken_Create shall return NULL.]*/
TEST_FUNCTION(SASToken_Create_uint64_tToString_fails)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);

    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME)).SetReturn(-1);

    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_06_024: [The uint64_t value ((uint64_t) (difftime(get_time(NULL),0) + 3600)) is converted to a string form.]*/
TEST_FUNCTION(SASToken_Create_buffer_new_fails)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);

    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(NULL);

    STRICT_EXPECTED_CALL(STRING_delete(NULL));
    STRICT_EXPECTED_CALL(BUFFER_delete(NULL));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_Create_first_string_new_fails)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);

    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(NULL);

    STRICT_EXPECTED_CALL(STRING_delete(NULL));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_Create_second_string_new_fails)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);

    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(NULL);

    STRICT_EXPECTED_CALL(STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_Create_build_to_be_hashed_part1_fails)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);

    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, IGNORED_PTR_ARG)).SetReturn(1);

    STRICT_EXPECTED_CALL(STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_06_009: [The scope is the basis for creating a STRING_HANDLE.]*/
TEST_FUNCTION(SASToken_Create_build_to_be_hashed_part2_fails)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);

    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, "\n")).SetReturn(1);

    STRICT_EXPECTED_CALL(STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_06_010: [A "\n" is appended to that string.]*/
TEST_FUNCTION(SASToken_Create_build_to_be_hashed_part3_fails)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);

    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME)).SetReturn(1);

    STRICT_EXPECTED_CALL(STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_06_011: [tokenExpirationTime is appended to that string.]*/
/*Tests_SRS_SASTOKEN_06_013: [If an error is returned from the HMAC256 function then NULL is returned from SASToken_Create.]*/
TEST_FUNCTION(SASToken_Create_HMAC256_fails)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);

    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(BUFFER_u_char(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).SetReturn(HMACSHA256_ERROR);

    STRICT_EXPECTED_CALL(STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(NULL));
    STRICT_EXPECTED_CALL(STRING_delete(NULL));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}


/*Tests_SRS_SASTOKEN_06_012: [An HMAC256 hash is calculated using the decodedKey, over toBeHashed.]*/
/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
TEST_FUNCTION(SASToken_Create_HMAC256_passes_signature_encode_fails)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);

    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(BUFFER_u_char(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(Azure_Base64_Encode(TEST_HASH_HANDLE)).SetReturn(NULL);

    STRICT_EXPECTED_CALL(STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(NULL));
    STRICT_EXPECTED_CALL(STRING_delete(NULL));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
/*Tests_SRS_SASTOKEN_06_015: [The hash is base 64 encoded.]*/
TEST_FUNCTION(SASToken_Create_building_token_signature_url_encoding_fails)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);

    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(BUFFER_u_char(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(Azure_Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(NULL);

    STRICT_EXPECTED_CALL(STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(NULL));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
/*Tests_SRS_SASTOKEN_06_028: [base64Signature shall be url encoded.]*/
TEST_FUNCTION(SASToken_Create_building_token_copy_scope_identifier_fails)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);

    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(BUFFER_u_char(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(Azure_Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr=")).SetReturn(1);

    STRICT_EXPECTED_CALL(STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_06_016: [The string "SharedAccessSignature sr=" is the first part of the result of SASToken_Create.]*/
/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
TEST_FUNCTION(SASToken_Create_building_token_concat_scope_fails)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);

    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(BUFFER_u_char(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(Azure_Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, IGNORED_PTR_ARG)).SetReturn(1);

    STRICT_EXPECTED_CALL(STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
/*Tests_SRS_SASTOKEN_06_017: [The scope parameter is appended to result.]*/
TEST_FUNCTION(SASToken_Create_building_token_concat_signature_identifier_fails)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);

    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(BUFFER_u_char(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(Azure_Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&sig=")).SetReturn(1);

    STRICT_EXPECTED_CALL(STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
/*Tests_SRS_SASTOKEN_06_018: [The string "&sig=" is appended to result.]*/
TEST_FUNCTION(SASToken_Create_building_token_concat_signature_fails)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);

    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(BUFFER_u_char(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(Azure_Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&sig="));
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_URLENCODEDSIGNATURE_HANDLE)).SetReturn(1);

    STRICT_EXPECTED_CALL(STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
/*Tests_SRS_SASTOKEN_06_019: [The string urlEncodedSignature shall be appended to result.]*/
TEST_FUNCTION(SASToken_Create_building_token_concat_token_expiration_time_identifier_fails)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);

    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(BUFFER_u_char(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(Azure_Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&sig="));
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&se=")).SetReturn(1);

    STRICT_EXPECTED_CALL(STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
/*Tests_SRS_SASTOKEN_06_020: [The string "&se=" shall be appended to result.]*/
TEST_FUNCTION(SASToken_Create_building_token_concat_token_expiration_time_fails)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);

    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(BUFFER_u_char(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(Azure_Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&sig="));
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&se="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, TEST_TOKEN_EXPIRATION_TIME)).SetReturn(1);

    STRICT_EXPECTED_CALL(STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
/*Tests_SRS_SASTOKEN_06_021: [tokenExpirationTime is appended to result.]*/
TEST_FUNCTION(SASToken_Create_building_token_concat_keyname_identifier_fails)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);

    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(BUFFER_u_char(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(Azure_Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&sig="));
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&se="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&skn=")).SetReturn(1);

    STRICT_EXPECTED_CALL(STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
/*Tests_SRS_SASTOKEN_06_022: [If keyName is non-NULL, the string "&skn=" is appended to result.]*/
TEST_FUNCTION(SASToken_Create_building_token_concat_keyname_fails)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);

    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(BUFFER_u_char(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(Azure_Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&sig="));
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&se="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&skn="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, IGNORED_PTR_ARG)).SetReturn(1);

    STRICT_EXPECTED_CALL(STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Tests_SRS_SASTOKEN_06_023: [The argument keyName is appended to result.]*/
TEST_FUNCTION(SASToken_Create_succeeds)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_SCOPE_HANDLE)).SetReturn(TEST_STRING_VALUE);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_KEYNAME_HANDLE)).SetReturn(TEST_STRING_VALUE);

    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(BUFFER_u_char(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(Azure_Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&sig="));
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&se="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&skn="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NOT_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SASToken_CreateString_succeeds)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(BUFFER_u_char(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(Azure_Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&sig="));
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&se="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&skn="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_CreateString(TEST_CHAR_ARRAY, TEST_STRING_VALUE, TEST_STRING_VALUE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NOT_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

// Testing for Y2K38.
TEST_FUNCTION(SASToken_CreateString_large_int_succeeds)
{
    // arrange
    STRING_HANDLE handle;

    STRICT_EXPECTED_CALL(Azure_Base64_Decode(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(uint64_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME_LARGE), TEST_EXPIRY_LARGE)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME_LARGE, sizeof(TEST_TOKEN_EXPIRATION_TIME_LARGE));

    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME_LARGE));

    STRICT_EXPECTED_CALL(STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(BUFFER_u_char(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(Azure_Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&sig="));
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&se="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, TEST_TOKEN_EXPIRATION_TIME_LARGE));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, "&skn="));
    STRICT_EXPECTED_CALL(STRING_concat(TEST_RESULT_HANDLE, IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_HASH_HANDLE));
    STRICT_EXPECTED_CALL(BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    // act
    handle = SASToken_CreateString(TEST_CHAR_ARRAY, TEST_STRING_VALUE, TEST_STRING_VALUE, TEST_EXPIRY_LARGE);

    // assert
    ASSERT_IS_NOT_NULL(handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(sastoken_unittests)
