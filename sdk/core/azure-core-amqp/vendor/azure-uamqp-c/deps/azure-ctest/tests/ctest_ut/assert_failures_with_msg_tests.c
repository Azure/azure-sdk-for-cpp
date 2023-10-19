// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "ctest.h"

CTEST_BEGIN_TEST_SUITE(AssertFailureTestsWithPrintfLikeMsgs)

typedef struct mystruct_tag
{
    unsigned char x;
} mystruct;

CTEST_COMPARE(mystruct_ptr, mystruct*)
{
    return (left->x != right->x);
}

CTEST_TO_STRING(mystruct_ptr, mystruct*, string, bufferSize, value)
{
    (void)snprintf(string, bufferSize, "{ %d }", (int)value->x);
}

/* CTEST_ASSERT_FAIL */

CTEST_FUNCTION(AssertFail_Fails)
{
    CTEST_ASSERT_FAIL("This test should fail, test arg=%d", 42);
}

/* CTEST_ASSERT_ARE_EQUAL when message is used */

CTEST_FUNCTION(Assert_Are_Equal_2_Ints_Fails)
{
    CTEST_ASSERT_ARE_EQUAL(int, 0, 1, "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_2_Chars_Fails)
{
    CTEST_ASSERT_ARE_EQUAL(char, (char)0, (char)1, "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_2_Shorts_Fails)
{
    CTEST_ASSERT_ARE_EQUAL(short, (short)0, (short)1, "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_2_Longs_Fails)
{
    CTEST_ASSERT_ARE_EQUAL(long, (long)0, (long)1, "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_2_uint8_t_Fails)
{
    CTEST_ASSERT_ARE_EQUAL(uint8_t, (uint8_t)0, (uint8_t)1, "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_2_int8_t_Fails)
{
    CTEST_ASSERT_ARE_EQUAL(int8_t, (int8_t)0, (int8_t)1, "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_2_uint16_t_Fails)
{
    CTEST_ASSERT_ARE_EQUAL(uint16_t, (uint16_t)0, (uint16_t)1, "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_2_int16_t_Fails)
{
    CTEST_ASSERT_ARE_EQUAL(int16_t, (int16_t)0, (int16_t)1, "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_2_uint32_t_Fails)
{
    CTEST_ASSERT_ARE_EQUAL(uint32_t, (uint32_t)0, (uint32_t)1, "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_2_int32_t_Fails)
{
    CTEST_ASSERT_ARE_EQUAL(int32_t, (int32_t)0, (int32_t)1, "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_2_uint64_t_Fails)
{
    CTEST_ASSERT_ARE_EQUAL(uint64_t, (uint64_t)0, (uint64_t)1, "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_2_int64_t_Fails)
{
    CTEST_ASSERT_ARE_EQUAL(int64_t, (int64_t)0, (int64_t)1, "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_2_strings_With_To_String_Type_Fails)
{
    CTEST_ASSERT_ARE_EQUAL(char_ptr, "test", "dan", "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_2_wide_strings_With_To_String_Type_Fails)
{
    CTEST_ASSERT_ARE_EQUAL(wchar_ptr, L"test", L"matt", "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_2_void_ptr_With_To_Different_Addresses_Fails)
{
    char c1, c2;
    CTEST_ASSERT_ARE_EQUAL(void_ptr, &c1, &c2, "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_2_Structures_Fails)
{
    mystruct a = { 1 };
    mystruct b = { 42 };

    CTEST_ASSERT_ARE_EQUAL(mystruct_ptr, &a, &b, "Test message:%d", 42);
}

/* CTEST_ASSERT_ARE_NOT_EQUAL with message */

CTEST_FUNCTION(Assert_Are_Not_Equal_2_Ints_Fails)
{
    CTEST_ASSERT_ARE_NOT_EQUAL(int, 1, 1, "Test message:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Not_Equal_2_Chars_Fails)
{
    CTEST_ASSERT_ARE_NOT_EQUAL(char, (char)1, (char)1, "Test message:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Not_Equal_2_Shorts_Fails)
{
    CTEST_ASSERT_ARE_NOT_EQUAL(short, (short)1, (short)1, "Test message:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Not_Equal_2_Longs_Fails)
{
    CTEST_ASSERT_ARE_NOT_EQUAL(long, (long)1, (long)1, "Test message:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Not_Equal_2_Unsigned_Longs_Fails)
{
    CTEST_ASSERT_ARE_NOT_EQUAL(unsigned_long, 1, 1, "Test message:%d", 42);
}

#if defined CTEST_USE_STDINT

CTEST_FUNCTION(Assert_Are_Not_Equal_2_uint8_t_Fails)
{
    CTEST_ASSERT_ARE_NOT_EQUAL(uint8_t, (uint8_t)1, (uint8_t)1, "Test message:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Not_Equal_2_int8_t_Fails)
{
    CTEST_ASSERT_ARE_NOT_EQUAL(int8_t, (int8_t)1, (int8_t)1, "Test message:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Not_Equal_2_uint16_t_Fails)
{
    CTEST_ASSERT_ARE_NOT_EQUAL(uint16_t, (uint16_t)1, (uint16_t)1, "Test message:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Not_Equal_2_int16_t_Fails)
{
    CTEST_ASSERT_ARE_NOT_EQUAL(int16_t, (int16_t)1, (int16_t)1, "Test message:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Not_Equal_2_uint32_t_Fails)
{
    CTEST_ASSERT_ARE_NOT_EQUAL(uint32_t, (uint32_t)1, (uint32_t)1, "Test message:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Not_Equal_2_int32_t_Fails)
{
    CTEST_ASSERT_ARE_NOT_EQUAL(int32_t, (int32_t)1, (int32_t)1, "Test message:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Not_Equal_2_uint64_t_Fails)
{
    CTEST_ASSERT_ARE_NOT_EQUAL(uint64_t, (uint64_t)1, (uint64_t)1, "Test message:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Not_Equal_2_int64_t_Fails)
{
    CTEST_ASSERT_ARE_NOT_EQUAL(int64_t, (int64_t)1, (int64_t)1, "Test message:%d", 42);
}

#endif

CTEST_FUNCTION(Assert_Are_Not_Equal_2_strings_With_To_String_Type_Fails)
{
    CTEST_ASSERT_ARE_NOT_EQUAL(char_ptr, "dan", "dan", "Test message:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Not_Equal_2_wide_strings_With_To_String_Type_Fails)
{
    CTEST_ASSERT_ARE_NOT_EQUAL(wchar_ptr, L"matt", L"matt", "Test message:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Not_Equal_2_void_ptr_With_2_same_addresses_Fails)
{
    char c1;
    CTEST_ASSERT_ARE_NOT_EQUAL(void_ptr, &c1, &c1, "Test message:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Not_Equal_2_Structures_Fails)
{
    mystruct a = { 1 };
    mystruct b = { 1 };

    CTEST_ASSERT_ARE_NOT_EQUAL(mystruct_ptr, &a, &b, "Test message:%d", 42);
}

/* CTEST_ASSERT_IS_NULL with message */

CTEST_FUNCTION(Assert_Is_NULL_Fails)
{
    int a;
    int *ptr = &a;
    CTEST_ASSERT_IS_NULL(ptr, "Test message:%d", 42);
}

/* CTEST_ASSERT_IS_NOT_NULL with message */

CTEST_FUNCTION(Assert_Is_Not_NULL_Fails)
{
    void* test_value = NULL;
    CTEST_ASSERT_IS_NOT_NULL(test_value, "Test message:%d", 42);
}

/* CTEST_ASSERT_IS_TRUE with message */

CTEST_FUNCTION(Assert_Is_True_Fails)
{
    int tested_value = 0;
    CTEST_ASSERT_IS_TRUE(tested_value, "Test message:%d", 42);
}

/* CTEST_ASSERT_IS_FALSE with message */

CTEST_FUNCTION(Assert_Is_False_Fails)
{
    int tested_value = 1;
    CTEST_ASSERT_IS_FALSE(tested_value, "Test message:%d", 42);
}

CTEST_END_TEST_SUITE(AssertFailureTestsWithPrintfLikeMsgs)
