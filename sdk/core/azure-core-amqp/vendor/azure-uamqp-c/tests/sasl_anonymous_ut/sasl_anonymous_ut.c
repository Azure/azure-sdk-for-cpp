// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdint>
#include <cstddef>
#else
#include <stdint.h>
#include <stddef.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"

#if defined _MSC_VER
#pragma warning(disable: 4054) /* MSC incorrectly fires this */
#endif

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#define ENABLE_MOCKS

#include "azure_c_shared_utility/gballoc.h"

#undef ENABLE_MOCKS

#include "azure_uamqp_c/sasl_anonymous.h"

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(sasl_anonymous_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* saslanonymous_create */

/* Tests_SRS_SASL_ANONYMOUS_01_001: [`saslanonymous_create` shall return on success a non-NULL handle to a new SASL anonymous mechanism.]*/
TEST_FUNCTION(saslanonymous_create_with_valid_args_succeeds)
{
    // arrange
    CONCRETE_SASL_MECHANISM_HANDLE result;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    // act
    result = saslanonymous_get_interface()->concrete_sasl_mechanism_create((void*)0x4242);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslanonymous_get_interface()->concrete_sasl_mechanism_destroy(result);
}

/* Tests_SRS_SASL_ANONYMOUS_01_002: [If allocating the memory needed for the SASL anonymous instance fails then `saslanonymous_create` shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_fails_then_saslanonymous_create_fails)
{
    // arrange
    CONCRETE_SASL_MECHANISM_HANDLE result;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    result = saslanonymous_get_interface()->concrete_sasl_mechanism_create((void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_SASL_ANONYMOUS_01_003: [Since this is the ANONYMOUS SASL mechanism, `config` shall be ignored.]*/
TEST_FUNCTION(saslanonymous_create_with_NULL_config_succeeds)
{
    // arrange
    CONCRETE_SASL_MECHANISM_HANDLE result;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    // act
    result = saslanonymous_get_interface()->concrete_sasl_mechanism_create(NULL);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslanonymous_get_interface()->concrete_sasl_mechanism_destroy(result);
}

/* saslanonymous_destroy */

/* Tests_SRS_SASL_ANONYMOUS_01_004: [`saslanonymous_destroy` shall free all resources associated with the SASL mechanism.] */
TEST_FUNCTION(saslanonymous_destroy_frees_the_allocated_resources)
{
    // arrange
    CONCRETE_SASL_MECHANISM_HANDLE result = saslanonymous_get_interface()->concrete_sasl_mechanism_create(NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    saslanonymous_get_interface()->concrete_sasl_mechanism_destroy(result);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_ANONYMOUS_01_005: [If the argument `concrete_sasl_mechanism` is NULL, `saslanonymous_destroy` shall do nothing.]*/
TEST_FUNCTION(saslanonymous_destroy_with_NULL_argument_does_nothing)
{
    // arrange

    // act
    saslanonymous_get_interface()->concrete_sasl_mechanism_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* saslanonymous_get_init_bytes */

/* Tests_SRS_SASL_ANONYMOUS_01_006: [`saslanonymous_get_init_bytes` shall validate the `concrete_sasl_mechanism` argument and set the length of the `init_bytes` argument to be zero.] */
/* Tests_SRS_SASL_ANONYMOUS_01_012: [The bytes field of `init_buffer` shall be set to NULL.] */
/* Tests_SRS_SASL_ANONYMOUS_01_011: [On success `saslanonymous_get_init_bytes` shall return zero.] */
TEST_FUNCTION(saslannymous_get_init_bytes_sets_the_bytes_to_NULL_and_length_to_zero)
{
    // arrange
    CONCRETE_SASL_MECHANISM_HANDLE saslanonymous = saslanonymous_get_interface()->concrete_sasl_mechanism_create(NULL);
    SASL_MECHANISM_BYTES init_bytes;
    int result;
    umock_c_reset_all_calls();

    // act
    result = saslanonymous_get_interface()->concrete_sasl_mechanism_get_init_bytes(saslanonymous, &init_bytes);

    // assert
    ASSERT_IS_NULL(init_bytes.bytes);
    ASSERT_ARE_EQUAL(size_t, 0, init_bytes.length);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslanonymous_get_interface()->concrete_sasl_mechanism_destroy(saslanonymous);
}

/* Tests_SRS_SASL_ANONYMOUS_01_007: [If any argument is NULL, `saslanonymous_get_init_bytes` shall return a non-zero value.]*/
TEST_FUNCTION(saslannymous_get_init_bytes_with_NULL_concrete_sasl_mechanism_fails)
{
    // arrange
    SASL_MECHANISM_BYTES init_bytes;
    int result;
    umock_c_reset_all_calls();

    // act
    result = saslanonymous_get_interface()->concrete_sasl_mechanism_get_init_bytes(NULL, &init_bytes);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_SASL_ANONYMOUS_01_007: [If any argument is NULL, `saslanonymous_get_init_bytes` shall return a non-zero value.]*/
TEST_FUNCTION(saslannymous_get_init_bytes_with_NULL_init_bytes_fails)
{
    // arrange
    CONCRETE_SASL_MECHANISM_HANDLE saslanonymous = saslanonymous_get_interface()->concrete_sasl_mechanism_create(NULL);
    int result;
    umock_c_reset_all_calls();

    // act
    result = saslanonymous_get_interface()->concrete_sasl_mechanism_get_init_bytes(saslanonymous, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslanonymous_get_interface()->concrete_sasl_mechanism_destroy(saslanonymous);
}

/* saslanonymous_get_mechanism_name */

/* Tests_SRS_SASL_ANONYMOUS_01_008: [`saslanonymous_get_mechanism_name` shall validate the argument `concrete_sasl_mechanism` and on success it shall return a pointer to the string `ANONYMOUS`.] */
TEST_FUNCTION(saslanonymous_get_mechanism_name_with_non_NULL_concrete_sasl_mechanism_succeeds)
{
    // arrange
    CONCRETE_SASL_MECHANISM_HANDLE saslanonymous = saslanonymous_get_interface()->concrete_sasl_mechanism_create(NULL);
    const char* result;
    umock_c_reset_all_calls();

    // act
    result = saslanonymous_get_interface()->concrete_sasl_mechanism_get_mechanism_name(saslanonymous);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "ANONYMOUS", result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslanonymous_get_interface()->concrete_sasl_mechanism_destroy(saslanonymous);
}

/* Tests_SRS_SASL_ANONYMOUS_01_009: [If the argument `concrete_sasl_mechanism` is NULL, `saslanonymous_get_mechanism_name` shall return NULL.] */
TEST_FUNCTION(saslanonymous_get_mechanism_name_with_NULL_concrete_sasl_mechanism_fails)
{
    // arrange

    // act
    const char* result = saslanonymous_get_interface()->concrete_sasl_mechanism_get_mechanism_name(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* saslanonymous_challenge */

/* Tests_SRS_SASL_ANONYMOUS_01_013: [`saslanonymous_challenge` shall set the `buffer` field to NULL and `size` to 0 in the `response_bytes` argument as the ANONYMOUS SASL mechanism does not implement challenge/response.] */
/* Tests_SRS_SASL_ANONYMOUS_01_014: [On success, `saslanonymous_challenge` shall return 0.] */
TEST_FUNCTION(saslanonymous_challenge_returns_a_NULL_response_bytes_buffer)
{
    // arrange
    CONCRETE_SASL_MECHANISM_HANDLE saslanonymous = saslanonymous_get_interface()->concrete_sasl_mechanism_create(NULL);
    SASL_MECHANISM_BYTES challenge_bytes;
    SASL_MECHANISM_BYTES response_bytes;
    int result;
    umock_c_reset_all_calls();

    // act
    result = saslanonymous_get_interface()->concrete_sasl_mechanism_challenge(saslanonymous, &challenge_bytes, &response_bytes);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_IS_NULL(response_bytes.bytes);
    ASSERT_ARE_EQUAL(size_t, 0, response_bytes.length);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslanonymous_get_interface()->concrete_sasl_mechanism_destroy(saslanonymous);
}

/* Tests_SRS_SASL_ANONYMOUS_01_014: [On success, `saslanonymous_challenge` shall return 0.] */
TEST_FUNCTION(saslanonymous_with_NULL_challenge_bytes_returns_a_NULL_response_bytes_buffer)
{
    // arrange
    CONCRETE_SASL_MECHANISM_HANDLE saslanonymous = saslanonymous_get_interface()->concrete_sasl_mechanism_create(NULL);
    SASL_MECHANISM_BYTES response_bytes;
    int result;
    umock_c_reset_all_calls();

    // act
    result = saslanonymous_get_interface()->concrete_sasl_mechanism_challenge(saslanonymous, NULL, &response_bytes);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_IS_NULL(response_bytes.bytes);
    ASSERT_ARE_EQUAL(size_t, 0, response_bytes.length);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslanonymous_get_interface()->concrete_sasl_mechanism_destroy(saslanonymous);
}

/* Tests_SRS_SASL_ANONYMOUS_01_015: [If the `concrete_sasl_mechanism` or `response_bytes` argument is NULL then `saslanonymous_challenge` shall fail and return a non-zero value.] */
TEST_FUNCTION(saslanonymous_challenge_with_NULL_handle_fails)
{
    // arrange
    SASL_MECHANISM_BYTES challenge_bytes;
    SASL_MECHANISM_BYTES response_bytes;
    int result;

    // act
    result = saslanonymous_get_interface()->concrete_sasl_mechanism_challenge(NULL, &challenge_bytes, &response_bytes);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_SASL_ANONYMOUS_01_015: [If the `concrete_sasl_mechanism` or `response_bytes` argument is NULL then `saslanonymous_challenge` shall fail and return a non-zero value.] */
TEST_FUNCTION(saslanonymous_challenge_with_NULL_response_bytes_fails)
{
    // arrange
    CONCRETE_SASL_MECHANISM_HANDLE saslanonymous = saslanonymous_get_interface()->concrete_sasl_mechanism_create(NULL);
    SASL_MECHANISM_BYTES challenge_bytes;
    int result;
    umock_c_reset_all_calls();

    // act
    result = saslanonymous_get_interface()->concrete_sasl_mechanism_challenge(saslanonymous, &challenge_bytes, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslanonymous_get_interface()->concrete_sasl_mechanism_destroy(saslanonymous);
}

/* saslanonymous_get_interface */

/* Tests_SRS_SASL_ANONYMOUS_01_010: [`saslanonymous_get_interface` shall return a pointer to a `SASL_MECHANISM_INTERFACE_DESCRIPTION` structure that contains pointers to the functions: `saslanonymous_create`, `saslanonymous_destroy`, `saslanonymous_get_init_bytes`, `saslanonymous_get_mechanism_name`, `saslanonymous_challenge`.] */
TEST_FUNCTION(saslanonymous_get_interface_returns_the_sasl_anonymous_mechanism_interface)
{
    // arrange

    // act
    const SASL_MECHANISM_INTERFACE_DESCRIPTION* result = saslanonymous_get_interface();

    // assert
    ASSERT_IS_NOT_NULL(result->concrete_sasl_mechanism_create);
    ASSERT_IS_NOT_NULL(result->concrete_sasl_mechanism_destroy);
    ASSERT_IS_NOT_NULL(result->concrete_sasl_mechanism_get_init_bytes);
    ASSERT_IS_NOT_NULL(result->concrete_sasl_mechanism_get_mechanism_name);
    ASSERT_IS_NOT_NULL(result->concrete_sasl_mechanism_challenge);
}

END_TEST_SUITE(sasl_anonymous_ut)
