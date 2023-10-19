// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#include "azure_macro_utils/macro_utils.h"

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void my_gballoc_free(void* s)
{
    free(s);
}

void* my_gballoc_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

#include "azure_c_shared_utility/crt_abstractions.h"

#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_bool.h"
#include "umock_c/umocktypes_stdint.h"
#include "umock_c/umock_c_negative_tests.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/map.h"
#include "azure_c_shared_utility/string_tokenizer.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"
#undef ENABLE_MOCKS

#include "azure_c_shared_utility/connection_string_parser.h"

#include "real_map.h"
#include "real_string_tokenizer.h"
#include "real_strings.h"

IMPLEMENT_UMOCK_C_ENUM_TYPE(MAP_RESULT, MAP_RESULT_VALUES);

static TEST_MUTEX_HANDLE g_testByTest;

static const char* TEST_STRING_PAIR = "key1=value1";
STRING_HANDLE TEST_STRING_HANDLE_PAIR;
static const char* TEST_STRING_KEY = "key1=";
STRING_HANDLE TEST_STRING_HANDLE_KEY;
static const char* TEST_STRING_2_PAIR = "key1=value1;key2=value2";
STRING_HANDLE TEST_STRING_HANDLE_2_PAIR;
static const char* TEST_STRING_2_PAIR_SEMICOLON = "key1=value1;key2=value2;";
STRING_HANDLE TEST_STRING_HANDLE_2_PAIR_SEMICOLON;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(connectionstringparser_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_TYPE(MAP_RESULT, MAP_RESULT);
    REGISTER_UMOCK_ALIAS_TYPE(STRING_TOKENIZER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(STRING_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(MAP_FILTER_CALLBACK, void*);
    REGISTER_UMOCK_ALIAS_TYPE(MAP_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(STRING_HANDLE, void*);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);

    REGISTER_STRING_GLOBAL_MOCK_HOOK;
    REGISTER_STRING_TOKENIZER_GLOBAL_MOCK_HOOK;
    REGISTER_MAP_GLOBAL_MOCK_HOOK;

    TEST_STRING_HANDLE_PAIR = STRING_construct(TEST_STRING_PAIR);
    TEST_STRING_HANDLE_KEY = STRING_construct(TEST_STRING_KEY);
    TEST_STRING_HANDLE_2_PAIR = STRING_construct(TEST_STRING_2_PAIR);
    TEST_STRING_HANDLE_2_PAIR_SEMICOLON = STRING_construct(TEST_STRING_2_PAIR_SEMICOLON);
}

TEST_SUITE_CLEANUP(TestClassCleanup)
{
    STRING_delete(TEST_STRING_HANDLE_PAIR);
    STRING_delete(TEST_STRING_HANDLE_KEY);
    STRING_delete(TEST_STRING_HANDLE_2_PAIR);
    STRING_delete(TEST_STRING_HANDLE_2_PAIR_SEMICOLON);

    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("Could not acquire test serialization mutex.");
    }

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* connectionstringparser_parse */

/* Tests_SRS_CONNECTIONSTRINGPARSER_01_001: [connectionstringparser_parse shall parse all key value pairs from the connection_string passed in as argument and return a new map that holds the key/value pairs.]  */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_003: [connectionstringparser_parse shall create a STRING tokenizer to be used for parsing the connection string, by calling STRING_TOKENIZER_create.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_004: [connectionstringparser_parse shall start scanning at the beginning of the connection string.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_016: [2 STRINGs shall be allocated in order to hold the to be parsed key and value tokens.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_005: [The following actions shall be repeated until parsing is complete:] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_006: [connectionstringparser_parse shall find a token (the key of the key/value pair) delimited by the "=" character, by calling STRING_TOKENIZER_get_next_token.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_007: [If STRING_TOKENIZER_get_next_token fails, parsing shall be considered complete.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_014: [After the parsing is complete the previously allocated STRINGs and STRING tokenizer shall be freed by calling STRING_TOKENIZER_destroy.] */
TEST_FUNCTION(connectionstringparser_parse_with_an_empty_string_yields_an_empty_map)
{
    // arrange
    MAP_HANDLE result;
    STRING_HANDLE connectionString = STRING_new();
    STRING_HANDLE key = STRING_new();
    STRING_HANDLE value = STRING_new();
    STRING_TOKENIZER_HANDLE tokens = STRING_TOKENIZER_create(connectionString);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_create(connectionString)).SetReturn(tokens);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(key);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(value);
    STRICT_EXPECTED_CALL(Map_Create(NULL));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, key, "="));
    STRICT_EXPECTED_CALL(STRING_delete(value));
    STRICT_EXPECTED_CALL(STRING_delete(key));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_destroy(tokens));

    // act
    result = connectionstringparser_parse(connectionString);

    // assert
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    STRING_delete(connectionString);
    Map_Destroy(result);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_01_002: [If connection_string is NULL then connectionstringparser_parse shall fail and return NULL.] */
TEST_FUNCTION(connectionstringparser_parse_with_NULL_connection_string_fails)
{
    // arrange

    // act
    MAP_HANDLE result = connectionstringparser_parse(NULL);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_01_015: [If STRING_TOKENIZER_create fails, connectionstringparser_parse shall fail and return NULL.] */
TEST_FUNCTION(when_creating_the_string_tokenizer_fails_then_connectionstringparser_fails)
{
    // arrange
    MAP_HANDLE result;
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_create(TEST_STRING_HANDLE_PAIR))
        .SetReturn((STRING_TOKENIZER_HANDLE)NULL);

    // act
    result = connectionstringparser_parse(TEST_STRING_HANDLE_PAIR);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);

    // cleanup
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_01_017: [If allocating the STRINGs fails connectionstringparser_parse shall fail and return NULL.] */
TEST_FUNCTION(when_allocating_the_key_token_string_fails_then_connectionstringparser_fails)
{
    // arrange
    MAP_HANDLE result;
    STRING_TOKENIZER_HANDLE tokens = STRING_TOKENIZER_create(TEST_STRING_HANDLE_PAIR);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_create(TEST_STRING_HANDLE_PAIR)).SetReturn(tokens);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn((STRING_HANDLE)NULL);
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_destroy(tokens));

    // act
    result = connectionstringparser_parse(TEST_STRING_HANDLE_PAIR);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_01_017: [If allocating the STRINGs fails connectionstringparser_parse shall fail and return NULL.] */
TEST_FUNCTION(when_allocating_the_value_token_string_fails_then_connectionstringparser_fails)
{
    // arrange
    MAP_HANDLE result;
    STRING_HANDLE key = STRING_new();
    STRING_TOKENIZER_HANDLE tokens = STRING_TOKENIZER_create(TEST_STRING_HANDLE_PAIR);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_create(TEST_STRING_HANDLE_PAIR)).SetReturn(tokens);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(key);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn((STRING_HANDLE)NULL);
    STRICT_EXPECTED_CALL(STRING_delete(key));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_destroy(tokens));

    // act
    result = connectionstringparser_parse(TEST_STRING_HANDLE_PAIR);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_01_018: [If creating the result map fails, then connectionstringparser_parse shall return NULL.] */
TEST_FUNCTION(when_allocating_the_result_map_fails_then_connectionstringparser_fails)
{
    // arrange
    MAP_HANDLE result;
    STRING_HANDLE key = STRING_new();
    STRING_HANDLE value = STRING_new();
    STRING_TOKENIZER_HANDLE tokens = STRING_TOKENIZER_create(TEST_STRING_HANDLE_PAIR);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_create(TEST_STRING_HANDLE_PAIR)).SetReturn(tokens);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(key);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(value);
    STRICT_EXPECTED_CALL(Map_Create(NULL)).SetReturn((MAP_HANDLE)NULL);
    STRICT_EXPECTED_CALL(STRING_delete(value));
    STRICT_EXPECTED_CALL(STRING_delete(key));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_destroy(tokens));

    // act
    result = connectionstringparser_parse(TEST_STRING_HANDLE_PAIR);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_01_001: [connectionstringparser_parse shall parse all key value pairs from the connection_string passed in as argument and return a new map that holds the key/value pairs.]  */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_003: [connectionstringparser_parse shall create a STRING tokenizer to be used for parsing the connection string, by calling STRING_TOKENIZER_create.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_004: [connectionstringparser_parse shall start scanning at the beginning of the connection string.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_016: [2 STRINGs shall be allocated in order to hold the to be parsed key and value tokens.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_005: [The following actions shall be repeated until parsing is complete:] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_006: [connectionstringparser_parse shall find a token (the key of the key/value pair) delimited by the "=" character, by calling STRING_TOKENIZER_get_next_token.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_007: [If STRING_TOKENIZER_get_next_token fails, parsing shall be considered complete.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_014: [After the parsing is complete the previously allocated STRINGs and STRING tokenizer shall be freed by calling STRING_TOKENIZER_destroy.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_008: [connectionstringparser_parse shall find a token (the value of the key/value pair) delimited by the ";" character, by calling STRING_TOKENIZER_get_next_token.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_010: [The key and value shall be added to the result map by using Map_Add.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_011: [The C strings for the key and value shall be extracted from the previously parsed STRINGs by using STRING_c_str.] */
TEST_FUNCTION(connectionstringparser_parse_with_a_key_value_pair_adds_it_to_the_result_map)
{
    // arrange
    MAP_HANDLE result;
    STRING_HANDLE key = STRING_new();
    STRING_HANDLE value = STRING_new();
    STRING_TOKENIZER_HANDLE tokens = STRING_TOKENIZER_create(TEST_STRING_HANDLE_PAIR);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_create(TEST_STRING_HANDLE_PAIR)).SetReturn(tokens);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(key);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(value);
    STRICT_EXPECTED_CALL(Map_Create(NULL));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, key, "="));
    STRICT_EXPECTED_CALL(STRING_copy_n(key, TEST_STRING_PAIR, 4));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, value, ";"));
    STRICT_EXPECTED_CALL(STRING_copy_n(value, (TEST_STRING_PAIR +5), 6));
    STRICT_EXPECTED_CALL(STRING_c_str(key));
    STRICT_EXPECTED_CALL(STRING_c_str(value));
    STRICT_EXPECTED_CALL(Map_Add(IGNORED_PTR_ARG, "key1", "value1")).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_malloc(5));
    STRICT_EXPECTED_CALL(gballoc_malloc(7));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, key, "="));
    STRICT_EXPECTED_CALL(STRING_delete(value));
    STRICT_EXPECTED_CALL(STRING_delete(key));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_destroy(tokens));

    // act
    result = connectionstringparser_parse(TEST_STRING_HANDLE_PAIR);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, "value1", Map_GetValueFromKey(result, "key1"));

    // cleanup
    Map_Destroy(result);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_01_009: [If STRING_TOKENIZER_get_next_token fails, connectionstringparser_parse shall fail and return NULL (freeing the allocated result map).] */
TEST_FUNCTION(when_getting_the_value_token_fails_then_connectionstringparser_parse_fails)
{
    // arrange
    MAP_HANDLE result;
    STRING_HANDLE key = STRING_new();
    STRING_HANDLE value = STRING_new();
    MAP_HANDLE map = Map_Create(NULL);
    STRING_TOKENIZER_HANDLE tokens = STRING_TOKENIZER_create(TEST_STRING_HANDLE_KEY);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_create(TEST_STRING_HANDLE_KEY)).SetReturn(tokens);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(key);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(value);
    STRICT_EXPECTED_CALL(Map_Create(NULL)).SetReturn(map);
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, key, "="));
    STRICT_EXPECTED_CALL(STRING_copy_n(key, TEST_STRING_KEY, 4));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, value, ";"));
    STRICT_EXPECTED_CALL(Map_Destroy(map));
    STRICT_EXPECTED_CALL(STRING_delete(value));
    STRICT_EXPECTED_CALL(STRING_delete(key));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_destroy(tokens));

    // act
    result = connectionstringparser_parse(TEST_STRING_HANDLE_KEY);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_01_019: [If the key length is zero then connectionstringparser_parse shall fail and return NULL (freeing the allocated result map).] */
TEST_FUNCTION(when_the_key_is_zero_length_then_connectionstringparser_parse_fails)
{
    // arrange
    MAP_HANDLE result;
    STRING_HANDLE key = STRING_new();
    STRING_HANDLE value = STRING_new();
    MAP_HANDLE map = Map_Create(NULL);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_create(TEST_STRING_HANDLE_PAIR));
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_STRING_HANDLE_PAIR));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreAllArguments();
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(key);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(value);
    STRICT_EXPECTED_CALL(Map_Create(NULL)).SetReturn(map);
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(IGNORED_PTR_ARG, key, "=")).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(STRING_copy_n(key, TEST_STRING_PAIR, 4));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(IGNORED_PTR_ARG, value, ";")).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(STRING_copy_n(value, (TEST_STRING_PAIR + 5), 6));
    STRICT_EXPECTED_CALL(STRING_c_str(key)).SetReturn("");
    STRICT_EXPECTED_CALL(Map_Destroy(map));
    STRICT_EXPECTED_CALL(STRING_delete(value));
    STRICT_EXPECTED_CALL(STRING_delete(key));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_destroy(IGNORED_PTR_ARG)).IgnoreArgument(1);

    // act
    result = connectionstringparser_parse(TEST_STRING_HANDLE_PAIR);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_01_012: [If Map_Add fails connectionstringparser_parse shall fail and return NULL (freeing the allocated result map).] */
TEST_FUNCTION(when_adding_the_key_value_pair_to_the_map_fails_then_connectionstringparser_parse_fails)
{
    // arrange
    MAP_HANDLE result;
    STRING_HANDLE key = STRING_new();
    STRING_HANDLE value = STRING_new();
    MAP_HANDLE map = Map_Create(NULL);
    STRING_TOKENIZER_HANDLE tokens = STRING_TOKENIZER_create(TEST_STRING_HANDLE_PAIR);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_create(TEST_STRING_HANDLE_PAIR)).SetReturn(tokens);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(key);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(value);
    STRICT_EXPECTED_CALL(Map_Create(NULL)).SetReturn(map);
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, key, "="));
    STRICT_EXPECTED_CALL(STRING_copy_n(key, TEST_STRING_PAIR, 4));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, value, ";"));
    STRICT_EXPECTED_CALL(STRING_copy_n(value, (TEST_STRING_PAIR + 5), 6));
    STRICT_EXPECTED_CALL(STRING_c_str(key));
    STRICT_EXPECTED_CALL(STRING_c_str(value));
    STRICT_EXPECTED_CALL(Map_Add(map, "key1", "value1")).SetReturn((MAP_RESULT)MAP_INVALIDARG);
    STRICT_EXPECTED_CALL(Map_Destroy(map));
    STRICT_EXPECTED_CALL(STRING_delete(value));
    STRICT_EXPECTED_CALL(STRING_delete(key));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_destroy(tokens));

    // act
    result = connectionstringparser_parse(TEST_STRING_HANDLE_PAIR);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_01_013: [If STRING_c_str fails then connectionstringparser_parse shall fail and return NULL (freeing the allocated result map).] */
TEST_FUNCTION(when_getting_the_C_string_for_the_key_fails_then_connectionstringparser_parse_fails)
{
    // arrange
    MAP_HANDLE result;
    STRING_HANDLE key = STRING_new();
    STRING_HANDLE value = STRING_new();
    MAP_HANDLE map = Map_Create(NULL);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_create(TEST_STRING_HANDLE_PAIR));
    STRICT_EXPECTED_CALL(STRING_c_str(TEST_STRING_HANDLE_PAIR));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreAllArguments();
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(key);
    STRICT_EXPECTED_CALL(STRING_new()).SetReturn(value);
    STRICT_EXPECTED_CALL(Map_Create(NULL)).SetReturn(map);
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(IGNORED_PTR_ARG, key, "=")).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(STRING_copy_n(key, TEST_STRING_PAIR, 4));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(IGNORED_PTR_ARG, value, ";")).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(STRING_copy_n(value, (TEST_STRING_PAIR + 5), 6));
    STRICT_EXPECTED_CALL(STRING_c_str(key)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(Map_Destroy(map));
    STRICT_EXPECTED_CALL(STRING_delete(value));
    STRICT_EXPECTED_CALL(STRING_delete(key));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_destroy(IGNORED_PTR_ARG)).IgnoreArgument(1);

    // act
    result = connectionstringparser_parse(TEST_STRING_HANDLE_PAIR);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_01_013: [If STRING_c_str fails then connectionstringparser_parse shall fail and return NULL (freeing the allocated result map).] */
TEST_FUNCTION(when_getting_the_C_string_for_the_value_fails_then_connectionstringparser_parse_fails)
{
    // arrange
    MAP_HANDLE result;
    STRING_HANDLE key = STRING_new();
    STRING_HANDLE value = STRING_new();
    MAP_HANDLE map = Map_Create(NULL);
    STRING_TOKENIZER_HANDLE tokens = STRING_TOKENIZER_create(TEST_STRING_HANDLE_PAIR);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_create(TEST_STRING_HANDLE_PAIR)).SetReturn(tokens);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(key);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(value);
    STRICT_EXPECTED_CALL(Map_Create(NULL)).SetReturn(map);
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, key, "="));
    STRICT_EXPECTED_CALL(STRING_copy_n(key, TEST_STRING_PAIR, 4));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, value, ";"));
    STRICT_EXPECTED_CALL(STRING_copy_n(value, (TEST_STRING_PAIR + 5), 6));
    STRICT_EXPECTED_CALL(STRING_c_str(key));
    STRICT_EXPECTED_CALL(STRING_c_str(value)).SetReturn((const char*)NULL);
    STRICT_EXPECTED_CALL(Map_Destroy(map));
    STRICT_EXPECTED_CALL(STRING_delete(value));
    STRICT_EXPECTED_CALL(STRING_delete(key));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_destroy(tokens));

    // act
    result = connectionstringparser_parse(TEST_STRING_HANDLE_PAIR);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_01_001: [connectionstringparser_parse shall parse all key value pairs from the connection_string passed in as argument and return a new map that holds the key/value pairs.]  */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_003: [connectionstringparser_parse shall create a STRING tokenizer to be used for parsing the connection string, by calling STRING_TOKENIZER_create.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_004: [connectionstringparser_parse shall start scanning at the beginning of the connection string.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_016: [2 STRINGs shall be allocated in order to hold the to be parsed key and value tokens.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_005: [The following actions shall be repeated until parsing is complete:] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_006: [connectionstringparser_parse shall find a token (the key of the key/value pair) delimited by the "=" character, by calling STRING_TOKENIZER_get_next_token.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_007: [If STRING_TOKENIZER_get_next_token fails, parsing shall be considered complete.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_014: [After the parsing is complete the previously allocated STRINGs and STRING tokenizer shall be freed by calling STRING_TOKENIZER_destroy.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_008: [connectionstringparser_parse shall find a token (the value of the key/value pair) delimited by the ";" character, by calling STRING_TOKENIZER_get_next_token.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_010: [The key and value shall be added to the result map by using Map_Add.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_011: [The C strings for the key and value shall be extracted from the previously parsed STRINGs by using STRING_c_str.] */
TEST_FUNCTION(connectionstringparser_parse_with_2_key_value_pairs_adds_them_to_the_result_map)
{
    // arrange
    MAP_HANDLE result;
    STRING_HANDLE key = STRING_new();
    STRING_HANDLE value = STRING_new();
    MAP_HANDLE map = Map_Create(NULL);
    STRING_TOKENIZER_HANDLE tokens = STRING_TOKENIZER_create(TEST_STRING_HANDLE_2_PAIR);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_create(TEST_STRING_HANDLE_2_PAIR)).SetReturn(tokens);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(key);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(value);
    STRICT_EXPECTED_CALL(Map_Create(NULL)).SetReturn(map);

    // 1st kvp
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, key, "="));
    STRICT_EXPECTED_CALL(STRING_copy_n(key, TEST_STRING_2_PAIR, 4));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, value, ";"));
    STRICT_EXPECTED_CALL(STRING_copy_n(value, (TEST_STRING_2_PAIR + 5), 6));
    STRICT_EXPECTED_CALL(STRING_c_str(key));
    STRICT_EXPECTED_CALL(STRING_c_str(value));
    STRICT_EXPECTED_CALL(Map_Add(map, "key1", "value1"));
    STRICT_EXPECTED_CALL(gballoc_malloc(5));
    STRICT_EXPECTED_CALL(gballoc_malloc(7));

    // 2st kvp
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, key, "="));
    STRICT_EXPECTED_CALL(STRING_copy_n(key, (TEST_STRING_2_PAIR + 12), 4));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, value, ";"));
    STRICT_EXPECTED_CALL(STRING_copy_n(value, (TEST_STRING_2_PAIR + 17), 6));
    STRICT_EXPECTED_CALL(STRING_c_str(key));
    STRICT_EXPECTED_CALL(STRING_c_str(value));
    STRICT_EXPECTED_CALL(Map_Add(map, "key2", "value2"));
    STRICT_EXPECTED_CALL(gballoc_malloc(5));
    STRICT_EXPECTED_CALL(gballoc_malloc(7));

    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, key, "="));
    STRICT_EXPECTED_CALL(STRING_delete(value));
    STRICT_EXPECTED_CALL(STRING_delete(key));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_destroy(tokens));

    // act
    result = connectionstringparser_parse(TEST_STRING_HANDLE_2_PAIR);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(void_ptr, map, result);
    ASSERT_ARE_EQUAL(char_ptr, "value1", Map_GetValueFromKey(result, "key1"));
    ASSERT_ARE_EQUAL(char_ptr, "value2", Map_GetValueFromKey(result, "key2"));

    // cleanup
    Map_Destroy(result);

}

/* Tests_SRS_CONNECTIONSTRINGPARSER_01_001: [connectionstringparser_parse shall parse all key value pairs from the connection_string passed in as argument and return a new map that holds the key/value pairs.]  */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_003: [connectionstringparser_parse shall create a STRING tokenizer to be used for parsing the connection string, by calling STRING_TOKENIZER_create.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_004: [connectionstringparser_parse shall start scanning at the beginning of the connection string.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_016: [2 STRINGs shall be allocated in order to hold the to be parsed key and value tokens.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_005: [The following actions shall be repeated until parsing is complete:] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_006: [connectionstringparser_parse shall find a token (the key of the key/value pair) delimited by the "=" character, by calling STRING_TOKENIZER_get_next_token.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_007: [If STRING_TOKENIZER_get_next_token fails, parsing shall be considered complete.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_014: [After the parsing is complete the previously allocated STRINGs and STRING tokenizer shall be freed by calling STRING_TOKENIZER_destroy.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_008: [connectionstringparser_parse shall find a token (the value of the key/value pair) delimited by the ";" character, by calling STRING_TOKENIZER_get_next_token.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_010: [The key and value shall be added to the result map by using Map_Add.] */
/* Tests_SRS_CONNECTIONSTRINGPARSER_01_011: [The C strings for the key and value shall be extracted from the previously parsed STRINGs by using STRING_c_str.] */
TEST_FUNCTION(connectionstringparser_parse_with_2_key_value_pairs_ended_with_semicolon_adds_them_to_the_result_map)
{
    // arrange
    MAP_HANDLE result;
    STRING_HANDLE key = STRING_new();
    STRING_HANDLE value = STRING_new();
    MAP_HANDLE map = Map_Create(NULL);
    STRING_TOKENIZER_HANDLE tokens = STRING_TOKENIZER_create(TEST_STRING_HANDLE_2_PAIR_SEMICOLON);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_create(TEST_STRING_HANDLE_2_PAIR_SEMICOLON)).SetReturn(tokens);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(key);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(value);
    STRICT_EXPECTED_CALL(Map_Create(NULL)).SetReturn(map);

    // 1st kvp
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, key, "="));
    STRICT_EXPECTED_CALL(STRING_copy_n(key, TEST_STRING_2_PAIR_SEMICOLON, 4));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, value, ";"));
    STRICT_EXPECTED_CALL(STRING_copy_n(value, (TEST_STRING_2_PAIR_SEMICOLON + 5), 6));
    STRICT_EXPECTED_CALL(STRING_c_str(key));
    STRICT_EXPECTED_CALL(STRING_c_str(value));
    STRICT_EXPECTED_CALL(Map_Add(map, "key1", "value1"));
    STRICT_EXPECTED_CALL(gballoc_malloc(5));
    STRICT_EXPECTED_CALL(gballoc_malloc(7));

    // 2st kvp
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, key, "="));
    STRICT_EXPECTED_CALL(STRING_copy_n(key, (TEST_STRING_2_PAIR_SEMICOLON + 12), 4));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, value, ";"));
    STRICT_EXPECTED_CALL(STRING_copy_n(value, (TEST_STRING_2_PAIR_SEMICOLON + 17), 6));
    STRICT_EXPECTED_CALL(STRING_c_str(key));
    STRICT_EXPECTED_CALL(STRING_c_str(value));
    STRICT_EXPECTED_CALL(Map_Add(map, "key2", "value2"));
    STRICT_EXPECTED_CALL(gballoc_malloc(5));
    STRICT_EXPECTED_CALL(gballoc_malloc(7));

    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(tokens, key, "="));
    STRICT_EXPECTED_CALL(STRING_delete(value));
    STRICT_EXPECTED_CALL(STRING_delete(key));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_destroy(tokens));

    // act
    result = connectionstringparser_parse(TEST_STRING_HANDLE_2_PAIR_SEMICOLON);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(void_ptr, map, result);
    ASSERT_ARE_EQUAL(char_ptr, "value1", Map_GetValueFromKey(result, "key1"));
    ASSERT_ARE_EQUAL(char_ptr, "value2", Map_GetValueFromKey(result, "key2"));

    // cleanup
    Map_Destroy(result);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_21_020: [connectionstringparser_parse_from_char shall create a STRING_HANDLE from the connection_string passed in as argument and parse it using the connectionstringparser_parse.]*/
TEST_FUNCTION(connectionstringparser_parse_from_char_with_2_key_value_pairs_ended_with_semicolon_adds_them_to_the_result_map)
{
    // arrange
    MAP_HANDLE result;
    STRING_HANDLE key = STRING_new();
    STRING_HANDLE value = STRING_new();
    MAP_HANDLE map = Map_Create(NULL);
    STRING_TOKENIZER_HANDLE tokens = STRING_TOKENIZER_create(TEST_STRING_HANDLE_2_PAIR_SEMICOLON);
    STRING_HANDLE test_string_val = STRING_construct(TEST_STRING_2_PAIR_SEMICOLON);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(STRING_construct(TEST_STRING_2_PAIR_SEMICOLON)).SetReturn(test_string_val);
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_create(IGNORED_PTR_ARG)).SetReturn(tokens);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(key);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(value);
    STRICT_EXPECTED_CALL(Map_Create(NULL)).SetReturn(map);

    // 1st kvp
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(IGNORED_PTR_ARG, IGNORED_PTR_ARG, "="));
    STRICT_EXPECTED_CALL(STRING_copy_n(IGNORED_PTR_ARG, TEST_STRING_2_PAIR_SEMICOLON, 4));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(IGNORED_PTR_ARG, IGNORED_PTR_ARG, ";"));
    STRICT_EXPECTED_CALL(STRING_copy_n(IGNORED_PTR_ARG, (TEST_STRING_2_PAIR_SEMICOLON + 5), 6));
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Map_Add(IGNORED_PTR_ARG, "key1", "value1"));
    STRICT_EXPECTED_CALL(gballoc_malloc(5));
    STRICT_EXPECTED_CALL(gballoc_malloc(7));

    // 2st kvp
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(IGNORED_PTR_ARG, IGNORED_PTR_ARG, "="));
    STRICT_EXPECTED_CALL(STRING_copy_n(IGNORED_PTR_ARG, (TEST_STRING_2_PAIR_SEMICOLON + 12), 4));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(IGNORED_PTR_ARG, IGNORED_PTR_ARG, ";"));
    STRICT_EXPECTED_CALL(STRING_copy_n(IGNORED_PTR_ARG, (TEST_STRING_2_PAIR_SEMICOLON + 17), 6));
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Map_Add(IGNORED_PTR_ARG, "key2", "value2"));
    STRICT_EXPECTED_CALL(gballoc_malloc(5));
    STRICT_EXPECTED_CALL(gballoc_malloc(7));

    STRICT_EXPECTED_CALL(STRING_TOKENIZER_get_next_token(IGNORED_PTR_ARG, IGNORED_PTR_ARG, "="));
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_TOKENIZER_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG));

    // act
    result = connectionstringparser_parse_from_char(TEST_STRING_2_PAIR_SEMICOLON);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(void_ptr, map, result);
    ASSERT_ARE_EQUAL(char_ptr, "value1", Map_GetValueFromKey(result, "key1"));
    ASSERT_ARE_EQUAL(char_ptr, "value2", Map_GetValueFromKey(result, "key2"));

    // cleanup
    Map_Destroy(result);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_21_021: [If connectionstringparser_parse_from_char get error creating a STRING_HANDLE, it shall return NULL.]*/
TEST_FUNCTION(connectionstringparser_parse_from_char_with_NULL_connection_string_fails)
{
    // arrange

    // act
    MAP_HANDLE result = connectionstringparser_parse_from_char(NULL);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_21_022: [connectionstringparser_splitHostName_from_char shall split the provided hostName in name and suffix.]*/
/* Tests_SRS_CONNECTIONSTRINGPARSER_21_023: [connectionstringparser_splitHostName_from_char shall copy all characters, from the beginning of the hostName to the first . to the nameString.]*/
/* Tests_SRS_CONNECTIONSTRINGPARSER_21_024: [connectionstringparser_splitHostName_from_char shall copy all characters, from the first . to the end of the hostName, to the suffixString.]*/
/* Tests_SRS_CONNECTIONSTRINGPARSER_21_025: [If connectionstringparser_splitHostName_from_char get success splitting the hostName, it shall return 0.]*/
TEST_FUNCTION(connectionstringparser_splitHostName_from_char_with_success)
{
    // arrange
    int result;
    const char* hostName = "abc.bcd.efg";
    const char* startSuffix = hostName + 4;
    STRING_HANDLE nameString = STRING_new();
    STRING_HANDLE suffixString = STRING_new();
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, nameString);
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, suffixString);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(STRING_copy_n(nameString, hostName, 3));
    STRICT_EXPECTED_CALL(STRING_copy(suffixString, startSuffix));

    // act
    result = connectionstringparser_splitHostName_from_char(hostName, nameString, suffixString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, "abc", STRING_c_str(nameString));
    ASSERT_ARE_EQUAL(char_ptr, "bcd.efg", STRING_c_str(suffixString));

    // cleanup
    STRING_delete(nameString);
    STRING_delete(suffixString);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_21_026: [If the hostName is NULL, connectionstringparser_splitHostName_from_char shall return MU_FAILURE.]*/
TEST_FUNCTION(connectionstringparser_splitHostName_from_char_with_NULL_hostName_failed)
{
    // arrange
    int result;
    STRING_HANDLE nameString = STRING_new();
    STRING_HANDLE suffixString = STRING_new();
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, nameString);
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, suffixString);

    umock_c_reset_all_calls();

    // act
    result = connectionstringparser_splitHostName_from_char(NULL, nameString, suffixString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, "", STRING_c_str(nameString));
    ASSERT_ARE_EQUAL(char_ptr, "", STRING_c_str(suffixString));

    // cleanup
    STRING_delete(nameString);
    STRING_delete(suffixString);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_21_027: [If the hostName is an empty string, connectionstringparser_splitHostName_from_char shall return MU_FAILURE.]*/
TEST_FUNCTION(connectionstringparser_splitHostName_from_char_with_empty_hostName_failed)
{
    // arrange
    int result;
    const char* hostName = "";
    STRING_HANDLE nameString = STRING_new();
    STRING_HANDLE suffixString = STRING_new();
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, nameString);
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, suffixString);

    umock_c_reset_all_calls();

    // act
    result = connectionstringparser_splitHostName_from_char(hostName, nameString, suffixString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, "", STRING_c_str(nameString));
    ASSERT_ARE_EQUAL(char_ptr, "", STRING_c_str(suffixString));

    // cleanup
    STRING_delete(nameString);
    STRING_delete(suffixString);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_21_028: [If the nameString is NULL, connectionstringparser_splitHostName_from_char shall return MU_FAILURE.]*/
TEST_FUNCTION(connectionstringparser_splitHostName_from_char_with_NULL_nameString_failed)
{
    // arrange
    int result;
    const char* hostName = "abc.bcd.efg";
    STRING_HANDLE suffixString = STRING_new();
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, suffixString);

    umock_c_reset_all_calls();

    // act
    result = connectionstringparser_splitHostName_from_char(hostName, NULL, suffixString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, "", STRING_c_str(suffixString));

    // cleanup
    STRING_delete(suffixString);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_21_029: [If the suffixString is NULL, connectionstringparser_splitHostName_from_char shall return MU_FAILURE.]*/
TEST_FUNCTION(connectionstringparser_splitHostName_from_char_with_NULL_suffixString_failed)
{
    // arrange
    int result;
    const char* hostName = "abc.bcd.efg";
    STRING_HANDLE nameString = STRING_new();
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, nameString);

    umock_c_reset_all_calls();

    // act
    result = connectionstringparser_splitHostName_from_char(hostName, nameString, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, "", STRING_c_str(nameString));

    // cleanup
    STRING_delete(nameString);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_21_030: [If the hostName is not a valid host name, connectionstringparser_splitHostName_from_char shall return MU_FAILURE.]*/
TEST_FUNCTION(connectionstringparser_splitHostName_from_char_with_empty_name_failed)
{
    // arrange
    int result;
    const char* hostName = ".bcd.efg";
    STRING_HANDLE nameString = STRING_new();
    STRING_HANDLE suffixString = STRING_new();
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, nameString);
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, suffixString);

    umock_c_reset_all_calls();

    // act
    result = connectionstringparser_splitHostName_from_char(hostName, nameString, suffixString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, "", STRING_c_str(nameString));
    ASSERT_ARE_EQUAL(char_ptr, "", STRING_c_str(suffixString));

    // cleanup
    STRING_delete(nameString);
    STRING_delete(suffixString);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_21_030: [If the hostName is not a valid host name, connectionstringparser_splitHostName_from_char shall return MU_FAILURE.]*/
TEST_FUNCTION(connectionstringparser_splitHostName_from_char_with_empty_suffix_failed)
{
    // arrange
    int result;
    const char* hostName = "abc.";
    STRING_HANDLE nameString = STRING_new();
    STRING_HANDLE suffixString = STRING_new();
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, nameString);
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, suffixString);

    umock_c_reset_all_calls();

    // act
    result = connectionstringparser_splitHostName_from_char(hostName, nameString, suffixString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, "", STRING_c_str(nameString));
    ASSERT_ARE_EQUAL(char_ptr, "", STRING_c_str(suffixString));

    // cleanup
    STRING_delete(nameString);
    STRING_delete(suffixString);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_21_031: [If connectionstringparser_splitHostName_from_char get error copying the name to the nameString, it shall return MU_FAILURE.]*/
TEST_FUNCTION(connectionstringparser_splitHostName_from_char_error_on_nameString_copy_failed)
{
    // arrange
    int result;
    const char* hostName = "abc.bcd.efg";
    STRING_HANDLE nameString = STRING_new();
    STRING_HANDLE suffixString = STRING_new();
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, nameString);
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, suffixString);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(STRING_copy_n(nameString, hostName, 3)).SetReturn(10);

    // act
    result = connectionstringparser_splitHostName_from_char(hostName, nameString, suffixString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, "", STRING_c_str(nameString));
    ASSERT_ARE_EQUAL(char_ptr, "", STRING_c_str(suffixString));

    // cleanup
    STRING_delete(nameString);
    STRING_delete(suffixString);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_21_032: [If connectionstringparser_splitHostName_from_char get error copying the suffix to the suffixString, it shall return MU_FAILURE.]*/
TEST_FUNCTION(connectionstringparser_splitHostName_from_char_error_on_suffixString_copy_failed)
{
    // arrange
    int result;
    const char* hostName = "abc.bcd.efg";
    const char* startSuffix = hostName + 4;
    STRING_HANDLE nameString = STRING_new();
    STRING_HANDLE suffixString = STRING_new();
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, nameString);
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, suffixString);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(STRING_copy_n(nameString, hostName, 3));
    STRICT_EXPECTED_CALL(STRING_copy(suffixString, startSuffix)).SetReturn(10);

    // act
    result = connectionstringparser_splitHostName_from_char(hostName, nameString, suffixString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, "abc", STRING_c_str(nameString));
    ASSERT_ARE_EQUAL(char_ptr, "", STRING_c_str(suffixString));

    // cleanup
    STRING_delete(nameString);
    STRING_delete(suffixString);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_21_033: [connectionstringparser_splitHostName shall convert the hostNameString to a connection_string passed in as argument, and call connectionstringparser_splitHostName_from_char.]*/
TEST_FUNCTION(connectionstringparser_splitHostName_with_success)
{
    // arrange
    int result;
    const char* hostName;
    const char* startSuffix;
    STRING_HANDLE nameString = STRING_new();
    STRING_HANDLE suffixString = STRING_new();
    STRING_HANDLE hostNameString = STRING_construct("abc.bcd.efg");
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, hostNameString);
    hostName = STRING_c_str(hostNameString);
    startSuffix = hostName + 4;
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, nameString);
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, suffixString);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(STRING_c_str(hostNameString));
    STRICT_EXPECTED_CALL(STRING_copy_n(nameString, hostName, 3));
    STRICT_EXPECTED_CALL(STRING_copy(suffixString, startSuffix));

    // act
    result = connectionstringparser_splitHostName(hostNameString, nameString, suffixString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, "abc", STRING_c_str(nameString));
    ASSERT_ARE_EQUAL(char_ptr, "bcd.efg", STRING_c_str(suffixString));

    // cleanup
    STRING_delete(nameString);
    STRING_delete(suffixString);
    STRING_delete(hostNameString);
}

/* Tests_SRS_CONNECTIONSTRINGPARSER_21_034: [If the hostNameString is NULL, connectionstringparser_splitHostName shall return MU_FAILURE.]*/
TEST_FUNCTION(connectionstringparser_splitHostName_with_NULL_hostName_failed)
{
    // arrange
    int result;
    STRING_HANDLE nameString = STRING_new();
    STRING_HANDLE suffixString = STRING_new();
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, nameString);
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, suffixString);

    umock_c_reset_all_calls();

    // act
    result = connectionstringparser_splitHostName(NULL, nameString, suffixString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, "", STRING_c_str(nameString));
    ASSERT_ARE_EQUAL(char_ptr, "", STRING_c_str(suffixString));

    // cleanup
    STRING_delete(nameString);
    STRING_delete(suffixString);
}


END_TEST_SUITE(connectionstringparser_ut)
