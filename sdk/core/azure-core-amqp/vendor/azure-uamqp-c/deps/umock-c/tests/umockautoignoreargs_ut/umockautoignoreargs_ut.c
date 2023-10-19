// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif
#include "testrunnerswitcher.h"
#include "umock_c/umockautoignoreargs.h"
#include "umock_c/umock_log.h"

void UMOCK_LOG(const char* format, ...)
{
    (void)format;
}

typedef struct test_malloc_CALL_TAG
{
    size_t size;
} test_malloc_CALL;

static test_malloc_CALL* test_malloc_calls;
static size_t test_malloc_call_count;
static size_t when_shall_malloc_fail;

#ifdef __cplusplus
extern "C" {
#endif

    void* mock_malloc(size_t size)
    {
        void* result;

        test_malloc_CALL* new_calls = (test_malloc_CALL*)realloc(test_malloc_calls, sizeof(test_malloc_CALL) * (test_malloc_call_count + 1));
        if (new_calls != NULL)
        {
            test_malloc_calls = new_calls;
            test_malloc_calls[test_malloc_call_count].size = size;
            test_malloc_call_count++;
        }

        if (when_shall_malloc_fail == test_malloc_call_count)
        {
            result = NULL;
        }
        else
        {
            result = malloc(size);
        }

        return result;
    }

#ifdef __cplusplus
}
#endif

static TEST_MUTEX_HANDLE test_mutex;
static TEST_MUTEX_HANDLE global_mutex;

BEGIN_TEST_SUITE(umockautoignoreargs_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    test_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_mutex);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    TEST_MUTEX_DESTROY(test_mutex);
}

TEST_FUNCTION_INITIALIZE(test_function_init)
{
    int mutex_acquire_result = TEST_MUTEX_ACQUIRE(test_mutex);
    ASSERT_ARE_EQUAL(int, 0, mutex_acquire_result);

    when_shall_malloc_fail = 0;

    test_malloc_calls = NULL;
    test_malloc_call_count = 0;
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
    free(test_malloc_calls);
    test_malloc_calls = NULL;
    test_malloc_call_count = 0;

    TEST_MUTEX_RELEASE(test_mutex);
}

/* umockautoignoreargs_is_call_argument_ignored */

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_002: [ If call or is_argument_ignored is NULL, umockautoignoreargs_is_call_argument_ignored shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_with_NULL_call_fails)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored(NULL, 1, &is_ignored);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_002: [ If call or is_argument_ignored is NULL, umockautoignoreargs_is_call_argument_ignored shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_with_NULL_is_argument_ignored_argument_fails)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored(NULL, 1, &is_ignored);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_004: [ If umockautoignoreargs_is_call_argument_ignored fails parsing the call argument it shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_when_no_lparen_is_found_fails)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("a", 1, &is_ignored);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_009: [ If the number of arguments parsed from call is less than argument_index, umockautoignoreargs_is_call_argument_ignored shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_for_arg_1_when_no_args_in_call_fails)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("a()", 1, &is_ignored);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_006: [ If the argument value is IGNORED_PTR_ARG then is_argument_ignored shall be set to 1. ]*/
/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_001: [ umockautoignoreargs_is_call_argument_ignored shall determine whether argument argument_index shall be ignored or not. ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_for_arg_1_when_1_IGNORED_PTR_ARG)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("a(IGNORED_PTR_ARG)", 1, &is_ignored);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, is_ignored);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_007: [ If the argument value is IGNORED_NUM_ARG then is_argument_ignored shall be set to 1. ]*/
/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_001: [ umockautoignoreargs_is_call_argument_ignored shall determine whether argument argument_index shall be ignored or not. ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_for_arg_1_when_1_IGNORED_NUM_ARG)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("a(IGNORED_NUM_ARG)", 1, &is_ignored);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, is_ignored);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_006: [ If the argument value is IGNORED_PTR_ARG then is_argument_ignored shall be set to 1. ]*/
/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_001: [ umockautoignoreargs_is_call_argument_ignored shall determine whether argument argument_index shall be ignored or not. ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_for_arg_2_with_IGNORED_PTR_ARG)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("a(0, IGNORED_PTR_ARG)", 2, &is_ignored);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, is_ignored);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_007: [ If the argument value is IGNORED_NUM_ARG then is_argument_ignored shall be set to 1. ]*/
/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_001: [ umockautoignoreargs_is_call_argument_ignored shall determine whether argument argument_index shall be ignored or not. ]*/
/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_005: [ If umockautoignoreargs_is_call_argument_ignored was able to parse the argument_indexth argument it shall succeed and return 0, while writing whether the argument is ignored in the is_argument_ignored output argument. ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_for_arg_2_with_IGNORED_NUM_ARG)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("a(\"a\", IGNORED_NUM_ARG)", 2, &is_ignored);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, is_ignored);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_008: [ If the argument value is any other value then is_argument_ignored shall be set to 0. ]*/
/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_001: [ umockautoignoreargs_is_call_argument_ignored shall determine whether argument argument_index shall be ignored or not. ]*/
/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_005: [ If umockautoignoreargs_is_call_argument_ignored was able to parse the argument_indexth argument it shall succeed and return 0, while writing whether the argument is ignored in the is_argument_ignored output argument. ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_for_arg_2_no_match)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("a(\"a\", xx)", 2, &is_ignored);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 0, is_ignored);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_003: [ umockautoignoreargs_is_call_argument_ignored shall parse the call string as a function call: function_name(arg1, arg2, ...). ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_with_a_space_before_lparen_succeeds)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("a (\"a\", IGNORED_PTR_ARG)", 2, &is_ignored);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, is_ignored);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_003: [ umockautoignoreargs_is_call_argument_ignored shall parse the call string as a function call: function_name(arg1, arg2, ...). ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_with_a_space_after_lparen_succeeds)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("a( \"a\", IGNORED_PTR_ARG)", 2, &is_ignored);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, is_ignored);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_003: [ umockautoignoreargs_is_call_argument_ignored shall parse the call string as a function call: function_name(arg1, arg2, ...). ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_with_a_space_before_comma_succeeds)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("a(\"a\" , IGNORED_PTR_ARG)", 2, &is_ignored);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, is_ignored);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_003: [ umockautoignoreargs_is_call_argument_ignored shall parse the call string as a function call: function_name(arg1, arg2, ...). ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_with_a_space_before_right_paren_succeeds)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("a(\"a\", IGNORED_PTR_ARG )", 2, &is_ignored);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, is_ignored);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_003: [ umockautoignoreargs_is_call_argument_ignored shall parse the call string as a function call: function_name(arg1, arg2, ...). ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_for_2nd_arg_when_first_argument_is_a_function_call)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("a(b(1,2), IGNORED_PTR_ARG)", 2, &is_ignored);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, is_ignored);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_003: [ umockautoignoreargs_is_call_argument_ignored shall parse the call string as a function call: function_name(arg1, arg2, ...). ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_for_2nd_arg_when_first_argument_has_a_structre)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("a({1,2}, IGNORED_PTR_ARG)", 2, &is_ignored);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, is_ignored);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_010: [ umockautoignoreargs_is_call_argument_ignored shall look for the arguments as being the string contained in the scope of the rightmost parenthesis set in call. ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_for_ignored_ptr_arg_when_other_parens_are_present_in_function_call)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("WRAPPER(a)(IGNORED_PTR_ARG)", 1, &is_ignored);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, is_ignored);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_010: [ umockautoignoreargs_is_call_argument_ignored shall look for the arguments as being the string contained in the scope of the rightmost parenthesis set in call. ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_for_ignored_num_arg_when_other_parens_are_present_in_function_call)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("WRAPPER(a)(IGNORED_NUM_ARG)", 1, &is_ignored);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, is_ignored);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_010: [ umockautoignoreargs_is_call_argument_ignored shall look for the arguments as being the string contained in the scope of the rightmost parenthesis set in call. ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_when_RPAREN_missing_at_end_fails)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("WRAPPER(a)(IGNORED_NUM_ARG(", 1, &is_ignored);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_010: [ umockautoignoreargs_is_call_argument_ignored shall look for the arguments as being the string contained in the scope of the rightmost parenthesis set in call. ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_when_extra_LPAREN_at_end_fails)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("WRAPPER(a)(IGNORED_NUM_ARG)(", 1, &is_ignored);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_010: [ umockautoignoreargs_is_call_argument_ignored shall look for the arguments as being the string contained in the scope of the rightmost parenthesis set in call. ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_when_extra_LPAREN_RPAREN_at_end_fails)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("WRAPPER(a)(IGNORED_NUM_ARG)()", 1, &is_ignored);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_010: [ umockautoignoreargs_is_call_argument_ignored shall look for the arguments as being the string contained in the scope of the rightmost parenthesis set in call. ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_when_another_call_is_in_args_succeeds)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("WRAPPER(a)(IGNORED_NUM_ARG, b(0))", 1, &is_ignored);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, is_ignored);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_010: [ umockautoignoreargs_is_call_argument_ignored shall look for the arguments as being the string contained in the scope of the rightmost parenthesis set in call. ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_when_another_value_is_enclosed_with_parens_succeeds)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("WRAPPER(a)(IGNORED_NUM_ARG, (0))", 1, &is_ignored);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, is_ignored);
}

/* Tests_SRS_UMOCKAUTOIGNOREARGS_01_011: [ If a valid scope of the rightmost parenthesis set cannot be formed (imbalanced parenthesis for example), `umockautoignoreargs_is_call_argument_ignored` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(umockautoignoreargs_is_call_argument_ignored_with_not_enough_LPARENs_for_args_fails)
{
    // arrange
    int result;
    int is_ignored;

    // act
    result = umockautoignoreargs_is_call_argument_ignored("IGNORED_NUM_ARG, (0))", 1, &is_ignored);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

END_TEST_SUITE(umockautoignoreargs_unittests)
