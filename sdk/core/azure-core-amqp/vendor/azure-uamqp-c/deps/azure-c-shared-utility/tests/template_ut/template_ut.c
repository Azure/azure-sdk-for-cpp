// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

/**
 * The gballoc.h will replace the malloc, free, and realloc by the my_gballoc functions, in this case,
 *    if you define these mock functions after include the gballoc.h, you will create an infinity recursion,
 *    so, places the my_gballoc functions before the #include "azure_c_shared_utility/gballoc.h"
 */
void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

void* my_gballoc_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

void my_gballoc_free(void* ptr)
{
    free(ptr);
}

/**
 * Include the C standards here.
 */
#ifdef __cplusplus
#include <cstddef>
#include <ctime>
#else
#include <stddef.h>
#include <time.h>
#endif

/**
 * Include the test tools.
 */
#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umock_c_negative_tests.h"

/**
 * Include the mockable headers here.
 * These are the headers that contains the functions that you will replace to execute the test.
 *
 * For instance, if you will test a target_create() function in the target.c that calls a callee_open() function
 *   in the callee.c, you must define callee_open() as a mockable function in the callee.h.
 *
 * Observe that we will replace the functions in callee.h here, so we don't care about its real implementation,
 *   in fact, on this example, we even have the callee.c.
 *
 * Include all header files that you will replace the mockable functions in the ENABLE_MOCKS session below.
 *
 */
#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "target/callee.h"
#undef ENABLE_MOCKS

/**
 * Include the target header after the ENABLE_MOCKS session.
 */
#include "target/target.h"


/**
 * If your test need constants, this is a good place to define it. For examples:
 *
 * #define TEST_CREATE_CONNECTION_HOST_NAME (const char*)"https://test.azure-devices.net"
 *
 * static const char* SendBuffer = "Message to send";
 *
 */
#define SIZEOF_FOO_MEMORY   10

 /**
  * You can create some global variables that your test will need in some way.
  */
static void* g_GenericPointer;

 /**
  * Umock error will helps you to identify errors in the test suite or in the way that you are
  *    using it, just keep it as is.
  */
MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}


/**
 * Create the mock function that will replace your callee functions.
 * For this example, we will replace the functions open and close of the callee. So we
 *    need to create the mock functions my_callee_open(), and my_callee_close().
 */
bool my_callee_open_must_succeed;   //This bool will manually determine the happy and unhappy paths.
CALLEE_HANDLE my_callee_open(size_t a)
{
    void* result;

    if(my_callee_open_must_succeed)
    {
        // Do something like when callee_open succeed...
        result = malloc(a);
    }
    else
    {
        // Do something like when callee_open failed...
        result = NULL;
    }

    return result;
}

void my_callee_close(CALLEE_HANDLE handle)
{
    free(handle);
}

/**
 * This is necessary for the test suite, just keep as is.
 */
static TEST_MUTEX_HANDLE g_testByTest;

/**
 * Tests begin here. Give a name for your test, for instance template_ut, use the same
 *   name to close the test suite on END_TEST_SUITE(template_ut), and to identify the
 *   test suit in the main() function
 *
 *   RUN_TEST_SUITE(template_ut, failedTestCount);
 *
 */
BEGIN_TEST_SUITE(template_ut)

    /**
     * This is the place where we initialize the test system. Replace the test name to associate the test
     *   suite with your test cases.
     * It is called once, before start the tests.
     */
    TEST_SUITE_INITIALIZE(a)
    {
        int result;
        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        (void)umock_c_init(on_umock_c_error);

        result = umocktypes_charptr_register_types();
        ASSERT_ARE_EQUAL(int, 0, result);

        /**
         * It is necessary to identify the types defined on your target. With it, the test system will
         *    know how to use it.
         *
         * On the target.h example, there is the type TARGET_HANDLE that is a void*
         */
        REGISTER_UMOCK_ALIAS_TYPE(CALLEE_HANDLE, void*);

        /**
         * It is necessary to replace all mockable functions by the mock functions that you created here.
         * It will tell the test suite to call my_callee_open besides to call the real callee_open.
         */
        REGISTER_GLOBAL_MOCK_HOOK(callee_open, my_callee_open);
        REGISTER_GLOBAL_MOCK_HOOK(callee_close, my_callee_close);

        /**
         * If you don't care about what there is inside of the function in anyway, and you just need
         *   to control the function return you can use the REGISTER_GLOBAL_MOCK_RETURN and
         *   REGISTER_GLOBAL_MOCK_FAIL_RETURN.
         *
         * In the follow example, callee_bar_1 will always return CALLEE_RESULT_OK, so, we don't need to
         *   create the unhappy return; and callee_bar_2 can return CALLEE_RESULT_OK or CALLEE_RESULT_FAIL.
         */
        REGISTER_GLOBAL_MOCK_RETURN(callee_bar_1, CALLEE_RESULT_OK);
        REGISTER_GLOBAL_MOCK_RETURN(callee_bar_2, CALLEE_RESULT_OK);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(callee_bar_2, CALLEE_RESULT_FAIL);

        /**
         * Or you can combine, for example, in the success case malloc will call my_gballoc_malloc, and for
         *    the failed cases, it will return NULL.
         */
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(callee_open, NULL);    // Fail return for the callee_open.
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_realloc, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);

        /**
         * You can initialize other global variables here, for instance image that you have a standard void* that will be converted
         *   any pointer that your test needs.
         */
        g_GenericPointer = malloc(1);
        ASSERT_IS_NOT_NULL(g_GenericPointer);
    }

    /**
     * The test suite will call this function to cleanup your machine.
     * It is called only once, after all tests is done.
     */
    TEST_SUITE_CLEANUP(TestClassCleanup)
    {
        free(g_GenericPointer);

        umock_c_deinit();

        TEST_MUTEX_DESTROY(g_testByTest);
    }

    /**
     * The test suite will call this function to prepare the machine for the new test.
     * It is called before execute each test.
     */
    TEST_FUNCTION_INITIALIZE(initialize)
    {
        if (TEST_MUTEX_ACQUIRE(g_testByTest))
        {
            ASSERT_FAIL("Could not acquire test serialization mutex.");
        }

        umock_c_reset_all_calls();

        my_callee_open_must_succeed = true; //As default, callee_open will return a valid pointer.
    }

    /**
     * The test suite will call this function to cleanup your machine for the next test.
     * It is called after execute each test.
     */
    TEST_FUNCTION_CLEANUP(cleans)
    {
        TEST_MUTEX_RELEASE(g_testByTest);
    }


    /* Tests_SRS_TEMPLATE_21_001: [ The target_create shall call callee_open to do stuff and allocate the memory. ]*/
    TEST_FUNCTION(target_create_call_callee_open__succeed)
    {
        ///arrange
        TARGET_RESULT result;

        /**
         * The STRICT_EXPECTED_CALL creates a list of functions that we expect that the target calls.
         * The function umock_c_get_expected_calls() returns this list as a serialized string.
         * You can determine all parameters, with the expected value, or define that the argument must
         *    be ignored by the test suite.
         * During the execution, the suit will collect the same information, creating a second list of
         *   called functions.
         * The function umock_c_get_actual_calls() return this list as a serialized string.
         */
        STRICT_EXPECTED_CALL(callee_open(SIZEOF_FOO_MEMORY));
        STRICT_EXPECTED_CALL(gballoc_malloc(SIZEOF_FOO_MEMORY));    //This is the malloc in the mock my_callee_open().
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));    //This is the malloc in the target_create().
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));      //This is the free in the target_create().

        ///act
        result = target_create(SIZEOF_FOO_MEMORY);

        ///assert
        ASSERT_ARE_EQUAL(int, TARGET_RESULT_OK, result);
        /**
         * The follow assert will compare the expected calls with the actual calls. If it is different,
         *    it will show the serialized strings with the differences in the log.
         */
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        target_destroy();
    }

    /* Tests_SRS_TEMPLATE_21_001: [ The target_create shall call callee_open to do stuffs and allocate the memory. ]*/
    /* Tests_SRS_TEMPLATE_21_002: [ If callee_open return error, the target_create shall return TARGET_RESULT_FAIL. ]*/
    TEST_FUNCTION(target_create_call_callee_open_got_error_manual__failed)
    {
        ///arrange
        TARGET_RESULT result;

        /**
         * Here we are demonstrating how to manually interfere in the mock function execution.
         * The target_create() function will call the mock callee_open that will fail because
         *    we set my_callee_open_must_succeed to false.
         * For this manual unhappy test, you DON'T need to specify the fail return in
         *    REGISTER_GLOBAL_MOCK_FAIL_RETURN(callee_open, NULL)
         */
        my_callee_open_must_succeed = false;

        /**
         * In the path where callee_open failed, no memory will be allocated or free. So, the list
         *    of expected calls only have one function, the callee_open().
         */
        STRICT_EXPECTED_CALL(callee_open(SIZEOF_FOO_MEMORY));

        ///act
        result = target_create(SIZEOF_FOO_MEMORY);

        ///assert
        ASSERT_ARE_EQUAL(int, TARGET_RESULT_FAIL, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
    }

    /* Tests_SRS_TEMPLATE_21_001: [ The target_create shall call callee_open to do stuffs and allocate the memory. ]*/
    /* Tests_SRS_TEMPLATE_21_002: [ If callee_open return error, the target_create shall return TARGET_RESULT_FAIL. ]*/
    /* Tests_SRS_TEMPLATE_21_003: [ If there is no memory to control the target_create information, it shall return TARGET_RESULT_OUT_OF_MEMORY. ]*/
    TEST_FUNCTION(target_create_call_callee_open_automatic_unhappy_paths)
    {
        ///arrange
        TARGET_RESULT result;
        size_t i;
        int negativeTestsInitResult = umock_c_negative_tests_init();
        ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

        /**
        * Here we are demonstrating how to automatically test the multiples unhappy paths.
        * The umock_c_negative_tests_ functions will return all possible unhappy cases,
        *    this list is created based on the STRICT_EXPECTED_CALL below.
        *
        * For each follow STRICT_EXPECTED_CALL, the code inside of the for loop will call the
        *    target function, and the function in the list will return the values specified
        *    on REGISTER_GLOBAL_MOCK_FAIL_RETURN.
        *
        * In this example, there are tree STRICT_EXPECTED_CALL (unhappy path). So,
        *    umock_c_negative_tests_call_count() will returns 3.
        * In the first interaction of the loop, callee_open() will return NULL.
        * In the second, the malloc in the mock my_callee_open() will return NULL.
        * In the third, the malloc in the target_create() will return NULL.
        *
        * Observe that the second test do not make sense, because it is testing the mock
        *    function, not the target. In the next example we will reexecute this test
        *    avoiding the second test.
        */
        STRICT_EXPECTED_CALL(callee_open(SIZEOF_FOO_MEMORY));
        STRICT_EXPECTED_CALL(gballoc_malloc(SIZEOF_FOO_MEMORY));
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

        umock_c_negative_tests_snapshot();

        for (i = 0; i < umock_c_negative_tests_call_count(); i++)
        {
            char temp_str[128];

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            (void)sprintf(temp_str, "On failed call %lu", (unsigned long)i);

            ///act
            result = target_create(SIZEOF_FOO_MEMORY);

            ///assert
            ASSERT_ARE_NOT_EQUAL(int, TARGET_RESULT_OK, result, temp_str);
        }

        ///cleanup
        umock_c_negative_tests_deinit();
    }

    /* Tests_SRS_TEMPLATE_21_001: [ The target_create shall call callee_open to do stuff and allocate the memory. ]*/
    /* Tests_SRS_TEMPLATE_21_002: [ If callee_open return error, the target_create shall return TARGET_RESULT_FAIL. ]*/
    /* Tests_SRS_TEMPLATE_21_003: [ If there is no memory to control the target_create information, it shall return TARGET_RESULT_OUT_OF_MEMORY. ]*/
    TEST_FUNCTION(target_create_call_callee_open_automatic_unhappy_paths_avoid_nonsence_test)
    {
        ///arrange
        TARGET_RESULT result;
        int negativeTestsInitResult = umock_c_negative_tests_init();
        bool runTest[] = { true, false, true };
        size_t i;

        ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

        /**
        * As in the previus example, umock_c_negative_tests_call_count() will returns 3.
        *     1. callee_open() will return NULL.
        *     2. the malloc in the mock my_callee_open() will return NULL.
        *     3. the malloc in the target_create() will return NULL.
        *
        * But here we will create a simple logit that will avoid the second test, you can
        *    create a better logic to do the same.
        */
        STRICT_EXPECTED_CALL(callee_open(SIZEOF_FOO_MEMORY));
        STRICT_EXPECTED_CALL(gballoc_malloc(SIZEOF_FOO_MEMORY));
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

        umock_c_negative_tests_snapshot();

        for (i = 0; i < umock_c_negative_tests_call_count(); i++)
        {
            if (runTest[i])
            {
                char temp_str[128];

                umock_c_negative_tests_reset();
                umock_c_negative_tests_fail_call(i);

                (void)sprintf(temp_str, "On failed call %lu", (unsigned long)i);

                ///act
                result = target_create(SIZEOF_FOO_MEMORY);

                ///assert
                ASSERT_ARE_NOT_EQUAL(int, TARGET_RESULT_OK, result, temp_str);
            }
        }

        ///cleanup
        umock_c_negative_tests_deinit();
    }

    /* Tests_SRS_TEMPLATE_21_004: [ The target_foo shall do stuff calling callee_bar_1 and callee_bar_2. ]*/
    /* Tests_SRS_TEMPLATE_21_005: [ If target_foo is called but the connection is not created, the target_foo shall return TARGET_RESULT_FAIL. ]*/
    /* Tests_SRS_TEMPLATE_21_010: [ If target_foo cannot execute foo, the target_foo shall return TARGET_RESULT_FAIL. ]*/
    TEST_FUNCTION(target_foo_unhappy_paths)
    {
        ///arrange
        TARGET_RESULT result;
        size_t i;
        int negativeTestsInitResult = umock_c_negative_tests_init();
        ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

        result = target_create(SIZEOF_FOO_MEMORY);
        ASSERT_ARE_EQUAL(int, TARGET_RESULT_OK, result);

        /**
         * Clear the function call list, to remove the ones created by the target_create(), so,
         *    now we can concentrate on the functions in target_foo().
         */
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(callee_bar_2('a'));

        umock_c_negative_tests_snapshot();

        /**
        * Here we are demonstrating how to automatically test an unhappy path from a function that
        *    we do not implement any mock to replace it.
        */
        for (i = 0; i < umock_c_negative_tests_call_count(); i++)
        {
            char temp_str[128];

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            (void)sprintf(temp_str, "On failed call %lu", (unsigned long)i);

            ///act
            result = target_foo();

            ///assert
            ASSERT_ARE_EQUAL(int, TARGET_RESULT_FAIL, result, temp_str);
        }

        ///cleanup
        target_destroy();
        umock_c_negative_tests_deinit();
    }

END_TEST_SUITE(template_ut)
