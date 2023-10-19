// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#ifdef __cplusplus
#include <cstdint>
#include <cstdlib>
#else
#include <stdint.h>
#include <stdlib.h>
#endif

#include "testrunnerswitcher.h"

/* Tested by unit tests for umock_c:
Tests_SRS_UMOCK_C_LIB_01_006: [umock_c_init shall initialize umock_c.]
Tests_SRS_UMOCK_C_LIB_01_007: [umock_c_init called if already initialized shall fail and return a non-zero value.]
Tests_SRS_UMOCK_C_LIB_01_008: [umock_c_init shall initialize the umock supported types (C native types).]
Tests_SRS_UMOCK_C_LIB_01_009: [on_umock_c_error can be NULL.]
Tests_SRS_UMOCK_C_LIB_01_010: [If on_umock_c_error is non-NULL it shall be saved for later use (to be invoked whenever an umock_c error needs to be signaled to the user).]
Tests_SRS_UMOCK_C_LIB_01_011: [umock_c_deinit shall free all umock_c used resources.]
Tests_SRS_UMOCK_C_LIB_01_012: [If umock_c was not initialized, umock_c_deinit shall do nothing.]
*/

/* Tested by unittests of umocktypes and umocktypename:
Tests_SRS_UMOCK_C_LIB_01_145: [ Since umock_c needs to maintain a list of registered types, the following rules shall be applied: ]
Tests_SRS_UMOCK_C_LIB_01_146: [ Each type shall be normalized to a form where all extra spaces are removed. ]
Tests_SRS_UMOCK_C_LIB_01_147: [ Type names are case sensitive. ]
*/

/* Tested by unittests of umockcallpairs:
Tests_SRS_UMOCK_C_LIB_01_194: [ If the first argument passed to destroy_call is not found in the list of tracked handles (returned by create_call) then umock_c shall raise an error with the code UMOCK_C_INVALID_PAIRED_CALLS. ]
*/

#include "umock_c/umock_c.h"
#define ENABLE_MOCKS
#include "test_dependency.h"

/* Tests_SRS_UMOCK_C_LIB_01_067: [char\* and const char\* shall be supported out of the box through a separate header, umockvalue_charptr.h.]*/
#include "umock_c/umocktypes_charptr.h"

typedef struct test_on_umock_c_error_CALL_TAG
{
    UMOCK_C_ERROR_CODE error_code;
} test_on_umock_c_error_CALL;

static test_on_umock_c_error_CALL* test_on_umock_c_error_calls;
static size_t test_on_umock_c_error_call_count;

DECLARE_UMOCK_POINTER_TYPE_FOR_TYPE(int, int);
DECLARE_UMOCK_POINTER_TYPE_FOR_TYPE(unsigned char, unsignedchar);

TEST_DEFINE_ENUM_TYPE(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES);

static void test_on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    test_on_umock_c_error_CALL* new_calls = (test_on_umock_c_error_CALL*)realloc(test_on_umock_c_error_calls, sizeof(test_on_umock_c_error_CALL) * (test_on_umock_c_error_call_count + 1));
    if (new_calls != NULL)
    {
        test_on_umock_c_error_calls = new_calls;
        test_on_umock_c_error_calls[test_on_umock_c_error_call_count].error_code = error_code;
        test_on_umock_c_error_call_count++;
    }
}

static int my_hook_test_dependency_with_global_mock_hook(void)
{
    return 43;
}

static int my_hook_result;
static int my_hook_test_dependency_no_args(void)
{
    return my_hook_result++;
}

static int my_hook_test_dependency_no_args_2(void)
{
    return 0x21;
}

static int arg_a;
static int arg_b;
static int my_hook_test_dependency_2_args(int a, int b)
{
    arg_a = a;
    arg_b = b;
    return 0;
}

static unsigned int test_dependency_void_return_called = 0;
static void my_hook_test_dependency_void_return(void)
{
    test_dependency_void_return_called = 1;
}

char* stringify_func_TEST_STRUCT_COPY_FAILS(const TEST_STRUCT_COPY_FAILS* value)
{
    char* result = (char*)malloc(1);
    (void)value;
    result[0] = '\0';
    return result;
}

int are_equal_func_TEST_STRUCT_COPY_FAILS(const TEST_STRUCT_COPY_FAILS* left, const TEST_STRUCT_COPY_FAILS* right)
{
    (void)left;
    (void)right;
    return 1;
}

int copy_func_TEST_STRUCT_COPY_FAILS(TEST_STRUCT_COPY_FAILS* destination, const TEST_STRUCT_COPY_FAILS* source)
{
    (void)source;
    (void)destination;
    return 0;
}

void free_func_TEST_STRUCT_COPY_FAILS(TEST_STRUCT_COPY_FAILS* value)
{
    (void)value;
}

char* umocktypes_stringify_TEST_STRUCT_WITH_2_MEMBERS(const TEST_STRUCT_WITH_2_MEMBERS* value)
{
    char* result = (char*)malloc(1);
    (void)value;
    result[0] = '\0';
    return result;
}

int umocktypes_are_equal_TEST_STRUCT_WITH_2_MEMBERS(const TEST_STRUCT_WITH_2_MEMBERS* left, const TEST_STRUCT_WITH_2_MEMBERS* right)
{
    (void)left;
    (void)right;
    return 1;
}

int umocktypes_copy_TEST_STRUCT_WITH_2_MEMBERS(TEST_STRUCT_WITH_2_MEMBERS* destination, const TEST_STRUCT_WITH_2_MEMBERS* source)
{
    (void)source;
    (void)destination;
    return 0;
}

void umocktypes_free_TEST_STRUCT_WITH_2_MEMBERS(TEST_STRUCT_WITH_2_MEMBERS* value)
{
    (void)value;
}

typedef void* SOME_OTHER_TYPE;

char* umock_stringify_SOME_OTHER_TYPE(const SOME_OTHER_TYPE* value)
{
    char* result = (char*)malloc(1);
    (void)value;
    result[0] = '\0';
    return result;
}

int umock_are_equal_SOME_OTHER_TYPE(const SOME_OTHER_TYPE* left, const SOME_OTHER_TYPE* right)
{
    (void)left;
    (void)right;
    return 1;
}

int umock_copy_SOME_OTHER_TYPE(SOME_OTHER_TYPE* destination, const SOME_OTHER_TYPE* source)
{
    (void)source;
    (void)destination;
    return 0;
}

void umock_free_SOME_OTHER_TYPE(SOME_OTHER_TYPE* value)
{
    (void)value;
}

typedef struct MY_STRUCT_TAG
{
    int x;
} MY_STRUCT;

char* umocktypes_stringify_MY_STRUCT_ptr(const MY_STRUCT** value)
{
    char* result = (char*)malloc(1);
    (void)value;
    result[0] = '\0';
    return result;
}

int umocktypes_are_equal_MY_STRUCT_ptr(const MY_STRUCT** left, const MY_STRUCT** right)
{
    int result;

    if ((*left)->x == (*right)->x)
    {
        result = 1;
    }
    else
    {
        result = 0;
    }

    return result;
}

int umocktypes_copy_MY_STRUCT_ptr(MY_STRUCT** destination, const MY_STRUCT** source)
{
    int result;

    *destination = (MY_STRUCT*)malloc(sizeof(MY_STRUCT));
    if (*destination == NULL)
    {
        result = __LINE__;
    }
    else
    {
        (*destination)->x = (*source)->x;
        result = 0;
    }

    return result;
}

void umocktypes_free_MY_STRUCT_ptr(MY_STRUCT** value)
{
    free(*value);
}

char* umocktypes_stringify_ARRAY_TYPE(const ARRAY_TYPE* value)
{
    char* result = (char*)malloc(1);
    (void)value;
    result[0] = '\0';
    return result;
}

int umocktypes_are_equal_ARRAY_TYPE(const ARRAY_TYPE* left, const ARRAY_TYPE* right)
{
    int result;

    if (memcmp(*((ARRAY_TYPE**)left), *((ARRAY_TYPE**)right), 16) == 0)
    {
        result = 1;
    }
    else
    {
        result = 0;
    }

    return result;
}

int umocktypes_copy_ARRAY_TYPE(ARRAY_TYPE* destination, const ARRAY_TYPE* source)
{
    (void)memcpy(*destination, *source, 16);
    return 0;
}

void umocktypes_free_ARRAY_TYPE(ARRAY_TYPE* value)
{
    (void)value;
}

MOCK_FUNCTION_WITH_CODE(, void, another_test_function, SOME_OTHER_TYPE, a);
MOCK_FUNCTION_END()

static TEST_MUTEX_HANDLE test_mutex;

MOCK_FUNCTION_WITH_CODE(, void, test_mock_function_with_code_1_arg, int, a);
MOCK_FUNCTION_END()

MOCK_FUNCTION_WITH_CODE(, char*, test_mock_function_returning_string_with_code);
MOCK_FUNCTION_END("a")

typedef int funkytype;
typedef unsigned char type_of_1_byte;

/* Tests_SRS_UMOCK_C_LIB_01_150: [ MOCK_FUNCTION_WITH_CODE shall define a mock function and allow the user to embed code between this define and a MOCK_FUNCTION_END call. ]*/
MOCK_FUNCTION_WITH_CODE(, funkytype, test_mock_function_with_funkytype, funkytype, x);
MOCK_FUNCTION_END(42)

static unsigned char*** result_value = (unsigned char***)0x4242;

MOCK_FUNCTION_WITH_CODE(, unsigned char***, test_mock_function_with_unregistered_ptr_type, unsigned char***, x);
MOCK_FUNCTION_END(result_value)

IMPLEMENT_UMOCK_C_ENUM_TYPE(TEST_ENUM, TEST_ENUM_VALUE_1, TEST_ENUM_VALUE_2)

static int test_return_value = 42;

MOCK_FUNCTION_WITH_CODE(, int, test_dependency_for_capture_return)
MOCK_FUNCTION_END(test_return_value)

MOCK_FUNCTION_WITH_CODE(, int, test_dependency_for_capture_return_with_arg, int, a)
MOCK_FUNCTION_END(test_return_value)

typedef void* SOME_HANDLE;
static const SOME_HANDLE test_handle = (SOME_HANDLE)0x4242;

typedef struct SOME_STRUCT_TAG
{
    unsigned char a;
} SOME_STRUCT;

SOME_STRUCT test_struct = { 42 };

MOCK_FUNCTION_WITH_CODE(, SOME_HANDLE, some_create, int, a);
MOCK_FUNCTION_END(test_handle)
MOCK_FUNCTION_WITH_CODE(, void, some_destroy, SOME_HANDLE, h);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, some_create_void_return, int, a);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, some_destroy_void_return, SOME_HANDLE, h);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, SOME_HANDLE, some_create_no_args, int, a);
MOCK_FUNCTION_END(test_handle)
MOCK_FUNCTION_WITH_CODE(, void, some_destroy_no_args);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, SOME_HANDLE, some_create_arg_different, int, a);
MOCK_FUNCTION_END(test_handle)
MOCK_FUNCTION_WITH_CODE(, void, some_destroy_arg_different, int, a);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, SOME_STRUCT, some_create_with_struct, int, a);
MOCK_FUNCTION_END(test_struct)
MOCK_FUNCTION_WITH_CODE(, void, some_destroy_with_struct, SOME_STRUCT, s);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, mock_function_with_code_with_volatile_arg, volatile int, a);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, void, mock_function_with_code_with_volatile_pointer_arg, int volatile*, a);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, volatile void*, mock_function_with_code_with_volatile_ptr_return);
MOCK_FUNCTION_END(NULL)

MOCK_FUNCTION_WITH_CODE(, const TEST_STRUCT*, mock_function_with_code_return_const_struct_ptr);
MOCK_FUNCTION_END(NULL)


BEGIN_TEST_SUITE(umock_c_integrationtests)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    test_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_mutex);

    result = umock_c_init(test_on_umock_c_error);
    ASSERT_ARE_EQUAL(int, 0, result);
    /* Tests_SRS_UMOCK_C_LIB_01_069: [The signature shall be: ...*/
    /* Tests_SRS_UMOCK_C_LIB_01_070: [umockvalue_charptr_register_types shall return 0 on success and non-zero on failure.]*/
    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);
    /* Tests_SRS_UMOCK_C_LIB_01_065: [REGISTER_UMOCK_VALUE_TYPE shall register the type identified by value_type to be usable by umock_c for argument and return types and instruct umock_c which functions to use for getting the stringify, are_equal, copy and free.]*/
    REGISTER_UMOCK_VALUE_TYPE(int*, stringify_func_intptr, are_equal_func_intptr, copy_func_intptr, free_func_intptr);
    REGISTER_UMOCK_VALUE_TYPE(unsigned char*, stringify_func_unsignedcharptr, are_equal_func_unsignedcharptr, copy_func_unsignedcharptr, free_func_unsignedcharptr);
    REGISTER_UMOCK_VALUE_TYPE(TEST_STRUCT_COPY_FAILS, stringify_func_TEST_STRUCT_COPY_FAILS, are_equal_func_TEST_STRUCT_COPY_FAILS, copy_func_TEST_STRUCT_COPY_FAILS, free_func_TEST_STRUCT_COPY_FAILS);
    /* Tests_SRS_UMOCK_C_LIB_01_066: [If only the value_type is specified in the macro invocation then the stringify, are_equal, copy and free function names shall be automatically derived from the type as: umockvalue_stringify_value_type, umockvalue_are_equal_value_type, umockvalue_copy_value_type, umockvalue_free_value_type.]*/
    REGISTER_UMOCK_VALUE_TYPE(SOME_OTHER_TYPE);
    REGISTER_UMOCK_ALIAS_TYPE(SOME_HANDLE, void*);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(test_mutex);
}

TEST_FUNCTION_INITIALIZE(test_function_init)
{
    int mutex_acquire_result = TEST_MUTEX_ACQUIRE(test_mutex);
    ASSERT_ARE_EQUAL(int, 0, mutex_acquire_result);

    test_on_umock_c_error_calls = NULL;
    test_on_umock_c_error_call_count = 0;

    test_return_value = 42;
}

TEST_FUNCTION_CLEANUP(test_function_cleanup)
{
    umock_c_reset_all_calls();

    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_no_args, NULL);

    free(test_on_umock_c_error_calls);
    test_on_umock_c_error_calls = NULL;
    test_on_umock_c_error_call_count = 0;

    TEST_MUTEX_RELEASE(test_mutex);
}

/* STRICT_EXPECTED_CALL */

/* Tests_SRS_UMOCK_C_LIB_01_013: [STRICT_EXPECTED_CALL shall record that a certain call is expected.] */
/* Tests_SRS_UMOCK_C_LIB_01_015: [The call argument shall be the complete function invocation.]*/
TEST_FUNCTION(STRICT_EXPECTED_CALL_without_an_actual_call_yields_a_missing_call)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_no_args());

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_no_args()]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_013: [STRICT_EXPECTED_CALL shall record that a certain call is expected.] */
TEST_FUNCTION(two_STRICT_EXPECTED_CALL_without_an_actual_call_yields_2_missing_calls)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_no_args());
    STRICT_EXPECTED_CALL(test_dependency_no_args());

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_no_args()][test_dependency_no_args()]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_014: [For each argument the argument value shall be stored for later comparison with actual calls.] */
TEST_FUNCTION(a_STRICT_EXPECTED_CALL_with_one_argument_without_an_actual_call_yields_1_missing_call)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_1_arg(42)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_014: [For each argument the argument value shall be stored for later comparison with actual calls.] */
TEST_FUNCTION(a_STRICT_EXPECTED_CALL_matched_with_an_actual_call_yields_no_differences_for_const_void_ptr)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_with_const_void_ptr((void*)0x4242));

    // act
    test_dependency_with_const_void_ptr((void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_014: [For each argument the argument value shall be stored for later comparison with actual calls.] */
TEST_FUNCTION(a_STRICT_EXPECTED_CALL_matched_with_an_actual_call_yields_no_differences_for_array_arg)
{
    // arrange
    ARRAY_TYPE x = { 0 };

    REGISTER_TYPE(ARRAY_TYPE, ARRAY_TYPE);
    STRICT_EXPECTED_CALL(test_dependency_with_array_arg(x));

    // act
    test_dependency_with_array_arg(x);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_014: [For each argument the argument value shall be stored for later comparison with actual calls.] */
TEST_FUNCTION(a_STRICT_EXPECTED_CALL_with_2_arguments_without_an_actual_call_yields_1_missing_call)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_2_args(42, 43));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_013: [STRICT_EXPECTED_CALL shall record that a certain call is expected.] */
TEST_FUNCTION(two_different_STRICT_EXPECTED_CALL_instances_without_an_actual_call_yields_2_missing_calls)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_no_args());
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_no_args()][test_dependency_1_arg(42)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_115: [ umock_c shall compare calls in order. ]*/
TEST_FUNCTION(two_different_STRICT_EXPECTED_CALL_instances_without_an_actual_call_yields_2_missing_calls_with_order_preserved)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42));
    STRICT_EXPECTED_CALL(test_dependency_no_args());

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_1_arg(42)][test_dependency_no_args()]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_115: [ umock_c shall compare calls in order. ]*/
TEST_FUNCTION(inverted_order_for_calls_is_detected_as_mismatch)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42));
    STRICT_EXPECTED_CALL(test_dependency_no_args());

    // act
    test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_1_arg(42)][test_dependency_no_args()]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_no_args()]", umock_c_get_actual_calls());
}

/* EXPECTED_CALL */

/* Tests_SRS_UMOCK_C_LIB_01_016: [EXPECTED_CALL shall record that a certain call is expected.] */
/* Tests_SRS_UMOCK_C_LIB_01_018: [The call argument shall be the complete function invocation.] */
TEST_FUNCTION(EXPECTED_CALL_without_an_actual_call_yields_a_missing_call)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_no_args());

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_no_args()]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_016: [EXPECTED_CALL shall record that a certain call is expected.] */
TEST_FUNCTION(two_EXPECTED_CALL_without_an_actual_call_yields_2_missing_calls)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_no_args());
    EXPECTED_CALL(test_dependency_no_args());

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_no_args()][test_dependency_no_args()]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_017: [No arguments shall be saved by default, unless other modifiers state it.] */
TEST_FUNCTION(an_EXPECTED_CALL_with_one_argument_without_an_actual_call_yields_1_missing_call)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_1_arg(42));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_1_arg(42)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_017: [No arguments shall be saved by default, unless other modifiers state it.] */
TEST_FUNCTION(an_EXPECTED_CALL_with_2_arguments_without_an_actual_call_yields_1_missing_call)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_2_args(42, 43));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_016: [EXPECTED_CALL shall record that a certain call is expected.] */
TEST_FUNCTION(two_different_EXPECTED_CALL_instances_without_an_actual_call_yields_2_missing_calls)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_no_args());
    EXPECTED_CALL(test_dependency_1_arg(42));

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_no_args()][test_dependency_1_arg(42)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_115: [ umock_c shall compare calls in order. ]*/
TEST_FUNCTION(two_different_EXPECTED_CALL_instances_without_an_actual_call_yields_2_missing_calls_with_order_preserved)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_1_arg(42));
    EXPECTED_CALL(test_dependency_no_args());

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_1_arg(42)][test_dependency_no_args()]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_017: [No arguments shall be saved by default, unless other modifiers state it.]*/
TEST_FUNCTION(EXPECTED_CALL_does_not_compare_arguments)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_1_arg(42));

    test_dependency_1_arg(43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_017: [No arguments shall be saved by default, unless other modifiers state it.]*/
TEST_FUNCTION(EXPECTED_CALL_with_2_args_does_not_compare_arguments)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_2_args(42, 43));

    test_dependency_2_args(43, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Call modifiers */

/* Tests_SRS_UMOCK_C_LIB_01_074: [When an expected call is recorded a call modifier interface in the form of a structure containing function pointers shall be returned to the caller.] */
TEST_FUNCTION(STRICT_EXPECTED_CALL_allows_call_modifiers)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateAllArguments();

    test_dependency_2_args(42, 43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Chaining modifiers */

/* Tests_SRS_UMOCK_C_LIB_01_075: [The last modifier in a chain overrides previous modifiers if any collision occurs.]*/
TEST_FUNCTION(STRICT_EXPECTED_CALL_with_ignore_all_arguments_and_then_validate_all_args_still_validates_args)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_2_args(42, 43))
        .IgnoreAllArguments()
        .ValidateAllArguments();

    test_dependency_2_args(43, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(43,44)]", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_075: [The last modifier in a chain overrides previous modifiers if any collision occurs.]*/
TEST_FUNCTION(EXPECTED_CALL_with_validate_all_arguments_and_then_ignore_all_args_still_ignores_args)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateAllArguments()
        .IgnoreAllArguments();

    test_dependency_2_args(43, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_075: [The last modifier in a chain overrides previous modifiers if any collision occurs.]*/
TEST_FUNCTION(STRICT_EXPECTED_CALL_with_ignore_validate_ignore_all_arguments_ignores_args)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_2_args(42, 43))
        .IgnoreAllArguments()
        .ValidateAllArguments()
        .IgnoreAllArguments();

    test_dependency_2_args(43, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_075: [The last modifier in a chain overrides previous modifiers if any collision occurs.]*/
TEST_FUNCTION(STRICT_EXPECTED_CALL_with_validate_ignore_validate_all_arguments_validates_args)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateAllArguments()
        .IgnoreAllArguments()
        .ValidateAllArguments();

    test_dependency_2_args(43, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(43,44)]", umock_c_get_actual_calls());
}

/* IgnoreAllArguments */

/* Tests_SRS_UMOCK_C_LIB_01_076: [The IgnoreAllArguments call modifier shall record that for that specific call all arguments will be ignored for that specific call.] */
TEST_FUNCTION(IgnoreAllArguments_ignores_args_on_a_STRICT_EXPECTED_CALL)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_2_args(42, 43))
        .IgnoreAllArguments();

    test_dependency_2_args(43, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* ValidateAllArguments */

/* Tests_SRS_UMOCK_C_LIB_01_077: [The ValidateAllArguments call modifier shall record that for that specific call all arguments will be validated.] */
TEST_FUNCTION(ValidateAllArguments_validates_all_args_on_an_EXPECTED_CALL)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateAllArguments();

    test_dependency_2_args(43, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(43,44)]", umock_c_get_actual_calls());
}

/* IgnoreArgument_{arg_name} */

/* Tests_SRS_UMOCK_C_LIB_01_078: [The IgnoreArgument_{arg_name} call modifier shall record that the argument identified by arg_name will be ignored for that specific call.] */
TEST_FUNCTION(IgnoreArgument_by_name_ignores_only_that_argument_on_a_STRICT_EXPECTED_CALL)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_2_args(42, 43))
        .IgnoreArgument_a();

    test_dependency_2_args(41, 43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_078: [The IgnoreArgument_{arg_name} call modifier shall record that the argument identified by arg_name will be ignored for that specific call.] */
TEST_FUNCTION(IgnoreArgument_by_name_with_second_argument_ignores_only_that_argument_on_a_STRICT_EXPECTED_CALL)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_2_args(42, 43))
        .IgnoreArgument_b();

    // act
    test_dependency_2_args(42, 42);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* ValidateArgument_{arg_name} */

/* Tests_SRS_UMOCK_C_LIB_01_079: [The ValidateArgument_{arg_name} call modifier shall record that the argument identified by arg_name will be validated for that specific call.]*/
TEST_FUNCTION(ValidateArgument_by_name_validates_only_that_argument_on_an_EXPECTED_CALL)
{
    // arrange
    EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateArgument_a();

    // act
    test_dependency_2_args(42, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_079: [The ValidateArgument_{arg_name} call modifier shall record that the argument identified by arg_name will be validated for that specific call.]*/
TEST_FUNCTION(ValidateArgument_by_name_validates_only_that_argument_on_an_EXPECTED_CALL_and_args_are_different)
{
    // arrange
    EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateArgument_a();

    // act
    test_dependency_2_args(41, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(41,44)]", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_079: [The ValidateArgument_{arg_name} call modifier shall record that the argument identified by arg_name will be validated for that specific call.]*/
TEST_FUNCTION(ValidateArgument_by_name_2nd_arg_validates_only_that_argument_on_an_EXPECTED_CALL)
{
    // arrange
    EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateArgument_b();

    // act
    test_dependency_2_args(41, 43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_079: [The ValidateArgument_{arg_name} call modifier shall record that the argument identified by arg_name will be validated for that specific call.]*/
TEST_FUNCTION(ValidateArgument_by_name_2nd_arg_validates_only_that_argument_on_an_EXPECTED_CALL_and_args_are_different)
{
    // arrange
    EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateArgument_b();

    // act
    test_dependency_2_args(42, 44);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,44)]", umock_c_get_actual_calls());
}

/* IgnoreArgument */

/* Tests_SRS_UMOCK_C_LIB_01_080: [The IgnoreArgument call modifier shall record that the indexth argument will be ignored for that specific call.]*/
TEST_FUNCTION(IgnoreArgument_by_index_for_first_arg_ignores_the_first_argument)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_2_args(42, 43))
        .IgnoreArgument(1);

    // act
    test_dependency_2_args(41, 43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_080: [The IgnoreArgument call modifier shall record that the indexth argument will be ignored for that specific call.]*/
TEST_FUNCTION(IgnoreArgument_by_index_for_second_arg_ignores_the_second_argument)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_2_args(41, 42))
        .IgnoreArgument(2);

    // act
    test_dependency_2_args(41, 43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_080: [The IgnoreArgument call modifier shall record that the indexth argument will be ignored for that specific call.]*/
TEST_FUNCTION(IgnoreArgument_by_index_for_first_arg_ignores_only_the_first_argument)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_2_args(42, 43))
        .IgnoreArgument(1);

    // act
    test_dependency_2_args(42, 42);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,42)]", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_080: [The IgnoreArgument call modifier shall record that the indexth argument will be ignored for that specific call.]*/
TEST_FUNCTION(IgnoreArgument_by_index_for_second_arg_ignores_only_the_second_argument)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_2_args(41, 42))
        .IgnoreArgument(2);

    // act
    test_dependency_2_args(42, 42);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(41,42)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,42)]", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_081: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.] */
TEST_FUNCTION(IgnoreArgument_by_index_with_index_0_triggers_the_on_error_callback)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_2_args(41, 42))
        .IgnoreArgument(0);

    // assert
    //TFS661968 ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    //TFS661968 ASSERT_ARE_EQUAL(int, (int)UMOCK_C_ARG_INDEX_OUT_OF_RANGE, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_081: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.] */
TEST_FUNCTION(IgnoreArgument_by_index_with_index_greater_than_arg_count_triggers_the_on_error_callback)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_2_args(41, 42))
        .IgnoreArgument(3);

    // assert
    //TFS661968 ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    //TFS661968 ASSERT_ARE_EQUAL(int, (int)UMOCK_C_ARG_INDEX_OUT_OF_RANGE, test_on_umock_c_error_calls[0].error_code);
}

/* ValidateArgument */

/* Tests_SRS_UMOCK_C_LIB_01_082: [The ValidateArgument call modifier shall record that the indexth argument will be validated for that specific call.]*/
TEST_FUNCTION(ValidateArgument_by_index_for_first_arg_ignores_the_first_argument)
{
    // arrange
    EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateArgument(1);

    // act
    test_dependency_2_args(41, 43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(41,43)]", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_082: [The ValidateArgument call modifier shall record that the indexth argument will be validated for that specific call.]*/
TEST_FUNCTION(ValidateArgument_by_index_for_second_arg_validates_the_second_argument)
{
    // arrange
    EXPECTED_CALL(test_dependency_2_args(42, 42))
        .ValidateArgument(2);

    // act
    test_dependency_2_args(42, 43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,42)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_2_args(42,43)]", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_082: [The ValidateArgument call modifier shall record that the indexth argument will be validated for that specific call.]*/
TEST_FUNCTION(ValidateArgument_by_index_for_first_arg_validates_only_the_first_argument)
{
    // arrange
    EXPECTED_CALL(test_dependency_2_args(42, 43))
        .ValidateArgument(1);

    // act
    test_dependency_2_args(42, 42);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_082: [The ValidateArgument call modifier shall record that the indexth argument will be validated for that specific call.]*/
TEST_FUNCTION(ValidateArgument_by_index_for_second_arg_validates_only_the_second_argument)
{
    // arrange
    EXPECTED_CALL(test_dependency_2_args(42, 42))
        .ValidateArgument(2);

    // act
    test_dependency_2_args(43, 42);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_083: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.]*/
TEST_FUNCTION(ValidateArgument_by_index_with_0_index_triggers_the_on_error_callback)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_2_args(42, 42))
        .ValidateArgument(0);

    // assert
    //TFS661968 ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    //TFS661968 ASSERT_ARE_EQUAL(int, (int)UMOCK_C_ARG_INDEX_OUT_OF_RANGE, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_083: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.]*/
TEST_FUNCTION(ValidateArgument_by_index_with_index_greater_than_arg_count_triggers_the_on_error_callback)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_2_args(42, 42))
        .ValidateArgument(3);

    // assert
    //TFS661968 ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    //TFS661968 ASSERT_ARE_EQUAL(int, (int)UMOCK_C_ARG_INDEX_OUT_OF_RANGE, test_on_umock_c_error_calls[0].error_code);
}

/* SetReturn */

/* Tests_SRS_UMOCK_C_LIB_01_084: [The SetReturn call modifier shall record that when an actual call is matched with the specific expected call, it shall return the result value to the code under test.] */
TEST_FUNCTION(SetReturn_sets_the_return_value_for_a_strict_expected_call)
{
    // arrange
    int result;
    STRICT_EXPECTED_CALL(test_dependency_no_args())
        .SetReturn(42);

    // act
    result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(int, 42, result);
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_084: [The SetReturn call modifier shall record that when an actual call is matched with the specific expected call, it shall return the result value to the code under test.] */
TEST_FUNCTION(SetReturn_sets_the_return_value_for_an_expected_call)
{
    // arrange
    int result;
    EXPECTED_CALL(test_dependency_no_args())
        .SetReturn(42);

    // act
    result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(int, 42, result);
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_084: [The SetReturn call modifier shall record that when an actual call is matched with the specific expected call, it shall return the result value to the code under test.] */
TEST_FUNCTION(SetReturn_sets_the_return_value_only_for_a_matched_call)
{
    // arrange
    int result;
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42))
        .SetReturn(42);

    // act
    result = test_dependency_1_arg(41);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_084: [The SetReturn call modifier shall record that when an actual call is matched with the specific expected call, it shall return the result value to the code under test.] */
TEST_FUNCTION(SetReturn_sets_independent_return_values_for_each_call)
{
    // arrange
    int result1;
    int result2;
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42))
        .SetReturn(142);
    STRICT_EXPECTED_CALL(test_dependency_1_arg(43))
        .SetReturn(143);

    // act
    result1 = test_dependency_1_arg(42);
    result2 = test_dependency_1_arg(43);

    // assert
    ASSERT_ARE_EQUAL(int, 142, result1);
    ASSERT_ARE_EQUAL(int, 143, result2);
}

/* CopyOutArgumentBuffer */

/* Tests_SRS_UMOCK_C_LIB_01_087: [The CopyOutArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later injected as an out argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_LIB_01_116: [ The argument targetted by CopyOutArgument shall also be marked as ignored. ] */
TEST_FUNCTION(CopyOutArgumentBuffer_copies_bytes_to_the_out_argument_for_a_strict_expected_call)
{
    // arrange
    int injected_int = 0x42;
    int actual_int = 0;
    STRICT_EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(1, &injected_int, sizeof(injected_int));


    // act
    (void)test_dependency_1_out_arg(&actual_int);

    // assert
    ASSERT_ARE_EQUAL(int, injected_int, actual_int);
}

/* Tests_SRS_UMOCK_C_LIB_01_087: [The CopyOutArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later injected as an out argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_LIB_01_116: [ The argument targetted by CopyOutArgument shall also be marked as ignored. ] */
TEST_FUNCTION(CopyOutArgumentBuffer_copies_bytes_to_the_out_argument_for_an_expected_call)
{
    // arrange
    int injected_int = 0x42;
    int actual_int = 0;
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(1, &injected_int, sizeof(injected_int));


    // act
    (void)test_dependency_1_out_arg(&actual_int);

    // assert
    ASSERT_ARE_EQUAL(int, injected_int, actual_int);
}

/* Tests_SRS_UMOCK_C_LIB_01_087: [The CopyOutArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later injected as an out argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_LIB_01_116: [ The argument targetted by CopyOutArgument shall also be marked as ignored. ] */
TEST_FUNCTION(CopyOutArgumentBuffer_only_copies_bytes_to_the_out_argument_that_was_specified)
{
    // arrange
    int injected_int = 0x42;
    int actual_int_1 = 0;
    int actual_int_2 = 0;
    EXPECTED_CALL(test_dependency_2_out_args(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(1, &injected_int, sizeof(injected_int));


    // act
    (void)test_dependency_2_out_args(&actual_int_1, &actual_int_2);

    // assert
    ASSERT_ARE_EQUAL(int, injected_int, actual_int_1);
    ASSERT_ARE_EQUAL(int, 0, actual_int_2);
}

/* Tests_SRS_UMOCK_C_LIB_01_087: [The CopyOutArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later injected as an out argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_LIB_01_116: [ The argument targetted by CopyOutArgument shall also be marked as ignored. ] */
TEST_FUNCTION(CopyOutArgumentBuffer_only_copies_bytes_to_the_second_out_argument)
{
    // arrange
    int injected_int = 0x42;
    int actual_int_1 = 0;
    int actual_int_2 = 0;
    EXPECTED_CALL(test_dependency_2_out_args(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &injected_int, sizeof(injected_int));

    // act
    (void)test_dependency_2_out_args(&actual_int_1, &actual_int_2);

    // assert
    ASSERT_ARE_EQUAL(int, 0, actual_int_1);
    ASSERT_ARE_EQUAL(int, injected_int, actual_int_2);
}

/* Tests_SRS_UMOCK_C_LIB_01_088: [The memory shall be copied.]*/
TEST_FUNCTION(CopyOutArgumentBuffer_copies_the_memory_for_later_use)
{
    // arrange
    int injected_int = 0x42;
    int actual_int = 0;
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(1, &injected_int, sizeof(injected_int));

    injected_int = 0;

    // act
    (void)test_dependency_1_out_arg(&actual_int);

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, actual_int);
}

/* Tests_SRS_UMOCK_C_LIB_01_089: [The buffers for previous CopyOutArgumentBuffer calls shall be freed.]*/
/* Tests_SRS_UMOCK_C_LIB_01_133: [ If several calls to CopyOutArgumentBuffer are made, only the last buffer shall be kept. ]*/
TEST_FUNCTION(CopyOutArgumentBuffer_frees_allocated_buffers_for_previous_CopyOutArgumentBuffer)
{
    // arrange
    int injected_int = 0x42;
    int actual_int = 0;
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(1, &injected_int, sizeof(injected_int))
        .CopyOutArgumentBuffer(1, &injected_int, sizeof(injected_int));

    // act
    (void)test_dependency_1_out_arg(&actual_int);

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, actual_int);
}

/* Tests_SRS_UMOCK_C_LIB_01_091: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.]*/
TEST_FUNCTION(CopyOutArgumentBuffer_with_0_index_triggers_the_error_callback)
{
    // arrange
    int injected_int = 0x42;

    // act
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(0, &injected_int, sizeof(injected_int));

    // assert
    //TFS661968 ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    //TFS661968 ASSERT_ARE_EQUAL(int, (int)UMOCK_C_ARG_INDEX_OUT_OF_RANGE, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_091: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.]*/
TEST_FUNCTION(CopyOutArgumentBuffer_with_index_higher_than_count_of_args_triggers_the_error_callback)
{
    // arrange
    int injected_int = 0x42;

    // act
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &injected_int, sizeof(injected_int));

    // assert
    //TFS661968 ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    //TFS661968 ASSERT_ARE_EQUAL(int, (int)UMOCK_C_ARG_INDEX_OUT_OF_RANGE, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_092: [If bytes is NULL or length is 0, umock_c shall raise an error with the code UMOCK_C_INVALID_ARGUMENT_BUFFER.] */
TEST_FUNCTION(CopyOutArgumentBuffer_with_NULL_bytes_triggers_the_error_callback)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(1, NULL, sizeof(int));

    // assert
    //TFS661968 ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    // TFS661968ASSERT_ARE_EQUAL(int, (int)UMOCK_C_INVALID_ARGUMENT_BUFFER, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_092: [If bytes is NULL or length is 0, umock_c shall raise an error with the code UMOCK_C_INVALID_ARGUMENT_BUFFER.] */
TEST_FUNCTION(CopyOutArgumentBuffer_with_0_length_triggers_the_error_callback)
{
    // arrange
    int injected_int = 0x42;

    // act
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(1, &injected_int, 0);

    // assert
    //TFS661968 ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    //TFS661968 ASSERT_ARE_EQUAL(int, (int)UMOCK_C_INVALID_ARGUMENT_BUFFER, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_087: [The CopyOutArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later injected as an out argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_LIB_01_116: [ The argument targetted by CopyOutArgument shall also be marked as ignored. ] */
TEST_FUNCTION(CopyOutArgumentBuffer_when_an_error_occurs_preserves_the_previous_state)
{
    // arrange
    int injected_int = 0x42;
    int injected_int_2 = 0x43;
    int actual_int_1 = 0;
    int actual_int_2 = 0;
    EXPECTED_CALL(test_dependency_2_out_args(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &injected_int, sizeof(injected_int))
        .CopyOutArgumentBuffer(0, &injected_int_2, sizeof(injected_int_2));


    // act
    (void)test_dependency_2_out_args(&actual_int_1, &actual_int_2);

    // assert
    ASSERT_ARE_EQUAL(int, 0, actual_int_1);
    ASSERT_ARE_EQUAL(int, injected_int, actual_int_2);
}

/* CopyOutArgumentBuffer_{arg_name} */

/* Tests_SRS_UMOCK_C_LIB_01_154: [ The CopyOutArgumentBuffer_{arg_name} call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later injected as an out argument when the code under test calls the mock function. ] */
/* Tests_SRS_UMOCK_C_LIB_01_159: [ The argument targetted by CopyOutArgumentBuffer_{arg_name} shall also be marked as ignored. ] */
TEST_FUNCTION(CopyOutArgumentBuffer_arg_name_copies_bytes_to_the_out_argument_for_a_strict_expected_call)
{
    // arrange
    int injected_int = 0x42;
    int actual_int = 0;
    STRICT_EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_a(&injected_int, sizeof(injected_int));


    // act
    (void)test_dependency_1_out_arg(&actual_int);

    // assert
    ASSERT_ARE_EQUAL(int, injected_int, actual_int);
}

/* Tests_SRS_UMOCK_C_LIB_01_154: [ The CopyOutArgumentBuffer_{arg_name} call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later injected as an out argument when the code under test calls the mock function. ] */
/* Tests_SRS_UMOCK_C_LIB_01_159: [ The argument targetted by CopyOutArgumentBuffer_{arg_name} shall also be marked as ignored. ] */
TEST_FUNCTION(CopyOutArgumentBuffer_arg_name_copies_bytes_to_the_out_argument_for_an_expected_call)
{
    // arrange
    int injected_int = 0x42;
    int actual_int = 0;
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_a(&injected_int, sizeof(injected_int));

    // act
    (void)test_dependency_1_out_arg(&actual_int);

    // assert
    ASSERT_ARE_EQUAL(int, injected_int, actual_int);
}

/* Tests_SRS_UMOCK_C_LIB_01_154: [ The CopyOutArgumentBuffer_{arg_name} call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later injected as an out argument when the code under test calls the mock function. ] */
/* Tests_SRS_UMOCK_C_LIB_01_159: [ The argument targetted by CopyOutArgumentBuffer_{arg_name} shall also be marked as ignored. ] */
TEST_FUNCTION(CopyOutArgumentBuffer_arg_name_only_copies_bytes_to_the_out_argument_that_was_specified)
{
    // arrange
    int injected_int = 0x42;
    int actual_int_1 = 0;
    int actual_int_2 = 0;
    EXPECTED_CALL(test_dependency_2_out_args(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_a(&injected_int, sizeof(injected_int));

    // act
    (void)test_dependency_2_out_args(&actual_int_1, &actual_int_2);

    // assert
    ASSERT_ARE_EQUAL(int, injected_int, actual_int_1);
    ASSERT_ARE_EQUAL(int, 0, actual_int_2);
}

/* Tests_SRS_UMOCK_C_LIB_01_154: [ The CopyOutArgumentBuffer_{arg_name} call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later injected as an out argument when the code under test calls the mock function. ] */
/* Tests_SRS_UMOCK_C_LIB_01_159: [ The argument targetted by CopyOutArgumentBuffer_{arg_name} shall also be marked as ignored. ] */
TEST_FUNCTION(CopyOutArgumentBuffer_arg_name_only_copies_bytes_to_the_second_out_argument)
{
    // arrange
    int injected_int = 0x42;
    int actual_int_1 = 0;
    int actual_int_2 = 0;
    EXPECTED_CALL(test_dependency_2_out_args(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_b(&injected_int, sizeof(injected_int));

    // act
    (void)test_dependency_2_out_args(&actual_int_1, &actual_int_2);

    // assert
    ASSERT_ARE_EQUAL(int, 0, actual_int_1);
    ASSERT_ARE_EQUAL(int, injected_int, actual_int_2);
}

/* Tests_SRS_UMOCK_C_LIB_01_155: [ The memory shall be copied. ]*/
TEST_FUNCTION(CopyOutArgumentBuffer_arg_name_copies_the_memory_for_later_use)
{
    // arrange
    int injected_int = 0x42;
    int actual_int = 0;
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_a(&injected_int, sizeof(injected_int));

    injected_int = 0;

    // act
    (void)test_dependency_1_out_arg(&actual_int);

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, actual_int);
}

/* Tests_SRS_UMOCK_C_LIB_01_163: [ The buffers for previous CopyOutArgumentBuffer calls shall be freed. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_156: [ If several calls to CopyOutArgumentBuffer are made, only the last buffer shall be kept. ]*/
TEST_FUNCTION(CopyOutArgumentBuffer_arg_name_frees_allocated_buffers_for_previous_CopyOutArgumentBuffer)
{
    // arrange
    int injected_int = 0x42;
    int actual_int = 0;
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_a(&injected_int, sizeof(injected_int))
        .CopyOutArgumentBuffer_a(&injected_int, sizeof(injected_int));

    // act
    (void)test_dependency_1_out_arg(&actual_int);

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, actual_int);
}

/* Tests_SRS_UMOCK_C_LIB_01_158: [ If bytes is NULL or length is 0, umock_c shall raise an error with the code UMOCK_C_INVALID_ARGUMENT_BUFFER. ] */
TEST_FUNCTION(CopyOutArgumentBuffer_arg_name_with_NULL_bytes_triggers_the_error_callback)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_a(NULL, sizeof(int));

    // assert
    //TFS661968 ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    //TFS661968 ASSERT_ARE_EQUAL(int, (int)UMOCK_C_INVALID_ARGUMENT_BUFFER, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_158: [ If bytes is NULL or length is 0, umock_c shall raise an error with the code UMOCK_C_INVALID_ARGUMENT_BUFFER. ] */
TEST_FUNCTION(CopyOutArgumentBuffer_arg_name_with_0_length_triggers_the_error_callback)
{
    // arrange
    int injected_int = 0x42;

    // act
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_a(&injected_int, 0);

    // assert
    //TFS661968 ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    //TFS661968 ASSERT_ARE_EQUAL(int, (int)UMOCK_C_INVALID_ARGUMENT_BUFFER, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_154: [ The CopyOutArgumentBuffer_{arg_name} call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later injected as an out argument when the code under test calls the mock function. ] */
/* Tests_SRS_UMOCK_C_LIB_01_159: [ The argument targetted by CopyOutArgumentBuffer_{arg_name} shall also be marked as ignored. ] */
TEST_FUNCTION(CopyOutArgumentBuffer_arg_name_when_an_error_occurs_preserves_the_previous_state)
{
    // arrange
    int injected_int = 0x42;
    int injected_int_2 = 0x43;
    int actual_int_1 = 0;
    int actual_int_2 = 0;
    EXPECTED_CALL(test_dependency_2_out_args(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_b(&injected_int, sizeof(injected_int))
        .CopyOutArgumentBuffer(0, &injected_int_2, sizeof(injected_int_2));


    // act
    (void)test_dependency_2_out_args(&actual_int_1, &actual_int_2);

    // assert
    ASSERT_ARE_EQUAL(int, 0, actual_int_1);
    ASSERT_ARE_EQUAL(int, injected_int, actual_int_2);
}

TEST_FUNCTION(CopyOutArgumentBuffer_arg_name_overrides_the_buffer_for_CopyOutArgumentBuffer)
{
    // arrange
    int injected_int = 0x42;
    int injected_int_2 = 0x43;
    int actual_int = 0;
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(1, &injected_int, sizeof(injected_int))
        .CopyOutArgumentBuffer_a(&injected_int_2, sizeof(injected_int_2));


    // act
    (void)test_dependency_1_out_arg(&actual_int);

    // assert
    ASSERT_ARE_EQUAL(int, injected_int_2, actual_int);
}

TEST_FUNCTION(CopyOutArgumentBuffer_overrides_the_buffer_for_CopyOutArgumentBuffer_arg_name)
{
    // arrange
    int injected_int = 0x42;
    int injected_int_2 = 0x43;
    int actual_int = 0;
    EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_a(&injected_int_2, sizeof(injected_int_2))
        .CopyOutArgumentBuffer(1, &injected_int, sizeof(injected_int));


    // act
    (void)test_dependency_1_out_arg(&actual_int);

    // assert
    ASSERT_ARE_EQUAL(int, injected_int, actual_int);
}

/* ValidateArgumentBuffer */

/* Tests_SRS_UMOCK_C_LIB_01_095: [The ValidateArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later compared against a pointer type argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_LIB_01_096: [If the content of the code under test buffer and the buffer supplied to ValidateArgumentBuffer does not match then this should be treated as a mismatch in argument comparison for that argument.]*/
/* Tests_SRS_UMOCK_C_LIB_01_097: [ValidateArgumentBuffer shall implicitly perform an IgnoreArgument on the indexth argument.]*/
TEST_FUNCTION(ValidateArgumentBuffer_checks_the_argument_buffer)
{
    // arrange
    int expected_int = 0x42;
    int actual_int = 0x42;
    STRICT_EXPECTED_CALL(test_dependency_1_out_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &expected_int, sizeof(expected_int));

    // act
    (void)test_dependency_1_out_arg(&actual_int);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_095: [The ValidateArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later compared against a pointer type argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_LIB_01_096: [If the content of the code under test buffer and the buffer supplied to ValidateArgumentBuffer does not match then this should be treated as a mismatch in argument comparison for that argument.]*/
/* Tests_SRS_UMOCK_C_LIB_01_097: [ValidateArgumentBuffer shall implicitly perform an IgnoreArgument on the indexth argument.]*/
TEST_FUNCTION(ValidateArgumentBuffer_checks_the_argument_buffer_and_mismatch_is_detected_when_content_does_not_match)
{
    // arrange
    unsigned char expected_buffer[] = { 0x42 };
    unsigned char actual_buffer[] = { 0x43 };
    char actual_string[64];
    STRICT_EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, expected_buffer, sizeof(expected_buffer));

    // act
    test_dependency_buffer_arg(actual_buffer);

    // assert
    (void)sprintf(actual_string, "[test_dependency_buffer_arg(%p)]", actual_buffer);
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_buffer_arg([0x42])]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, actual_string, umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_095: [The ValidateArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later compared against a pointer type argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_LIB_01_096: [If the content of the code under test buffer and the buffer supplied to ValidateArgumentBuffer does not match then this should be treated as a mismatch in argument comparison for that argument.]*/
/* Tests_SRS_UMOCK_C_LIB_01_097: [ValidateArgumentBuffer shall implicitly perform an IgnoreArgument on the indexth argument.]*/
TEST_FUNCTION(ValidateArgumentBuffer_checks_the_argument_buffer_and_mismatch_is_detected_when_content_does_not_match_for_expected_call)
{
    // arrange
    unsigned char expected_buffer[] = { 0x42 };
    unsigned char actual_buffer[] = { 0x43 };
    char actual_string[64];
    EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, expected_buffer, sizeof(expected_buffer));

    // act
    test_dependency_buffer_arg(actual_buffer);

    // assert
    (void)sprintf(actual_string, "[test_dependency_buffer_arg(%p)]", actual_buffer);
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_buffer_arg([0x42])]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, actual_string, umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_099: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.]*/
TEST_FUNCTION(ValidateArgumentBuffer_with_0_index_triggers_an_error)
{
    // arrange
    unsigned char expected_buffer[] = { 0x42 };
    unsigned char actual_buffer[] = { 0x43 };
    EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(0, expected_buffer, sizeof(expected_buffer));

    // act
    test_dependency_buffer_arg(actual_buffer);

    // assert
    //TFS661968 ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    //TFS661968 ASSERT_ARE_EQUAL(int, (int)UMOCK_C_ARG_INDEX_OUT_OF_RANGE, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_099: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.]*/
TEST_FUNCTION(ValidateArgumentBuffer_with_index_higher_than_the_Arg_count_triggers_an_error)
{
    // arrange
    unsigned char expected_buffer[] = { 0x42 };
    unsigned char actual_buffer[] = { 0x43 };
    EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(2, expected_buffer, sizeof(expected_buffer));

    // act
    test_dependency_buffer_arg(actual_buffer);

    // assert
    //TFS661968 ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    //TFS661968 ASSERT_ARE_EQUAL(int, (int)UMOCK_C_ARG_INDEX_OUT_OF_RANGE, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_100: [If bytes is NULL or length is 0, umock_c shall raise an error with the code UMOCK_C_INVALID_ARGUMENT_BUFFER.] */
TEST_FUNCTION(ValidateArgumentBuffer_with_NULL_buffer_triggers_the_error_callback)
{
    // arrange

    // act
    EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, NULL, 1);

    // assert
    //TFS661968 ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    //TFS661968 ASSERT_ARE_EQUAL(int, (int)UMOCK_C_INVALID_ARGUMENT_BUFFER, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_100: [If bytes is NULL or length is 0, umock_c shall raise an error with the code UMOCK_C_INVALID_ARGUMENT_BUFFER.] */
TEST_FUNCTION(ValidateArgumentBuffer_with_0_length_triggers_the_error_callback)
{
    // arrange
    unsigned char expected_buffer[] = { 0x42 };

    // act
    EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, expected_buffer, 0);

    // assert
    //TFS661968 ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    //TFS661968 ASSERT_ARE_EQUAL(int, (int)UMOCK_C_INVALID_ARGUMENT_BUFFER, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_095: [The ValidateArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later compared against a pointer type argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_LIB_01_096: [If the content of the code under test buffer and the buffer supplied to ValidateArgumentBuffer does not match then this should be treated as a mismatch in argument comparison for that argument.]*/
/* Tests_SRS_UMOCK_C_LIB_01_097: [ValidateArgumentBuffer shall implicitly perform an IgnoreArgument on the indexth argument.]*/
TEST_FUNCTION(ValidateArgumentBuffer_with_2_bytes_and_first_byte_different_checks_the_content)
{
    // arrange
    unsigned char expected_buffer[] = { 0x42, 0x41 };
    unsigned char actual_buffer[] = { 0x43, 0x41 };
    char actual_string[64];
    EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, expected_buffer, sizeof(expected_buffer));

    // act
    test_dependency_buffer_arg(actual_buffer);

    // assert
    (void)sprintf(actual_string, "[test_dependency_buffer_arg(%p)]", actual_buffer);
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_buffer_arg([0x42 0x41])]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, actual_string, umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_095: [The ValidateArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later compared against a pointer type argument when the code under test calls the mock function.] */
/* Tests_SRS_UMOCK_C_LIB_01_096: [If the content of the code under test buffer and the buffer supplied to ValidateArgumentBuffer does not match then this should be treated as a mismatch in argument comparison for that argument.]*/
/* Tests_SRS_UMOCK_C_LIB_01_097: [ValidateArgumentBuffer shall implicitly perform an IgnoreArgument on the indexth argument.]*/
TEST_FUNCTION(ValidateArgumentBuffer_with_2_bytes_and_second_byte_different_checks_the_content)
{
    // arrange
    unsigned char expected_buffer[] = { 0x42, 0x41 };
    unsigned char actual_buffer[] = { 0x42, 0x42 };
    char actual_string[64];
    EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, expected_buffer, sizeof(expected_buffer));

    // act
    test_dependency_buffer_arg(actual_buffer);

    // assert
    (void)sprintf(actual_string, "[test_dependency_buffer_arg(%p)]", actual_buffer);
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_buffer_arg([0x42 0x41])]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, actual_string, umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_131: [ The memory pointed by bytes shall be copied. ]*/
TEST_FUNCTION(ValidateArgumentBuffer_copies_the_bytes_to_compare)
{
    // arrange
    unsigned char expected_buffer[] = { 0x42 };
    unsigned char actual_buffer[] = { 0x42 };
    EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, expected_buffer, sizeof(expected_buffer));

    expected_buffer[0] = 0x43;

    // act
    test_dependency_buffer_arg(actual_buffer);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_132: [ If several calls to ValidateArgumentBuffer are made, only the last buffer shall be kept. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_130: [ The buffers for previous ValidateArgumentBuffer calls shall be freed. ]*/
TEST_FUNCTION(When_ValidateArgumentBuffer_is_called_twice_the_last_buffer_is_used)
{
    // arrange
    unsigned char expected_buffer1[] = { 0x43 };
    unsigned char expected_buffer2[] = { 0x42 };
    unsigned char actual_buffer[] = { 0x42 };
    EXPECTED_CALL(test_dependency_buffer_arg(IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, expected_buffer1, sizeof(expected_buffer1))
        .ValidateArgumentBuffer(1, expected_buffer2, sizeof(expected_buffer2));

    // act
    test_dependency_buffer_arg(actual_buffer);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* REGISTER_GLOBAL_MOCK_HOOK */

/* Tests_SRS_UMOCK_C_LIB_01_104: [The REGISTER_GLOBAL_MOCK_HOOK shall register a mock hook to be called every time the mocked function is called by production code.]*/
/* Tests_SRS_UMOCK_C_LIB_01_105: [The hook's result shall be returned by the mock to the production code.]*/
/* Tests_SRS_UMOCK_C_LIB_01_106: [The signature for the hook shall be assumed to have exactly the same arguments and return as the mocked function.]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_HOOK_registers_a_hook_for_the_mock)
{
    // arrange
    int result;
    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_no_args, my_hook_test_dependency_no_args);
    my_hook_result = 0x42;

    // act
    result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_104: [The REGISTER_GLOBAL_MOCK_HOOK shall register a mock hook to be called every time the mocked function is called by production code.]*/
/* Tests_SRS_UMOCK_C_LIB_01_105: [The hook's result shall be returned by the mock to the production code.]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_HOOK_registers_a_hook_for_the_mock_that_returns_2_different_values)
{
    // arrange
    int call1_result;
    int call2_result;
    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_no_args, my_hook_test_dependency_no_args);
    my_hook_result = 0x42;

    // act
    call1_result = test_dependency_no_args();
    call2_result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, call1_result);
    ASSERT_ARE_EQUAL(int, 0x43, call2_result);
}

/* Tests_SRS_UMOCK_C_LIB_01_107: [If there are multiple invocations of REGISTER_GLOBAL_MOCK_HOOK, the last one shall take effect over the previous ones.] */
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_HOOK_twice_makes_the_last_hook_stick)
{
    // arrange
    int result;
    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_no_args, my_hook_test_dependency_no_args);
    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_no_args, my_hook_test_dependency_no_args_2);
    my_hook_result = 0x42;

    // act
    result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(int, 0x21, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_134: [ REGISTER_GLOBAL_MOCK_HOOK called with a NULL hook unregisters a previously registered hook. ]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_HOOK_with_NULL_unregisters_a_previously_registered_hook)
{
    // arrange
    int result;
    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_no_args, my_hook_test_dependency_no_args);
    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_no_args, NULL);

    // act
    result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_135: [ All parameters passed to the mock shall be passed down to the mock hook. ]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_HOOK_the_args_of_the_mock_get_passed_to_the_hook)
{
    // arrange
    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_2_args, my_hook_test_dependency_2_args);

    // act
    (void)test_dependency_2_args(0x42, 0x43);

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, arg_a);
    ASSERT_ARE_EQUAL(int, 0x43, arg_b);
}

/* Tests_SRS_UMOCK_C_LIB_01_104: [The REGISTER_GLOBAL_MOCK_HOOK shall register a mock hook to be called every time the mocked function is called by production code.]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_HOOK_with_a_function_that_returns_void_works)
{
    // arrange
    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_void_return, my_hook_test_dependency_void_return);

    // act
    test_dependency_void_return();

    // assert
    ASSERT_ARE_EQUAL(int, 1, test_dependency_void_return_called);
}

/* REGISTER_GLOBAL_MOCK_RETURN */

/* Tests_SRS_UMOCK_C_LIB_01_108: [The REGISTER_GLOBAL_MOCK_RETURN shall register a return value to always be returned by a mock function.]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_RETURN_makes_a_subsequent_call_to_the_mock_return_the_value)
{
    // arrange
    int result;
    REGISTER_GLOBAL_MOCK_RETURN(test_dependency_global_mock_return_test, 0x45);

    // act
    result = test_dependency_global_mock_return_test();

    // assert
    ASSERT_ARE_EQUAL(int, 0x45, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_109: [If there are multiple invocations of REGISTER_GLOBAL_MOCK_RETURN, the last one shall take effect over the previous ones.]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_RETURN_twice_only_makes_the_second_call_stick)
{
    // arrange
    int result;
    REGISTER_GLOBAL_MOCK_RETURN(test_dependency_global_mock_return_test, 0x45);
    REGISTER_GLOBAL_MOCK_RETURN(test_dependency_global_mock_return_test, 0x46);

    // act
    result = test_dependency_global_mock_return_test();

    // assert
    ASSERT_ARE_EQUAL(int, 0x46, result);
}

/* REGISTER_GLOBAL_MOCK_FAIL_RETURN */

/* Tests_SRS_UMOCK_C_LIB_01_111: [The REGISTER_GLOBAL_MOCK_FAIL_RETURN shall register a fail return value to be returned by a mock function when marked as failed in the expected calls.]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_FAIL_RETURN_is_possible_and_does_not_affect_the_return_value)
{
    // arrange
    int result;
    REGISTER_GLOBAL_MOCK_RETURN(test_dependency_global_mock_return_test, 0x42);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(test_dependency_global_mock_return_test, 0x45);

    // act
    result = test_dependency_global_mock_return_test();

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_112: [If there are multiple invocations of REGISTER_GLOBAL_FAIL_MOCK_RETURN, the last one shall take effect over the previous ones.]*/
TEST_FUNCTION(Multiple_REGISTER_GLOBAL_MOCK_FAIL_RETURN_calls_are_possible)
{
    // arrange
    int result;
    REGISTER_GLOBAL_MOCK_RETURN(test_dependency_global_mock_return_test, 0x42);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(test_dependency_global_mock_return_test, 0x45);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(test_dependency_global_mock_return_test, 0x46);

    // act
    result = test_dependency_global_mock_return_test();

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_142: [ If any error occurs during REGISTER_GLOBAL_MOCK_FAIL_RETURN, umock_c shall raise an error with the code UMOCK_C_ERROR. ]*/
TEST_FUNCTION(When_copy_fails_in_REGISTER_GLOBAL_MOCK_FAIL_RETURN_then_on_error_is_triggered)
{
    // arrange
    int result;
    REGISTER_GLOBAL_MOCK_RETURN(test_dependency_global_mock_return_test, 0x42);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(test_dependency_global_mock_return_test, 0x45);

    // act
    result = test_dependency_global_mock_return_test();

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, result);
}

/* REGISTER_GLOBAL_MOCK_RETURNS */

/* Tests_SRS_UMOCK_C_LIB_01_113: [The REGISTER_GLOBAL_MOCK_RETURNS shall register both a success and a fail return value associated with a mock function.]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_RETURNS_registers_the_return_value)
{
    // arrange
    int result;
    REGISTER_GLOBAL_MOCK_RETURNS(test_dependency_global_mock_return_test, 0xAA, 0x43);

    // act
    result = test_dependency_global_mock_return_test();

    // assert
    ASSERT_ARE_EQUAL(int, 0xAA, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_114: [If there are multiple invocations of REGISTER_GLOBAL_MOCK_RETURNS, the last one shall take effect over the previous ones.]*/
TEST_FUNCTION(REGISTER_GLOBAL_MOCK_RETURNS_twice_makes_only_the_last_call_stick)
{
    // arrange
    int result;
    REGISTER_GLOBAL_MOCK_RETURNS(test_dependency_global_mock_return_test, 0xAA, 0x43);
    REGISTER_GLOBAL_MOCK_RETURNS(test_dependency_global_mock_return_test, 0xAB, 0x44);

    // act
    result = test_dependency_global_mock_return_test();

    // assert
    ASSERT_ARE_EQUAL(int, 0xAB, result);
}

/* Type names */

/* Tests_SRS_UMOCK_C_LIB_01_145: [ Since umock_c needs to maintain a list of registered types, the following rules shall be applied: ]*/
/* Tests_SRS_UMOCK_C_LIB_01_146: [ Each type shall be normalized to a form where all extra spaces are removed. ]*/
TEST_FUNCTION(spaces_are_stripped_from_typenames)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_type_with_space("b"));

    // act
    test_dependency_type_with_space("b");

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Supported types */

/* Tests_SRS_UMOCK_C_LIB_01_144: [ Out of the box umock_c shall support the following types through the header umocktypes_c.h: ]*/
/* Tests_SRS_UMOCK_C_LIB_01_028: [**char**] */
/* Tests_SRS_UMOCK_C_LIB_01_029 : [**unsigned char**] */
/* Tests_SRS_UMOCK_C_LIB_01_030 : [**short**] */
/* Tests_SRS_UMOCK_C_LIB_01_031 : [**unsigned short**] */
/* Tests_SRS_UMOCK_C_LIB_01_032 : [**int**] */
/* Tests_SRS_UMOCK_C_LIB_01_033 : [**unsigned int**] */
/* Tests_SRS_UMOCK_C_LIB_01_034 : [**long**] */
/* Tests_SRS_UMOCK_C_LIB_01_035 : [**unsigned long**] */
/* Tests_SRS_UMOCK_C_LIB_01_036 : [**long long**] */
/* Tests_SRS_UMOCK_C_LIB_01_037 : [**unsigned long long**] */
/* Tests_SRS_UMOCK_C_LIB_01_038 : [**float**] */
/* Tests_SRS_UMOCK_C_LIB_01_039 : [**double**] */
/* Tests_SRS_UMOCK_C_LIB_01_040 : [**long double**] */
/* Tests_SRS_UMOCK_C_LIB_01_041 : [**size_t**] */
/* Tests_SRS_UMOCK_C_LIB_01_151: [ void\* ]*/
/* Tests_SRS_UMOCK_C_LIB_01_152: [ const void\* ]*/
TEST_FUNCTION(native_c_types_are_supported)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_all_types(-42, 42, /* char */
        -43, 43, /* short */
        -44, 44, /* int */
        -45, 45, /* long */
        -46, 46, /* long long */
        -42.42f,  /* float */
        4242.42, /* double */
        4242.42, /* long double */
        0x42, /* size_t*/
        (void*)0x42, /* void* */
        (const void*)0x42 /* const void* */
        ));

    // act
    test_dependency_all_types(-42, 42, /* char */
        -43, 43, /* short */
        -44, 44, /* int */
        -45, 45, /* long */
        -46, 46, /* long long */
        -42.42f,  /* float */
        4242.42, /* double */
        4242.42, /* long double */
        0x42, /* size_t*/
        (void*)0x42, /* void* */
        (const void*)0x42 /* const void* */
        );

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(size_t, 0, test_on_umock_c_error_call_count);
}

/* Tests_SRS_UMOCK_C_LIB_01_148: [ If call comparison fails an error shall be indicated by calling the error callback with UMOCK_C_COMPARE_CALL_ERROR. ]*/
TEST_FUNCTION(when_a_type_is_not_supported_an_error_is_triggered)
{
    TEST_STRUCT_NOT_REGISTERED a = { 0 };

    // arrange
    STRICT_EXPECTED_CALL(test_dependency_type_not_registered(a));

    // act
    test_dependency_type_not_registered(a);

    // assert
    //TFS661968 ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
}

/* Call comparison rules */

/* Tests_SRS_UMOCK_C_LIB_01_136: [ When multiple return values are set for a mock function by using different means (such as SetReturn), the following order shall be in effect: ]*/
/* Tests_SRS_UMOCK_C_LIB_01_137: [ - If a return value has been specified for an expected call then that value shall be returned. ]*/
TEST_FUNCTION(when_the_return_value_is_given_by_SetReturn_then_it_is_returned)
{
    // arrange
    int result;
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42))
        .SetReturn(42);

    // act
    result = test_dependency_1_arg(42);

    // assert
    ASSERT_ARE_EQUAL(int, 42, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_136: [ When multiple return values are set for a mock function by using different means (such as SetReturn), the following order shall be in effect: ]*/
/* Tests_SRS_UMOCK_C_LIB_01_137: [ - If a return value has been specified for an expected call then that value shall be returned. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_138: [ - If a global mock hook has been specified then it shall be called and its result returned. ]*/
TEST_FUNCTION(when_the_return_value_is_given_by_SetReturn_for_a_function_with_a_global_return_hook_the_SetReturn_value_is_returned)
{
    // arrange
    int result;

    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_with_global_mock_hook, my_hook_test_dependency_with_global_mock_hook);

    STRICT_EXPECTED_CALL(test_dependency_with_global_mock_hook())
        .SetReturn(42);

    // act
    result = test_dependency_with_global_mock_hook();

    // assert
    ASSERT_ARE_EQUAL(int, 42, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_136: [ When multiple return values are set for a mock function by using different means (such as SetReturn), the following order shall be in effect: ]*/
/* Tests_SRS_UMOCK_C_LIB_01_137: [ - If a return value has been specified for an expected call then that value shall be returned. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_138: [ - If a global mock hook has been specified then it shall be called and its result returned. ]*/
TEST_FUNCTION(when_the_return_value_is_not_given_by_SetReturn_for_a_function_with_a_global_return_hook_the_mock_hook_return_value_is_returned)
{
    // arrange
    int result;
    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_with_global_mock_hook, my_hook_test_dependency_with_global_mock_hook);
    STRICT_EXPECTED_CALL(test_dependency_with_global_mock_hook());

    // act
    result = test_dependency_with_global_mock_hook();

    // assert
    ASSERT_ARE_EQUAL(int, 43, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_139: [ - If a global return value has been specified then it shall be returned. ]*/
TEST_FUNCTION(when_the_return_value_is_given_by_SetReturn_for_a_function_with_a_global_return_hook_and_global_return_the_SetReturn_value_is_returned)
{
    // arrange
    int result;
    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_with_global_mock_hook, my_hook_test_dependency_with_global_mock_hook);
    REGISTER_GLOBAL_MOCK_RETURN(test_dependency_with_global_mock_hook, 44);
    STRICT_EXPECTED_CALL(test_dependency_with_global_mock_hook())
        .SetReturn(42);

    // act
    result = test_dependency_with_global_mock_hook();

    // assert
    ASSERT_ARE_EQUAL(int, 42, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_139: [ - If a global return value has been specified then it shall be returned. ]*/
TEST_FUNCTION(when_the_return_value_is_not_given_by_SetReturn_for_a_function_with_a_global_return_hook_and_global_return_the_global_mock_hook_value_is_returned)
{
    // arrange
    int result;
    REGISTER_GLOBAL_MOCK_HOOK(test_dependency_with_global_mock_hook, my_hook_test_dependency_with_global_mock_hook);
    REGISTER_GLOBAL_MOCK_RETURN(test_dependency_with_global_mock_hook, 44);

    STRICT_EXPECTED_CALL(test_dependency_with_global_mock_hook());

    // act
    result = test_dependency_with_global_mock_hook();

    // assert
    ASSERT_ARE_EQUAL(int, 43, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_139: [ - If a global return value has been specified then it shall be returned. ]*/
TEST_FUNCTION(when_the_return_value_is_specified_only_by_global_return_that_global_return_value_is_returned)
{
    // arrange
    int result;
    REGISTER_GLOBAL_MOCK_RETURN(test_dependency_with_global_return, 44);
    STRICT_EXPECTED_CALL(test_dependency_with_global_return());

    // act
    result = test_dependency_with_global_return();

    // assert
    ASSERT_ARE_EQUAL(int, 44, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_140: [ - Otherwise the value of a static variable of the same type as the return type shall be returned. ]*/
TEST_FUNCTION(when_no_return_value_is_specified_for_a_function_returning_int_0_is_returned)
{
    // arrange
    int result;
    STRICT_EXPECTED_CALL(test_dependency_returning_int());

    // act
    result = test_dependency_returning_int();

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
}

/* MOCK_FUNCTION_WITH_CODE tests */

TEST_FUNCTION(a_strict_expected_Call_mock_function_with_code_validates_args)
{
    // arrange
    STRICT_EXPECTED_CALL(test_mock_function_with_code_1_arg(42));

    // act
    test_mock_function_with_code_1_arg(42);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

TEST_FUNCTION(an_expected_call_for_a_mock_function_with_code_ignores_args)
{
    // arrange
    EXPECTED_CALL(test_mock_function_with_code_1_arg(0));

    // act
    test_mock_function_with_code_1_arg(42);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

TEST_FUNCTION(the_value_for_a_function_that_returns_a_char_ptr_is_freed)
{
    // arrange
    const char* result;
    EXPECTED_CALL(test_mock_function_returning_string())
        .SetReturn("a");

    // act
    result = test_mock_function_returning_string();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "a", result);
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

TEST_FUNCTION(the_value_for_a_function_that_returns_a_char_ptr_is_freed_when_no_matched_return)
{
    // arrange

    // act
    const char* result = test_mock_function_returning_string();

    // assert
    ASSERT_IS_NULL(result);
}

TEST_FUNCTION(the_value_for_a_function_that_returns_a_char_ptr_with_a_default_is_freed_when_no_matched_return)
{
    // arrange

    // act
    const char* result = test_mock_function_returning_string_with_code();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "a", result);
}

TEST_FUNCTION(the_value_for_a_function_that_returns_a_char_ptr_set_by_macro_is_freed)
{
    // arrange
    const char* result;
    REGISTER_GLOBAL_MOCK_RETURN(test_mock_function_returning_string_with_macro, "a");

    // act
    result = test_mock_function_returning_string_with_macro();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "a", result);
}

/* REGISTER_UMOCK_ALIAS_TYPE */

/* Tests_SRS_UMOCK_C_LIB_01_149: [ REGISTER_UMOCK_ALIAS_TYPE registers a new alias type for another type. ]*/
TEST_FUNCTION(registering_an_alias_type_works)
{
    // arrange
    funkytype result;
    REGISTER_UMOCK_ALIAS_TYPE(funkytype, int);
    STRICT_EXPECTED_CALL(test_mock_function_with_funkytype(42))
        .SetReturn(42);

    // act
    result = test_mock_function_with_funkytype(42);

    // assert
    ASSERT_ARE_EQUAL(int, 42, (int)result);
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}


/* Tests_SRS_UMOCK_C_LIB_02_001: [ If the types do not have the same size the on_error callback shall be called with UMOCK_C_REGISTER_TYPE_FAILED. ]*/
TEST_FUNCTION(registering_an_alias_type_fails_on_different_sizes)
{
    /// arrange

#ifdef _MSC_VER 
#pragma warning( push )
#pragma warning( disable : 4127 ) /*warning C4127: conditional expression is constant*/ /*generated in REGISTER_UMOCK_ALIAS_TYPE because sizeof operator is evaluated at compile time (at least for Visual Studio... ) */
#endif

    /// act
    REGISTER_UMOCK_ALIAS_TYPE(type_of_1_byte, int);

#ifdef _MSC_VER 
#pragma warning( pop )
#endif

    /// assert
    ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
}


/* Tests_SRS_UMOCK_C_LIB_01_153: [ If no custom handler has beed registered for a pointer type, it shall be trated as void*. ] */
TEST_FUNCTION(when_an_unregistered_pointer_type_is_used_it_defaults_to_void_ptr)
{
    // arrange
    unsigned char*** result;
    REGISTER_UMOCK_ALIAS_TYPE(funkytype, int);
    STRICT_EXPECTED_CALL(test_mock_function_with_unregistered_ptr_type((unsigned char***)0x42))
        .SetReturn((unsigned char***)0x42);

    // act
    result = test_mock_function_with_unregistered_ptr_type((unsigned char***)0x42);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, (void*)0x42, result);
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_179: [ IMPLEMENT_UMOCK_C_ENUM_TYPE shall implement umock_c handlers for an enum type. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_180: [ The variable arguments are a list making up the enum values. ]*/
TEST_FUNCTION(matching_with_an_enum_type_works)
{
    // arrange
    REGISTER_TYPE(TEST_ENUM, TEST_ENUM);
    STRICT_EXPECTED_CALL(test_mock_function_with_enum_type(TEST_ENUM_VALUE_1));

    // act
    test_mock_function_with_enum_type(TEST_ENUM_VALUE_2);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_mock_function_with_enum_type(TEST_ENUM_VALUE_1)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_mock_function_with_enum_type(TEST_ENUM_VALUE_2)]", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_179: [ IMPLEMENT_UMOCK_C_ENUM_TYPE shall implement umock_c handlers for an enum type. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_181: [ If a value that is not part of the enum is used, it shall be treated as an int value. ]*/
TEST_FUNCTION(when_the_enum_value_is_not_within_the_enum_the_int_value_is_filled_in)
{
    // arrange
    REGISTER_TYPE(TEST_ENUM, TEST_ENUM);
    STRICT_EXPECTED_CALL(test_mock_function_with_enum_type((TEST_ENUM)(TEST_ENUM_VALUE_1+2)));

    // act
    test_mock_function_with_enum_type(TEST_ENUM_VALUE_2);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_mock_function_with_enum_type(2)]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_mock_function_with_enum_type(TEST_ENUM_VALUE_2)]", umock_c_get_actual_calls());
}

/* CaptureReturn */

/* Tests_SRS_UMOCK_C_LIB_01_179: [ The CaptureReturn call modifier shall copy the return value that is being returned to the code under test when an actual call is matched with the expected call. ]*/
TEST_FUNCTION(capture_return_captures_the_return_value)
{
    // arrange
    int captured_return;

    STRICT_EXPECTED_CALL(test_dependency_for_capture_return())
        .CaptureReturn(&captured_return);

    // act
    test_dependency_for_capture_return();

    // assert
    ASSERT_ARE_EQUAL(int, 42, captured_return);
}

/* Tests_SRS_UMOCK_C_LIB_01_180: [ If CaptureReturn is called multiple times for the same call, an error shall be indicated with the code UMOCK_C_CAPTURE_RETURN_ALREADY_USED. ]*/
TEST_FUNCTION(capture_return_twice_captures_the_return_value_in_the_pointer_indicated_by_the_second_call)
{
    // arrange
    int captured_return_1;
    int captured_return_2;

    STRICT_EXPECTED_CALL(test_dependency_for_capture_return())
        .CaptureReturn(&captured_return_1)
        .CaptureReturn(&captured_return_2);

    // act
    test_dependency_for_capture_return();

    // assert
    //TFS661968 ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    //TFS661968 ASSERT_ARE_EQUAL(UMOCK_C_ERROR_CODE, UMOCK_C_CAPTURE_RETURN_ALREADY_USED, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_182: [ If captured_return_value is NULL, umock_c shall raise an error with the code UMOCK_C_NULL_ARGUMENT. ]*/
TEST_FUNCTION(capture_return_with_NULL_argument_indicates_an_error)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_for_capture_return())
        .CaptureReturn(NULL);

    // act
    test_dependency_for_capture_return();

    // assert
    //TFS661968 ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    //TFS661968 ASSERT_ARE_EQUAL(UMOCK_C_ERROR_CODE, UMOCK_C_NULL_ARGUMENT, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_179: [ The CaptureReturn call modifier shall copy the return value that is being returned to the code under test when an actual call is matched with the expected call. ]*/
TEST_FUNCTION(capture_return_when_no_actual_call_does_not_capture_anything)
{
    // arrange
    int captured_return = 0;

    STRICT_EXPECTED_CALL(test_dependency_for_capture_return())
        .CaptureReturn(&captured_return);

    // act

    // assert
    ASSERT_ARE_EQUAL(int, 0, captured_return);
}

/* Tests_SRS_UMOCK_C_LIB_01_179: [ The CaptureReturn call modifier shall copy the return value that is being returned to the code under test when an actual call is matched with the expected call. ]*/
TEST_FUNCTION(capture_return_when_no_matching_actual_call_does_not_capture_anything)
{
    // arrange
    int captured_return = 0;

    STRICT_EXPECTED_CALL(test_dependency_for_capture_return_with_arg(42))
        .CaptureReturn(&captured_return);

    // act
    test_dependency_for_capture_return_with_arg(41);

    // assert
    ASSERT_ARE_EQUAL(int, 0, captured_return);
}

/* Tests_SRS_UMOCK_C_LIB_01_179: [ The CaptureReturn call modifier shall copy the return value that is being returned to the code under test when an actual call is matched with the expected call. ]*/
TEST_FUNCTION(capture_return_takes_into_account_a_set_return_call)
{
    // arrange
    int captured_return = 0;

    STRICT_EXPECTED_CALL(test_dependency_for_capture_return())
        .SetReturn(42)
        .CaptureReturn(&captured_return);

    // act
    test_dependency_for_capture_return();

    // assert
    ASSERT_ARE_EQUAL(int, 42, captured_return);
}

/* Tests_SRS_UMOCK_C_LIB_01_179: [ The CaptureReturn call modifier shall copy the return value that is being returned to the code under test when an actual call is matched with the expected call. ]*/
TEST_FUNCTION(capture_return_captures_the_return_value_different_value)
{
    // arrange
    int captured_return = 0;

    STRICT_EXPECTED_CALL(test_dependency_for_capture_return())
        .CaptureReturn(&captured_return);

    test_return_value = 45;

    // act
    test_dependency_for_capture_return();

    // assert
    ASSERT_ARE_EQUAL(int, 45, captured_return);
}

/* ValidateArgumentValue_{arg_name} */

/* Tests_SRS_UMOCK_C_LIB_01_183: [ The ValidateArgumentValue_{arg_name} shall validate that the value of an argument matches the value pointed by arg_value. ]*/
TEST_FUNCTION(validate_argument_value_validates_the_value_pointed_by_arg_value)
{
    // arrange
    int arg_value = 0;

    STRICT_EXPECTED_CALL(test_dependency_1_arg(0))
        .ValidateArgumentValue_a(&arg_value);

    arg_value = 42;

    // act
    (void)test_dependency_1_arg(42);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_183: [ The ValidateArgumentValue_{arg_name} shall validate that the value of an argument matches the value pointed by arg_value. ]*/
TEST_FUNCTION(validate_argument_value_validates_the_value_pointed_by_arg_value_for_a_char_star)
{
    // arrange
    char* arg_value = "42";

    STRICT_EXPECTED_CALL(test_dependency_char_star_arg(NULL))
        .ValidateArgumentValue_s(&arg_value);

    arg_value = "43";

    // act
    (void)test_dependency_char_star_arg("43");

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_184: [ If arg_value is NULL, umock_c shall raise an error with the code UMOCK_C_NULL_ARGUMENT. ]*/
TEST_FUNCTION(validate_argument_value_with_NULL_value_triggers_an_error)
{
    // arrange

    // act
    STRICT_EXPECTED_CALL(test_dependency_char_star_arg(NULL))
        .ValidateArgumentValue_s(NULL);

    // assert
    //TFS661968 ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    //TFS661968 ASSERT_ARE_EQUAL(int, (int)UMOCK_C_NULL_ARGUMENT, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_185: [ The ValidateArgumentValue_{arg_name} modifier shall inhibit comparing with any value passed directly as an argument in the expected call. ]*/
TEST_FUNCTION(validate_argument_value_overrides_existing_arg_value)
{
    // arrange
    char* arg_value = "42";

    STRICT_EXPECTED_CALL(test_dependency_char_star_arg("42"))
        .ValidateArgumentValue_s(&arg_value);

    arg_value = "43";

    // act
    (void)test_dependency_char_star_arg("43");

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_186: [ The ValidateArgumentValue_{arg_name} shall implicitly do a ValidateArgument for the arg_name argument, making sure the argument is not ignored. ]*/
TEST_FUNCTION(validate_argument_value_shall_implicitly_validate_the_argument)
{
    // arrange
    char* arg_value = "42";

    STRICT_EXPECTED_CALL(test_dependency_char_star_arg("42"))
        .IgnoreArgument_s()
        .ValidateArgumentValue_s(&arg_value);

    arg_value = "41";

    // act
    (void)test_dependency_char_star_arg("43");

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_char_star_arg(\"41\")]", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_char_star_arg(\"43\")]", umock_c_get_actual_calls());
}

/* pair calls */

/* Tests_SRS_UMOCK_C_LIB_01_187: [ REGISTER_UMOCKC_PAIRED_CREATE_DESTROY_CALLS shall register with umock two calls that are expected to be paired. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_188: [ The create call shall have a non-void return type. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_191: [ At each create_call a memory block shall be allocated so that it can be reported as a leak by any memory checker. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_193: [ When a destroy_call happens the memory block associated with the argument passed to it shall be freed. ] */
TEST_FUNCTION(paired_calls_are_checked_and_no_leak_happens)
{
    // arrange
    SOME_HANDLE h;
    REGISTER_UMOCKC_PAIRED_CREATE_DESTROY_CALLS(some_create, some_destroy);

    // act
    h = some_create(42);
    some_destroy(h);

    // assert
    // no explicit assert
}

/* Tests_SRS_UMOCK_C_LIB_01_190: [ If create_call or destroy_call do not obey these rules, at the time of calling REGISTER_UMOCKC_PAIRED_CREATE_DESTROY_CALLS umock_c shall raise an error with the code UMOCK_C_INVALID_PAIRED_CALLS. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_188: [ The create call shall have a non-void return type. ]*/
TEST_FUNCTION(when_registering_paired_calls_for_a_create_with_void_return_an_error_is_fired)
{
    // arrange

    // act
    REGISTER_UMOCKC_PAIRED_CREATE_DESTROY_CALLS(some_create_void_return, some_destroy_void_return);

    // assert
    ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(UMOCK_C_ERROR_CODE, UMOCK_C_INVALID_PAIRED_CALLS, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_189: [ The destroy call shall take as argument at least one argument. The type of the first argument shall be of the same type as the return type for the create_call. ]*/
TEST_FUNCTION(when_registering_paired_calls_for_a_destroy_with_no_args_an_error_is_fired)
{
    // arrange

    // act
    REGISTER_UMOCKC_PAIRED_CREATE_DESTROY_CALLS(some_create_no_args, some_destroy_no_args);

    // assert
    ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(UMOCK_C_ERROR_CODE, UMOCK_C_INVALID_PAIRED_CALLS, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_189: [ The destroy call shall take as argument at least one argument. The type of the first argument shall be of the same type as the return type for the create_call. ]*/
TEST_FUNCTION(when_registering_paired_calls_for_a_destroy_with_different_arg_type_an_error_is_fired)
{
    // arrange

    // act
    REGISTER_UMOCKC_PAIRED_CREATE_DESTROY_CALLS(some_create_arg_different, some_destroy_arg_different);

    // assert
    ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(UMOCK_C_ERROR_CODE, UMOCK_C_INVALID_PAIRED_CALLS, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_196: [ The type used for the return of create_call and first argument of destroy_call shall be allowed to be any type registered with umock. ]*/
TEST_FUNCTION(paired_calls_are_checked_with_a_struct_as_instance_type)
{
    // arrange
    SOME_STRUCT s;
    REGISTER_UMOCKC_PAIRED_CREATE_DESTROY_CALLS(some_create_with_struct, some_destroy_with_struct);

    // act
    s = some_create_with_struct(42);
    some_destroy_with_struct(s);

    // assert
    // no explicit assert
}

/* Tests_SRS_UMOCK_C_LIB_01_066: [If only the value_type is specified in the macro invocation then the stringify, are_equal, copy and free function names shall be automatically derived from the type as: umockvalue_stringify_value_type, umockvalue_are_equal_value_type, umockvalue_copy_value_type, umockvalue_free_value_type.]*/
TEST_FUNCTION(using_a_type_registered_with_a_register_call_only_with_the_first_arg_succeeds)
{
    // arrange
    STRICT_EXPECTED_CALL(another_test_function((void*)0x4242));

    // act
    another_test_function((void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(size_t, 0, test_on_umock_c_error_call_count);
}

/* ValidateArgumentValue_{arg_name}_AsType */

/* Tests_SRS_UMOCK_C_LIB_01_199: [ ValidateArgumentValue_{arg_name}_AsType shall ensure that validation of the argument arg_name is done as if the argument is of type type_name. ]*/
TEST_FUNCTION(validate_argument_value_as_type_validates_the_value_pointed_by_arg_value_int)
{
    // arrange
    MY_STRUCT expected_arg_value = { 42 };
    MY_STRUCT actual_arg_value = { 42 };

    REGISTER_TYPE(MY_STRUCT*, MY_STRUCT_ptr);

    STRICT_EXPECTED_CALL(test_dependency_with_void_ptr(&expected_arg_value))
        .ValidateArgumentValue_argument_AsType(UMOCK_TYPE(MY_STRUCT*));

    // act
    (void)test_dependency_with_void_ptr(&actual_arg_value);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_200: [ If type_name is NULL, umock_c shall raise an error with the code UMOCK_C_NULL_ARGUMENT. ]*/
TEST_FUNCTION(ValidateArgumentValue_argument_AsType_with_NULL_yields_an_error)
{
    // arrange
    MY_STRUCT expected_arg_value = { 42 };

    REGISTER_TYPE(MY_STRUCT*, MY_STRUCT_ptr);

    // act
    STRICT_EXPECTED_CALL(test_dependency_with_void_ptr(&expected_arg_value))
        .ValidateArgumentValue_argument_AsType(NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(size_t, 0, test_on_umock_c_error_call_count);
    //TFS661968 ASSERT_ARE_EQUAL(size_t, 1, test_on_umock_c_error_call_count);
    //TFS661968 ASSERT_ARE_EQUAL(int, (int)UMOCK_C_NULL_ARGUMENT, test_on_umock_c_error_calls[0].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_199: [ ValidateArgumentValue_{arg_name}_AsType shall ensure that validation of the argument arg_name is done as if the argument is of type type_name. ]*/
TEST_FUNCTION(validate_argument_value_as_type_2_times_with_same_type_does_not_leak)
{
    // arrange
    MY_STRUCT expected_arg_value = { 42 };
    MY_STRUCT actual_arg_value = { 42 };

    REGISTER_TYPE(MY_STRUCT*, MY_STRUCT_ptr);

    STRICT_EXPECTED_CALL(test_dependency_with_void_ptr(&expected_arg_value))
        .ValidateArgumentValue_argument_AsType(UMOCK_TYPE(MY_STRUCT*))
        .ValidateArgumentValue_argument_AsType(UMOCK_TYPE(MY_STRUCT*));

    // act
    (void)test_dependency_with_void_ptr(&actual_arg_value);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_203: [ If ValidateArgumentValue_{arg_name}_AsType is used multiple times on the same argument, the last call shall apply. ]*/
TEST_FUNCTION(validate_argument_value_as_type_2_times_makes_the_last_call_stick)
{
    // arrange
    MY_STRUCT expected_arg_value = { 42 };
    MY_STRUCT actual_arg_value = { 42 };

    REGISTER_TYPE(MY_STRUCT*, MY_STRUCT_ptr);

    STRICT_EXPECTED_CALL(test_dependency_with_void_ptr(&expected_arg_value))
        .ValidateArgumentValue_argument_AsType(UMOCK_TYPE(int*))
        .ValidateArgumentValue_argument_AsType(UMOCK_TYPE(MY_STRUCT*));

    // act
    (void)test_dependency_with_void_ptr(&actual_arg_value);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_205: [ If IGNORED_PTR_ARG or IGNORED_NUM_ARG is used as an argument value with STRICT_EXPECTED_CALL, the argument shall be automatically ignored. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_207: [ IGNORED_NUM_ARG shall be defined to 0 so that it can be used for numeric type arguments. ]*/
TEST_FUNCTION(auto_ignore_ignores_a_numeric_argument)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_1_arg(IGNORED_NUM_ARG));

    // act
    (void)test_dependency_1_arg(42);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_205: [ If IGNORED_PTR_ARG or IGNORED_NUM_ARG is used as an argument value with STRICT_EXPECTED_CALL, the argument shall be automatically ignored. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_206: [ IGNORED_PTR_ARG shall be defined as NULL so that it can be used for pointer type arguments. ]*/
TEST_FUNCTION(auto_ignore_ignores_a_pointer_argument)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_char_star_arg(IGNORED_PTR_ARG));

    // act
    (void)test_dependency_char_star_arg("cucu");

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_205: [ If IGNORED_PTR_ARG or IGNORED_NUM_ARG is used as an argument value with STRICT_EXPECTED_CALL, the argument shall be automatically ignored. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_207: [ IGNORED_NUM_ARG shall be defined to 0 so that it can be used for numeric type arguments. ]*/
TEST_FUNCTION(auto_ignore_ignores_a_2nd_numeric_argument)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_2_args(1, IGNORED_NUM_ARG));

    // act
    (void)test_dependency_2_args(1, 42);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_205: [ If IGNORED_PTR_ARG or IGNORED_NUM_ARG is used as an argument value with STRICT_EXPECTED_CALL, the argument shall be automatically ignored. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_206: [ IGNORED_PTR_ARG shall be defined as NULL so that it can be used for pointer type arguments. ]*/
TEST_FUNCTION(auto_ignore_ignores_a_2nd_pointer_argument)
{
    // arrange
    int a = 42;
    int b = 43;
    STRICT_EXPECTED_CALL(test_dependency_2_out_args(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    (void)test_dependency_2_out_args(&a, &b);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

#define test(a,b) 42

/* Tests_SRS_UMOCK_C_LIB_01_205: [ If IGNORED_PTR_ARG or IGNORED_NUM_ARG is used as an argument value with STRICT_EXPECTED_CALL, the argument shall be automatically ignored. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_206: [ IGNORED_PTR_ARG shall be defined as NULL so that it can be used for pointer type arguments. ]*/
TEST_FUNCTION(auto_ignore_when_first_arg_is_a_macro_succeeds_for_2nd_arg)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_2_args(test(1,2), IGNORED_NUM_ARG));

    // act
    (void)test_dependency_2_args(42, 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_205: [ If IGNORED_PTR_ARG or IGNORED_NUM_ARG is used as an argument value with STRICT_EXPECTED_CALL, the argument shall be automatically ignored. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_206: [ IGNORED_PTR_ARG shall be defined as NULL so that it can be used for pointer type arguments. ]*/
TEST_FUNCTION(auto_ignore_when_first_arg_is_a_nested_macro_succeeds_for_2nd_arg)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_2_args(test(test(1, 2), 4), IGNORED_NUM_ARG));

    // act
    (void)test_dependency_2_args(42, 1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}


#if !defined(_MSC_VER) || _MSC_VER >= 1600
/* Tests_SRS_UMOCK_C_LIB_01_205: [ If IGNORED_PTR_ARG or IGNORED_NUM_ARG is used as an argument value with STRICT_EXPECTED_CALL, the argument shall be automatically ignored. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_206: [ IGNORED_PTR_ARG shall be defined as NULL so that it can be used for pointer type arguments. ]*/
TEST_FUNCTION(auto_ignore_when_first_arg_is_a_struct_succeeds_for_2nd_arg)
{
#ifdef _MSC_VER
#ifdef __cplusplus
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_struct_with_2_members({ 2, 3 }, IGNORED_NUM_ARG));

    // act
    (void)test_dependency_struct_with_2_members({ 2, 3 }, 1);
#else
	// arrange
	STRICT_EXPECTED_CALL(test_dependency_struct_with_2_members((struct TEST_STRUCT_WITH_2_MEMBERS_TAG) { 2, 3 }, IGNORED_NUM_ARG));

	// act
	(void)test_dependency_struct_with_2_members((struct TEST_STRUCT_WITH_2_MEMBERS_TAG) { 2, 3 }, 1);
#endif
#else
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_struct_with_2_members((struct TEST_STRUCT_WITH_2_MEMBERS_TAG) { 2, 3 }, IGNORED_NUM_ARG));

    // act
    (void)test_dependency_struct_with_2_members((struct TEST_STRUCT_WITH_2_MEMBERS_TAG) { 2, 3 }, 1);
#endif

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}
#endif

/* Tests_SRS_UMOCK_C_LIB_01_101: [The IgnoreAllCalls call modifier shall record that all calls matching the expected call shall be ignored. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_208: [ If no matching call occurs no missing call shall be reported. ]*/
TEST_FUNCTION(IgnoreAllCalls_does_not_record_an_expected_call)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_no_args())
        .IgnoreAllCalls();

    // act

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_101: [The IgnoreAllCalls call modifier shall record that all calls matching the expected call shall be ignored. ]*/
/* Tests_SRS_UMOCK_C_LIB_01_208: [ If no matching call occurs no missing call shall be reported. ]*/
TEST_FUNCTION(IgnoreAllCalls_ignores_the_call)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_no_args())
        .IgnoreAllCalls();

    // act
    test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_102: [If multiple matching actual calls occur no unexpected calls shall be reported.]*/
TEST_FUNCTION(IgnoreAllCalls_ignores_2_calls)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_no_args())
        .IgnoreAllCalls();

    test_dependency_no_args();

    // act
    test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_102: [If multiple matching actual calls occur no unexpected calls shall be reported.]*/
/* Tests_SRS_UMOCK_C_LIB_01_103: [The call matching shall be done taking into account arguments and call modifiers referring to arguments.]*/
TEST_FUNCTION(IgnoreAllCalls_ignores_2_calls_with_matching_1_arg)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42))
        .IgnoreAllCalls();

    test_dependency_1_arg(42);

    // act
    test_dependency_1_arg(42);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_102: [If multiple matching actual calls occur no unexpected calls shall be reported.]*/
/* Tests_SRS_UMOCK_C_LIB_01_103: [The call matching shall be done taking into account arguments and call modifiers referring to arguments.]*/
TEST_FUNCTION(IgnoreAllCalls_ignores_only_calls_with_matching_args)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42))
        .IgnoreAllCalls();

    test_dependency_1_arg(42);

    // act
    test_dependency_1_arg(43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_1_arg(43)]", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_102: [If multiple matching actual calls occur no unexpected calls shall be reported.]*/
/* Tests_SRS_UMOCK_C_LIB_01_103: [The call matching shall be done taking into account arguments and call modifiers referring to arguments.]*/
TEST_FUNCTION(IgnoreAllCalls_ignores_only_calls_with_matching_args_2)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42))
        .IgnoreAllCalls();
    STRICT_EXPECTED_CALL(test_dependency_1_arg(43));

    // act
    test_dependency_1_arg(42);
    test_dependency_1_arg(43);
    test_dependency_1_arg(42);
    test_dependency_1_arg(43);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "[test_dependency_1_arg(43)]", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_31_209: [call_cannot_fail_func__{name} call modifier shall record that when performing failure case run, this call should be skipped. ] */
TEST_FUNCTION(CallCannotFail_sets_cannot_fail_for_strict_expected_call)
{
    // arrange
    int result;
    STRICT_EXPECTED_CALL(test_dependency_no_args())
        .CallCannotFail();

    // act
    result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_31_209: [call_cannot_fail_func__{name} call modifier shall record that when performing failure case run, this call should be skipped. ] */
TEST_FUNCTION(CallCannotFail_sets_cannot_fail_for_expected_call)
{
    // arrange
    int result;

    EXPECTED_CALL(test_dependency_no_args())
        .CallCannotFail();

    // act
    result = test_dependency_no_args();

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* CaptureArgumentValue_{arg_name}(arg_type* arg_value) */

/* Tests_SRS_UMOCK_C_LIB_01_210: [ If arg_value is NULL, umock_c shall raise an error with the code UMOCK_C_NULL_ARGUMENT. ]*/
TEST_FUNCTION(CaptureArgumentValue_with_NULL_arg_value_indicates_an_error)
{
    // arrange
    EXPECTED_CALL(test_dependency_1_arg_no_return(IGNORED_NUM_ARG))
        .CaptureArgumentValue_a(NULL);

    // act
    test_dependency_1_arg_no_return(43);

    // assert
    // gah, there is a TFS item apparently since forever to fix this issue: the extra ERROR notification should not happen
    ASSERT_ARE_EQUAL(size_t, 2, test_on_umock_c_error_call_count);
    ASSERT_ARE_EQUAL(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR, test_on_umock_c_error_calls[0].error_code);
    ASSERT_ARE_EQUAL(UMOCK_C_ERROR_CODE, UMOCK_C_NULL_ARGUMENT, test_on_umock_c_error_calls[1].error_code);
}

/* Tests_SRS_UMOCK_C_LIB_01_209: [ The CaptureArgumentValue_{arg_name} shall copy the value of the argument at the time of the call to arg_value. ]*/
TEST_FUNCTION(CaptureArgumentValue_captures_the_argument_value)
{
    // arrange
    int captured_arg_value = 42;

    STRICT_EXPECTED_CALL(test_dependency_1_arg_no_return(IGNORED_NUM_ARG))
        .CaptureArgumentValue_a(&captured_arg_value);

    // act
    test_dependency_1_arg_no_return(43);

    // assert
    ASSERT_ARE_EQUAL(int, 43, captured_arg_value);
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_209: [ The CaptureArgumentValue_{arg_name} shall copy the value of the argument at the time of the call to arg_value. ]*/
TEST_FUNCTION(CaptureArgumentValue_does_not_capture_when_matching_does_not_happen)
{
    // arrange
    int captured_arg_value = 42;

    STRICT_EXPECTED_CALL(test_dependency_1_arg_no_return(41))
        .CaptureArgumentValue_a(&captured_arg_value);

    // act
    test_dependency_1_arg_no_return(43);

    // assert
    ASSERT_ARE_EQUAL(int, 42, captured_arg_value);
}

/* Tests_SRS_UMOCK_C_LIB_01_209: [ The CaptureArgumentValue_{arg_name} shall copy the value of the argument at the time of the call to arg_value. ]*/
TEST_FUNCTION(CaptureArgumentValue_does_not_capture_when_matching_does_not_happen_on_another_argument)
{
    // arrange
    int captured_arg_value = 42;

    STRICT_EXPECTED_CALL(test_dependency_2_args(IGNORED_NUM_ARG, 41))
        .CaptureArgumentValue_a(&captured_arg_value);

    // act
    test_dependency_2_args(43, 42);

    // assert
    ASSERT_ARE_EQUAL(int, 42, captured_arg_value);
}

/* Tests_SRS_UMOCK_C_LIB_01_211: [ The CaptureArgumentValue_{arg_name} shall not change the how the argument is validated. ]*/
TEST_FUNCTION(CaptureArgumentValue_does_not_disable_argument_validation)
{
    // arrange
    int captured_arg_value = 42;

    EXPECTED_CALL(test_dependency_2_args(41, 41))
        .ValidateAllArguments()
        .CaptureArgumentValue_a(&captured_arg_value);

    // act
    test_dependency_2_args(43, 41);

    // assert
    ASSERT_ARE_EQUAL(int, 42, captured_arg_value);
}

/* Tests_SRS_UMOCK_C_LIB_01_211: [ The CaptureArgumentValue_{arg_name} shall not change the how the argument is validated. ]*/
TEST_FUNCTION(CaptureArgumentValue_captures_for_a_function_that_returns_something)
{
    // arrange
    int captured_arg_value = 42;
    int result;

    EXPECTED_CALL(test_dependency_1_arg(41))
        .CaptureArgumentValue_a(&captured_arg_value)
        .SetReturn(44);

    // act
    result = test_dependency_1_arg(43);

    // assert
    ASSERT_ARE_EQUAL(int, 43, captured_arg_value);
    ASSERT_ARE_EQUAL(int, 44, result);
}

/* MOCKABLE_FUNCTION_WITH_RETURNS */

/* Tests_SRS_UMOCK_C_LIB_01_212: [ MOCKABLE_FUNCTION_WITH_RETURNS shall be used to wrap function definitions, allowing the user to declare a function that can be mocked and aditionally declares the values that are to be returned in case of success and failure. ]*/
TEST_FUNCTION(MOCKABLE_FUNCTION_WITH_RETURNS_with_one_arg_and_int_return_registers_the_success_return_value)
{
    // arrange
    int result;

    STRICT_EXPECTED_CALL(test_dependency_with_returns_no_args_returning_int());

    // act
    result = test_dependency_with_returns_no_args_returning_int();

    // assert
    ASSERT_ARE_EQUAL(int, 42, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_212: [ MOCKABLE_FUNCTION_WITH_RETURNS shall be used to wrap function definitions, allowing the user to declare a function that can be mocked and aditionally declares the values that are to be returned in case of success and failure. ]*/
TEST_FUNCTION(MOCKABLE_FUNCTION_WITH_RETURNS_with_one_arg_and_void_ptr_return_registers_the_success_return_value)
{
    // arrange
    void* result;

    STRICT_EXPECTED_CALL(test_dependency_with_returns_no_args_returning_void_ptr());

    // act
    result = test_dependency_with_returns_no_args_returning_void_ptr();

    // assert
    ASSERT_ARE_EQUAL(void_ptr, (void*)0x4242, result);
}

/* Tests_SRS_UMOCK_C_LIB_01_212: [ MOCKABLE_FUNCTION_WITH_RETURNS shall be used to wrap function definitions, allowing the user to declare a function that can be mocked and aditionally declares the values that are to be returned in case of success and failure. ]*/
TEST_FUNCTION(MOCKABLE_FUNCTION_WITH_RETURNS_with_one_arg_and_struct_return_registers_the_success_return_value)
{
    // arrange
    TEST_STRUCT result;

    STRICT_EXPECTED_CALL(test_dependency_with_returns_no_args_returning_struct());

    // act
    result = test_dependency_with_returns_no_args_returning_struct();

    // assert
    ASSERT_ARE_EQUAL(int, 0x42, result.x);
}

/* Tests_SRS_UMOCK_C_LIB_01_212: [ MOCKABLE_FUNCTION_WITH_RETURNS shall be used to wrap function definitions, allowing the user to declare a function that can be mocked and aditionally declares the values that are to be returned in case of success and failure. ]*/
TEST_FUNCTION(SetReturn_overrides_MOCKABLE_FUNCTION_WITH_RETURNS)
{
    // arrange
    int result;

    STRICT_EXPECTED_CALL(test_dependency_with_returns_no_args_returning_int())
        .SetReturn(44);

    // act
    result = test_dependency_with_returns_no_args_returning_int();

    // assert
    ASSERT_ARE_EQUAL(int, 44, result);
}

#define WRAPPER_MACRO(a) a

/* Tests_SRS_UMOCK_C_LIB_01_205: [ If `IGNORED_PTR_ARG` or `IGNORED_NUM_ARG` is used as an argument value with `STRICT_EXPECTED_CALL`, the argument shall be automatically ignored. ]*/
TEST_FUNCTION(IGNORED_PTR_ARG_works_with_another_macro_wrapping_function_name)
{
    // arrange
    unsigned char x[1] = { 42 };

    STRICT_EXPECTED_CALL(WRAPPER_MACRO(test_dependency_buffer_arg)(IGNORED_PTR_ARG));

    // act
    WRAPPER_MACRO(test_dependency_buffer_arg)(x);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

/* Tests_SRS_UMOCK_C_LIB_01_205: [ If `IGNORED_PTR_ARG` or `IGNORED_NUM_ARG` is used as an argument value with `STRICT_EXPECTED_CALL`, the argument shall be automatically ignored. ]*/
TEST_FUNCTION(IGNORED_NUM_ARG_works_with_another_macro_wrapping_function_name)
{
    // arrange
    int x = 42;

    STRICT_EXPECTED_CALL(WRAPPER_MACRO(test_dependency_1_arg_no_return)(IGNORED_NUM_ARG));

    // act
    WRAPPER_MACRO(test_dependency_1_arg_no_return)(x);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_expected_calls());
    ASSERT_ARE_EQUAL(char_ptr, "", umock_c_get_actual_calls());
}

END_TEST_SUITE(umock_c_integrationtests)
