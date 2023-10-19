// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "ctest.h"

int main()
{
    size_t failedTests = 0;

    /* This first suite is ran without counting failed tests to prove that the argument is optional. */
    CTEST_RUN_TEST_SUITE(SimpleTestSuiteOneTest);

    {
        size_t temp_failed_tests = 0;
        CTEST_RUN_TEST_SUITE(enum_define_tests, temp_failed_tests);
        if (temp_failed_tests != 2) // 2 expected failing tests
        {
            failedTests++;
        }
    }
    CTEST_RUN_TEST_SUITE(SimpleTestSuiteOneTest, failedTests);
    CTEST_RUN_TEST_SUITE(SimpleTestSuiteTwoTests, failedTests);
    CTEST_RUN_TEST_SUITE(TestSuiteInitializeCleanupTests, failedTests);
    CTEST_RUN_TEST_SUITE(AssertSuccessTests, failedTests);
    {
        // tests with regular assert failures, no special messages
        size_t temp_failed_tests = 0;
        CTEST_RUN_TEST_SUITE(AssertFailureTests, temp_failed_tests);
        if (temp_failed_tests != 75)
        {
            failedTests ++;
        }
    }
    {
        // tests with assert failures with printf like messages
        size_t temp_failed_tests = 0;
        CTEST_RUN_TEST_SUITE(AssertFailureTestsWithPrintfLikeMsgs, temp_failed_tests);
        if (temp_failed_tests != 38)
        {
            failedTests++;
        }
    }
    CTEST_RUN_TEST_SUITE(TestFunctionInitializeTests, failedTests);
    CTEST_RUN_TEST_SUITE(TestFunctionCleanupTests, failedTests);

    {
        size_t temp_failed_tests = 0;
        CTEST_RUN_TEST_SUITE(whentestfunctioninitializefailstests, temp_failed_tests);
        if (temp_failed_tests != 2)
        {
            failedTests ++;
        }
    }
    {
        size_t temp_failed_tests = 0;
        CTEST_RUN_TEST_SUITE(whentestsuiteinitializefailstests, temp_failed_tests);
        if (temp_failed_tests != 1)
        {
            failedTests ++;
        }
    }

    {
        size_t temp_failed_tests = 0;
        CTEST_RUN_TEST_SUITE(testfunctioncleanupfailstests, temp_failed_tests);
        if (temp_failed_tests != 2)
        {
            failedTests ++;
        }
    }

    {
        size_t temp_failed_tests = 0;
        CTEST_RUN_TEST_SUITE(TestSuiteCleanupTests, temp_failed_tests);
        if (temp_failed_tests == 0) /*the tests in TestSuiteCleanupTests HAVE to fail*/
        {
            failedTests++;
        }
    }

    {
        size_t temp_failed_tests = 0;
        CTEST_RUN_TEST_SUITE(TestSuiteCleanupTests2, temp_failed_tests);
        if (temp_failed_tests == 0) /*the tests in TestSuiteCleanupTests HAVE to fail*/
        {
            failedTests++;
        }
    }

#if defined _MSC_VER
    {
        size_t temp_failed_tests = 0;
        CTEST_RUN_TEST_SUITE(WindowsTypesTests, temp_failed_tests);
        if (temp_failed_tests != 5) // 5 expected failing tests
        {
            failedTests++;
        }
    }
#endif

    return failedTests;
}
