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

#include "azure_uamqp_c/sasl_mechanism.h"

static const CONCRETE_SASL_MECHANISM_HANDLE test_concrete_sasl_mechanism_handle = (CONCRETE_SASL_MECHANISM_HANDLE)0x4242;
static const char* test_mechanism_name = "TestMechName";

/* sasl mechanism concrete implementation mocks */
MOCK_FUNCTION_WITH_CODE(, CONCRETE_SASL_MECHANISM_HANDLE, test_saslmechanism_create, void*, config)
MOCK_FUNCTION_END(test_concrete_sasl_mechanism_handle);
MOCK_FUNCTION_WITH_CODE(, void, test_saslmechanism_destroy, CONCRETE_SASL_MECHANISM_HANDLE, concrete_sasl_mechanism)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, int, test_saslmechanism_get_init_bytes, CONCRETE_SASL_MECHANISM_HANDLE, concrete_sasl_mechanism, SASL_MECHANISM_BYTES*, init_bytes);
MOCK_FUNCTION_END(0);
MOCK_FUNCTION_WITH_CODE(, const char*, test_saslmechanism_get_mechanism_name, CONCRETE_SASL_MECHANISM_HANDLE, concrete_sasl_mechanism);
MOCK_FUNCTION_END(test_mechanism_name);
MOCK_FUNCTION_WITH_CODE(, int, test_saslmechanism_challenge, CONCRETE_SASL_MECHANISM_HANDLE, concrete_sasl_mechanism, const SASL_MECHANISM_BYTES*, challenge_bytes, SASL_MECHANISM_BYTES*, response_bytes)
MOCK_FUNCTION_END(0);

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

static const SASL_MECHANISM_INTERFACE_DESCRIPTION test_io_description =
{
    test_saslmechanism_create,
    test_saslmechanism_destroy,
    test_saslmechanism_get_init_bytes,
    test_saslmechanism_get_mechanism_name,
    test_saslmechanism_challenge
};

BEGIN_TEST_SUITE(sasl_mechanism_ut)

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

    REGISTER_UMOCK_ALIAS_TYPE(CONCRETE_SASL_MECHANISM_HANDLE, void*);
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

/* saslmechanism_create */

/* Tests_SRS_SASL_MECHANISM_01_001: [`saslmechanism_create` shall return on success a non-NULL handle to a new SASL mechanism interface.] */
/* Tests_SRS_SASL_MECHANISM_01_002: [In order to instantiate the concrete SASL mechanism implementation the function `concrete_sasl_mechanism_create` from the `sasl_mechanism_interface_description` shall be called, passing the `sasl_mechanism_create_parameters` to it.] */
TEST_FUNCTION(saslmechanism_create_with_all_args_except_interface_description_NULL_succeeds)
{
    // arrange
    SASL_MECHANISM_HANDLE result;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_saslmechanism_create(NULL));

    // act
    result = saslmechanism_create(&test_io_description, NULL);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslmechanism_destroy(result);
}

/* Tests_SRS_SASL_MECHANISM_01_001: [`saslmechanism_create` shall return on success a non-NULL handle to a new SASL mechanism interface.] */
/* Tests_SRS_SASL_MECHANISM_01_002: [In order to instantiate the concrete SASL mechanism implementation the function `concrete_sasl_mechanism_create` from the `sasl_mechanism_interface_description` shall be called, passing the `sasl_mechanism_create_parameters` to it.] */
TEST_FUNCTION(the_config_argument_is_passed_to_the_concrete_saslmechanism_create)
{
    // arrange
    SASL_MECHANISM_HANDLE result;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_saslmechanism_create((void*)0x4242));

    // act
    result = saslmechanism_create(&test_io_description, (void*)0x4242);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslmechanism_destroy(result);
}

/* Tests_SRS_SASL_MECHANISM_01_003: [If the underlying `concrete_sasl_mechanism_create` call fails, `saslmechanism_create` shall return NULL.] */
TEST_FUNCTION(when_concrete_create_fails_then_saslmechanism_create_fails)
{
    // arrange
    SASL_MECHANISM_HANDLE result;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_saslmechanism_create(NULL))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = saslmechanism_create(&test_io_description, NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_MECHANISM_01_004: [If the argument `sasl_mechanism_interface_description` is NULL, `saslmechanism_create` shall return NULL.] */
TEST_FUNCTION(when_the_interface_description_is_NULL_then_saslmechanism_create_fails)
{
    // arrange

    // act
    SASL_MECHANISM_HANDLE result = saslmechanism_create(NULL, NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_MECHANISM_01_005: [If any `sasl_mechanism_interface_description` member is NULL, `saslmechanism_create` shall fail and return NULL.] */
TEST_FUNCTION(when_the_concrete_create_is_NULL_then_saslmechanism_create_fails)
{
    // arrange
    const SASL_MECHANISM_INTERFACE_DESCRIPTION io_description_with_NULL_entry =
    {
        NULL,
        test_saslmechanism_destroy,
        test_saslmechanism_get_init_bytes,
        test_saslmechanism_get_mechanism_name
    };

    // act
    SASL_MECHANISM_HANDLE result = saslmechanism_create(&io_description_with_NULL_entry, NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_MECHANISM_01_005: [If any `sasl_mechanism_interface_description` member is NULL, `saslmechanism_create` shall fail and return NULL.] */
TEST_FUNCTION(when_the_concrete_destroy_is_NULL_then_saslmechanism_create_fails)
{
    // arrange
    const SASL_MECHANISM_INTERFACE_DESCRIPTION io_description_with_NULL_entry =
    {
        test_saslmechanism_create,
        NULL,
        test_saslmechanism_get_init_bytes,
        test_saslmechanism_get_mechanism_name
    };

    // act
    SASL_MECHANISM_HANDLE result = saslmechanism_create(&io_description_with_NULL_entry, NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_MECHANISM_01_005: [If any `sasl_mechanism_interface_description` member is NULL, `saslmechanism_create` shall fail and return NULL.] */
TEST_FUNCTION(when_the_concrete_get_init_bytes_is_NULL_then_saslmechanism_create_fails)
{
    // arrange
    const SASL_MECHANISM_INTERFACE_DESCRIPTION io_description_with_NULL_entry =
    {
        test_saslmechanism_create,
        test_saslmechanism_destroy,
        NULL,
        test_saslmechanism_get_mechanism_name
    };

    // act
    SASL_MECHANISM_HANDLE result = saslmechanism_create(&io_description_with_NULL_entry, NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_MECHANISM_01_005: [If any `sasl_mechanism_interface_description` member is NULL, `saslmechanism_create` shall fail and return NULL.] */
TEST_FUNCTION(when_the_concrete_get_mechanism_name_is_NULL_then_saslmechanism_create_fails)
{
    // arrange
    const SASL_MECHANISM_INTERFACE_DESCRIPTION io_description_with_NULL_entry =
    {
        test_saslmechanism_create,
        test_saslmechanism_destroy,
        test_saslmechanism_get_init_bytes,
        NULL
    };

    // act
    SASL_MECHANISM_HANDLE result = saslmechanism_create(&io_description_with_NULL_entry, NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_MECHANISM_01_006: [If allocating the memory needed for the SASL mechanism interface fails then `saslmechanism_create` shall fail and return NULL.] */
TEST_FUNCTION(when_allocating_memory_fails_then_saslmechanism_create_fails)
{
    // arrange
    SASL_MECHANISM_HANDLE result;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    result = saslmechanism_create(&test_io_description, NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* saslmechanism_destroy */

/* Tests_SRS_SASL_MECHANISM_01_007: [`saslmechanism_destroy` shall free all resources associated with the SASL mechanism handle.] */
/* Tests_SRS_SASL_MECHANISM_01_008: [`saslmechanism_destroy` shall also call the `concrete_sasl_mechanism_destroy` function that is member of the `sasl_mechanism_interface_description` argument passed to `saslmechanism_create`, while passing as argument to `concrete_sasl_mechanism_destroy` the result of the underlying concrete SASL mechanism handle.] */
TEST_FUNCTION(saslmechanism_destroy_frees_memory_and_calls_the_underlying_concrete_destroy)
{
    // arrange
    SASL_MECHANISM_HANDLE sasl_mechanism = saslmechanism_create(&test_io_description, (void*)0x4242);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_saslmechanism_destroy(test_concrete_sasl_mechanism_handle));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    saslmechanism_destroy(sasl_mechanism);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_MECHANISM_01_009: [If the argument `sasl_mechanism` is NULL, `saslmechanism_destroy` shall do nothing.] */
TEST_FUNCTION(saslmechanism_destroy_with_NULL_argument_does_nothing)
{
    // arrange

    // act
    saslmechanism_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* saslmechanism_get_init_bytes */

/* Tests_SRS_SASL_MECHANISM_01_010: [`saslmechanism_get_init_bytes` shall call the specific `concrete_sasl_mechanism_get_init_bytes` function specified in `saslmechanism_create`, passing the `init_bytes` argument to it.] */
/* Tests_SRS_SASL_MECHANISM_01_011: [On success, `saslmechanism_get_init_bytes` shall return 0.] */
TEST_FUNCTION(saslmechanism_get_init_bytes_calls_the_underlying_concrete_sasl_mechanism)
{
    // arrange
    SASL_MECHANISM_HANDLE sasl_mechanism = saslmechanism_create(&test_io_description, (void*)0x4242);
    SASL_MECHANISM_BYTES init_bytes;
    SASL_MECHANISM_BYTES expected_init_bytes = { (void*)0x4242, 42 };
    int result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_saslmechanism_get_init_bytes(test_concrete_sasl_mechanism_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &expected_init_bytes, sizeof(expected_init_bytes));

    // act
    result = saslmechanism_get_init_bytes(sasl_mechanism, &init_bytes);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)0x4242, init_bytes.bytes);
    ASSERT_ARE_EQUAL(size_t, 42, init_bytes.length);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslmechanism_destroy(sasl_mechanism);
}

/* Tests_SRS_SASL_MECHANISM_01_012: [If the argument `sasl_mechanism` is NULL, `saslmechanism_get_init_bytes` shall fail and return a non-zero value.] */
TEST_FUNCTION(saslmechanism_get_init_bytes_with_NULL_handle_fails)
{
    // arrange
    SASL_MECHANISM_BYTES init_bytes;

    // act
    int result = saslmechanism_get_init_bytes(NULL, &init_bytes);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_MECHANISM_01_013: [If the underlying `concrete_sasl_mechanism_get_init_bytes` fails, `saslmechanism_get_init_bytes` shall fail and return a non-zero value.] */
TEST_FUNCTION(when_the_underlying_get_init_bytes_fails_then_saslmechanism_get_init_bytes_fails)
{
    // arrange
    SASL_MECHANISM_HANDLE sasl_mechanism = saslmechanism_create(&test_io_description, (void*)0x4242);
    SASL_MECHANISM_BYTES init_bytes;
    SASL_MECHANISM_BYTES expected_init_bytes = { (void*)0x4242, 42 };
    int result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_saslmechanism_get_init_bytes(test_concrete_sasl_mechanism_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &expected_init_bytes, sizeof(expected_init_bytes))
        .SetReturn(1);

    // act
    result = saslmechanism_get_init_bytes(sasl_mechanism, &init_bytes);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslmechanism_destroy(sasl_mechanism);
}

/* saslmechanism_get_mechanism_name */

/* Tests_SRS_SASL_MECHANISM_01_014: [`saslmechanism_get_mechanism_name` shall call the specific `concrete_sasl_mechanism_get_mechanism_name` function specified in `saslmechanism_create`.] */
/* Tests_SRS_SASL_MECHANISM_01_015: [On success, `saslmechanism_get_mechanism_name` shall return a pointer to a string with the mechanism name.] */
TEST_FUNCTION(saslmechanism_get_mechanism_name_calls_the_underlying_get_mechanism_name_and_succeeds)
{
    // arrange
    SASL_MECHANISM_HANDLE sasl_mechanism = saslmechanism_create(&test_io_description, (void*)0x4242);
    const char* result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_saslmechanism_get_mechanism_name(test_concrete_sasl_mechanism_handle));

    // act
    result = saslmechanism_get_mechanism_name(sasl_mechanism);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, test_mechanism_name, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslmechanism_destroy(sasl_mechanism);
}

/* Tests_SRS_SASL_MECHANISM_01_014: [`saslmechanism_get_mechanism_name` shall call the specific `concrete_sasl_mechanism_get_mechanism_name` function specified in `saslmechanism_create`.] */
/* Tests_SRS_SASL_MECHANISM_01_015: [On success, `saslmechanism_get_mechanism_name` shall return a pointer to a string with the mechanism name.] */
TEST_FUNCTION(saslmechanism_get_mechanism_name_calls_the_underlying_get_mechanism_name_and_succeeds_another_mechanism_name)
{
    // arrange
    SASL_MECHANISM_HANDLE sasl_mechanism = saslmechanism_create(&test_io_description, (void*)0x4242);
    const char* result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_saslmechanism_get_mechanism_name(test_concrete_sasl_mechanism_handle))
        .SetReturn("boo");

    // act
    result = saslmechanism_get_mechanism_name(sasl_mechanism);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "boo", result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslmechanism_destroy(sasl_mechanism);
}

/* Tests_SRS_SASL_MECHANISM_01_016: [If the argument `sasl_mechanism` is NULL, `saslmechanism_get_mechanism_name` shall fail and return a non-zero value.] */
TEST_FUNCTION(saslmechanism_get_mechanism_name_with_NULL_handle_fails)
{
    // arrange

    // act
    const char* result = saslmechanism_get_mechanism_name(NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_MECHANISM_01_017: [If the underlying `concrete_sasl_mechanism_get_mechanism_name` fails, `saslmechanism_get_mechanism_name` shall return NULL.] */
TEST_FUNCTION(when_the_underlying_mechanism_returns_NULL_saslmechanism_get_mechanism_name_fails)
{
    // arrange
    SASL_MECHANISM_HANDLE sasl_mechanism = saslmechanism_create(&test_io_description, (void*)0x4242);
    const char* result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_saslmechanism_get_mechanism_name(test_concrete_sasl_mechanism_handle))
        .SetReturn((const char*)NULL);

    // act
    result = saslmechanism_get_mechanism_name(sasl_mechanism);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslmechanism_destroy(sasl_mechanism);
}

/* saslmechanism_challenge */

/* Tests_SRS_SASL_MECHANISM_01_018: [`saslmechanism_challenge` shall call the specific `concrete_sasl_mechanism_challenge` function specified in `saslmechanism_create`, while passing the `challenge_bytes` and `response_bytes` arguments to it.] */
/* Tests_SRS_SASL_MECHANISM_01_019: [On success, `saslmechanism_challenge` shall return 0.] */
TEST_FUNCTION(saslmechanism_challenge_calls_the_concrete_implementation_and_passes_the_proper_arguments)
{
    // arrange
    SASL_MECHANISM_HANDLE sasl_mechanism = saslmechanism_create(&test_io_description, (void*)0x4242);
    SASL_MECHANISM_BYTES challenge_bytes = { NULL, 0 };
    SASL_MECHANISM_BYTES response_bytes = { NULL, 0 };
    int result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_saslmechanism_challenge(test_concrete_sasl_mechanism_handle, &challenge_bytes, &response_bytes));

    // act
    result = saslmechanism_challenge(sasl_mechanism, &challenge_bytes, &response_bytes);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslmechanism_destroy(sasl_mechanism);
}

/* Tests_SRS_SASL_MECHANISM_01_020: [If the argument `sasl_mechanism` is NULL, `saslmechanism_challenge` shall fail and return a non-zero value.] */
TEST_FUNCTION(saslmechanism_challenge_with_NULL_sasl_mechanism_fails)
{
    // arrange
    SASL_MECHANISM_BYTES challenge_bytes = { NULL, 0 };
    SASL_MECHANISM_BYTES response_bytes = { NULL, 0 };

    // act
    int result = saslmechanism_challenge(NULL, &challenge_bytes, &response_bytes);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_MECHANISM_01_021: [If the underlying `concrete_sasl_mechanism_challenge` fails, `saslmechanism_challenge` shall fail and return a non-zero value.] */
TEST_FUNCTION(when_the_underlying_concrete_challenge_fails_then_saslmechanism_challenge_fails)
{
    // arrange
    SASL_MECHANISM_HANDLE sasl_mechanism = saslmechanism_create(&test_io_description, (void*)0x4242);
    SASL_MECHANISM_BYTES challenge_bytes = { NULL, 0 };
    SASL_MECHANISM_BYTES response_bytes = { NULL, 0 };
    int result;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_saslmechanism_challenge(test_concrete_sasl_mechanism_handle, &challenge_bytes, &response_bytes))
        .SetReturn(1);

    // act
    result = saslmechanism_challenge(sasl_mechanism, &challenge_bytes, &response_bytes);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslmechanism_destroy(sasl_mechanism);
}

END_TEST_SUITE(sasl_mechanism_ut)
