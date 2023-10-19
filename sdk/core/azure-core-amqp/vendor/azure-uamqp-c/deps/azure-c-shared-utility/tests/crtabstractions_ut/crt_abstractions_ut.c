// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <climits>
#include <cfloat>
#include <cmath>
#else
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"

// VS 2008 does not have INFINITY and all the nice goodies...
#if defined (TIZENRT)
#define DEFINE_INFINITY 1
#else

#if defined _MSC_VER
#if _MSC_VER <= 1500
#define DEFINE_INFINITY 1
#endif
#endif
#endif

#if defined DEFINE_INFINITY

#pragma warning(disable:4756 4056) // warning C4756: overflow in constant arithmetic

// These defines are missing in math.h for WEC2013 SDK
#ifndef _HUGE_ENUF
#define _HUGE_ENUF  1e+300  // _HUGE_ENUF*_HUGE_ENUF must overflow
#endif

#define INFINITY   ((float)(_HUGE_ENUF * _HUGE_ENUF))
#define HUGE_VALF  ((float)INFINITY)
#define HUGE_VALL  ((long double)INFINITY)
#define NAN        ((float)(INFINITY * 0.0F))

#define isnan _isnan

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

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#undef ENABLE_MOCKS

#include "azure_c_shared_utility/crt_abstractions.h"
#include <errno.h>

#if defined _MSC_VER
#include "crtdbg.h"
static _invalid_parameter_handler oldInvalidParameterHandler;
static int oldReportType;

static void my_invalid_parameter_handler(
    const wchar_t * expression,
    const wchar_t * function,
    const wchar_t * file,
    unsigned int line,
    uintptr_t pReserved
    )
{
    (void)expression;
    (void)function;
    (void)file;
    (void)line;
    (void)pReserved;
    /*do nothing*/
}
/* The below defines are because on Windows platform, the secure version of the CRT functions will invoke WATSON if no invalid parameter handler provided by the user */
#define HOOK_INVALID_PARAMETER_HANDLER() {oldInvalidParameterHandler = _set_invalid_parameter_handler(my_invalid_parameter_handler);oldReportType=_CrtSetReportMode(_CRT_ASSERT, 0);}
#define UNHOOK_INVALID_PARAMETER_HANDLER() {(void)_CrtSetReportMode(_CRT_ASSERT, oldReportType); (void)_set_invalid_parameter_handler(oldInvalidParameterHandler);}

#else /* _MSC_VER */
#define HOOK_INVALID_PARAMETER_HANDLER() do{}while(0)
#define UNHOOK_INVALID_PARAMETER_HANDLER() do{}while(0)
#endif /* _MSC_VER */

static const unsigned int interestingUnsignedIntNumbersToBeConverted[] =
{
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    21,
    32,
    43,
    54,
    65,
    76,
    87,
    98,
    123,
    1234,
    12341,
    UINT_MAX / 2,
    UINT_MAX -1,
    UINT_MAX,
    42,
    0x42
};

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)~(size_t)0)
#endif

static const size_t interestingSize_tNumbersToBeConverted[] =
{
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    21,
    32,
    43,
    54,
    65,
    76,
    87,
    98,
    123,
    1234,
    12341,
    SIZE_MAX / 2,
    SIZE_MAX -1,
    SIZE_MAX,
    42,
    0x42
};

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(CRTAbstractions_UnitTests)

TEST_SUITE_INITIALIZE(a)
{
    umock_c_init(on_umock_c_error);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, real_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, real_free);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, real_realloc);
}

TEST_SUITE_CLEANUP(b)
{
    umock_c_deinit();
}

/* strcat_s */

// Tests_SRS_CRT_ABSTRACTIONS_99_008: [strcat_s shall append the src to dst and terminates the resulting string with a null character.]
// Tests_SRS_CRT_ABSTRACTIONS_99_009: [The initial character of src shall overwrite the terminating null character of dst.]
// Tests_SRS_CRT_ABSTRACTIONS_99_003: [strcat_s shall return Zero upon success.]
TEST_FUNCTION(strcat_s_Appends_Source_To_Destination)
{
    // arrange
    char dstString[128] = "Destination";
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "Source";
    int result;

    // act
    result = strcat_s(dstString, dstSizeInBytes, srcString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "DestinationSource", dstString);
    ASSERT_ARE_EQUAL(int, 0, result);
}

TEST_FUNCTION(strcat_s_Appends_Empty_Source_To_Destination)
{
    // arrange
    char dstString[128] = "Destination";
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "";
    int result;

    // act
    result = strcat_s(dstString, dstSizeInBytes, srcString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "Destination", dstString);
    ASSERT_ARE_EQUAL(int, 0, result);
}

TEST_FUNCTION(strcat_s_Appends_Source_To_Empty_Destination)
{
    // arrange
    char dstString[128] = "";
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "Source";
    int result;

    // act
    result = strcat_s(dstString, dstSizeInBytes, srcString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "Source", dstString);
    ASSERT_ARE_EQUAL(int, 0, result);
}

TEST_FUNCTION(strcat_s_Appends_Empty_Source_To_Empty_Destination)
{
    // arrange
    char dstString[128] = "";
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "";
    int result;

    // act
    result = strcat_s(dstString, dstSizeInBytes, srcString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", dstString);
    ASSERT_ARE_EQUAL(int, 0, result);
}

// Tests_SRS_CRT_ABSTRACTIONS_99_004: [If dst is NULL or unterminated, the error code returned shall be EINVAL & dst shall not be modified.]
TEST_FUNCTION(strcat_s_With_NULL_Destination_Fails)
{
    // arrange
    char* dstString = NULL;
    size_t sizeInBytes = sizeof(dstString);
    char srcString[] = "Source";
    int result;

    // act
    HOOK_INVALID_PARAMETER_HANDLER();
#ifdef _MSC_VER
#pragma warning(suppress: 6387) /* This is test code, explictly calling with NULL argument */
#endif
    result = strcat_s(dstString, sizeInBytes, srcString);
    UNHOOK_INVALID_PARAMETER_HANDLER();

    // assert
    ASSERT_IS_NULL(dstString);
    ASSERT_ARE_EQUAL(int, EINVAL, result);
}

TEST_FUNCTION(strcat_s_With_Unterminated_Destination_Fails)
{
    // arrange
    size_t i;
    char dstString[128];
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "Source";
    int result;
    for (i = 0; i < dstSizeInBytes; i++)
    {
        dstString[i] = 'z';
    }

    // act
    HOOK_INVALID_PARAMETER_HANDLER();
    result = strcat_s(dstString, dstSizeInBytes, srcString);
    UNHOOK_INVALID_PARAMETER_HANDLER();

    // assert
#ifndef _MSC_VER    /* MSDN claims that content of destination buffer is not modified, but that is not true. Filing bug. */
    for (size_t i = 0; i < dstSizeInBytes; i++)
    {
        ASSERT_ARE_EQUAL(char, 'z', dstString[i]);
    }
#endif
    ASSERT_ARE_EQUAL(int, EINVAL, result);
}

// Tests_SRS_CRT_ABSTRACTIONS_99_005: [If src is NULL, the error code returned shall be EINVAL and dst[0] shall be set to 0.]
TEST_FUNCTION(strcat_s_With_NULL_Source_Fails)
{
    // arrange
    char dstString[128] = "Source";
    size_t dstSizeInBytes = sizeof(dstString);
    char* srcString = NULL;
    int result;

    // act
    HOOK_INVALID_PARAMETER_HANDLER();
#ifdef _MSC_VER
#pragma warning(suppress: 6387) /* This is test code, explictly calling with NULL argument */
#endif
    result = strcat_s(dstString, dstSizeInBytes, srcString);
    UNHOOK_INVALID_PARAMETER_HANDLER();

    // assert
    ASSERT_ARE_EQUAL(char,'\0', dstString[0]);
    ASSERT_ARE_EQUAL(int, EINVAL, result);
}

// Tests_SRS_CRT_ABSTRACTIONS_99_006: [If the dstSizeInBytes is 0 or smaller than the required size for dst & src, the error code returned shall be ERANGE & dst[0] set to 0.]
TEST_FUNCTION(strcat_s_With_dstSizeInBytes_Equals_Zero_Fails)
{
    // arrange
    char dstString[128] = "Destination";
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "Source";
    int result;

    // act
    dstSizeInBytes = 0;
    HOOK_INVALID_PARAMETER_HANDLER();
    result = strcat_s(dstString, dstSizeInBytes, srcString);
    UNHOOK_INVALID_PARAMETER_HANDLER();

    // assert
#ifdef _MSC_VER /*MSDN Claims that destination buffer would be set to empty & ERANGE is the error, but not the case.  Filing bug.*/
    ASSERT_ARE_EQUAL(char_ptr, "Destination", dstString);
    ASSERT_ARE_EQUAL(int, EINVAL, result);
#else
    ASSERT_ARE_EQUAL(char, '\0', dstString[0]);
    ASSERT_ARE_EQUAL(int, ERANGE, result);
#endif
}

TEST_FUNCTION(strcat_s_With_dstSizeInBytes_Smaller_Than_dst_and_src_Fails)
{
    // arrange
    char dstString[128] = "Destination";
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "Source";
    int result;

    // act
    HOOK_INVALID_PARAMETER_HANDLER();
    dstSizeInBytes = strlen(dstString) + (strlen(srcString) - 3);
    result = strcat_s(dstString, dstSizeInBytes, srcString);
    UNHOOK_INVALID_PARAMETER_HANDLER();

    // assert
    ASSERT_ARE_EQUAL(char,'\0', dstString[0]);
    ASSERT_ARE_EQUAL(int, ERANGE, result);
}

/* strcpy_s */

// Tests_SRS_CRT_ABSTRACTIONS_99_016: [strcpy_s shall copy the contents in the address of src, including the terminating null character, to the location that's specified by dst.]
// Tests_SRS_CRT_ABSTRACTIONS_99_011 : [strcpy_s shall return Zero upon success]
TEST_FUNCTION(strcpy_s_copies_Source_into_Destination)
{
    // arrange
    char dstString[128] = "Destination";
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "Source";
    int result = 0;

    // act
    result = strcpy_s(dstString, dstSizeInBytes, srcString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "Source", dstString);
    ASSERT_ARE_EQUAL(int, 0, result);
}

TEST_FUNCTION(strcpy_s_copies_Empty_Source_into_Destination)
{
    // arrange
    char dstString[128] = "Destination";
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "";
    int result = 0;

    // act
    result = strcpy_s(dstString, dstSizeInBytes, srcString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", dstString);
    ASSERT_ARE_EQUAL(int, 0, result);
}

TEST_FUNCTION(strcpy_s_copies_Source_into_Empty_Destination)
{
    // arrange
    char dstString[128] = "";
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "Source";
    int result = 0;

    // act
    result = strcpy_s(dstString, dstSizeInBytes, srcString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "Source", dstString);
    ASSERT_ARE_EQUAL(int, 0, result);
}

TEST_FUNCTION(strcpy_s_copies_Empty_Source_into_Empty_Destination)
{
    // arrange
    char dstString[128] = "";
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "";
    int result = 0;

    // act
    result = strcpy_s(dstString, dstSizeInBytes, srcString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", dstString);
    ASSERT_ARE_EQUAL(int, 0, result);
}

// Tests_SRS_CRT_ABSTRACTIONS_99_012 : [If dst is NULL, the error code returned shall be EINVAL & dst shall not be modified.]
TEST_FUNCTION(strcpy_s_With_NULL_Destination_Fails)
{
    // arrange
    char* dstString = NULL;
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "Source";
    int result = 0;

    // act
    HOOK_INVALID_PARAMETER_HANDLER();
#ifdef _MSC_VER
#pragma warning(suppress: 6387) /* This is test code, explictly calling with NULL argument */
#endif
    result = strcpy_s(dstString, dstSizeInBytes, srcString);
    UNHOOK_INVALID_PARAMETER_HANDLER();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, NULL, dstString);
    ASSERT_ARE_EQUAL(int, EINVAL, result);
}

// Tests_SRS_CRT_ABSTRACTIONS_99_013 : [If src is NULL, the error code returned shall be EINVAL and dst[0] shall be set to 0.]
TEST_FUNCTION(strcpy_s_With_NULL_Source_Fails)
{
    // arrange
    char dstString[128] = "Destination";
    size_t dstSizeInBytes = sizeof(dstString);
    char* srcString = NULL;
    int result = 0;

    // act
    HOOK_INVALID_PARAMETER_HANDLER();
#ifdef _MSC_VER
#pragma warning(suppress: 6387) /* This is test code, explictly calling with NULL argument */
#endif
    result = strcpy_s(dstString, dstSizeInBytes, srcString);
    UNHOOK_INVALID_PARAMETER_HANDLER();

    // assert
    ASSERT_ARE_EQUAL(char,'\0', dstString[0]);
    ASSERT_ARE_EQUAL(int, EINVAL, result);
}

// Tests_SRS_CRT_ABSTRACTIONS_99_014 : [If the dstSizeInBytes is 0 or smaller than the required size for the src string, the error code returned shall be ERANGE & dst[0] set to 0.]
TEST_FUNCTION(strcpy_s_With_dstSizeInBytes_Equals_Zero_Fails)
{
    // arrange
    char dstString[128] = "Destination";
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "Source";
    int result;

    // act
    dstSizeInBytes = 0;
    HOOK_INVALID_PARAMETER_HANDLER();
    result = strcpy_s(dstString, dstSizeInBytes, srcString);
    UNHOOK_INVALID_PARAMETER_HANDLER();

    // assert
#ifdef _MSC_VER /*MSDN Claims that destination buffer would be set to empty & ERANGE is the error, but not the case.  Filing bug.*/
    ASSERT_ARE_EQUAL(char_ptr, "Destination", dstString);
    ASSERT_ARE_EQUAL(int, EINVAL, result);
#else
    ASSERT_ARE_EQUAL(char, '\0', dstString[0]);
    ASSERT_ARE_EQUAL(int, ERANGE, result);
#endif
}

TEST_FUNCTION(strcpy_s_With_dstSizeInBytes_Smaller_Than_source_Fails)
{
    // arrange
    char dstString[128] = "Destination";
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "Source";
    int result;

    // act
    HOOK_INVALID_PARAMETER_HANDLER();
    dstSizeInBytes = sizeof(srcString) - 2;
    result = strcpy_s(dstString, dstSizeInBytes, srcString);
    UNHOOK_INVALID_PARAMETER_HANDLER();

    // assert
    ASSERT_ARE_EQUAL(char,'\0', dstString[0]);
    ASSERT_ARE_EQUAL(int, ERANGE, result);
}

/* strncpy_s */

// Tests_SRS_CRT_ABSTRACTIONS_99_025 : [strncpy_s shall copy the first N characters of src to dst, where N is the lesser of MaxCount and the length of src.]
// Tests_SRS_CRT_ABSTRACTIONS_99_041 : [If those N characters will fit within dst(whose size is given as dstSizeInBytes) and still leave room for a null terminator, then those characters shall be copied and a terminating null is appended; otherwise, strDest[0] is set to the null character and ERANGE error code returned per requirement below.]
// Tests_SRS_CRT_ABSTRACTIONS_99_018: [strncpy_s shall return Zero upon success]
TEST_FUNCTION(strncpy_s_copies_N_chars_of_source_to_destination_where_maxCount_equals_source_Length)
{
    // arrange
    char dstString[] = "Destination";
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "Source";
    size_t maxCount = sizeof(srcString);
    int result;

    // act
    result = strncpy_s(dstString, dstSizeInBytes, srcString, maxCount);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "Source", dstString);
    ASSERT_ARE_EQUAL(int, 0, result);
}

TEST_FUNCTION(strncpy_s_copies_N_chars_of_source_to_destination_where_maxCount_is_larger_than_Source_Length)
{
    // arrange
    char dstString[] = "Destination";
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "Source";
    size_t maxCount = sizeof(srcString);
    int result;

    // act
    result = strncpy_s(dstString, dstSizeInBytes, srcString, maxCount+5);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "Source", dstString);
    ASSERT_ARE_EQUAL(int, 0, result);
}

TEST_FUNCTION(strncpy_s_copies_N_chars_of_source_to_destination_where_maxCount_is_less_than_source_length)
{
    // arrange
    char dstString[] = "Destination";
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "Source";
    size_t maxCount = sizeof(srcString);
    int result;

    // act
    result = strncpy_s(dstString, dstSizeInBytes, srcString, maxCount - 3);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "Sour", dstString);
    ASSERT_ARE_EQUAL(int, 0, result);
}

// Tests_SRS_CRT_ABSTRACTIONS_99_026 : [If MaxCount is _TRUNCATE(defined as - 1), then as much of src as will fit into dst shall be copied while still leaving room for the terminating null to be appended.]
TEST_FUNCTION(strncpy_s_with_maxCount_set_to_TRUNCATE_and_destination_fits_source)
{
    // arrange
    char dstString[] = "Destination";
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "Source";
    size_t maxCount = sizeof(srcString);
    int result;

    // act
    maxCount = _TRUNCATE;
    result = strncpy_s(dstString, dstSizeInBytes, srcString, maxCount);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "Source", dstString);
    ASSERT_ARE_EQUAL(int, 0, result);
}

// Tests_SRS_CRT_ABSTRACTIONS_99_026 : [If MaxCount is _TRUNCATE(defined as - 1), then as much of src as will fit into dst shall be copied while still leaving room for the terminating null to be appended.]
// Tests_SRS_CRT_ABSTRACTIONS_99_019: [If truncation occurred as a result of the copy, the error code returned shall be STRUNCATE .]
TEST_FUNCTION(strncpy_s_with_maxCount_set_to_TRUNCATE_and_destination_is_smaller_than_source)
{
    // arrange
    char dstString[] = "Dest";
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "Source";
    size_t maxCount = sizeof(srcString);
    int result;

    // act
    maxCount = _TRUNCATE;
    result = strncpy_s(dstString, dstSizeInBytes, srcString, maxCount);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "Sour", dstString);
    ASSERT_ARE_EQUAL(int, STRUNCATE, result);
}

// Tests_SRS_CRT_ABSTRACTIONS_99_020 : [If dst is NULL, the error code returned shall be EINVAL and dst shall not be modified.]
TEST_FUNCTION(strncpy_s_fails_with_destination_set_to_NULL)
{
    // arrange
    char* dstString = NULL;
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "Source";
    size_t maxCount = sizeof(srcString);
    int result;

    // act
    HOOK_INVALID_PARAMETER_HANDLER();
#ifdef _MSC_VER
#pragma warning(suppress: 6387) /* This is test code, explictly calling with NULL argument */
#endif
    result = strncpy_s(dstString, dstSizeInBytes, srcString, maxCount);
    UNHOOK_INVALID_PARAMETER_HANDLER();

    // assert
    ASSERT_IS_NULL(dstString);
    ASSERT_ARE_EQUAL(int, EINVAL, result);
}

// Tests_SRS_CRT_ABSTRACTIONS_99_021: [If src is NULL, the error code returned shall be EINVAL and dst[0] shall be set to 0.]
TEST_FUNCTION(strncpy_s_fails_with_source_set_to_NULL)
{
    // arrange
    char dstString[] = "Destination";
    size_t dstSizeInBytes = sizeof(dstString);
    char* srcString = NULL;
    size_t maxCount = sizeof(srcString);
    int result;

    // act
    HOOK_INVALID_PARAMETER_HANDLER();
#ifdef _MSC_VER
#pragma warning(suppress: 6387) /* This is test code, explictly calling with NULL argument */
#endif
    result = strncpy_s(dstString, dstSizeInBytes, srcString, maxCount);
    UNHOOK_INVALID_PARAMETER_HANDLER();

    // assert
    ASSERT_ARE_EQUAL(char,'\0', dstString[0]);
    ASSERT_ARE_EQUAL(int, EINVAL, result);
}

// Tests_SRS_CRT_ABSTRACTIONS_99_022: [If the dstSizeInBytes is 0, the error code returned shall be EINVAL and dst shall not be modified.]
TEST_FUNCTION(strncpy_s_fails_with_dstSizeInBytes_set_to_Zero)
{
    // arrange
    char dstString[] = "Destination";
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "Source";
    size_t maxCount = sizeof(srcString);
    int result;

    // act
    HOOK_INVALID_PARAMETER_HANDLER();
    dstSizeInBytes = 0;
    result = strncpy_s(dstString, dstSizeInBytes, srcString, maxCount);
    UNHOOK_INVALID_PARAMETER_HANDLER();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "Destination", dstString);
    ASSERT_ARE_EQUAL(int, EINVAL, result);
}

// Tests_SRS_CRT_ABSTRACTIONS_99_023 : [If dst is not NULL & dstSizeInBytes is smaller than the required size for the src string, the error code returned shall be ERANGE and dst[0] shall be set to 0.]
TEST_FUNCTION(strncpy_s_dstSizeInBytes_is_smaller_than_the_required_size_for_source)
{
    // arrange
    char dstString[] = "Dest";
    size_t dstSizeInBytes = sizeof(dstString);
    char srcString[] = "Source";
    size_t maxCount = sizeof(srcString);
    int result;

    // act
    HOOK_INVALID_PARAMETER_HANDLER();
    result = strncpy_s(dstString, dstSizeInBytes, srcString, maxCount);
    UNHOOK_INVALID_PARAMETER_HANDLER();

    // assert
    ASSERT_ARE_EQUAL(char,'\0', dstString[0]);
    ASSERT_ARE_EQUAL(int, ERANGE, result);
}

/* sprintf_s */

// Tests_SRS_CRT_ABSTRACTIONS_99_029: [The sprintf_s function shall format and store series of characters and values in dst.Each argument(if any) is converted and output according to the corresponding Format Specification in the format variable.]
// Tests_SRS_CRT_ABSTRACTIONS_99_031: [A null character is appended after the last character written.]
// Tests_SRS_CRT_ABSTRACTIONS_99_027: [sprintf_s shall return the number of characters stored in dst upon success.  This number shall not include the terminating null character.]
TEST_FUNCTION(sprintf_s_formats_and_stores_chars_and_values_in_destination)
{
    // arrange
    char dstString[1024];
    size_t dstSizeInBytes = sizeof(dstString);
    char expected_string[] = "sprintf_s: 123, hello, Z, 1.5";
    int expected_string_size = (int)(sizeof(expected_string));
    int result;

    // act
    result = sprintf_s(dstString, dstSizeInBytes, "sprintf_s: %d, %s, %c, %3.1f", 123, "hello", 'Z', 1.5f);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, expected_string, dstString);
    ASSERT_ARE_EQUAL(int, expected_string_size -1, result);
}

// Tests_SRS_CRT_ABSTRACTIONS_99_028: [If dst or format is a null pointer, sprintf_s shall return -1.]
TEST_FUNCTION(sprintf_s_fails_with_dst_set_to_null)
{
    // arrange
    char* dstString = NULL;
    size_t dstSizeInBytes = sizeof(dstString);
    int result;

    // act
    HOOK_INVALID_PARAMETER_HANDLER();
#ifdef _MSC_VER
#pragma warning(suppress: 6387) /* This is test code, explictly calling with NULL argument */
#endif
    result = sprintf_s(dstString, dstSizeInBytes, "sprintf_s: %d, %s, %c, %3.1f", 123, "hello", 'Z', 1.5f);
    UNHOOK_INVALID_PARAMETER_HANDLER();

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
    ASSERT_ARE_EQUAL(int, EINVAL, errno);
}

TEST_FUNCTION(sprintf_s_fails_with_format_set_to_null)
{
    // arrange
    char dstString[1024];
    size_t dstSizeInBytes = sizeof(dstString);
    int result;

    // act
    HOOK_INVALID_PARAMETER_HANDLER();
#ifdef _MSC_VER
#pragma warning(suppress: 6387) /* This is test code, explictly calling with NULL argument */
#endif
    result = sprintf_s(dstString, dstSizeInBytes, NULL);
    UNHOOK_INVALID_PARAMETER_HANDLER();

    // assert
    ASSERT_ARE_EQUAL(int, -1, result);
    ASSERT_ARE_EQUAL(int, EINVAL, errno);
}

// Tests_SRS_CRT_ABSTRACTIONS_99_034 : [If the dst buffer is too small for the text being printed, then dst is set to an empty string and the function shall return -1.]
TEST_FUNCTION(sprintf_s_fails_with_dst_too_small)
{
    // arrange
    char dstString[5];
    size_t dstSizeInBytes = sizeof(dstString);
    int result;

    // act
    HOOK_INVALID_PARAMETER_HANDLER();
    result = sprintf_s(dstString, dstSizeInBytes, "sprintf_s: %d, %s, %c, %3.1f", 123, "hello", 'Z', 1.5f);
    UNHOOK_INVALID_PARAMETER_HANDLER();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", dstString);
    ASSERT_ARE_EQUAL(int, -1, result);
}

TEST_FUNCTION(sprintf_s_fails_with_dst_buffer_size_not_fitting_null_char)
{
    // arrange
    char dstString[5];
    size_t dstSizeInBytes = sizeof(dstString);
    int result;

    // act
    HOOK_INVALID_PARAMETER_HANDLER();
    result = sprintf_s(dstString, dstSizeInBytes, "12345");
    UNHOOK_INVALID_PARAMETER_HANDLER();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", dstString);
    ASSERT_ARE_EQUAL(int, -1, result);
}

/* strtoull_s */

/*Tests_SRS_CRT_ABSTRACTIONS_21_014: [If the correct value is outside the range, the strtoull_s returns the value ULLONG_MAX, and errno will receive the value ERANGE.]*/
TEST_FUNCTION(strtoull_s_decimal_base_max_ull_64bit_success)
{
    // arrange
    const char* subjectStr = "18446744073709551615";
    char* endptr;
    int base = 10;
    uint64_t result;

    uint64_t expectedResult = ULLONG_MAX;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(int, 0, errno);
    ASSERT_ARE_EQUAL(char_ptr, "18446744073709551615", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_014: [If the correct value is outside the range, the strtoull_s returns the value ULLONG_MAX, and errno will receive the value ERANGE.]*/
TEST_FUNCTION(strtoull_s_hexadecimal_base_max_ull_128bit_success)
{
    // arrange
    const char* subjectStr = "0xffffffffffffffffffffffffffffffff";
    char* endptr;
    int base = 16;
    unsigned long long result;
    size_t ull_size = sizeof(unsigned long long);

    unsigned long long expectedResult = ULLONG_MAX;

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(int, ERANGE, errno);
    if (ull_size == 128)
    {
        uint64_t upperResult = (uint64_t)((result >> 63) / 2);
        uint64_t lowerResult = (uint64_t)(result & ULLONG_MAX);
        ASSERT_ARE_EQUAL(uint64_t, expectedResult, upperResult);
        ASSERT_ARE_EQUAL(uint64_t, expectedResult, lowerResult);
    }
    else
    {
        ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    }
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_038: [If the subject sequence starts with a negative sign, the strtoull_s will convert it to the posive representation of the negative value.]*/
TEST_FUNCTION(strtoull_s_negative_nanber_decimal_base_ull_success)
{
    // arrange
    const char* subjectStr = "-5";
    char* endptr;
    int base = 10;
    uint64_t result;

    uint64_t expectedResult = (uint64_t)(-5);
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(int, 0, errno);
    ASSERT_ARE_EQUAL(char_ptr, "-5", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_014: [If the correct value is outside the range, the strtoull_s returns the value ULLONG_MAX, and errno will receive the value ERANGE.]*/
TEST_FUNCTION(strtoull_s_decimal_base_overflow_max_ull_fail)
{
    // arrange
    const char* subjectStr = "18446744073709551616";
    char* endptr;
    int base = 10;
    uint64_t result;

    uint64_t expectedResult = ULLONG_MAX;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(int, ERANGE, errno);
    ASSERT_ARE_EQUAL(char_ptr, "18446744073709551616", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_014: [If the correct value is outside the range, the strtoull_s returns the value ULLONG_MAX, and errno will receive the value ERANGE.]*/
TEST_FUNCTION(strtoull_s_hexadecimal_base_overflow_max_ull_fail)
{
    // arrange
    const char* subjectStr = "0xFFFFFFFFFFFFFFFFF";
    char* endptr;
    int base = 16;
    uint64_t result;

    uint64_t expectedResult = ULLONG_MAX;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(int, ERANGE, errno);
    ASSERT_ARE_EQUAL(char_ptr, "0xFFFFFFFFFFFFFFFFF", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_001: [The strtoull_s must convert the initial portion of the string pointed to by nptr to uint64_t int representation.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_002: [The strtoull_s must resembling an integer represented in some radix determined by the value of base.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_003: [The strtoull_s must return the integer that represents the value in the initial part of the string. If any.]*/
TEST_FUNCTION(strtoull_s_decimal_base_success)
{
    // arrange
    const char* subjectStr = "123456";
    char* endptr;
    int base = 10;
    uint64_t result;

    uint64_t expectedResult = 123456ULL;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "123456", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_011: [The valid sequence starts after the first non-white-space character, followed by an optional positive or negative sign, a number or a letter(depending of the base).]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_010: [The white-space must be one of the characters ' ', '\f', '\n', '\r', '\t', '\v'.]*/
TEST_FUNCTION(strtoull_s_decimal_base_with_spaces_success)
{
    // arrange
    const char* subjectStr = "  123456";
    char* endptr;
    int base = 10;
    uint64_t result;

    uint64_t expectedResult = 123456ULL;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "  123456", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_010: [The white-space must be one of the characters ' ', '\f', '\n', '\r', '\t', '\v'.]*/
TEST_FUNCTION(strtoull_s_decimal_base_with_tab_success)
{
    // arrange
    const char* subjectStr = " \t 123456";
    char* endptr;
    int base = 10;
    uint64_t result;

    uint64_t expectedResult = 123456ULL;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, " \t 123456", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_011: [The valid sequence starts after the first non-white-space character, followed by an optional positive or negative sign, a number or a letter(depending of the base).]*/
TEST_FUNCTION(strtoull_s_decimal_base_with_plus_signal_success)
{
    // arrange
    const char* subjectStr = "  +123456";
    char* endptr;
    int base = 10;
    uint64_t result;

    uint64_t expectedResult = 123456ULL;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "  +123456", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_038: [If the subject sequence starts with a negative sign, the strtoull_s will convert it to the posive representation of the negative value.]*/
TEST_FUNCTION(strtoull_s_decimal_base_with_minus_sign_fail)
{
    // arrange
    const char* subjectStr = "  -123456";
    char* endptr;
    int base = 10;
    uint64_t result;

    uint64_t expectedResult = (uint64_t)(-123456);
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "  -123456", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_004: [The strtoull_s must return in endptr a final string of one or more unrecognized characters, including the terminating null character of the input string.]*/
TEST_FUNCTION(strtoull_s_decimal_base_follow_by_spaces_success)
{
    // arrange
    const char* subjectStr = "123456   ";
    char* endptr;
    int base = 10;
    uint64_t result;

    uint64_t expectedResult = 123456ULL;
    char* expectedEndptr = (char*)subjectStr + 6;

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "123456   ", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_004: [The strtoull_s must return in endptr a final string of one or more unrecognized characters, including the terminating null character of the input string.]*/
TEST_FUNCTION(strtoull_s_decimal_base_follow_by_spaces_and_number_success)
{
    // arrange
    const char* subjectStr = "123456 789";
    char* endptr;
    int base = 10;
    uint64_t result;

    uint64_t expectedResult = 123456ULL;
    char* expectedEndptr = (char*)subjectStr + 6;

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "123456 789", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_004: [The strtoull_s must return in endptr a final string of one or more unrecognized characters, including the terminating null character of the input string.]*/
TEST_FUNCTION(strtoull_s_decimal_base_follow_by_percent_success)
{
    // arrange
    const char* subjectStr = "123456%%";
    char* endptr;
    int base = 10;
    uint64_t result;

    uint64_t expectedResult = 123456ULL;
    char* expectedEndptr = (char*)subjectStr + 6;

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "123456%%", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_004: [The strtoull_s must return in endptr a final string of one or more unrecognized characters, including the terminating null character of the input string.]*/
TEST_FUNCTION(strtoull_s_decimal_base_follow_by_string_success)
{
    // arrange
    const char* subjectStr = "123456abc";
    char* endptr;
    int base = 10;
    uint64_t result;

    uint64_t expectedResult = 123456ULL;
    char* expectedEndptr = (char*)subjectStr + 6;

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "123456abc", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_005: [The strtoull_s must convert number using base 2 to 36.]*/
TEST_FUNCTION(strtoull_s_hexadecimal_base_uppercase_success)
{
    // arrange
    const char* subjectStr = "1E240";
    char* endptr;
    int base = 16;
    uint64_t result;

    uint64_t expectedResult = 123456ULL;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "1E240", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_005: [The strtoull_s must convert number using base 2 to 36.]*/
TEST_FUNCTION(strtoull_s_hexadecimal_base_lowercase_success)
{
    // arrange
    const char* subjectStr = "1e240";
    char* endptr;
    int base = 16;
    uint64_t result;

    uint64_t expectedResult = 123456ULL;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "1e240", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_005: [The strtoull_s must convert number using base 2 to 36.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_008: [If the base is 0 and '0x' or '0X' precedes the number, strtoull_s must convert to a hexadecimal (base 16).]*/
TEST_FUNCTION(strtoull_s_0x_hexadecimal_base_uppercase_success)
{
    // arrange
    const char* subjectStr = "0X1e240";
    char* endptr;
    int base = 16;
    uint64_t result;

    uint64_t expectedResult = 123456ULL;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "0X1e240", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_005: [The strtoull_s must convert number using base 2 to 36.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_008: [If the base is 0 and '0x' or '0X' precedes the number, strtoull_s must convert to a hexadecimal (base 16).]*/
TEST_FUNCTION(strtoull_s_0x_hexadecimal_base_lowercase_success)
{
    // arrange
    const char* subjectStr = "0x1e240";
    char* endptr;
    int base = 16;
    uint64_t result;

    uint64_t expectedResult = 123456ULL;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "0x1e240", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_005: [The strtoull_s must convert number using base 2 to 36.]*/
TEST_FUNCTION(strtoull_s_0x_hexadecimal_base_out_of_base_range_character_fail)
{
    // arrange
    const char* subjectStr = "0xje240";
    char* endptr;
    int base = 16;
    uint64_t result;

    uint64_t expectedResult = 0ULL;
    char* expectedEndptr = (char*)subjectStr;

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "0xje240", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_005: [The strtoull_s must convert number using base 2 to 36.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_008: [If the base is 0 and '0x' or '0X' precedes the number, strtoull_s must convert to a hexadecimal (base 16).]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_009: [If the base is 0 and '0' precedes the number, strtoull_s must convert to an octal (base 8).]*/
TEST_FUNCTION(strtoull_s_0x_hexadecimal_with_base_8_character_success)
{
    // arrange
    const char* subjectStr = "0x1e240";
    char* endptr;
    int base = 8;
    uint64_t result;

    uint64_t expectedResult = 0ULL;
    char* expectedEndptr = (char*)subjectStr + 1;

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "0x1e240", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_005: [The strtoull_s must convert number using base 2 to 36.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_008: [If the base is 0 and '0x' or '0X' precedes the number, strtoull_s must convert to a hexadecimal (base 16).]*/
TEST_FUNCTION(strtoull_s_0_base_with_0x_hexadecimal_success)
{
    // arrange
    const char* subjectStr = "0x1e240";
    char* endptr;
    int base = 0;
    uint64_t result;

    uint64_t expectedResult = 123456ULL;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "0x1e240", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_005: [The strtoull_s must convert number using base 2 to 36.]*/
TEST_FUNCTION(strtoull_s_octal_base_success)
{
    // arrange
    const char* subjectStr = "361100";
    char* endptr;
    int base = 8;
    uint64_t result;

    uint64_t expectedResult = 123456ULL;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "361100", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_005: [The strtoull_s must convert number using base 2 to 36.]*/
TEST_FUNCTION(strtoull_s_binary_base_success)
{
    // arrange
    const char* subjectStr = "11110001001000000";
    char* endptr;
    int base = 2;
    uint64_t result;

    uint64_t expectedResult = 123456ULL;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "11110001001000000", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_005: [The strtoull_s must convert number using base 2 to 36.]*/
TEST_FUNCTION(strtoull_s_36_base_success)
{
    // arrange
    const char* subjectStr = "hello";
    char* endptr;
    int base = 36;
    uint64_t result;

    uint64_t expectedResult = (uint64_t)((17*1679616)+(14*46656)+(21*1296)+(21*36)+(24));
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "hello", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_005: [The strtoull_s must convert number using base 2 to 36.]*/
TEST_FUNCTION(strtoull_s_36_base_looks_like_hexadecimal_with_0x_success)
{
    // arrange
    const char* subjectStr = "0x1";
    char* endptr;
    int base = 36;
    uint64_t result;

    uint64_t expectedResult = (uint64_t)((33 * 36) + (1));
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "0x1", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_005: [The strtoull_s must convert number using base 2 to 36.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_009: [If the base is 0 and '0' precedes the number, strtoull_s must convert to an octal (base 8).]*/
TEST_FUNCTION(strtoull_s_0_base_with_actal_success)
{
    // arrange
    const char* subjectStr = "0361100";
    char* endptr;
    int base = 0;
    uint64_t result;

    uint64_t expectedResult = 123456ULL;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "0361100", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_005: [The strtoull_s must convert number using base 2 to 36.]*/
TEST_FUNCTION(strtoull_s_base_out_of_the_range_underflow_fail)
{
    // arrange
    const char* subjectStr = "10";
    char* endptr;
    int base = 1;
    uint64_t result;

    uint64_t expectedResult = 0ULL;
    char* expectedEndptr = (char*)subjectStr;

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_005: [The strtoull_s must convert number using base 2 to 36.]*/
TEST_FUNCTION(strtoull_s_base_out_of_the_range_overflow_fail)
{
    // arrange
    const char* subjectStr = "10";
    char* endptr;
    int base = 37;
    uint64_t result;

    uint64_t expectedResult = 0ULL;
    char* expectedEndptr = (char*)subjectStr;

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_013: [If no conversion could be performed, the strtoull_s returns the value 0L.]*/
TEST_FUNCTION(strtoull_s_invalid_string_blahblah_fail)
{
    // arrange
    const char* subjectStr = "blahblah";
    char* endptr;
    int base = 10;
    uint64_t result;

    uint64_t expectedResult = 0ULL;
    char* expectedEndptr = (char*)subjectStr;

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "blahblah", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_013: [If no conversion could be performed, the strtoull_s returns the value 0L.]*/
TEST_FUNCTION(strtoull_s_empty_string_fail)
{
    // arrange
    const char* subjectStr = "";
    char* endptr;
    int base = 10;
    uint64_t result;

    uint64_t expectedResult = 0ULL;
    char* expectedEndptr = (char*)subjectStr;

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", subjectStr);
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_013: [If no conversion could be performed, the strtoull_s returns the value 0L.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_035: [If the nptr is NULL, the strtoull_s must not perform any conversion and must returns 0L; endptr must receive NULL, provided that endptr is not a NULL pointer.]*/
TEST_FUNCTION(strtoull_s_null_ptr_to_string_fail)
{
    // arrange
    const char* subjectStr = NULL;
    char* endptr;
    int base = 10;
    uint64_t result;

    uint64_t expectedResult = 0ULL;
    char* expectedEndptr = (char*)subjectStr;

    // act
    result = strtoull_s(subjectStr, &endptr, base);

    // assert
    ASSERT_ARE_EQUAL(uint64_t, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/* strtof_s */

/*Tests_SRS_CRT_ABSTRACTIONS_21_015: [The strtof_s must convert the initial portion of the string pointed to by nptr to float representation.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_016: [The strtof_s must return the float that represents the value in the initial part of the string. If any.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_019: [The valid sequence for strtof_s starts after the first non-white - space character, followed by an optional positive or negative sign, a number, 'INF', or 'NAN' (ignoring case).]*/
TEST_FUNCTION(strtof_s_exponential_number_success)
{
    // arrange
    const char* subjectStr = "1.0e5";
    char* endptr;
    float result;

    float expectedResult = 1.0e5;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "1.0e5", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_016: [The strtof_s must return the float that represents the value in the initial part of the string. If any.]*/
TEST_FUNCTION(strtof_s_uppercase_exponential_number_success)
{
    // arrange
    const char* subjectStr = "1.98E5";
    char* endptr;
    float result;

    float expectedResult = 1.98E5;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "1.98E5", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_016: [The strtof_s must return the float that represents the value in the initial part of the string. If any.]*/
TEST_FUNCTION(strtof_s_float_without_exponential_number_success)
{
    // arrange
    const char* subjectStr = "1234.5678910";
    char* endptr;
    float result;

    float expectedResult = 1234.5678910f;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "1234.5678910", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_016: [The strtof_s must return the float that represents the value in the initial part of the string. If any.]*/
TEST_FUNCTION(strtof_s_integer_number_success)
{
    // arrange
    const char* subjectStr = "12345678910";
    char* endptr;
    float result;

    float expectedResult = 12345678910.0f;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "12345678910", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_016: [The strtof_s must return the float that represents the value in the initial part of the string. If any.]*/
TEST_FUNCTION(strtof_s_only_fraction_number_success)
{
    // arrange
    const char* subjectStr = "0.12345678910";
    char* endptr;
    float result;

    float expectedResult = 0.12345678910f;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "0.12345678910", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_016: [The strtof_s must return the float that represents the value in the initial part of the string. If any.]*/
TEST_FUNCTION(strtof_s_0_with_exponential_number_success)
{
    // arrange
    const char* subjectStr = "0.0e10";
    char* endptr;
    float result;

    float expectedResult = 0.0f;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "0.0e10", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_016: [The strtof_s must return the float that represents the value in the initial part of the string. If any.]*/
TEST_FUNCTION(strtof_s_float_positive_number_success)
{
    // arrange
    const char* subjectStr = "42.42";
    char* endptr;
    float result;

    float expectedResult = 42.42f;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "42.42", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_016: [The strtof_s must return the float that represents the value in the initial part of the string. If any.]*/
TEST_FUNCTION(strtof_s_float_negative_number_success)
{
    // arrange
    const char* subjectStr = "-42.42";
    char* endptr;
    float result;

    float expectedResult = -42.42f;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "-42.42", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_018: [The white-space for strtof_s must be one of the characters ' ', '\f', '\n', '\r', '\t', '\v'.]*/
TEST_FUNCTION(strtof_s_exponential_number_with_spaces_before_the_number_success)
{
    // arrange
    const char* subjectStr = "\r\n1.0e5";
    char* endptr;
    float result;

    float expectedResult = 1.0e5;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "\r\n1.0e5", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_018: [The white-space for strtof_s must be one of the characters ' ', '\f', '\n', '\r', '\t', '\v'.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_017: [The strtof_s must return in endptr a final string of one or more unrecognized characters, including the terminating null character of the input string.]*/
TEST_FUNCTION(strtof_s_exponential_number_with_characters_after_the_number_success)
{
    // arrange
    const char* subjectStr = "1.0e5 123";
    char* endptr;
    float result;

    float expectedResult = 1.0e5;
    char* expectedEndptr = (char*)subjectStr + 5;

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "1.0e5 123", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_016: [The strtof_s must return the float that represents the value in the initial part of the string. If any.]*/
TEST_FUNCTION(strtof_s_min_positive_value_success)
{
    // arrange
    const char* subjectStr = "1.175494351e-38";
    char* endptr;
    float result;

    float expectedResult = FLT_MIN;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(int, 0, errno);
    ASSERT_ARE_EQUAL(char_ptr, "1.175494351e-38", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_022: [If the correct value is outside the range, the strtof_s returns the value plus or minus HUGE_VALF, and errno will receive the value ERANGE.]*/
TEST_FUNCTION(strtof_s_min_negative_value_success)
{
    // arrange
    const char* subjectStr = "-3.402823466e+38";
    char* endptr;
    float result;

    float expectedResult = -3.402823466e+38f;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(int, 0, errno);
    ASSERT_ARE_EQUAL(char_ptr, "-3.402823466e+38", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_022: [If the correct value is outside the range, the strtof_s returns the value plus or minus HUGE_VALF, and errno will receive the value ERANGE.]*/
TEST_FUNCTION(strtof_s_overflow_max_positive_value_fail)
{
    // arrange
    const char* subjectStr = "3.402823467e+38";
    char* endptr;
    float result;

    float expectedResult = HUGE_VALF;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(int, ERANGE, errno);
    ASSERT_ARE_EQUAL(char_ptr, "3.402823467e+38", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_022: [If the correct value is outside the range, the strtof_s returns the value plus or minus HUGE_VALF, and errno will receive the value ERANGE.]*/
TEST_FUNCTION(strtof_s_overflow_in_the_integer_part_value_fail)
{
    // arrange
    const char* subjectStr = "18446744073709551616";
    char* endptr;
    float result;

    float expectedResult = HUGE_VALF;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(int, ERANGE, errno);
    ASSERT_ARE_EQUAL(char_ptr, "18446744073709551616", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_022: [If the correct value is outside the range, the strtof_s returns the value plus or minus HUGE_VALF, and errno will receive the value ERANGE.]*/
TEST_FUNCTION(strtof_s_big_integer_part_value_success)
{
    // arrange
    const char* subjectStr = "184467440737095516";
    char* endptr;
    float result;

    float expectedResult = 184467440737095516.0f;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(int, 0, errno);
    ASSERT_ARE_EQUAL(char_ptr, "184467440737095516", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_022: [If the correct value is outside the range, the strtof_s returns the value plus or minus HUGE_VALF, and errno will receive the value ERANGE.]*/
TEST_FUNCTION(strtof_s_exponential_number_overflow_max_positive_value_fail)
{
    // arrange
    const char* subjectStr = "1.0e39";
    char* endptr;
    float result;

    float expectedResult = HUGE_VALF;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(int, ERANGE, errno);
    ASSERT_ARE_EQUAL(char_ptr, "1.0e39", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_023: [If the string is 'INF' of 'INFINITY' (ignoring case), the strtof_s must return the INFINITY value for float.]*/
TEST_FUNCTION(strtof_s_short_infinity_uppercase_success)
{
    // arrange
    const char* subjectStr = "INF";
    char* endptr;
    float result;

    float expectedResult = INFINITY;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "INF", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_023: [If the string is 'INF' of 'INFINITY' (ignoring case), the strtof_s must return the INFINITY value for float.]*/
TEST_FUNCTION(strtof_s_short_negative_infinity_uppercase_success)
{
    // arrange
    const char* subjectStr = "-INF";
    char* endptr;
    float result;

    float expectedResult = -INFINITY;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "-INF", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_023: [If the string is 'INF' of 'INFINITY' (ignoring case), the strtof_s must return the INFINITY value for float.]*/
TEST_FUNCTION(strtof_s_long_infinity_uppercase_success)
{
    // arrange
    const char* subjectStr = "INFINITY";
    char* endptr;
    float result;

    float expectedResult = INFINITY;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "INFINITY", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_023: [If the string is 'INF' of 'INFINITY' (ignoring case), the strtof_s must return the INFINITY value for float.]*/
TEST_FUNCTION(strtof_s_long_infinity_mixedcase_success)
{
    // arrange
    const char* subjectStr = "InFINiTY";
    char* endptr;
    float result;

    float expectedResult = INFINITY;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "InFINiTY", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_024: [If the string is 'NAN' or 'NAN(...)' (ignoring case), the strtof_s must return 0.0f and points endptr to the first character after the 'NAN' sequence.]*/
TEST_FUNCTION(strtof_s_short_nan_uppercase_success)
{
    // arrange
    const char* subjectStr = "NAN";
    char* endptr;
    float result;

    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "NAN", subjectStr);
    ASSERT_IS_TRUE(isnan(result));
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_024: [If the string is 'NAN' or 'NAN(...)' (ignoring case), the strtof_s must return 0.0f and points endptr to the first character after the 'NAN' sequence.]*/
TEST_FUNCTION(strtof_s_long_nan_uppercase_success)
{
    // arrange
    const char* subjectStr = "NAN(1234)";
    char* endptr;
    float result;

    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "NAN(1234)", subjectStr);
    ASSERT_IS_TRUE(isnan(result));
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_024: [If the string is 'NAN' or 'NAN(...)' (ignoring case), the strtof_s must return 0.0f and points endptr to the first character after the 'NAN' sequence.]*/
TEST_FUNCTION(strtof_s_long_nan_mixedcase_success)
{
    // arrange
    const char* subjectStr = "NaN(1234)";
    char* endptr;
    float result;

    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "NaN(1234)", subjectStr);
    ASSERT_IS_TRUE(isnan(result));
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_024: [If the string is 'NAN' or 'NAN(...)' (ignoring case), the strtof_s must return 0.0f and points endptr to the first character after the 'NAN' sequence.]*/
TEST_FUNCTION(strtof_s_long_nan_withou_close_parenthesis_fail)
{
    // arrange
    const char* subjectStr = "NaN(1234";
    char* endptr;
    float result;

    float expectedResult = 0.0f;
    char* expectedEndptr = (char*)subjectStr;

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "NaN(1234", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_020: [If the subject sequence is empty or does not have the expected form, the strtof_s must not perform any conversion and must returns 0.0f; the value of nptr is stored in the object pointed to by endptr, provided that endptr is not a NULL pointer.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_021: [If no conversion could be performed, the strtof_s returns the value 0.0f.]*/
TEST_FUNCTION(strtof_s_empty_string_success)
{
    // arrange
    const char* subjectStr = "";
    char* endptr;
    float result;

    float expectedResult = 0.0f;
    char* expectedEndptr = (char*)subjectStr;

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_036: [**If the nptr is NULL, the strtof_s must not perform any conversion and must returns 0.0f; endptr must receive NULL, provided that endptr is not a NULL pointer.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_021: [If no conversion could be performed, the strtof_s returns the value 0.0f.]*/
TEST_FUNCTION(strtof_s_string_to_null_pointer_success)
{
    // arrange
    const char* subjectStr = NULL;
    char* endptr;
    float result;

    float expectedResult = 0.0f;
    char* expectedEndptr = NULL;

    // act
    result = strtof_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, NULL, subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_020: [If the subject sequence is empty or does not have the expected form, the strtof_s must not perform any conversion and must returns 0.0f; the value of nptr is stored in the object pointed to by endptr, provided that endptr is not a NULL pointer.]*/
TEST_FUNCTION(strtof_s_valid_conversion_with_return_string_null_pointer_success)
{
    // arrange
    const char* subjectStr = "1.0e5";
    float result;

    float expectedResult = 1.0e5;

    // act
    result = strtof_s(subjectStr, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "1.0e5", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_020: [If the subject sequence is empty or does not have the expected form, the strtof_s must not perform any conversion and must returns 0.0f; the value of nptr is stored in the object pointed to by endptr, provided that endptr is not a NULL pointer.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_021: [If no conversion could be performed, the strtof_s returns the value 0.0f.]*/
TEST_FUNCTION(strtof_s_invalid_conversion_with_return_string_null_pointer_success)
{
    // arrange
    const char* subjectStr = "blahblah";
    float result;

    float expectedResult = 0.0f;

    // act
    result = strtof_s(subjectStr, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "blahblah", subjectStr);
    ASSERT_ARE_EQUAL(float, expectedResult, result);
}

/* strtold_s */

/*Tests_SRS_CRT_ABSTRACTIONS_21_025: [The strtold_s must convert the initial portion of the string pointed to by nptr to long double representation.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_026: [The strtold_s must return the long double that represents the value in the initial part of the string. If any.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_029: [The valid sequence for strtold_s starts after the first non-white - space character, followed by an optional positive or negative sign, a number, 'INF', or 'NAN' (ignoring case).]*/
TEST_FUNCTION(strtold_s_exponential_number_success)
{
    // arrange
    const char* subjectStr = "1.0e5";
    char* endptr;
    long double result;

    double expectedResult = 1.0e5;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtold_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "1.0e5", subjectStr);
    ASSERT_IS_TRUE(expectedResult == result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_028: [The white-space for strtold_s must be one of the characters ' ', '\f', '\n', '\r', '\t', '\v'.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_027: [The strtold_s must return in endptr a final string of one or more unrecognized characters, including the terminating null character of the input string.]*/
TEST_FUNCTION(strtold_s_exponential_number_with_characters_after_the_number_success)
{
    // arrange
    const char* subjectStr = "1.0e5 123";
    char* endptr;
    long double result;

    long double expectedResult = 1.0e5;
    char* expectedEndptr = (char*)subjectStr + 5;

    // act
    result = strtold_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "1.0e5 123", subjectStr);
    ASSERT_IS_TRUE(expectedResult == result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}


/*Tests_SRS_CRT_ABSTRACTIONS_21_026: [The strtold_s must return the long double that represents the value in the initial part of the string. If any.]*/
TEST_FUNCTION(strtold_s_min_positive_value_success)
{
    // arrange
    const char* subjectStr = "2.225073858507201e-308";
    char* endptr;
    long double result;
    long double significant;

    long double maxExpectedSignificant = 2.225073858507202;
    long double minExpectedSignificant = 2.225073858507200;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtold_s(subjectStr, &endptr);
    significant = result*1e308;

    // assert
    ASSERT_ARE_EQUAL(int, 0, errno);
    ASSERT_ARE_EQUAL(char_ptr, "2.225073858507201e-308", subjectStr);
    ASSERT_IS_TRUE((significant >= minExpectedSignificant) && (significant <= maxExpectedSignificant));
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_032: [If the correct value is outside the range, the strtold_s returns the value plus or minus HUGE_VALL, and errno will receive the value ERANGE.]*/
TEST_FUNCTION(strtold_s_max_positive_exponential_number_success)
{
    // arrange
    const char* subjectStr = "1.797693134862315e+308";
    char* endptr;
    long double result;
    long double significant;

    long double maxExpectedSignificant = 1.797693134862316;
    long double minExpectedSignificant = 1.797693134862314;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtold_s(subjectStr, &endptr);
    significant = result*1e-308;

    // assert
    ASSERT_ARE_EQUAL(int, 0, errno);
    ASSERT_ARE_EQUAL(char_ptr, "1.797693134862315e+308", subjectStr);
    ASSERT_IS_TRUE((significant >= minExpectedSignificant) && (significant <= maxExpectedSignificant));
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_032: [If the correct value is outside the range, the strtold_s returns the value plus or minus HUGE_VALL, and errno will receive the value ERANGE.]*/
TEST_FUNCTION(strtold_s_min_negative_value_success)
{
    // arrange
    const char* subjectStr = "-1.797693134862315e+308";
    char* endptr;
    long double result;
    long double significant;

    long double maxExpectedSignificant = -1.797693134862314;
    long double minExpectedSignificant = -1.797693134862316;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtold_s(subjectStr, &endptr);
    significant = result*1e-308;

    // assert
    ASSERT_ARE_EQUAL(int, 0, errno);
    ASSERT_ARE_EQUAL(char_ptr, "-1.797693134862315e+308", subjectStr);
    ASSERT_IS_TRUE((significant >= minExpectedSignificant) && (significant <= maxExpectedSignificant));
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_032: [If the correct value is outside the range, the strtold_s returns the value plus or minus HUGE_VALL, and errno will receive the value ERANGE.]*/
TEST_FUNCTION(strtold_s_overflow_max_positive_value_fail)
{
    // arrange
    const char* subjectStr = "1.8e+308";
    char* endptr;
    long double result;

    long double expectedResult = HUGE_VALL;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtold_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(int, ERANGE, errno);
    ASSERT_ARE_EQUAL(char_ptr, "1.8e+308", subjectStr);
    ASSERT_IS_TRUE(expectedResult == result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_032: [If the correct value is outside the range, the strtold_s returns the value plus or minus HUGE_VALL, and errno will receive the value ERANGE.]*/
TEST_FUNCTION(strtold_s_exponential_number_overflow_max_positive_value_fail)
{
    // arrange
    const char* subjectStr = "1.0e309";
    char* endptr;
    long double result;

    long double expectedResult = HUGE_VALF;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtold_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(int, ERANGE, errno);
    ASSERT_ARE_EQUAL(char_ptr, "1.0e309", subjectStr);
    ASSERT_IS_TRUE(expectedResult == result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_033: [If the string is 'INF' of 'INFINITY' (ignoring case), the strtold_s must return the INFINITY value for long double.]*/
TEST_FUNCTION(strtold_s_short_infinity_uppercase_success)
{
    // arrange
    const char* subjectStr = "INF";
    char* endptr;
    long double result;

    long double expectedResult = INFINITY;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtold_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "INF", subjectStr);
    ASSERT_IS_TRUE(expectedResult == result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_033: [If the string is 'INF' of 'INFINITY' (ignoring case), the strtold_s must return the INFINITY value for long double.]*/
TEST_FUNCTION(strtold_s_short_negative_infinity_uppercase_success)
{
    // arrange
    const char* subjectStr = "-INF";
    char* endptr;
    long double result;

    long double expectedResult = -INFINITY;
    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtold_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "-INF", subjectStr);
    ASSERT_IS_TRUE(expectedResult == result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_034: [If the string is 'NAN' or 'NAN(...)' (ignoring case), the strtold_s must return 0.0 and points endptr to the first character after the 'NAN' sequence.]*/
TEST_FUNCTION(strtold_s_long_nan_mixedcase_success)
{
    // arrange
    const char* subjectStr = "NaN(1234)";
    char* endptr;
    long double result;

    char* expectedEndptr = (char*)subjectStr + strlen(subjectStr);

    // act
    result = strtold_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "NaN(1234)", subjectStr);
    ASSERT_IS_TRUE(isnan((double)result));
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_030: [If the subject sequence is empty or does not have the expected form, the strtold_s must not perform any conversion and must returns 0.0; the value of nptr is stored in the object pointed to by endptr, provided that endptr is not a NULL pointer.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_031: [If no conversion could be performed, the strtold_s returns the value 0.0.]*/
TEST_FUNCTION(strtold_s_empty_string_success)
{
    // arrange
    const char* subjectStr = "";
    char* endptr;
    long double result;

    long double expectedResult = 0.0;
    char* expectedEndptr = (char*)subjectStr;

    // act
    result = strtold_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", subjectStr);
    ASSERT_IS_TRUE(expectedResult == result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_030: [If the subject sequence is empty or does not have the expected form, the strtold_s must not perform any conversion and must returns 0.0; the value of nptr is stored in the object pointed to by endptr, provided that endptr is not a NULL pointer.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_031: [If no conversion could be performed, the strtold_s returns the value 0.0.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_037: [If the nptr is NULL, the strtold_s must not perform any conversion and must returns 0.0; endptr must receive NULL, provided that endptr is not a NULL pointer.]*/
TEST_FUNCTION(strtold_s_string_to_null_pointer_success)
{
    // arrange
    const char* subjectStr = NULL;
    char* endptr;
    long double result;

    long double expectedResult = 0.0;
    char* expectedEndptr = NULL;

    // act
    result = strtold_s(subjectStr, &endptr);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, NULL, subjectStr);
    ASSERT_IS_TRUE(expectedResult == result);
    ASSERT_ARE_EQUAL(void_ptr, expectedEndptr, endptr);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_030: [If the subject sequence is empty or does not have the expected form, the strtold_s must not perform any conversion and must returns 0.0; the value of nptr is stored in the object pointed to by endptr, provided that endptr is not a NULL pointer.]*/
TEST_FUNCTION(strtold_s_valid_conversion_with_return_string_null_pointer_success)
{
    // arrange
    const char* subjectStr = "1.0e5";
    long double result;

    long double expectedResult = 1.0e5;

    // act
    result = strtold_s(subjectStr, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "1.0e5", subjectStr);
    ASSERT_IS_TRUE(expectedResult == result);
}

/*Tests_SRS_CRT_ABSTRACTIONS_21_030: [If the subject sequence is empty or does not have the expected form, the strtold_s must not perform any conversion and must returns 0.0; the value of nptr is stored in the object pointed to by endptr, provided that endptr is not a NULL pointer.]*/
/*Tests_SRS_CRT_ABSTRACTIONS_21_031: [If no conversion could be performed, the strtold_s returns the value 0.0.]*/
TEST_FUNCTION(strtold_s_invalid_conversion_with_return_string_null_pointer_success)
{
    // arrange
    const char* subjectStr = "blahblah";
    long double result;

    long double expectedResult = 0.0;

    // act
    result = strtold_s(subjectStr, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "blahblah", subjectStr);
    ASSERT_IS_TRUE(expectedResult == result);
}

/* mallocAndStrcpy_s */

// Tests_SRS_CRT_ABSTRACTIONS_99_038 : [mallocAndstrcpy_s shall allocate memory for destination buffer to fit the string in the source parameter.]
// Tests_SRS_CRT_ABSTRACTIONS_99_039 : [mallocAndstrcpy_s shall copy the contents in the address source, including the terminating null character into location specified by the destination pointer after the memory allocation.]
// Tests_SRS_CRT_ABSTRACTIONS_99_035: [mallocAndstrcpy_s shall return Zero upon success]
TEST_FUNCTION(mallocAndStrcpy_s_copies_source_string_into_allocated_memory)
{
    // arrange
    char* destString = NULL;
    char srcString[] = "Source";
    int result;

    // act
    result = mallocAndStrcpy_s(&destString, srcString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, destString, srcString);
    ASSERT_ARE_EQUAL(int, 0, result);

    ///cleanup
    free(destString);
}

// Tests_SRS_CRT_ABSTRACTIONS_99_036: [destination parameter or source parameter is NULL, the error code returned shall be EINVAL and destination shall not be modified.]
TEST_FUNCTION(mallocAndStrcpy_s_fails_with_destination_pointer_set_to_null)
{
    // arrange
    char** destPointer = NULL;
    char srcString[] = "Source";
    int result;

    // act
    result = mallocAndStrcpy_s(destPointer, srcString);

    // assert
    ASSERT_ARE_EQUAL(int, EINVAL, result);
}

TEST_FUNCTION(mallocAndStrcpy_s_fails_with_source_set_to_null)
{
    // arrange
    char* destString = (char*)("Destination");
    char* srcString = NULL;
    int result;

    // act
    result = mallocAndStrcpy_s(&destString, srcString);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "Destination", destString);
    ASSERT_ARE_EQUAL(int, EINVAL, result);
}

/*http://vstfrd:8080/Azure/RD/_workitems/edit/3216760*/
#if 0
// Tests_SRS_CRT_ABSTRACTIONS_99_037: [Upon failure to allocate memory for the destination, the function will return ENOMEM.]
TEST_FUNCTION(mallocAndStrcpy_s_fails_upon_failure_to_allocate_memory)
{
    // arrange
    char* destString = NULL;
    char* srcString = "Source";
    int result;
    mallocSize = 0;

    // act
#ifdef _CRTDBG_MAP_ALLOC
    HOOK_FUNCTION(_malloc_dbg, malloc_null);
    result = mallocAndStrcpy_s(&destString, srcString);
    UNHOOK_FUNCTION(_malloc_dbg, malloc_null);
#else
    HOOK_FUNCTION(malloc, malloc_null);
    result = mallocAndStrcpy_s(&destString, srcString);
    UNHOOK_FUNCTION(malloc, malloc_null);
#endif

    // assert
    ASSERT_ARE_EQUAL(int, ENOMEM, result);
    ASSERT_ARE_EQUAL(size_t,strlen(srcString)+1, mallocSize);
}
#endif

/*Tests_SRS_CRT_ABSTRACTIONS_02_003: [If destination is NULL then unsignedIntToString shall fail.] */
TEST_FUNCTION(unsignedIntToString_fails_when_destination_is_NULL)
{
    // arrange

    // act
    int result = unsignedIntToString(NULL, 100, 43);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/*Tests_SRS_CRT_ABSTRACTIONS_02_002: [If the conversion fails for any reason (for example, insufficient buffer space), a non-zero return value shall be supplied and unsignedIntToString shall fail.] */
TEST_FUNCTION(unsignedIntToString_fails_when_destination_is_not_sufficient_for_1_digit)
{
    // arrange
    char destination[1000];
    unsigned int toBeConverted = 1;
    size_t destinationSize = 1;

    ///act
    int result = unsignedIntToString(destination, destinationSize, toBeConverted);

    ///assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

}

/*Tests_SRS_CRT_ABSTRACTIONS_02_002: [If the conversion fails for any reason (for example, insufficient buffer space), a non-zero return value shall be supplied and unsignedIntToString shall fail.] */
TEST_FUNCTION(unsignedIntToString_fails_when_destination_is_not_sufficient_for_more_than_1_digit)
{
    // arrange
    char destination[1000];
    unsigned int toBeConverted = 1; /*7 would not be a right starting digit*/
    size_t destinationSize = 1;
    while (toBeConverted <= (UINT_MAX / 10))
    {
        ///arrange
        int result;
        destinationSize++;
        toBeConverted *= 10;

        ///act
        result = unsignedIntToString(destination, destinationSize, toBeConverted);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
    }
}

/*Tests_SRS_CRT_ABSTRACTIONS_02_001: [unsignedIntToString shall convert the parameter value to its decimal representation as a string in the buffer indicated by parameter destination having the size indicated by parameter destinationSize.] */
TEST_FUNCTION(unsignedIntToString_succeeds_1_digit)
{
    // arrange
    char destination[1000];
    unsigned int toBeConverted = 2;
    size_t destinationSize = 2;

    ///act
    int result = unsignedIntToString(destination, destinationSize, toBeConverted);

    ///assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, "2", destination);

}

/*Tests_SRS_CRT_ABSTRACTIONS_02_001: [unsignedIntToString shall convert the parameter value to its decimal representation as a string in the buffer indicated by parameter destination having the size indicated by parameter destinationSize.] */
/*Tests_SRS_CRT_ABSTRACTIONS_02_004: [If the conversion has been successfull then unsignedIntToString shall return 0.] */
TEST_FUNCTION(unsignedIntToString_succeeds_for_interesting_numbers)
{
    // arrange
    char destination[1000];
    size_t i;
    for (i = 0; i<sizeof(interestingUnsignedIntNumbersToBeConverted) / sizeof(interestingUnsignedIntNumbersToBeConverted[0]); i++)
    {
        ///act
        unsigned int valueFromString = 0;
        size_t pos = 0;
        int result = unsignedIntToString(destination, 1000, interestingUnsignedIntNumbersToBeConverted[i]);

        ///assert
        ASSERT_ARE_EQUAL(int, 0, result);

        while (destination[pos] != '\0')
        {
            if (valueFromString > (UINT_MAX / 10))
            {
                ASSERT_FAIL("string produced was too big... ");
            }
            else
            {
                valueFromString *= 10;
                valueFromString += (destination[pos] - '0');
            }
            pos++;
        }
        if (interestingUnsignedIntNumbersToBeConverted[i] != valueFromString)
        {
            ASSERT_FAIL("unexpected value");
        }
    }
}

/*Tests_SRS_CRT_ABSTRACTIONS_02_001: [unsignedIntToString shall convert the parameter value to its decimal representation as a string in the buffer indicated by parameter destination having the size indicated by parameter destinationSize.] */
/*Tests_SRS_CRT_ABSTRACTIONS_02_004: [If the conversion has been successfull then unsignedIntToString shall return 0.] */
TEST_FUNCTION(unsignedIntToString_succeeds_for_space_just_about_right)
{
    // arrange
    char destination[1000];
    unsigned int toBeConverted = 1; /*7 would not be a right starting digit*/
    size_t destinationSize = 2;
    while (toBeConverted <= (UINT_MAX / 10))
    {
        ///arrange
        int result;
        unsigned int valueFromString = 0;
        size_t pos = 0;
        destinationSize++;
        toBeConverted *= 10;

        ///act
        result = unsignedIntToString(destination, destinationSize, toBeConverted);

        ///assert
        ASSERT_ARE_EQUAL(int, 0, result);

        while (destination[pos] != '\0')
        {
            if (valueFromString > (UINT_MAX / 10))
            {
                ASSERT_FAIL("string produced was too big... ");
            }
            else
            {
                valueFromString *= 10;
                valueFromString += (destination[pos] - '0');
            }
            pos++;
        }
        if (toBeConverted != valueFromString)
        {
            ASSERT_FAIL("unexpected value");
        }
    }
}


/*Tests_SRS_CRT_ABSTRACTIONS_02_007: [If destination is NULL then size_tToString shall fail.] */
TEST_FUNCTION(size_tToString_fails_when_destination_is_NULL)
{
    // arrange

    // act
    int result = size_tToString(NULL, 100, 43);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/*Tests_SRS_CRT_ABSTRACTIONS_02_006: [If the conversion fails for any reason (for example, insufficient buffer space), a non-zero return value shall be supplied and size_tToString shall fail.] */
TEST_FUNCTION(size_tToString_fails_when_destination_is_not_sufficient_for_1_digit)
{
    // arrange
    char destination[1000];
    size_t toBeConverted = 1;
    size_t destinationSize = 1;

    ///act
    int result = size_tToString(destination, destinationSize, toBeConverted);

    ///assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

}

/*Tests_SRS_CRT_ABSTRACTIONS_02_006: [If the conversion fails for any reason (for example, insufficient buffer space), a non-zero return value shall be supplied and size_tToString shall fail.] */
TEST_FUNCTION(size_tToString_fails_when_destination_is_not_sufficient_for_more_than_1_digit)
{
    // arrange
    char destination[1000];
    size_t toBeConverted = 1; /*7 would not be a right starting digit*/
    size_t destinationSize = 1;
    while (toBeConverted <= (UINT_MAX / 10))
    {
        ///arrange
        int result;
        destinationSize++;
        toBeConverted *= 10;

        ///act
        result = size_tToString(destination, destinationSize, toBeConverted);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
    }
}

/*Tests_SRS_CRT_ABSTRACTIONS_02_001: [size_tToString shall convert the parameter value to its decimal representation as a string in the buffer indicated by parameter destination having the size indicated by parameter destinationSize.] */
TEST_FUNCTION(size_tToString_succeeds_1_digit)
{
    // arrange
    char destination[1000];
    size_t toBeConverted = 2;
    size_t destinationSize = 2;

    ///act
    int result = size_tToString(destination, destinationSize, toBeConverted);

    ///assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, "2", destination);

}

/*Tests_SRS_CRT_ABSTRACTIONS_02_001: [size_tToString shall convert the parameter value to its decimal representation as a string in the buffer indicated by parameter destination having the size indicated by parameter destinationSize.] */
/*Tests_SRS_CRT_ABSTRACTIONS_02_004: [If the conversion has been successfull then size_tToString shall return 0.] */
TEST_FUNCTION(size_tToString_succeeds_for_interesting_numbers)
{
    // arrange
    char destination[1000];
    size_t i;
    for (i = 0; i<sizeof(interestingSize_tNumbersToBeConverted) / sizeof(interestingSize_tNumbersToBeConverted[0]); i++)
    {
        ///act
        size_t valueFromString = 0;
        size_t pos = 0;
        int result = size_tToString(destination, 1000, interestingSize_tNumbersToBeConverted[i]);

        ///assert
        ASSERT_ARE_EQUAL(int, 0, result);

        while (destination[pos] != '\0')
        {
            if (valueFromString > (SIZE_MAX / 10))
            {
                ASSERT_FAIL("string produced was too big... ");
            }
            else
            {
                valueFromString *= 10;
                valueFromString += (destination[pos] - '0');
            }
            pos++;
        }
        if (interestingSize_tNumbersToBeConverted[i] != valueFromString)
        {
            ASSERT_FAIL("unexpected value");
        }
    }
}

/*Tests_SRS_CRT_ABSTRACTIONS_02_001: [size_tToString shall convert the parameter value to its decimal representation as a string in the buffer indicated by parameter destination having the size indicated by parameter destinationSize.] */
/*Tests_SRS_CRT_ABSTRACTIONS_02_004: [If the conversion has been successfull then size_tToString shall return 0.] */
TEST_FUNCTION(size_tToString_succeeds_for_space_just_about_right)
{
    // arrange
    char destination[1000];
    size_t toBeConverted = 1; /*7 would not be a right starting digit*/
    size_t destinationSize = 2;
    while (toBeConverted <= (SIZE_MAX / 10))
    {
        ///arrange
        int result;
        size_t valueFromString = 0;
        size_t pos = 0;
        destinationSize++;
        toBeConverted *= 10;

        ///act
        result = size_tToString(destination, destinationSize, toBeConverted);

        ///assert
        ASSERT_ARE_EQUAL(int, 0, result);

        while (destination[pos] != '\0')
        {
            if (valueFromString > (SIZE_MAX / 10))
            {
                ASSERT_FAIL("string produced was too big... ");
            }
            else
            {
                valueFromString *= 10;
                valueFromString += (destination[pos] - '0');
            }
            pos++;
        }
        if (toBeConverted != valueFromString)
        {
            ASSERT_FAIL("unexpected value");
        }
    }
}

END_TEST_SUITE(CRTAbstractions_UnitTests)
