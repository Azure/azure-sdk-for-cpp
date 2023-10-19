// Copyright(C) Microsoft Corporation.All rights reserved.

#ifdef __cplusplus
#include <cinttypes>
#include <clocale>
#else
#include <inttypes.h>
#include <stdbool.h>
#include <locale.h>
#endif

#include "windows.h"

#include "testrunnerswitcher.h"

#include "azure_c_shared_utility/string_utils.h"


BEGIN_TEST_SUITE(string_utils_int_tests)

TEST_SUITE_INITIALIZE(suite_init)
{

}

TEST_FUNCTION_INITIALIZE(init)
{

}

TEST_FUNCTION_CLEANUP(cleanup)
{

}

TEST_FUNCTION(mbs_to_wcs_converts_a_simple_LOCALE_C_string)
{
    ///arrange
    const char* s = "a";

    ///act
    wchar_t* result = mbs_to_wcs(s);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_IS_TRUE(result[0] == L'a');
    ASSERT_IS_TRUE(result[1] == L'\0');

    ///clean
    free(result);
}

/*the values in this test are taken from https://docs.microsoft.com/cpp/c-runtime-library/reference/mbstowcs-mbstowcs-l?view=vs-2017*/
TEST_FUNCTION(mbs_to_wcs_converts_a_Japanese_string)
{
    ///arrange
    char* localeInfo = setlocale(LC_ALL, "Japanese_Japan.932");
    ASSERT_IS_NOT_NULL(localeInfo);

    const char* multibyteJapanese = "\x82\xa0\x82\xa1";

    ///act
    wchar_t* result = mbs_to_wcs(multibyteJapanese);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_IS_TRUE(result[0] == L'\x3042');
    ASSERT_IS_TRUE(result[1] == L'\x3043');

    ///clean
    free(result);
    localeInfo=setlocale(LC_ALL, "");
    ASSERT_IS_NOT_NULL(localeInfo);
}

TEST_FUNCTION(wcs_to_mbs_converts_a_simple_LOCALE_C_string)
{
    ///arrange
    const wchar_t* s = L"a";

    ///act
    char* result = wcs_to_mbs(s);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_IS_TRUE(result[0] == 'a');
    ASSERT_IS_TRUE(result[1] == '\0');

    ///clean
    free(result);
}

/*the values in this test are taken from https://docs.microsoft.com/cpp/c-runtime-library/reference/mbstowcs-mbstowcs-l?view=vs-2017*/
TEST_FUNCTION(wcs_to_mbs_converts_a_Japanese_string)
{
    ///arrange
    char* localeInfo = setlocale(LC_ALL, "Japanese_Japan.932");
    ASSERT_IS_NOT_NULL(localeInfo);

    const wchar_t* wideJapanese = L"\x3042\x3043";

    ///act
    char* result = wcs_to_mbs(wideJapanese);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_IS_TRUE(result[0] == '\x82');
    ASSERT_IS_TRUE(result[1] == '\xa0');
    ASSERT_IS_TRUE(result[2] == '\x82');
    ASSERT_IS_TRUE(result[3] == '\xa1');

    ///clean
    free(result);
    localeInfo = setlocale(LC_ALL, "");
    ASSERT_IS_NOT_NULL(localeInfo);
}

TEST_FUNCTION(GUID_FORMAT_AND_VALUES)
{
    ///arrange
    struct GUID_AND_EXPECTED_STRINGS
    {
        GUID guid;
        const char* expectedGuidAsString;
    } guidAndExpectedStrings[] =
    {
        {/*[0]*/
            {
                0,                                          /*unsigned long  Data1;     */
                0,                                          /*unsigned short Data2;     */
                0,                                          /*unsigned short Data3;     */
                {0,0,0,0,0,0,0,0}                           /*unsigned char  Data4[8];  */
            },
            "00000000-0000-0000-0000-000000000000"
        },
        {/*[1]*/
            {
                0xFFFFFFFF,                                 /*unsigned long  Data1;     */
                0xFFFF,                                     /*unsigned short Data2;     */
                0xFFFF,                                     /*unsigned short Data3;     */
                {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}   /*unsigned char  Data4[8];  */
            },
            "ffffffff-ffff-ffff-ffff-ffffffffffff"
        },
        {/*[2]*/
            /*a most famous bug that needed testing - it was producing 1f018f1a-1b1f-40ad-b78d-d577f2b27821
            instead of the expected                                    1f018f1a-1b1f-40ad-b78d-d578f2b27821 and we had great fun with it!*/
            {
                0x1f018f1a,                                 /*unsigned long  Data1;     */
                0x1b1f,                                     /*unsigned short Data2;     */
                0x40ad,                                     /*unsigned short Data3;     */
                {0xb7,0x8d,0xd5,0x78,0xf2,0xb2,0x78,0x21}   /*unsigned char  Data4[8];  */
            },
            "1f018f1a-1b1f-40ad-b78d-d578f2b27821"
        }
    };

    ///act
    for (size_t i = 0; i < sizeof(guidAndExpectedStrings) / sizeof(guidAndExpectedStrings[0]); i++)
    {
        char* actual = sprintf_char("%" GUID_FORMAT "", GUID_VALUES(guidAndExpectedStrings[i].guid));
        ASSERT_IS_NOT_NULL(actual);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, guidAndExpectedStrings[i].expectedGuidAsString, actual);

        ///clean
        free(actual);
    }
}

TEST_FUNCTION(sprintf_char_with_empty_string_succeeds)
{
    ///arrange
    char* result;

    ///act
    result = sprintf_char("%s", "");

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, "", result);

    /// cleanup
    free(result);
}

TEST_FUNCTION(sprintf_char_with_a_non_empty_string_succeeds)
{
    ///arrange
    char* result;

    ///act
    result = sprintf_char("%s", "Kardel Sharpeye");

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, "Kardel Sharpeye", result);

    /// cleanup
    free(result);
}

static char* vsprintf_char_wrapper_function(const char* format, ...)
{
    char* result;
    va_list va;
    va_start(va, format);
    result = vsprintf_char(format, va);
    va_end(va);
    return result;
}

TEST_FUNCTION(vsprintf_char_with_empty_string_succeeds)
{
    ///arrange
    char* result;

    ///act
    result = vsprintf_char_wrapper_function("%s", "");

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, "", result);

    /// cleanup
    free(result);
}

TEST_FUNCTION(vsprintf_char_with_a_non_empty_string_succeeds)
{
    ///arrange
    char* result;

    ///act
    result = vsprintf_char_wrapper_function("%s", "Kardel Sharpeye");

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, "Kardel Sharpeye", result);

    /// cleanup
    free(result);
}

TEST_FUNCTION(sprintf_wchar_with_empty_string_succeeds)
{
    ///arrange
    wchar_t* result;

    ///act
    result = sprintf_wchar(L"%ls", L"");

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, "", result);

    /// cleanup
    free(result);
}

TEST_FUNCTION(sprintf_wchar_with_a_non_empty_string_succeeds)
{
    ///arrange
    wchar_t* result;

    ///act
    result = sprintf_wchar(L"%s", L"Kardel Sharpeye");

    ///assert
    // ctest could use a wchar_t
    ASSERT_ARE_EQUAL(int, 0, wcscmp(result, L"Kardel Sharpeye"));

    /// cleanup
    free(result);
}

static wchar_t* vsprintf_wchar_wrapper_function(const wchar_t* format, ...)
{
    wchar_t* result;
    va_list va;
    va_start(va, format);
    result = vsprintf_wchar(format, va);
    va_end(va);
    return result;
}

TEST_FUNCTION(vsprintf_wchar_with_empty_string_succeeds)
{
    ///arrange
    wchar_t* result;

    ///act
    result = vsprintf_wchar_wrapper_function(L"%s", L"");

    ///assert
    ASSERT_ARE_EQUAL(int, 0, wcscmp(result, L""));

    /// cleanup
    free(result);
}

TEST_FUNCTION(vsprintf_wchar_with_a_non_empty_string_succeeds)
{
    ///arrange
    wchar_t* result;

    ///act
    result = vsprintf_wchar_wrapper_function(L"%s", L"Kardel Sharpeye");

    ///assert
    ASSERT_ARE_EQUAL(int, 0, wcscmp(result, L"Kardel Sharpeye"));

    /// cleanup
    free(result);
}

END_TEST_SUITE(string_utils_int_tests)
