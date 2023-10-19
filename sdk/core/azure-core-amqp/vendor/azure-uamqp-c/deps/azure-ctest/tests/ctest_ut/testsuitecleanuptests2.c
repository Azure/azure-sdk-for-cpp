// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "ctest.h"

CTEST_BEGIN_TEST_SUITE(TestSuiteCleanupTests2)

/*when CTEST_SUITE_CLEANUP asserts then no tests should be considered as passed. Note: there is 1 test in this suite*/
CTEST_SUITE_CLEANUP()
{
    CTEST_ASSERT_FAIL();
}

CTEST_FUNCTION(some_test_function_that_does_nothing)
{

}


CTEST_END_TEST_SUITE(TestSuiteCleanupTests2)
