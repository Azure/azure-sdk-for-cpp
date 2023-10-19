// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "ctest.h"

CTEST_BEGIN_TEST_SUITE(TestSuiteCleanupTests)

/*when CTEST_SUITE_CLEANUP asserts then no tests should be considered as passed. Note: there are no tests here at all*/
CTEST_SUITE_CLEANUP()
{
    CTEST_ASSERT_FAIL();
}


CTEST_END_TEST_SUITE(TestSuiteCleanupTests)
