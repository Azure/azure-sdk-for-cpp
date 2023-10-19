// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "ctest.h"

CTEST_BEGIN_TEST_SUITE(WindowsTypesTests)

/* ULONG */

CTEST_FUNCTION(Assert_Are_Equal_ULONG_Succeeds)
{
    CTEST_ASSERT_ARE_EQUAL(ULONG, 1, 1, "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_ULONG_Fails)
{
    CTEST_ASSERT_ARE_EQUAL(ULONG, 0, 1, "test arg:%d", 42);
}

/* LONG */

CTEST_FUNCTION(Assert_Are_Equal_LONG_Succeeds)
{
    CTEST_ASSERT_ARE_EQUAL(LONG, 1, 1, "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_LONG_Fails)
{
    CTEST_ASSERT_ARE_EQUAL(LONG, 0, 1, "test arg:%d", 42);
}

/* LONG64 */

CTEST_FUNCTION(Assert_Are_Equal_LONG64_Succeeds)
{
    CTEST_ASSERT_ARE_EQUAL(LONG64, 1, 1, "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_LONG64_Fails)
{
    CTEST_ASSERT_ARE_EQUAL(LONG64, 0, 1, "test arg:%d", 42);
}

/* ULONG64 */

CTEST_FUNCTION(Assert_Are_Equal_ULONG64_Succeeds)
{
    CTEST_ASSERT_ARE_EQUAL(ULONG64, 1, 1, "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_ULONG64_Fails)
{
    CTEST_ASSERT_ARE_EQUAL(ULONG64, 0, 1, "test arg:%d", 42);
}

/* HRESULT */

CTEST_FUNCTION(Assert_Are_Equal_HRESULT_Succeeds)
{
    CTEST_ASSERT_ARE_EQUAL(HRESULT, S_OK, S_OK, "test arg:%d", 42);
}

CTEST_FUNCTION(Assert_Are_Equal_HRESULT_Fails)
{
    CTEST_ASSERT_ARE_EQUAL(HRESULT, S_OK, E_INVALIDARG, "test arg:%d", 42);
}

CTEST_END_TEST_SUITE(WindowsTypesTests)
