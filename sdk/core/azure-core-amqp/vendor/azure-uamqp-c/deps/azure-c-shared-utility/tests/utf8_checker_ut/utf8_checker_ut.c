// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#include <stdbool.h>
#endif

#include "testrunnerswitcher.h"
#include "azure_c_shared_utility/utf8_checker.h"

static TEST_MUTEX_HANDLE g_testByTest;

BEGIN_TEST_SUITE(utf8_checker_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    TEST_MUTEX_DESTROY(g_testByTest);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("Could not acquire test serialization mutex.");
    }
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* utf8_checker_is_valid_utf8 */

/* Tests_SRS_UTF8_CHECKER_01_002: [ If utf8_checker_is_valid_utf8 is called with NULL utf8_str it shall return false. ]*/
TEST_FUNCTION(utf8_checker_is_valid_utf8_with_NULL_string_fails)
{
    // arrange
    bool result;

    // act
    result = utf8_checker_is_valid_utf8(NULL, 1);

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_003: [ If length is 0, utf8_checker_is_valid_utf8 shall consider utf8_str to be valid UTF-8 and return true. ]*/
/* Tests_SRS_UTF8_CHECKER_01_005: [ On success it shall return true. ]*/
TEST_FUNCTION(utf8_checker_is_valid_with_0_length_succeeds)
{
    // arrange
    bool result;

    // act
    result = utf8_checker_is_valid_utf8((const unsigned char*)"", 0);

    // assert
    ASSERT_IS_TRUE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_006: [ 00000000 0xxxxxxx 0xxxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_a_NULL_succeeds)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0x00 };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_TRUE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_006: [ 00000000 0xxxxxxx 0xxxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_1_succeeds)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0x01 };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_TRUE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_006: [ 00000000 0xxxxxxx 0xxxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_max_1_byte_code_succeeds)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0x7F };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_TRUE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_007: [ 00000yyy yyxxxxxx 110yyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_a_valid_2_byte_code_succeeds)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xC2, 0x80 };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_TRUE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_007: [ 00000yyy yyxxxxxx 110yyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_a_valid_max_2_byte_code_succeeds)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xDF, 0xBF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_TRUE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_007: [ 00000yyy yyxxxxxx 110yyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_a_too_low_codepoint_for_2_bytes_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xC1, 0xBF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_007: [ 00000yyy yyxxxxxx 110yyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_the_second_byte_header_00_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xDF, 0x00 };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_007: [ 00000yyy yyxxxxxx 110yyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_the_second_byte_header_11_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xDF, 0xC0 };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_007: [ 00000yyy yyxxxxxx 110yyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_2_bytes_code_too_few_bytes_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xDF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_008: [ zzzzyyyy yyxxxxxx 1110zzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_a_valid_3_byte_code_succeeds)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xE0, 0xA0, 0x80 };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_TRUE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_008: [ zzzzyyyy yyxxxxxx 1110zzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_a_max_valid_3_byte_code_succeeds)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xEF, 0xBF, 0xBF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_TRUE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_008: [ zzzzyyyy yyxxxxxx 1110zzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_a_too_low_3_byte_code_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xE0, 0x9F, 0xBF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_008: [ zzzzyyyy yyxxxxxx 1110zzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_3_bytes_code_the_second_byte_header_00_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xEF, 0x3F, 0xBF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_008: [ zzzzyyyy yyxxxxxx 1110zzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_3_bytes_code_the_second_byte_header_11_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xE0, 0xFF, 0xBF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_008: [ zzzzyyyy yyxxxxxx 1110zzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_3_bytes_code_the_third_byte_header_00_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xEF, 0xBF, 0x3F };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_008: [ zzzzyyyy yyxxxxxx 1110zzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_3_bytes_code_the_third_byte_header_11_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xE0, 0xBF, 0xFF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_008: [ zzzzyyyy yyxxxxxx 1110zzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_3_bytes_code_too_few_bytes_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xE0, 0xBF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_009: [ 000uuuuu zzzzyyyy yyxxxxxx 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_a_valid_4_byte_code_succeeds)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xF0, 0x90, 0x80, 0x80 };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_TRUE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_009: [ 000uuuuu zzzzyyyy yyxxxxxx 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_a_max_valid_4_byte_code_succeeds)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xF7, 0xBF, 0xBF, 0xBF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_TRUE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_009: [ 000uuuuu zzzzyyyy yyxxxxxx 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_a_too_low_4_byte_code_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xF0, 0x8F, 0xBF, 0xBF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_009: [ 000uuuuu zzzzyyyy yyxxxxxx 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_4_byte_code_second_byte_with_00_header_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xF7, 0x3F, 0xBF, 0xBF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_009: [ 000uuuuu zzzzyyyy yyxxxxxx 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_4_byte_code_second_byte_with_11_header_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xF7, 0xFF, 0xBF, 0xBF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_009: [ 000uuuuu zzzzyyyy yyxxxxxx 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_4_byte_code_third_byte_with_00_header_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xF7, 0xBF, 0x3F, 0xBF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_009: [ 000uuuuu zzzzyyyy yyxxxxxx 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_4_byte_code_third_byte_with_11_header_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xF7, 0xBF, 0xFF, 0xBF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_009: [ 000uuuuu zzzzyyyy yyxxxxxx 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_4_byte_code_fourth_byte_with_00_header_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xF7, 0xBF, 0xBF, 0x3F };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_009: [ 000uuuuu zzzzyyyy yyxxxxxx 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_4_byte_code_fourth_byte_with_11_header_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xF7, 0xBF, 0xBF, 0xFF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_009: [ 000uuuuu zzzzyyyy yyxxxxxx 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_4_byte_code_too_few_bytes_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xF7, 0xBF, 0xBF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_009: [ 000uuuuu zzzzyyyy yyxxxxxx 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_bad_1st_byte_header_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0xFF, 0xBF, 0xBF, 0xBF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_009: [ 000uuuuu zzzzyyyy yyxxxxxx 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_all_length_chars_succeeds)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0x01, 0xC2, 0x80, 0xEF, 0xBF, 0xBF, 0xF7, 0xBF, 0xBF, 0xBF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_TRUE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_009: [ 000uuuuu zzzzyyyy yyxxxxxx 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_2nd_char_bad_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0x01, 0xC2, 0xFF, 0xEF, 0xBF, 0xBF, 0xF7, 0xBF, 0xBF, 0xBF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_009: [ 000uuuuu zzzzyyyy yyxxxxxx 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_3rd_char_bad_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0x01, 0xC2, 0x80, 0xEF, 0xFF, 0xBF, 0xF7, 0xBF, 0xBF, 0xBF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

/* Tests_SRS_UTF8_CHECKER_01_001: [ utf8_checker_is_valid_utf8 shall verify that the sequence of chars pointed to by utf8_str represent UTF-8 encoded codepoints. ]*/
/* Tests_SRS_UTF8_CHECKER_01_009: [ 000uuuuu zzzzyyyy yyxxxxxx 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx ]*/
TEST_FUNCTION(utf8_checker_with_4th_char_bad_fails)
{
    // arrange
    bool result;
    unsigned char test_str[] = { 0x01, 0xC2, 0x80, 0xEF, 0xBF, 0xBF, 0xF7, 0xFF, 0xBF, 0xBF };

    // act
    result = utf8_checker_is_valid_utf8(test_str, sizeof(test_str));

    // assert
    ASSERT_IS_FALSE(result);
}

END_TEST_SUITE(utf8_checker_ut)
