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

#include "azure_uamqp_c/sasl_server_mechanism.h"

static const CONCRETE_SASL_SERVER_MECHANISM_HANDLE test_concrete_sasl_server_mechanism_handle = (CONCRETE_SASL_SERVER_MECHANISM_HANDLE)0x4242;
static const char* test_mechanism_name = "test_mechanism_name";

/* sasl mechanism concrete implementation mocks */
MOCK_FUNCTION_WITH_CODE(, CONCRETE_SASL_SERVER_MECHANISM_HANDLE, test_sasl_server_mechanism_create, void*, create_parameters)
MOCK_FUNCTION_END(test_concrete_sasl_server_mechanism_handle);
MOCK_FUNCTION_WITH_CODE(, void, test_sasl_server_mechanism_destroy, CONCRETE_SASL_SERVER_MECHANISM_HANDLE, concrete_sasl_server_mechanism)
MOCK_FUNCTION_END();
MOCK_FUNCTION_WITH_CODE(, int, test_sasl_server_mechanism_handle_initial_response, CONCRETE_SASL_SERVER_MECHANISM_HANDLE, concrete_sasl_server_mechanism, const SASL_SERVER_MECHANISM_BYTES*, initial_response_bytes, const char*, hostname, bool*, send_challenge, SASL_SERVER_MECHANISM_BYTES*, challenge_bytes)
MOCK_FUNCTION_END(0);
MOCK_FUNCTION_WITH_CODE(, int, test_sasl_server_mechanism_handle_response, CONCRETE_SASL_SERVER_MECHANISM_HANDLE, concrete_sasl_server_mechanism, const SASL_SERVER_MECHANISM_BYTES*, response_bytes, bool*, send_next_challenge, SASL_SERVER_MECHANISM_BYTES*, next_challenge_bytes)
MOCK_FUNCTION_END(0);
MOCK_FUNCTION_WITH_CODE(, const char*, test_sasl_server_mechanism_get_mechanism_name)
MOCK_FUNCTION_END(test_mechanism_name);

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

static const SASL_SERVER_MECHANISM_INTERFACE_DESCRIPTION test_sasl_server_mechanism_interface_description =
{
    test_sasl_server_mechanism_create,
    test_sasl_server_mechanism_destroy,
    test_sasl_server_mechanism_handle_initial_response,
    test_sasl_server_mechanism_handle_response,
    test_sasl_server_mechanism_get_mechanism_name
};

BEGIN_TEST_SUITE(sasl_server_mechanism_ut)

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

    REGISTER_UMOCK_ALIAS_TYPE(CONCRETE_SASL_SERVER_MECHANISM_HANDLE, void*);
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

/* sasl_server_mechanism_create */

/* Tests_SRS_SASL_SERVER_MECHANISM_01_001: [`sasl_server_mechanism_create` shall return on success a non-NULL handle to a new SASL server mechanism interface.]*/
/* Tests_SRS_SASL_SERVER_MECHANISM_01_002: [ In order to instantiate the concrete SASL server mechanism implementation the function `create` from the `sasl_server_mechanism_interface_description` shall be called, passing the `sasl_server_mechanism_create_parameters` to it.]*/
TEST_FUNCTION(sasl_server_mechanism_create_with_non_NULL_create_parameters_succeeds)
{
    // arrange
    SASL_SERVER_MECHANISM_HANDLE result;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_sasl_server_mechanism_create((void*)0x4242));

    // act
    result = sasl_server_mechanism_create(&test_sasl_server_mechanism_interface_description, (void*)0x4242);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_server_mechanism_destroy(result);
}

/* Tests_SRS_SASL_SERVER_MECHANISM_01_001: [`sasl_server_mechanism_create` shall return on success a non-NULL handle to a new SASL server mechanism interface.]*/
/* Tests_SRS_SASL_SERVER_MECHANISM_01_002: [ In order to instantiate the concrete SASL server mechanism implementation the function `create` from the `sasl_server_mechanism_interface_description` shall be called, passing the `sasl_server_mechanism_create_parameters` to it.]*/
TEST_FUNCTION(sasl_server_mechanism_create_with_NULL_create_parameters_succeeds)
{
    // arrange
    SASL_SERVER_MECHANISM_HANDLE result;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_sasl_server_mechanism_create(NULL));

    // act
    result = sasl_server_mechanism_create(&test_sasl_server_mechanism_interface_description, NULL);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    sasl_server_mechanism_destroy(result);
}

/* Tests_SRS_SASL_SERVER_MECHANISM_01_003: [ If the underlying `create` call fails, `sasl_server_mechanism_create` shall return NULL. ]*/
TEST_FUNCTION(sasl_server_mechanism_create_succeeds)
{
    // arrange
    SASL_SERVER_MECHANISM_HANDLE result;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_sasl_server_mechanism_create((void*)0x4242))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = sasl_server_mechanism_create(&test_sasl_server_mechanism_interface_description, (void*)0x4242);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_SERVER_MECHANISM_01_004: [ If the argument `sasl_server_mechanism_interface_description` is NULL, `sasl_server_mechanism_create` shall return NULL.]*/
TEST_FUNCTION(sasl_server_mechanism_create_with_NULL_interface_description_fails)
{
    // arrange
    SASL_SERVER_MECHANISM_HANDLE result;

    // act
    result = sasl_server_mechanism_create(NULL, (void*)0x4242);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_SERVER_MECHANISM_01_005: [ If any `sasl_server_mechanism_interface_description` member is NULL, `sasl_server_mechanism_create` shall fail and return NULL.]*/
TEST_FUNCTION(sasl_server_mechanism_create_with_NULL_create_fails)
{
    // arrange
    SASL_SERVER_MECHANISM_HANDLE result;
    const SASL_SERVER_MECHANISM_INTERFACE_DESCRIPTION test_sasl_server_mechanism_interface_description_NULL =
    {
        NULL,
        test_sasl_server_mechanism_destroy,
        test_sasl_server_mechanism_handle_initial_response,
        test_sasl_server_mechanism_handle_response,
        test_sasl_server_mechanism_get_mechanism_name
    };

    // act
    result = sasl_server_mechanism_create(&test_sasl_server_mechanism_interface_description_NULL, (void*)0x4242);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_SERVER_MECHANISM_01_005: [ If any `sasl_server_mechanism_interface_description` member is NULL, `sasl_server_mechanism_create` shall fail and return NULL.]*/
TEST_FUNCTION(sasl_server_mechanism_create_with_NULL_destroy_fails)
{
    // arrange
    SASL_SERVER_MECHANISM_HANDLE result;
    const SASL_SERVER_MECHANISM_INTERFACE_DESCRIPTION test_sasl_server_mechanism_interface_description_NULL =
    {
        test_sasl_server_mechanism_create,
        NULL,
        test_sasl_server_mechanism_handle_initial_response,
        test_sasl_server_mechanism_handle_response,
        test_sasl_server_mechanism_get_mechanism_name
    };

    // act
    result = sasl_server_mechanism_create(&test_sasl_server_mechanism_interface_description_NULL, (void*)0x4242);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_SERVER_MECHANISM_01_005: [ If any `sasl_server_mechanism_interface_description` member is NULL, `sasl_server_mechanism_create` shall fail and return NULL.]*/
TEST_FUNCTION(sasl_server_mechanism_create_with_NULL_handle_initial_response_fails)
{
    // arrange
    SASL_SERVER_MECHANISM_HANDLE result;
    const SASL_SERVER_MECHANISM_INTERFACE_DESCRIPTION test_sasl_server_mechanism_interface_description_NULL =
    {
        test_sasl_server_mechanism_create,
        test_sasl_server_mechanism_destroy,
        NULL,
        test_sasl_server_mechanism_handle_response,
        test_sasl_server_mechanism_get_mechanism_name
    };

    // act
    result = sasl_server_mechanism_create(&test_sasl_server_mechanism_interface_description_NULL, (void*)0x4242);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_SERVER_MECHANISM_01_005: [ If any `sasl_server_mechanism_interface_description` member is NULL, `sasl_server_mechanism_create` shall fail and return NULL.]*/
TEST_FUNCTION(sasl_server_mechanism_create_with_NULL_handle_response_fails)
{
    // arrange
    SASL_SERVER_MECHANISM_HANDLE result;
    const SASL_SERVER_MECHANISM_INTERFACE_DESCRIPTION test_sasl_server_mechanism_interface_description_NULL =
    {
        test_sasl_server_mechanism_create,
        test_sasl_server_mechanism_destroy,
        test_sasl_server_mechanism_handle_initial_response,
        NULL,
        test_sasl_server_mechanism_get_mechanism_name
    };

    // act
    result = sasl_server_mechanism_create(&test_sasl_server_mechanism_interface_description_NULL, (void*)0x4242);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_SERVER_MECHANISM_01_005: [ If any `sasl_server_mechanism_interface_description` member is NULL, `sasl_server_mechanism_create` shall fail and return NULL.]*/
TEST_FUNCTION(sasl_server_mechanism_create_with_NULL_get_mechanism_name_fails)
{
    // arrange
    SASL_SERVER_MECHANISM_HANDLE result;
    const SASL_SERVER_MECHANISM_INTERFACE_DESCRIPTION test_sasl_server_mechanism_interface_description_NULL =
    {
        test_sasl_server_mechanism_create,
        test_sasl_server_mechanism_destroy,
        test_sasl_server_mechanism_handle_initial_response,
        test_sasl_server_mechanism_handle_response,
        NULL
    };

    // act
    result = sasl_server_mechanism_create(&test_sasl_server_mechanism_interface_description_NULL, (void*)0x4242);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_SERVER_MECHANISM_01_006: [ If allocating the memory needed for the SASL server mechanism interface fails then `sasl_server_mechanism_create` shall fail and return NULL. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_sasl_server_mechanism_fails_sasl_server_mechanism_create_fails)
{
    // arrange
    SASL_SERVER_MECHANISM_HANDLE result;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    result = sasl_server_mechanism_create(&test_sasl_server_mechanism_interface_description, (void*)0x4242);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* sasl_server_mechanism_destroy */

/* Tests_SRS_SASL_SERVER_MECHANISM_01_007: [ `sasl_server_mechanism_destroy` shall free all resources associated with the SASL mechanism handle. ]*/
/* Tests_SRS_SASL_SERVER_MECHANISM_01_008: [ `sasl_server_mechanism_destroy` shall also call the `destroy` function that is member of the `sasl_mechanism_interface_description` argument passed to `sasl_server_mechanism_create`, while passing as argument to `destroy` the result of the underlying concrete SASL mechanism handle. ]*/
TEST_FUNCTION(sasl_server_mechanism_destroy_frees_the_resources)
{
    // arrange
    SASL_SERVER_MECHANISM_HANDLE sasl_server_mechanism;
    sasl_server_mechanism = sasl_server_mechanism_create(&test_sasl_server_mechanism_interface_description, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(test_sasl_server_mechanism_destroy(test_concrete_sasl_server_mechanism_handle));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    sasl_server_mechanism_destroy(sasl_server_mechanism);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_SERVER_MECHANISM_01_009: [ If the argument `sasl_server_mechanism` is NULL, `sasl_server_mechanism_destroy` shall do nothing. ]*/
TEST_FUNCTION(sasl_server_mechanism_destroy_with_NULL_handle_does_nothing)
{
    // arrange

    // act
    sasl_server_mechanism_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* sasl_server_mechanism_handle_initial_response */

/* Tests_SRS_SASL_SERVER_MECHANISM_01_010: [ `sasl_server_mechanism_handle_initial_response` shall call the specific `handle_initial_response` function specified in `sasl_server_mechanism_create`, passing the `initial_response_bytes`, `hostname`, `send_challenge` and `challenge_bytes` arguments to it. ]*/
/* Tests_SRS_SASL_SERVER_MECHANISM_01_011: [ On success, `sasl_server_mechanism_handle_initial_response` shall return 0. ]*/
TEST_FUNCTION(sasl_server_mechanism_handle_initial_response_calls_the_underlying_handle_initial_response)
{
    // arrange
    SASL_SERVER_MECHANISM_HANDLE sasl_server_mechanism;
    int result;
    SASL_SERVER_MECHANISM_BYTES initial_response_bytes;
    SASL_SERVER_MECHANISM_BYTES challenge_bytes;
    bool send_challenge;

    sasl_server_mechanism = sasl_server_mechanism_create(&test_sasl_server_mechanism_interface_description, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(test_sasl_server_mechanism_handle_initial_response(test_concrete_sasl_server_mechanism_handle, &initial_response_bytes, "test_host", &send_challenge, &challenge_bytes));

    // act
    result = sasl_server_mechanism_handle_initial_response(sasl_server_mechanism, &initial_response_bytes, "test_host", &send_challenge, &challenge_bytes);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    sasl_server_mechanism_destroy(sasl_server_mechanism);
}

/* Tests_SRS_SASL_SERVER_MECHANISM_01_012: [ If the argument `sasl_server_mechanism` is NULL, `sasl_server_mechanism_handle_initial_response` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(sasl_server_mechanism_handle_initial_response_with_NULL_handle_fails)
{
    // arrange
    int result;
    SASL_SERVER_MECHANISM_BYTES initial_response_bytes;
    SASL_SERVER_MECHANISM_BYTES challenge_bytes;
    bool send_challenge;

    // act
    result = sasl_server_mechanism_handle_initial_response(NULL, &initial_response_bytes, "test_host", &send_challenge, &challenge_bytes);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_SASL_SERVER_MECHANISM_01_013: [ If the underlying `handle_initial_response` fails, `sasl_server_mechanism_handle_initial_response` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_the_underlying_handle_initial_response_fails_sasl_server_mechanism_handle_initial_response_also_fails)
{
    // arrange
    SASL_SERVER_MECHANISM_HANDLE sasl_server_mechanism;
    int result;
    SASL_SERVER_MECHANISM_BYTES initial_response_bytes;
    SASL_SERVER_MECHANISM_BYTES challenge_bytes;
    bool send_challenge;

    sasl_server_mechanism = sasl_server_mechanism_create(&test_sasl_server_mechanism_interface_description, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(test_sasl_server_mechanism_handle_initial_response(test_concrete_sasl_server_mechanism_handle, &initial_response_bytes, "test_host", &send_challenge, &challenge_bytes))
        .SetReturn(1);

    // act
    result = sasl_server_mechanism_handle_initial_response(sasl_server_mechanism, &initial_response_bytes, "test_host", &send_challenge, &challenge_bytes);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    sasl_server_mechanism_destroy(sasl_server_mechanism);
}

/* sasl_server_mechanism_handle_response */

/* Tests_SRS_SASL_SERVER_MECHANISM_01_014: [ `sasl_server_mechanism_handle_response` shall call the specific `handle_response` function specified in `sasl_server_mechanism_create`, passing the `response_bytes`, `send_next_challenge` and `next_challenge_bytes` arguments to it. ]*/
/* Tests_SRS_SASL_SERVER_MECHANISM_01_016: [ On success, `sasl_server_mechanism_handle_response` shall return 0. ]*/
TEST_FUNCTION(sasl_server_mechanism_handle_response_calls_the_underlying_handle_response)
{
    // arrange
    SASL_SERVER_MECHANISM_HANDLE sasl_server_mechanism;
    int result;
    SASL_SERVER_MECHANISM_BYTES response_bytes;
    SASL_SERVER_MECHANISM_BYTES next_challenge_bytes;
    bool send_next_challenge;

    sasl_server_mechanism = sasl_server_mechanism_create(&test_sasl_server_mechanism_interface_description, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(test_sasl_server_mechanism_handle_response(test_concrete_sasl_server_mechanism_handle, &response_bytes, &send_next_challenge, &next_challenge_bytes));

    // act
    result = sasl_server_mechanism_handle_response(sasl_server_mechanism, &response_bytes, &send_next_challenge, &next_challenge_bytes);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // cleanup
    sasl_server_mechanism_destroy(sasl_server_mechanism);
}

/* Tests_SRS_SASL_SERVER_MECHANISM_01_017: [ If the argument `sasl_server_mechanism` is NULL, `sasl_server_mechanism_handle_response` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(sasl_server_mechanism_handle_response_with_NULL_mechanism_fails)
{
    // arrange
    int result;
    SASL_SERVER_MECHANISM_BYTES response_bytes;
    SASL_SERVER_MECHANISM_BYTES next_challenge_bytes;
    bool send_next_challenge;

    // act
    result = sasl_server_mechanism_handle_response(NULL, &response_bytes, &send_next_challenge, &next_challenge_bytes);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_SASL_SERVER_MECHANISM_01_018: [ If the underlying `handle_response` fails, `sasl_server_mechanism_handle_response` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_the_underlying_handle_response_fails_then_sasl_server_mechanism_handle_response_also_fails)
{
    // arrange
    SASL_SERVER_MECHANISM_HANDLE sasl_server_mechanism;
    int result;
    SASL_SERVER_MECHANISM_BYTES response_bytes;
    SASL_SERVER_MECHANISM_BYTES next_challenge_bytes;
    bool send_next_challenge;

    sasl_server_mechanism = sasl_server_mechanism_create(&test_sasl_server_mechanism_interface_description, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(test_sasl_server_mechanism_handle_response(test_concrete_sasl_server_mechanism_handle, &response_bytes, &send_next_challenge, &next_challenge_bytes))
        .SetReturn(1);

    // act
    result = sasl_server_mechanism_handle_response(sasl_server_mechanism, &response_bytes, &send_next_challenge, &next_challenge_bytes);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // cleanup
    sasl_server_mechanism_destroy(sasl_server_mechanism);
}

/* sasl_server_mechanism_get_mechanism_name */

/* Tests_SRS_SASL_SERVER_MECHANISM_01_019: [ `sasl_server_mechanism_get_mechanism_name` shall call the specific `get_mechanism_name` function specified in `sasl_server_mechanism_create`. ]*/
/* Tests_SRS_SASL_SERVER_MECHANISM_01_020: [ On success, `sasl_server_mechanism_get_mechanism_name` shall return a pointer to a string with the mechanism name. ]*/
TEST_FUNCTION(sasl_server_mechanism_get_mechanism_name_calls_the_underlying_get_mechanism_name)
{
    // arrange
    SASL_SERVER_MECHANISM_HANDLE sasl_server_mechanism;
    const char* result;

    sasl_server_mechanism = sasl_server_mechanism_create(&test_sasl_server_mechanism_interface_description, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(test_sasl_server_mechanism_get_mechanism_name());

    // act
    result = sasl_server_mechanism_get_mechanism_name(sasl_server_mechanism);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(char_ptr, test_mechanism_name, result);

    // cleanup
    sasl_server_mechanism_destroy(sasl_server_mechanism);
}

/* Tests_SRS_SASL_SERVER_MECHANISM_01_019: [ `sasl_server_mechanism_get_mechanism_name` shall call the specific `get_mechanism_name` function specified in `sasl_server_mechanism_create`. ]*/
/* Tests_SRS_SASL_SERVER_MECHANISM_01_020: [ On success, `sasl_server_mechanism_get_mechanism_name` shall return a pointer to a string with the mechanism name. ]*/
TEST_FUNCTION(sasl_server_mechanism_get_mechanism_name_calls_the_underlying_get_mechanism_name_with_another_name)
{
    // arrange
    SASL_SERVER_MECHANISM_HANDLE sasl_server_mechanism;
    const char* result;

    sasl_server_mechanism = sasl_server_mechanism_create(&test_sasl_server_mechanism_interface_description, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(test_sasl_server_mechanism_get_mechanism_name())
        .SetReturn("another_name");

    // act
    result = sasl_server_mechanism_get_mechanism_name(sasl_server_mechanism);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(char_ptr, "another_name", result);

    // cleanup
    sasl_server_mechanism_destroy(sasl_server_mechanism);
}

/* Tests_SRS_SASL_SERVER_MECHANISM_01_021: [ If the argument `sasl_server_mechanism` is NULL, `sasl_server_mechanism_get_mechanism_name` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(sasl_server_mechanism_get_mechanism_name_with_NULL_mechanism_fails)
{
    // arrange
    const char* result;

    // act
    result = sasl_server_mechanism_get_mechanism_name(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_SASL_SERVER_MECHANISM_01_022: [ If the underlying `get_mechanism_name` fails, `sasl_server_mechanism_get_mechanism_name` shall return NULL. ]*/
TEST_FUNCTION(when_the_underlying_get_mechanism_name_fails_then_sasl_server_mechanism_get_mechanism_name_also_fails)
{
    // arrange
    SASL_SERVER_MECHANISM_HANDLE sasl_server_mechanism;
    const char* result;

    sasl_server_mechanism = sasl_server_mechanism_create(&test_sasl_server_mechanism_interface_description, (void*)0x4242);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(test_sasl_server_mechanism_get_mechanism_name())
        .SetReturn(NULL);

    // act
    result = sasl_server_mechanism_get_mechanism_name(sasl_server_mechanism);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // cleanup
    sasl_server_mechanism_destroy(sasl_server_mechanism);
}

END_TEST_SUITE(sasl_server_mechanism_ut)
