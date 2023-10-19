// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstdio>
#include <cstddef>
#else
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#endif

void* real_malloc(size_t size)
{
    return malloc(size);
}

void* real_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

void real_free(void* ptr)
{
    free(ptr);
}

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#undef ENABLE_MOCKS

#include "azure_c_shared_utility/urlencode.h"
#include "azure_c_shared_utility/strings.h"

static const struct
{
    const char* inputData;
    const char* expectedOutput;
} testVector[] =
{
    { "\x01", "%01" },
    { "\x02", "%02" },
    { "\x03", "%03" },
    { "\x04", "%04" },
    { "\x05", "%05" },
    { "\x06", "%06" },
    { "\x07", "%07" },
    { "\x08", "%08" },
    { "\x09", "%09" },
    { "\x0a", "%0a" },
    { "\x0b", "%0b" },
    { "\x0c", "%0c" },
    { "\x0d", "%0d" },
    { "\x0e", "%0e" },
    { "\x0f", "%0f" },
    { "\x10", "%10" },
    { "\x11", "%11" },
    { "\x12", "%12" },
    { "\x13", "%13" },
    { "\x14", "%14" },
    { "\x15", "%15" },
    { "\x16", "%16" },
    { "\x17", "%17" },
    { "\x18", "%18" },
    { "\x19", "%19" },
    { "\x1a", "%1a" },
    { "\x1b", "%1b" },
    { "\x1c", "%1c" },
    { "\x1d", "%1d" },
    { "\x1e", "%1e" },
    { "\x1f", "%1f" },
    { "\x20", "%20" },
    { "\x21", "!" },
    { "\x22", "%22" },
    { "\x23", "%23" },
    { "\x24", "%24" },
    { "\x25", "%25" },
    { "\x26", "%26" },
    { "\x27", "%27" },
    { "\x28", "(" },
    { "\x29", ")" },
    { "\x2a", "*" },
    { "\x2b", "%2b" },
    { "\x2c", "%2c" },
    { "\x2d", "-" },
    { "\x2e", "." },
    { "\x2f", "%2f" },
    { "\x30", "0" },
    { "\x31", "1" },
    { "\x32", "2" },
    { "\x33", "3" },
    { "\x34", "4" },
    { "\x35", "5" },
    { "\x36", "6" },
    { "\x37", "7" },
    { "\x38", "8" },
    { "\x39", "9" },
    { "\x3a", "%3a" },
    { "\x3b", "%3b" },
    { "\x3c", "%3c" },
    { "\x3d", "%3d" },
    { "\x3e", "%3e" },
    { "\x3f", "%3f" },
    { "\x40", "%40" },
    { "\x41", "A" },
    { "\x42", "B" },
    { "\x43", "C" },
    { "\x44", "D" },
    { "\x45", "E" },
    { "\x46", "F" },
    { "\x47", "G" },
    { "\x48", "H" },
    { "\x49", "I" },
    { "\x4a", "J" },
    { "\x4b", "K" },
    { "\x4c", "L" },
    { "\x4d", "M" },
    { "\x4e", "N" },
    { "\x4f", "O" },
    { "\x50", "P" },
    { "\x51", "Q" },
    { "\x52", "R" },
    { "\x53", "S" },
    { "\x54", "T" },
    { "\x55", "U" },
    { "\x56", "V" },
    { "\x57", "W" },
    { "\x58", "X" },
    { "\x59", "Y" },
    { "\x5a", "Z" },
    { "\x5b", "%5b" },
    { "\x5c", "%5c" },
    { "\x5d", "%5d" },
    { "\x5e", "%5e" },
    { "\x5f", "_" },
    { "\x60", "%60" },
    { "\x61", "a" },
    { "\x62", "b" },
    { "\x63", "c" },
    { "\x64", "d" },
    { "\x65", "e" },
    { "\x66", "f" },
    { "\x67", "g" },
    { "\x68", "h" },
    { "\x69", "i" },
    { "\x6a", "j" },
    { "\x6b", "k" },
    { "\x6c", "l" },
    { "\x6d", "m" },
    { "\x6e", "n" },
    { "\x6f", "o" },
    { "\x70", "p" },
    { "\x71", "q" },
    { "\x72", "r" },
    { "\x73", "s" },
    { "\x74", "t" },
    { "\x75", "u" },
    { "\x76", "v" },
    { "\x77", "w" },
    { "\x78", "x" },
    { "\x79", "y" },
    { "\x7a", "z" },
    { "\x7b", "%7b" },
    { "\x7c", "%7c" },
    { "\x7d", "%7d" },
    { "\x7e", "%7e" },
    { "\x7f", "%7f" },
    { "\x80", "%c2%80" },
    { "\x81", "%c2%81" },
    { "\x82", "%c2%82" },
    { "\x83", "%c2%83" },
    { "\x84", "%c2%84" },
    { "\x85", "%c2%85" },
    { "\x86", "%c2%86" },
    { "\x87", "%c2%87" },
    { "\x88", "%c2%88" },
    { "\x89", "%c2%89" },
    { "\x8a", "%c2%8a" },
    { "\x8b", "%c2%8b" },
    { "\x8c", "%c2%8c" },
    { "\x8d", "%c2%8d" },
    { "\x8e", "%c2%8e" },
    { "\x8f", "%c2%8f" },
    { "\x90", "%c2%90" },
    { "\x91", "%c2%91" },
    { "\x92", "%c2%92" },
    { "\x93", "%c2%93" },
    { "\x94", "%c2%94" },
    { "\x95", "%c2%95" },
    { "\x96", "%c2%96" },
    { "\x97", "%c2%97" },
    { "\x98", "%c2%98" },
    { "\x99", "%c2%99" },
    { "\x9a", "%c2%9a" },
    { "\x9b", "%c2%9b" },
    { "\x9c", "%c2%9c" },
    { "\x9d", "%c2%9d" },
    { "\x9e", "%c2%9e" },
    { "\x9f", "%c2%9f" },
    { "\xa0", "%c2%a0" },
    { "\xa1", "%c2%a1" },
    { "\xa2", "%c2%a2" },
    { "\xa3", "%c2%a3" },
    { "\xa4", "%c2%a4" },
    { "\xa5", "%c2%a5" },
    { "\xa6", "%c2%a6" },
    { "\xa7", "%c2%a7" },
    { "\xa8", "%c2%a8" },
    { "\xa9", "%c2%a9" },
    { "\xaa", "%c2%aa" },
    { "\xab", "%c2%ab" },
    { "\xac", "%c2%ac" },
    { "\xad", "%c2%ad" },
    { "\xae", "%c2%ae" },
    { "\xaf", "%c2%af" },
    { "\xb0", "%c2%b0" },
    { "\xb1", "%c2%b1" },
    { "\xb2", "%c2%b2" },
    { "\xb3", "%c2%b3" },
    { "\xb4", "%c2%b4" },
    { "\xb5", "%c2%b5" },
    { "\xb6", "%c2%b6" },
    { "\xb7", "%c2%b7" },
    { "\xb8", "%c2%b8" },
    { "\xb9", "%c2%b9" },
    { "\xba", "%c2%ba" },
    { "\xbb", "%c2%bb" },
    { "\xbc", "%c2%bc" },
    { "\xbd", "%c2%bd" },
    { "\xbe", "%c2%be" },
    { "\xbf", "%c2%bf" },
    { "\xc0", "%c3%80" },
    { "\xc1", "%c3%81" },
    { "\xc2", "%c3%82" },
    { "\xc3", "%c3%83" },
    { "\xc4", "%c3%84" },
    { "\xc5", "%c3%85" },
    { "\xc6", "%c3%86" },
    { "\xc7", "%c3%87" },
    { "\xc8", "%c3%88" },
    { "\xc9", "%c3%89" },
    { "\xca", "%c3%8a" },
    { "\xcb", "%c3%8b" },
    { "\xcc", "%c3%8c" },
    { "\xcd", "%c3%8d" },
    { "\xce", "%c3%8e" },
    { "\xcf", "%c3%8f" },
    { "\xd0", "%c3%90" },
    { "\xd1", "%c3%91" },
    { "\xd2", "%c3%92" },
    { "\xd3", "%c3%93" },
    { "\xd4", "%c3%94" },
    { "\xd5", "%c3%95" },
    { "\xd6", "%c3%96" },
    { "\xd7", "%c3%97" },
    { "\xd8", "%c3%98" },
    { "\xd9", "%c3%99" },
    { "\xda", "%c3%9a" },
    { "\xdb", "%c3%9b" },
    { "\xdc", "%c3%9c" },
    { "\xdd", "%c3%9d" },
    { "\xde", "%c3%9e" },
    { "\xdf", "%c3%9f" },
    { "\xe0", "%c3%a0" },
    { "\xe1", "%c3%a1" },
    { "\xe2", "%c3%a2" },
    { "\xe3", "%c3%a3" },
    { "\xe4", "%c3%a4" },
    { "\xe5", "%c3%a5" },
    { "\xe6", "%c3%a6" },
    { "\xe7", "%c3%a7" },
    { "\xe8", "%c3%a8" },
    { "\xe9", "%c3%a9" },
    { "\xea", "%c3%aa" },
    { "\xeb", "%c3%ab" },
    { "\xec", "%c3%ac" },
    { "\xed", "%c3%ad" },
    { "\xee", "%c3%ae" },
    { "\xef", "%c3%af" },
    { "\xf0", "%c3%b0" },
    { "\xf1", "%c3%b1" },
    { "\xf2", "%c3%b2" },
    { "\xf3", "%c3%b3" },
    { "\xf4", "%c3%b4" },
    { "\xf5", "%c3%b5" },
    { "\xf6", "%c3%b6" },
    { "\xf7", "%c3%b7" },
    { "\xf8", "%c3%b8" },
    { "\xf9", "%c3%b9" },
    { "\xfa", "%c3%ba" },
    { "\xfb", "%c3%bb" },
    { "\xfc", "%c3%bc" },
    { "\xfd", "%c3%bd" },
    { "\xfe", "%c3%be" },
    { "\xff", "%c3%bf" }
};

static const struct
{
    const char* charRep;
    const char* endcodedRep;
} testVectorASCII[] =
{
    { "\x01", "%01" },
    { "\x02", "%02" },
    { "\x03", "%03" },
    { "\x04", "%04" },
    { "\x05", "%05" },
    { "\x06", "%06" },
    { "\x07", "%07" },
    { "\x08", "%08" },
    { "\x09", "%09" },
    { "\x0a", "%0a" },
    { "\x0b", "%0b" },
    { "\x0c", "%0c" },
    { "\x0d", "%0d" },
    { "\x0e", "%0e" },
    { "\x0f", "%0f" },
    { "\x10", "%10" },
    { "\x11", "%11" },
    { "\x12", "%12" },
    { "\x13", "%13" },
    { "\x14", "%14" },
    { "\x15", "%15" },
    { "\x16", "%16" },
    { "\x17", "%17" },
    { "\x18", "%18" },
    { "\x19", "%19" },
    { "\x1a", "%1a" },
    { "\x1b", "%1b" },
    { "\x1c", "%1c" },
    { "\x1d", "%1d" },
    { "\x1e", "%1e" },
    { "\x1f", "%1f" },
    { "\x20", "%20" },
    { "\x21", "!" },
    { "\x22", "%22" },
    { "\x23", "%23" },
    { "\x24", "%24" },
    { "\x25", "%25" },
    { "\x26", "%26" },
    { "\x27", "%27" },
    { "\x28", "(" },
    { "\x29", ")" },
    { "\x2a", "*" },
    { "\x2b", "%2b" },
    { "\x2c", "%2c" },
    { "\x2d", "-" },
    { "\x2e", "." },
    { "\x2f", "%2f" },
    { "\x30", "0" },
    { "\x31", "1" },
    { "\x32", "2" },
    { "\x33", "3" },
    { "\x34", "4" },
    { "\x35", "5" },
    { "\x36", "6" },
    { "\x37", "7" },
    { "\x38", "8" },
    { "\x39", "9" },
    { "\x3a", "%3a" },
    { "\x3b", "%3b" },
    { "\x3c", "%3c" },
    { "\x3d", "%3d" },
    { "\x3e", "%3e" },
    { "\x3f", "%3f" },
    { "\x40", "%40" },
    { "\x41", "A" },
    { "\x42", "B" },
    { "\x43", "C" },
    { "\x44", "D" },
    { "\x45", "E" },
    { "\x46", "F" },
    { "\x47", "G" },
    { "\x48", "H" },
    { "\x49", "I" },
    { "\x4a", "J" },
    { "\x4b", "K" },
    { "\x4c", "L" },
    { "\x4d", "M" },
    { "\x4e", "N" },
    { "\x4f", "O" },
    { "\x50", "P" },
    { "\x51", "Q" },
    { "\x52", "R" },
    { "\x53", "S" },
    { "\x54", "T" },
    { "\x55", "U" },
    { "\x56", "V" },
    { "\x57", "W" },
    { "\x58", "X" },
    { "\x59", "Y" },
    { "\x5a", "Z" },
    { "\x5b", "%5b" },
    { "\x5c", "%5c" },
    { "\x5d", "%5d" },
    { "\x5e", "%5e" },
    { "\x5f", "_" },
    { "\x60", "%60" },
    { "\x61", "a" },
    { "\x62", "b" },
    { "\x63", "c" },
    { "\x64", "d" },
    { "\x65", "e" },
    { "\x66", "f" },
    { "\x67", "g" },
    { "\x68", "h" },
    { "\x69", "i" },
    { "\x6a", "j" },
    { "\x6b", "k" },
    { "\x6c", "l" },
    { "\x6d", "m" },
    { "\x6e", "n" },
    { "\x6f", "o" },
    { "\x70", "p" },
    { "\x71", "q" },
    { "\x72", "r" },
    { "\x73", "s" },
    { "\x74", "t" },
    { "\x75", "u" },
    { "\x76", "v" },
    { "\x77", "w" },
    { "\x78", "x" },
    { "\x79", "y" },
    { "\x7a", "z" },
    { "\x7b", "%7b" },
    { "\x7c", "%7c" },
    { "\x7d", "%7d" },
    { "\x7e", "%7e" },
    { "\x7f", "%7f" },
};

static const struct
{
    const char* charRep;
    const char* endcodedRep;
} testVectorExtendedASCII[] =
{
    { "\x80", "%c2%80" },
    { "\x81", "%c2%81" },
    { "\x82", "%c2%82" },
    { "\x83", "%c2%83" },
    { "\x84", "%c2%84" },
    { "\x85", "%c2%85" },
    { "\x86", "%c2%86" },
    { "\x87", "%c2%87" },
    { "\x88", "%c2%88" },
    { "\x89", "%c2%89" },
    { "\x8a", "%c2%8a" },
    { "\x8b", "%c2%8b" },
    { "\x8c", "%c2%8c" },
    { "\x8d", "%c2%8d" },
    { "\x8e", "%c2%8e" },
    { "\x8f", "%c2%8f" },
    { "\x90", "%c2%90" },
    { "\x91", "%c2%91" },
    { "\x92", "%c2%92" },
    { "\x93", "%c2%93" },
    { "\x94", "%c2%94" },
    { "\x95", "%c2%95" },
    { "\x96", "%c2%96" },
    { "\x97", "%c2%97" },
    { "\x98", "%c2%98" },
    { "\x99", "%c2%99" },
    { "\x9a", "%c2%9a" },
    { "\x9b", "%c2%9b" },
    { "\x9c", "%c2%9c" },
    { "\x9d", "%c2%9d" },
    { "\x9e", "%c2%9e" },
    { "\x9f", "%c2%9f" },
    { "\xa0", "%c2%a0" },
    { "\xa1", "%c2%a1" },
    { "\xa2", "%c2%a2" },
    { "\xa3", "%c2%a3" },
    { "\xa4", "%c2%a4" },
    { "\xa5", "%c2%a5" },
    { "\xa6", "%c2%a6" },
    { "\xa7", "%c2%a7" },
    { "\xa8", "%c2%a8" },
    { "\xa9", "%c2%a9" },
    { "\xaa", "%c2%aa" },
    { "\xab", "%c2%ab" },
    { "\xac", "%c2%ac" },
    { "\xad", "%c2%ad" },
    { "\xae", "%c2%ae" },
    { "\xaf", "%c2%af" },
    { "\xb0", "%c2%b0" },
    { "\xb1", "%c2%b1" },
    { "\xb2", "%c2%b2" },
    { "\xb3", "%c2%b3" },
    { "\xb4", "%c2%b4" },
    { "\xb5", "%c2%b5" },
    { "\xb6", "%c2%b6" },
    { "\xb7", "%c2%b7" },
    { "\xb8", "%c2%b8" },
    { "\xb9", "%c2%b9" },
    { "\xba", "%c2%ba" },
    { "\xbb", "%c2%bb" },
    { "\xbc", "%c2%bc" },
    { "\xbd", "%c2%bd" },
    { "\xbe", "%c2%be" },
    { "\xbf", "%c2%bf" },
    { "\xc0", "%c3%80" },
    { "\xc1", "%c3%81" },
    { "\xc2", "%c3%82" },
    { "\xc3", "%c3%83" },
    { "\xc4", "%c3%84" },
    { "\xc5", "%c3%85" },
    { "\xc6", "%c3%86" },
    { "\xc7", "%c3%87" },
    { "\xc8", "%c3%88" },
    { "\xc9", "%c3%89" },
    { "\xca", "%c3%8a" },
    { "\xcb", "%c3%8b" },
    { "\xcc", "%c3%8c" },
    { "\xcd", "%c3%8d" },
    { "\xce", "%c3%8e" },
    { "\xcf", "%c3%8f" },
    { "\xd0", "%c3%90" },
    { "\xd1", "%c3%91" },
    { "\xd2", "%c3%92" },
    { "\xd3", "%c3%93" },
    { "\xd4", "%c3%94" },
    { "\xd5", "%c3%95" },
    { "\xd6", "%c3%96" },
    { "\xd7", "%c3%97" },
    { "\xd8", "%c3%98" },
    { "\xd9", "%c3%99" },
    { "\xda", "%c3%9a" },
    { "\xdb", "%c3%9b" },
    { "\xdc", "%c3%9c" },
    { "\xdd", "%c3%9d" },
    { "\xde", "%c3%9e" },
    { "\xdf", "%c3%9f" },
    { "\xe0", "%c3%a0" },
    { "\xe1", "%c3%a1" },
    { "\xe2", "%c3%a2" },
    { "\xe3", "%c3%a3" },
    { "\xe4", "%c3%a4" },
    { "\xe5", "%c3%a5" },
    { "\xe6", "%c3%a6" },
    { "\xe7", "%c3%a7" },
    { "\xe8", "%c3%a8" },
    { "\xe9", "%c3%a9" },
    { "\xea", "%c3%aa" },
    { "\xeb", "%c3%ab" },
    { "\xec", "%c3%ac" },
    { "\xed", "%c3%ad" },
    { "\xee", "%c3%ae" },
    { "\xef", "%c3%af" },
    { "\xf0", "%c3%b0" },
    { "\xf1", "%c3%b1" },
    { "\xf2", "%c3%b2" },
    { "\xf3", "%c3%b3" },
    { "\xf4", "%c3%b4" },
    { "\xf5", "%c3%b5" },
    { "\xf6", "%c3%b6" },
    { "\xf7", "%c3%b7" },
    { "\xf8", "%c3%b8" },
    { "\xf9", "%c3%b9" },
    { "\xfa", "%c3%ba" },
    { "\xfb", "%c3%bb" },
    { "\xfc", "%c3%bc" },
    { "\xfd", "%c3%bd" },
    { "\xfe", "%c3%be" },
    { "\xff", "%c3%bf" }
};

const char* UNRESERVED_CHAR = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._";

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(URLEncode_UnitTests)

TEST_SUITE_INITIALIZE(TestClassInitialize)
{
    umock_c_init(on_umock_c_error);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, real_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, real_free);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, real_realloc);
}

TEST_SUITE_CLEANUP(TestClassCleanup)
{
    umock_c_deinit();
}

/* Encode Tests */
TEST_FUNCTION(URL_EncodeString_is_null_should_yield_NULL)
{
    // arrange
    // act
    STRING_HANDLE encodedURL = URL_EncodeString(NULL);

    //assert
    ASSERT_IS_NULL(encodedURL);
}

/*Tests_SRS_URL_ENCODE_06_003: [If input is a zero length string then URL_Encode will return a zero length string.]*/
TEST_FUNCTION(URL_EncodeString_new_should_yield_zeroLengthString)
{
    // arrange
    const char* newString = "";

    // act
    STRING_HANDLE encodedURL = URL_EncodeString(newString);

    //assert
    ASSERT_IS_NOT_NULL(encodedURL);
    ASSERT_ARE_EQUAL(size_t, strlen(STRING_c_str(encodedURL)),0);
    STRING_delete(encodedURL);
}

TEST_FUNCTION(URL_EncodeString_is_hello_world)
{
    // arrange
    const char* hello = "hello world";

    // act
    STRING_HANDLE encodedURL = URL_EncodeString(hello);

    //assert
    ASSERT_IS_NOT_NULL(encodedURL);
    ASSERT_ARE_EQUAL(char_ptr,"hello%20world", STRING_c_str(encodedURL) );
    STRING_delete(encodedURL);
}

TEST_FUNCTION(URL_EncodeString_unreserved_mapping)
{
    // arrange

    // act
    STRING_HANDLE encodedUnreservedURL = URL_EncodeString(UNRESERVED_CHAR);

    //assert
    ASSERT_IS_NOT_NULL(encodedUnreservedURL);
    ASSERT_ARE_EQUAL(char_ptr, UNRESERVED_CHAR, STRING_c_str(encodedUnreservedURL) );
    STRING_delete(encodedUnreservedURL);
}

TEST_FUNCTION(URL_EncodeString_path_with_device)
{
    // arrange
    const char* pathWithDevice = "/getalarm('Le Pichet')";

    // act
    STRING_HANDLE encodedPathWithDevice = URL_EncodeString(pathWithDevice);

    //assert
    ASSERT_IS_NOT_NULL(encodedPathWithDevice);
    ASSERT_ARE_EQUAL(char_ptr, "%2fgetalarm(%27Le%20Pichet%27)", STRING_c_str(encodedPathWithDevice));
    STRING_delete(encodedPathWithDevice);
}

TEST_FUNCTION(URL_EncodeString_a_few_bogus_characters)
{
    // arrange
    const char* bogusCharacters = "{}%";

    // act
    STRING_HANDLE encodedBogusCharacters = URL_EncodeString(bogusCharacters);

    //assert
    ASSERT_IS_NOT_NULL(encodedBogusCharacters);
    ASSERT_ARE_EQUAL(char_ptr, "%7b%7d%25", STRING_c_str(encodedBogusCharacters));
    STRING_delete(encodedBogusCharacters);
}

TEST_FUNCTION(URL_EncodeString_full_url)
{
    // arrange
    const char* fullUrl = "https://one.two.three.four-five.com/six/Seven('EightNine1234567890.Ten_Eleven')?twelve-thirteen=2015-11-31 HTTP/1.1";

    // act
    STRING_HANDLE encodedFullUrl = URL_EncodeString(fullUrl);

    //ASSERT
    ASSERT_IS_NOT_NULL(encodedFullUrl);
    ASSERT_ARE_EQUAL(char_ptr, "https%3a%2f%2fone.two.three.four-five.com%2fsix%2fSeven(%27EightNine1234567890.Ten_Eleven%27)%3ftwelve-thirteen%3d2015-11-31%20HTTP%2f1.1", STRING_c_str(encodedFullUrl));
    STRING_delete(encodedFullUrl);
}

TEST_FUNCTION(URL_EncodeString_Exhaustive_chars)
{
    size_t i;
    size_t numberOfTests = sizeof(testVector) / sizeof(testVector[i]);
    for (i = 0; i < numberOfTests; i++)
    {
        //arrange

        const char* original = testVector[i].inputData;

        //act
        STRING_HANDLE encodedOriginal = URL_EncodeString(original);

        //assert
        ASSERT_IS_NOT_NULL(encodedOriginal);
        ASSERT_ARE_EQUAL(char_ptr, testVector[i].expectedOutput, STRING_c_str(encodedOriginal));
        STRING_delete(encodedOriginal);
    }
}

/*Tests_SRS_URL_ENCODE_06_001: [If input is NULL then URL_Encode will return NULL.]*/
TEST_FUNCTION(URL_is_null_should_yield_NULL)
{
    // arrange
    // act
    STRING_HANDLE encodedURL = URL_Encode(NULL);

    //assert
    ASSERT_IS_NULL(encodedURL);
}
/*Tests_SRS_URL_ENCODE_06_003: [If input is a zero length string then URL_Encode will return a zero length string.]*/
TEST_FUNCTION(URL_new_should_yield_zeroLengthString)
{
    // arrange
    STRING_HANDLE newString = STRING_new();
    // act
    STRING_HANDLE encodedURL = URL_Encode(newString);

    //assert
    ASSERT_IS_NOT_NULL(encodedURL);
    ASSERT_ARE_EQUAL(size_t, strlen(STRING_c_str(encodedURL)),0);
    STRING_delete(newString);
    STRING_delete(encodedURL);
}

TEST_FUNCTION(URL_is_hello_world)
{
    // arrange
    STRING_HANDLE encodedURL;
    STRING_HANDLE hello = STRING_new();

    ASSERT_ARE_EQUAL(int, STRING_concat(hello, "hello world"),0);

    // act
    encodedURL = URL_Encode(hello);


    //assert
    ASSERT_IS_NOT_NULL(encodedURL);
    ASSERT_ARE_EQUAL(char_ptr,"hello%20world",STRING_c_str(encodedURL));
    STRING_delete(hello);
    STRING_delete(encodedURL);
}

TEST_FUNCTION(URL_unreserved_mapping)
{
    // arrange
    STRING_HANDLE encodedUnreservedURL;
    STRING_HANDLE unreserved = STRING_new();

    ASSERT_ARE_EQUAL(int, STRING_concat(unreserved, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._"), 0);

    // act
    encodedUnreservedURL = URL_Encode(unreserved);


    //assert
    ASSERT_IS_NOT_NULL(encodedUnreservedURL);
    ASSERT_ARE_EQUAL(char_ptr, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._", STRING_c_str(encodedUnreservedURL));
    STRING_delete(unreserved);
    STRING_delete(encodedUnreservedURL);
}

TEST_FUNCTION(URL_path_with_device)
{
    // arrange
    STRING_HANDLE encodedPathWithDevice;
    STRING_HANDLE pathWithDevice = STRING_new();

    ASSERT_ARE_EQUAL(int, STRING_concat(pathWithDevice, "/getalarm('Le Pichet')"), 0);

    // act
    encodedPathWithDevice = URL_Encode(pathWithDevice);


    //assert
    ASSERT_IS_NOT_NULL(encodedPathWithDevice);
    ASSERT_ARE_EQUAL(char_ptr, "%2fgetalarm(%27Le%20Pichet%27)", STRING_c_str(encodedPathWithDevice));
    STRING_delete(pathWithDevice);
    STRING_delete(encodedPathWithDevice);
}

TEST_FUNCTION(URL_a_few_bogus_characters)
{
    // arrange
    STRING_HANDLE encodedBogusCharacters;
    STRING_HANDLE bogusCharacters = STRING_new();

    ASSERT_ARE_EQUAL(int, STRING_concat(bogusCharacters, "{}%"), 0);

    // act
    encodedBogusCharacters = URL_Encode(bogusCharacters);


    //assert
    ASSERT_IS_NOT_NULL(encodedBogusCharacters);
    ASSERT_ARE_EQUAL(char_ptr, "%7b%7d%25", STRING_c_str(encodedBogusCharacters));
    STRING_delete(bogusCharacters);
    STRING_delete(encodedBogusCharacters);
}

TEST_FUNCTION(URL_full_IOT_url)
{
    // arrange
    STRING_HANDLE encodedFullUrl;
    STRING_HANDLE fullUrl = STRING_new();

    ASSERT_ARE_EQUAL(int, STRING_concat(fullUrl, "https://one.two.three.four-five.com/six/Seven('EightNine1234567890.Ten_Eleven')?twelve-thirteen=2015-11-31 HTTP/1.1"), 0);

    // act
    encodedFullUrl = URL_Encode(fullUrl);

    //ASSERT
    ASSERT_IS_NOT_NULL(encodedFullUrl);
    ASSERT_ARE_EQUAL(char_ptr, "https%3a%2f%2fone.two.three.four-five.com%2fsix%2fSeven(%27EightNine1234567890.Ten_Eleven%27)%3ftwelve-thirteen%3d2015-11-31%20HTTP%2f1.1", STRING_c_str(encodedFullUrl));
    STRING_delete(fullUrl);
    STRING_delete(encodedFullUrl);
}

TEST_FUNCTION(URL_Exhaustive_chars)
{
    size_t i;
    size_t numberOfTests = sizeof(testVector) / sizeof(testVector[i]);
    for (i = 0; i < numberOfTests; i++)
    {
        //arrange
        STRING_HANDLE encodedOriginal;
        STRING_HANDLE original = STRING_new();
        ASSERT_ARE_EQUAL(int, STRING_concat(original, testVector[i].inputData), 0);

        //act
        encodedOriginal = URL_Encode(original);

        //assert
        ASSERT_IS_NOT_NULL(encodedOriginal);
        ASSERT_ARE_EQUAL(char_ptr, testVector[i].expectedOutput, STRING_c_str(encodedOriginal));
        STRING_delete(original);
        STRING_delete(encodedOriginal);
    }
}

/* Decode Tests */
TEST_FUNCTION(URL_DecodeString_null_input)
{
    //arrange

    //act
    STRING_HANDLE decodedURL = URL_DecodeString(NULL);

    //assert
    ASSERT_IS_NULL(decodedURL);

    //cleanup
}

TEST_FUNCTION(URL_DecodeString_zerolength_input)
{
    //arrange
    const char* newString = "";

    //act
    STRING_HANDLE decodedURL = URL_DecodeString(newString);

    //assert
    ASSERT_IS_NOT_NULL(decodedURL);
    ASSERT_ARE_EQUAL(size_t, 0, strlen(STRING_c_str(decodedURL)));

    //cleanup
    STRING_delete(decodedURL);
}

TEST_FUNCTION(URL_DecodeString_unencoded_input)
{
    //arrange
    const char* newString = "hello world"; //space is not encoded

    //act
    STRING_HANDLE decodedURL = URL_DecodeString(newString);

    //assert
    ASSERT_IS_NULL(decodedURL);

    //cleanup
}

TEST_FUNCTION(URL_DecodeString_partially_unencoded_input)
{
    //arrange
    const char* newString = "hello%20world&mistake"; //& is an unencoded char

    //act
    STRING_HANDLE decodedURL = URL_DecodeString(newString);

    //assert
    ASSERT_IS_NULL(decodedURL);

    //cleanup
}

TEST_FUNCTION(URL_DecodeString_invalid_encoding_incomplete)
{
    //arrange
    const char* newString = "%7";

    //act
    STRING_HANDLE decodedURL = URL_DecodeString(newString);

    //assert
    ASSERT_IS_NULL(decodedURL);

    //cleanup
}

TEST_FUNCTION(URL_DecodeString_invalid_encoding_non_hex)
{
    //arrange
    const char* newString = "%G5";

    //act
    STRING_HANDLE decodedURL = URL_DecodeString(newString);

    //assert
    ASSERT_IS_NULL(decodedURL);

    //cleanup
}

TEST_FUNCTION(URL_DecodeString_invalid_has_multibyte_encoding)
{
    //arrange
    const char* newString = "%C2%B4";

    //act
    STRING_HANDLE decodedURL = URL_DecodeString(newString);

    //assert
    ASSERT_IS_NULL(decodedURL);

    //cleanup
}

TEST_FUNCTION(URL_DecodeString_unreserved_mapping)
{
    //arrange

    //act
    STRING_HANDLE decodedURL = URL_DecodeString(UNRESERVED_CHAR);

    //assert
    ASSERT_IS_NOT_NULL(decodedURL);
    ASSERT_ARE_EQUAL(char_ptr, UNRESERVED_CHAR, STRING_c_str(decodedURL));

    //cleanup
    STRING_delete(decodedURL);
}

TEST_FUNCTION(URL_DecodeString_path_with_device)
{
    //arrange
    const char* input = "%2fgetalarm(%27Le%20Pichet%27)";

    //act
    STRING_HANDLE decodedURL = URL_DecodeString(input);

    //assert
    ASSERT_IS_NOT_NULL(decodedURL);
    ASSERT_ARE_EQUAL(char_ptr, "/getalarm('Le Pichet')", STRING_c_str(decodedURL));

    //cleanup
    STRING_delete(decodedURL);
}

TEST_FUNCTION(URL_DecodeString_a_few_bogus_characters)
{
    //arrange
    const char* input = "%7b%7d%25";

    //act
    STRING_HANDLE decodedURL = URL_DecodeString(input);

    //assert
    ASSERT_IS_NOT_NULL(decodedURL);
    ASSERT_ARE_EQUAL(char_ptr, "{}%", STRING_c_str(decodedURL));

    //cleanup
    STRING_delete(decodedURL);
}

TEST_FUNCTION(URL_DecodeString_full_url)
{
    //arrange
    const char* input = "https%3a%2f%2fone.two.three.four-five.com%2fsix%2fSeven(%27EightNine1234567890.Ten_Eleven%27)%3ftwelve-thirteen%3d2015-11-31%20HTTP%2f1.1";

    //act
    STRING_HANDLE decodedURL = URL_DecodeString(input);

    //assert
    ASSERT_IS_NOT_NULL(decodedURL);
    ASSERT_ARE_EQUAL(char_ptr, "https://one.two.three.four-five.com/six/Seven('EightNine1234567890.Ten_Eleven')?twelve-thirteen=2015-11-31 HTTP/1.1", STRING_c_str(decodedURL));

    //cleanup
    STRING_delete(decodedURL);
}

TEST_FUNCTION(URL_DecodeString_ASCII_chars)
{
    size_t i;
    size_t numberOfTests = sizeof(testVectorASCII) / sizeof(testVectorASCII[i]);

    for (i = 0; i < numberOfTests; i++)
    {
        //arrange
        STRING_HANDLE decoded;

        //act
        decoded = URL_DecodeString(testVectorASCII[i].endcodedRep);

        //assert
        ASSERT_IS_NOT_NULL(decoded);
        ASSERT_ARE_EQUAL(char_ptr, testVectorASCII[i].charRep, STRING_c_str(decoded));
        STRING_delete(decoded);
    }
}

TEST_FUNCTION(URL_DecodeString_Extended_ASCII_chars)
{
    size_t i;
    size_t numberOfTests = sizeof(testVectorExtendedASCII) / sizeof(testVectorExtendedASCII[i]);

    for (i = 0; i < numberOfTests; i++)
    {
        //arrange
        STRING_HANDLE decoded;

        //act
        decoded = URL_DecodeString(testVectorExtendedASCII[i].endcodedRep);

        //assert
        ASSERT_IS_NULL(decoded); //these should be rejected by the function
        STRING_delete(decoded);
    }
}

TEST_FUNCTION(URL_Decode_null_input)
{
    //arrange

    //act
    STRING_HANDLE decodedURL = URL_Decode(NULL);

    //assert
    ASSERT_IS_NULL(decodedURL);

    //cleanup
}

TEST_FUNCTION(URL_Decode_zerolength_input)
{
    //arrange
    STRING_HANDLE decodedURL;
    STRING_HANDLE newString = STRING_new();
    ASSERT_ARE_EQUAL(int, 0, STRING_concat(newString, ""));

    //act
    decodedURL = URL_Decode(newString);

    //assert
    ASSERT_IS_NOT_NULL(decodedURL);
    ASSERT_ARE_EQUAL(size_t, 0, strlen(STRING_c_str(decodedURL)));

    //cleanup
    STRING_delete(decodedURL);
    STRING_delete(newString);
}

TEST_FUNCTION(URL_Decode_unencoded_input)
{
    //arrange
    STRING_HANDLE decodedURL;
    STRING_HANDLE newString = STRING_new();
    ASSERT_ARE_EQUAL(int, 0, STRING_concat(newString, "hello world")); //space is not encoded

    //act
    decodedURL = URL_Decode(newString);

    //assert
    ASSERT_IS_NULL(decodedURL);

    //cleanup
    STRING_delete(newString);
}

TEST_FUNCTION(URL_Decode_partially_unencoded_input)
{
    //arrange
    STRING_HANDLE decodedURL;
    STRING_HANDLE newString = STRING_new();
    ASSERT_ARE_EQUAL(int, 0, STRING_concat(newString, "hello%20world&mistake")); //& is an unencoded char

    //act
    decodedURL = URL_Decode(newString);

    //assert
    ASSERT_IS_NULL(decodedURL);

    //cleanup
    STRING_delete(newString);
}

TEST_FUNCTION(URL_Decode_invalid_encoding_incomplete)
{
    //arrange
    STRING_HANDLE decodedURL;
    STRING_HANDLE newString = STRING_new();
    ASSERT_ARE_EQUAL(int, 0, STRING_concat(newString, "%7"));

    //act
    decodedURL = URL_Decode(newString);

    //assert
    ASSERT_IS_NULL(decodedURL);

    //cleanup
    STRING_delete(newString);
}

TEST_FUNCTION(URL_Decode_invalid_encoding_non_hex)
{
    //arrange
    STRING_HANDLE decodedURL;
    STRING_HANDLE newString = STRING_new();
    ASSERT_ARE_EQUAL(int, 0, STRING_concat(newString, "%G5"));

    //act
    decodedURL = URL_Decode(newString);

    //assert
    ASSERT_IS_NULL(decodedURL);

    //cleanup
    STRING_delete(newString);
}

TEST_FUNCTION(URL_Decode_invalid_has_multibyte_encoding)
{
    //arrange
    STRING_HANDLE decodedURL;
    STRING_HANDLE newString = STRING_new();
    ASSERT_ARE_EQUAL(int, 0, STRING_concat(newString, "%C2%B4"));

    //act
    decodedURL = URL_Decode(newString);

    //assert
    ASSERT_IS_NULL(decodedURL);

    //cleanup
    STRING_delete(newString);
}

TEST_FUNCTION(URL_Decode_unreserved_mapping)
{
    //arrange
    STRING_HANDLE decodedURL;
    STRING_HANDLE newString = STRING_new();
    ASSERT_ARE_EQUAL(int, 0, STRING_concat(newString, UNRESERVED_CHAR));

    //act
    decodedURL = URL_Decode(newString);

    //assert
    ASSERT_IS_NOT_NULL(decodedURL);
    ASSERT_ARE_EQUAL(char_ptr, UNRESERVED_CHAR, STRING_c_str(decodedURL));

    //cleanup
    STRING_delete(decodedURL);
    STRING_delete(newString);
}

TEST_FUNCTION(URL_Decode_path_with_device)
{
    //arrange
    STRING_HANDLE decodedURL;
    STRING_HANDLE input = STRING_new();
    ASSERT_ARE_EQUAL(int, 0, STRING_concat(input, "%2fgetalarm(%27Le%20Pichet%27)"));

    //act
    decodedURL = URL_Decode(input);

    //assert
    ASSERT_IS_NOT_NULL(decodedURL);
    ASSERT_ARE_EQUAL(char_ptr, "/getalarm('Le Pichet')", STRING_c_str(decodedURL));

    //cleanup
    STRING_delete(decodedURL);
    STRING_delete(input);
}

TEST_FUNCTION(URL_Decode_a_few_bogus_characters)
{
    //arrange
    STRING_HANDLE decodedURL;
    STRING_HANDLE input = STRING_new();
    ASSERT_ARE_EQUAL(int, 0, STRING_concat(input, "%7b%7d%25"));

    //act
    decodedURL = URL_Decode(input);

    //assert
    ASSERT_IS_NOT_NULL(decodedURL);
    ASSERT_ARE_EQUAL(char_ptr, "{}%", STRING_c_str(decodedURL));

    //cleanup
    STRING_delete(decodedURL);
    STRING_delete(input);
}

TEST_FUNCTION(URL_Decode_full_url)
{
    //arrange
    STRING_HANDLE decodedURL;
    STRING_HANDLE input = STRING_new();
    ASSERT_ARE_EQUAL(int, 0, STRING_concat(input, "https%3a%2f%2fone.two.three.four-five.com%2fsix%2fSeven(%27EightNine1234567890.Ten_Eleven%27)%3ftwelve-thirteen%3d2015-11-31%20HTTP%2f1.1"));

    //act
    decodedURL = URL_Decode(input);

    //assert
    ASSERT_IS_NOT_NULL(decodedURL);
    ASSERT_ARE_EQUAL(char_ptr, "https://one.two.three.four-five.com/six/Seven('EightNine1234567890.Ten_Eleven')?twelve-thirteen=2015-11-31 HTTP/1.1", STRING_c_str(decodedURL));

    //cleanup
    STRING_delete(decodedURL);
    STRING_delete(input);
}

TEST_FUNCTION(URL_Decode_ASCII_chars)
{
    size_t i;
    size_t numberOfTests = sizeof(testVectorASCII) / sizeof(testVectorASCII[i]);

    for (i = 0; i < numberOfTests; i++)
    {
        //arrange
        STRING_HANDLE decoded;
        STRING_HANDLE encoded = STRING_new();
        ASSERT_ARE_EQUAL(int, STRING_concat(encoded, testVectorASCII[i].endcodedRep), 0);

        //act
        decoded = URL_Decode(encoded);

        //assert
        ASSERT_IS_NOT_NULL(decoded);
        ASSERT_ARE_EQUAL(char_ptr, testVectorASCII[i].charRep, STRING_c_str(decoded));
        STRING_delete(encoded);
        STRING_delete(decoded);
    }
}

TEST_FUNCTION(URL_Decode_Extended_ASCII_chars)
{
    size_t i;
    size_t numberOfTests = sizeof(testVectorExtendedASCII) / sizeof(testVectorExtendedASCII[i]);

    for (i = 0; i < numberOfTests; i++)
    {
        //arrange
        STRING_HANDLE decoded;
        STRING_HANDLE encoded = STRING_new();
        ASSERT_ARE_EQUAL(int, STRING_concat(encoded, testVectorExtendedASCII[i].endcodedRep), 0);

        //act
        decoded = URL_Decode(encoded);

        //assert
        ASSERT_IS_NULL(decoded); //these should be rejected by the function

        //cleanup
        STRING_delete(encoded);
    }
}

END_TEST_SUITE(URLEncode_UnitTests)
