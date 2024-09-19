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

static void* my_gballoc_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#define ENABLE_MOCKS

#include "azure_c_shared_utility/gballoc.h"

#undef ENABLE_MOCKS

#include "azure_uamqp_c/sasl_plain.h"

#define MAX_AUTHZID "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" "1234567890123456789012345678901234567890123456789012345"
#define MAX_AUTHCID MAX_AUTHZID
#define MAX_PASSWD MAX_AUTHZID

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(sasl_plain_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, my_gballoc_calloc);
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

/* saslplain_create */

/* Tests_SRS_SASL_PLAIN_01_001: [`saslplain_create` shall return on success a non-NULL handle to a new SASL plain mechanism.] */
TEST_FUNCTION(saslplain_create_with_valid_args_succeeds)
{
    // arrange
    SASL_PLAIN_CONFIG sasl_plain_config = { "test_authcid", "test_pwd", "test_authzid" };
    CONCRETE_SASL_MECHANISM_HANDLE result;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    // act
    result = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslplain_get_interface()->concrete_sasl_mechanism_destroy(result);
}

/* Tests_SRS_SASL_PLAIN_01_002: [If allocating the memory needed for the saslplain instance fails then `saslplain_create` shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_fails_then_saslplain_create_fails)
{
    // arrange
    SASL_PLAIN_CONFIG sasl_plain_config = { "test_authcid", "test_pwd", "test_authzid" };
    CONCRETE_SASL_MECHANISM_HANDLE result;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    result = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_SASL_PLAIN_01_002: [If allocating the memory needed for the saslplain instance fails then `saslplain_create` shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_the_config_fails_then_saslplain_create_fails)
{
    // arrange
    SASL_PLAIN_CONFIG sasl_plain_config = { "test_authcid", "test_pwd", "test_authzid" };
    CONCRETE_SASL_MECHANISM_HANDLE result;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_SASL_PLAIN_01_003: [If the `config` argument is NULL, then `saslplain_create` shall fail and return NULL.] */
TEST_FUNCTION(saslplain_create_with_NULL_config_fails)
{
    // arrange

    // act
    CONCRETE_SASL_MECHANISM_HANDLE result = saslplain_get_interface()->concrete_sasl_mechanism_create(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_SASL_PLAIN_01_004: [If either the `authcid` or `passwd` member of the `config` structure is NULL, then `saslplain_create` shall fail and return NULL.] */
TEST_FUNCTION(saslplain_create_with_NULL_authcid_fails)
{
    // arrange
    SASL_PLAIN_CONFIG sasl_plain_config = { NULL, "test_pwd", "test_authzid" };

    // act
    CONCRETE_SASL_MECHANISM_HANDLE result = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_SASL_PLAIN_01_004: [If either the `authcid` or `passwd` member of the `config` structure is NULL, then `saslplain_create` shall fail and return NULL.] */
TEST_FUNCTION(saslplain_create_with_NULL_passwd_fails)
{
    // arrange
    SASL_PLAIN_CONFIG sasl_plain_config = { "test_authcid", NULL, "test_authzid" };

    // act
    CONCRETE_SASL_MECHANISM_HANDLE result = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_SASL_PLAIN_01_004: [If either the `authcid` or `passwd` member of the `config` structure is NULL, then `saslplain_create` shall fail and return NULL.] */
/* Tests_SRS_SASL_PLAIN_01_025: [ `authzid` shall be optional. ]*/
TEST_FUNCTION(saslplain_create_with_NULL_authzid_succeeds)
{
    // arrange
    SASL_PLAIN_CONFIG sasl_plain_config = { "test_authcid", "test_pwd", NULL };
    CONCRETE_SASL_MECHANISM_HANDLE result;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    // act
    result = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslplain_get_interface()->concrete_sasl_mechanism_destroy(result);
}

/* saslplain_destroy */

/* Tests_SRS_SASL_PLAIN_01_005: [`saslplain_destroy` shall free all resources associated with the SASL mechanism.] */
TEST_FUNCTION(saslplain_destroy_frees_the_allocated_memory)
{
    // arrange
    SASL_PLAIN_CONFIG sasl_plain_config = { "test_authcid", "test_pwd", NULL };
    CONCRETE_SASL_MECHANISM_HANDLE sasl_plain = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    saslplain_get_interface()->concrete_sasl_mechanism_destroy(sasl_plain);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_SASL_PLAIN_01_006: [If the argument `concrete_sasl_mechanism` is NULL, `saslplain_destroy` shall do nothing.] */
TEST_FUNCTION(saslplain_destroy_with_NULL_handle_does_nothing)
{
    // arrange

    // act
    saslplain_get_interface()->concrete_sasl_mechanism_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* saslplain_get_init_bytes */

/* Tests_SRS_SASL_PLAIN_01_007: [`saslplain_get_init_bytes` shall construct the initial bytes per the RFC 4616.] */
/* Tests_SRS_SASL_PLAIN_01_008: [On success `saslplain_get_init_bytes` shall return zero.] */
/* Tests_SRS_SASL_PLAIN_01_016: [The mechanism consists of a single message, a string of [UTF-8] encoded [Unicode] characters, from the client to the server.] */
/* Tests_SRS_SASL_PLAIN_01_017: [The client presents the authorization identity (identity to act as), followed by a NUL (U+0000) character, followed by the authentication identity (identity whose password will be used), followed by a NUL (U+0000) character, followed by the clear-text password.] */
/* Tests_SRS_SASL_PLAIN_01_019: [   message   = [authzid] UTF8NUL authcid UTF8NUL passwd] */
/* Tests_SRS_SASL_PLAIN_01_023: [The authorization identity (authzid), authentication identity (authcid), password (passwd), and NUL character deliminators SHALL be transferred as [UTF-8] encoded strings of [Unicode] characters.] */
/* Tests_SRS_SASL_PLAIN_01_024: [As the NUL (U+0000) character is used as a deliminator, the NUL (U+0000) character MUST NOT appear in authzid, authcid, or passwd productions.] */
TEST_FUNCTION(saslplain_get_init_bytes_returns_the_correct_concateneted_bytes)
{
    // arrange
    int result;
    unsigned char expected_bytes[] = "test_authzid" "\0" "test_authcid" "\0" "test_pwd";
    SASL_PLAIN_CONFIG sasl_plain_config = { "test_authcid", "test_pwd", "test_authzid" };
    CONCRETE_SASL_MECHANISM_HANDLE sasl_plain = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);
    SASL_MECHANISM_BYTES init_bytes;

    umock_c_reset_all_calls();

    // act
    result = saslplain_get_interface()->concrete_sasl_mechanism_get_init_bytes(sasl_plain, &init_bytes);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, sizeof(expected_bytes) - 1, init_bytes.length);
    ASSERT_ARE_EQUAL(int, 0, memcmp(expected_bytes, init_bytes.bytes, sizeof(expected_bytes) - 1));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslplain_get_interface()->concrete_sasl_mechanism_destroy(sasl_plain);
}

/* Tests_SRS_SASL_PLAIN_01_007: [`saslplain_get_init_bytes` shall construct the initial bytes per the RFC 4616.] */
/* Tests_SRS_SASL_PLAIN_01_008: [On success `saslplain_get_init_bytes` shall return zero.] */
/* Tests_SRS_SASL_PLAIN_01_018: [As with other SASL mechanisms, the client does not provide an authorization identity when it wishes the server to derive an identity from the credentials and use that as the authorization identity.] */
TEST_FUNCTION(saslplain_get_init_bytes_with_NULL_authzid_returns_the_correct_concateneted_bytes)
{
    // arrange
    int result;
    unsigned char expected_bytes[] = "\0" "test_authcid" "\0" "test_pwd";
    SASL_PLAIN_CONFIG sasl_plain_config = { "test_authcid", "test_pwd", NULL };
    CONCRETE_SASL_MECHANISM_HANDLE sasl_plain = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);
    SASL_MECHANISM_BYTES init_bytes;
    umock_c_reset_all_calls();

    // act
    result = saslplain_get_interface()->concrete_sasl_mechanism_get_init_bytes(sasl_plain, &init_bytes);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, sizeof(expected_bytes) - 1, init_bytes.length);
    ASSERT_ARE_EQUAL(int, 0, memcmp(expected_bytes, init_bytes.bytes, sizeof(expected_bytes) - 1));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslplain_get_interface()->concrete_sasl_mechanism_destroy(sasl_plain);
}

/* Tests_SRS_SASL_PLAIN_01_018: [As with other SASL mechanisms, the client does not provide an authorization identity when it wishes the server to derive an identity from the credentials and use that as the authorization identity.] */
TEST_FUNCTION(saslplain_get_init_bytes_with_authzid_zero_length_succeeds)
{
    // arrange
    int result;
    unsigned char expected_bytes[] = "\0" "test_authcid" "\0" "test_pwd";
    SASL_PLAIN_CONFIG sasl_plain_config = { "test_authcid", "test_pwd", "" };
    CONCRETE_SASL_MECHANISM_HANDLE sasl_plain = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);
    SASL_MECHANISM_BYTES init_bytes;
    umock_c_reset_all_calls();

    // act
    result = saslplain_get_interface()->concrete_sasl_mechanism_get_init_bytes(sasl_plain, &init_bytes);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, sizeof(expected_bytes) - 1, init_bytes.length);
    ASSERT_ARE_EQUAL(int, 0, memcmp(expected_bytes, init_bytes.bytes, sizeof(expected_bytes) - 1));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslplain_get_interface()->concrete_sasl_mechanism_destroy(sasl_plain);
}

/* Tests_SRS_SASL_PLAIN_01_020: [   authcid   = 1*SAFE ; MUST accept up to 255 octets] */
TEST_FUNCTION(saslplain_get_init_bytes_with_1_byte_for_each_field_succeeds)
{
    // arrange
    int result;
    unsigned char expected_bytes[] = "1" "\0" "b" "\0" "c";
    SASL_PLAIN_CONFIG sasl_plain_config = { "b", "c", "1" };
    CONCRETE_SASL_MECHANISM_HANDLE sasl_plain = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);
    SASL_MECHANISM_BYTES init_bytes;
    umock_c_reset_all_calls();

    // act
    result = saslplain_get_interface()->concrete_sasl_mechanism_get_init_bytes(sasl_plain, &init_bytes);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, sizeof(expected_bytes) - 1, init_bytes.length);
    ASSERT_ARE_EQUAL(int, 0, memcmp(expected_bytes, init_bytes.bytes, sizeof(expected_bytes) - 1));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslplain_get_interface()->concrete_sasl_mechanism_destroy(sasl_plain);
}

/* Tests_SRS_SASL_PLAIN_01_020: [   authcid   = 1*SAFE ; MUST accept up to 255 octets] */
/* Tests_SRS_SASL_PLAIN_01_021: [   authzid   = 1*SAFE ; MUST accept up to 255 octets] */
/* Tests_SRS_SASL_PLAIN_01_022: [   passwd    = 1*SAFE ; MUST accept up to 255 octets] */
TEST_FUNCTION(saslplain_get_init_bytes_with_max_bytes_for_each_field_succeeds)
{
    // arrange
    int result;
    unsigned char expected_bytes[] = MAX_AUTHZID "\0" MAX_AUTHCID "\0" MAX_PASSWD;
    SASL_PLAIN_CONFIG sasl_plain_config = { MAX_AUTHCID, MAX_PASSWD, MAX_AUTHZID };
    CONCRETE_SASL_MECHANISM_HANDLE sasl_plain = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);
    SASL_MECHANISM_BYTES init_bytes;
    umock_c_reset_all_calls();

    // act
    result = saslplain_get_interface()->concrete_sasl_mechanism_get_init_bytes(sasl_plain, &init_bytes);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, sizeof(expected_bytes) - 1, init_bytes.length);
    ASSERT_ARE_EQUAL(int, 0, memcmp(expected_bytes, init_bytes.bytes, sizeof(expected_bytes) - 1));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslplain_get_interface()->concrete_sasl_mechanism_destroy(sasl_plain);
}

/* Tests_SRS_SASL_PLAIN_01_020: [   authcid   = 1*SAFE ; MUST accept up to 255 octets] */
TEST_FUNCTION(saslplain_get_init_bytes_with_authcid_over_max_bytes_fails)
{
    // arrange
    SASL_PLAIN_CONFIG sasl_plain_config = { MAX_AUTHCID "x", MAX_PASSWD, MAX_AUTHZID };

    // act
    CONCRETE_SASL_MECHANISM_HANDLE sasl_plain = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(sasl_plain);
}

/* Tests_SRS_SASL_PLAIN_01_021: [   authzid   = 1*SAFE ; MUST accept up to 255 octets] */
TEST_FUNCTION(saslplain_get_init_bytes_with_authzid_over_max_bytes_fails)
{
    // arrange
    SASL_PLAIN_CONFIG sasl_plain_config = { MAX_AUTHCID, MAX_PASSWD, MAX_AUTHZID "x" };

    // act
    CONCRETE_SASL_MECHANISM_HANDLE sasl_plain = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(sasl_plain);
}

/* Tests_SRS_SASL_PLAIN_01_022: [   passwd    = 1*SAFE ; MUST accept up to 255 octets] */
TEST_FUNCTION(saslplain_get_init_bytes_with_passwd_over_max_bytes_fails)
{
    // arrange
    SASL_PLAIN_CONFIG sasl_plain_config = { MAX_AUTHCID, MAX_PASSWD "x", MAX_AUTHZID };

    // act
    CONCRETE_SASL_MECHANISM_HANDLE sasl_plain = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(sasl_plain);
}

/* Tests_SRS_SASL_PLAIN_01_020: [   authcid   = 1*SAFE ; MUST accept up to 255 octets] */
TEST_FUNCTION(saslplain_get_init_bytes_with_authcid_zero_length_fails)
{
    // arrange
    SASL_PLAIN_CONFIG sasl_plain_config = { "", "passwd", "authzid" };

    // act
    CONCRETE_SASL_MECHANISM_HANDLE sasl_plain = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(sasl_plain);
}

/* Tests_SRS_SASL_PLAIN_01_022: [   passwd    = 1*SAFE ; MUST accept up to 255 octets] */
TEST_FUNCTION(saslplain_get_init_bytes_with_passwd_zero_length_fails)
{
    // arrange
    SASL_PLAIN_CONFIG sasl_plain_config = { "authcid", "", "authzid" };

    // act
    CONCRETE_SASL_MECHANISM_HANDLE sasl_plain = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(sasl_plain);
}

/* Tests_SRS_SASL_PLAIN_01_009: [If any argument is NULL, `saslplain_get_init_bytes` shall return a non-zero value.] */
TEST_FUNCTION(saslplain_get_init_bytes_with_NULL_sasl_plain_handle_fails)
{
    // arrange
    SASL_MECHANISM_BYTES init_bytes;

    // act
    int result = saslplain_get_interface()->concrete_sasl_mechanism_get_init_bytes(NULL, &init_bytes);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_SASL_PLAIN_01_009: [If any argument is NULL, `saslplain_get_init_bytes` shall return a non-zero value.] */
TEST_FUNCTION(saslplain_get_init_bytes_with_NULL_init_bytes_fails)
{
    // arrange
    int result;
    SASL_PLAIN_CONFIG sasl_plain_config = { "test_authcid", "test_pwd", NULL };
    CONCRETE_SASL_MECHANISM_HANDLE sasl_plain = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);
    umock_c_reset_all_calls();

    // act
    result = saslplain_get_interface()->concrete_sasl_mechanism_get_init_bytes(sasl_plain, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslplain_get_interface()->concrete_sasl_mechanism_destroy(sasl_plain);
}

/* saslplain_get_mechanism_name */

/* Tests_SRS_SASL_PLAIN_01_010: [`saslplain_get_mechanism_name` shall validate the argument `concrete_sasl_mechanism` and on success it shall return a pointer to the string "PLAIN".] */
TEST_FUNCTION(saslplain_get_mechanism_name_with_non_NULL_concrete_sasl_mechanism_succeeds)
{
    // arrange
    const char* result;
    SASL_PLAIN_CONFIG sasl_plain_config = { "test_authcid", "test_pwd", NULL };
    CONCRETE_SASL_MECHANISM_HANDLE sasl_plain = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);
    umock_c_reset_all_calls();

    // act
    result = saslplain_get_interface()->concrete_sasl_mechanism_get_mechanism_name(sasl_plain);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "PLAIN", result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslplain_get_interface()->concrete_sasl_mechanism_destroy(sasl_plain);
}

/* Tests_SRS_SASL_PLAIN_01_011: [If the argument `concrete_sasl_mechanism` is NULL, `saslplain_get_mechanism_name` shall return NULL.] */
TEST_FUNCTION(saslplain_get_mechanism_name_with_NULL_concrete_sasl_mechanism_fails)
{
    // arrange

    // act
    const char* result = saslplain_get_interface()->concrete_sasl_mechanism_get_mechanism_name(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* saslplain_challenge */

/* Tests_SRS_SASL_PLAIN_01_012: [`saslplain_challenge` shall set the `response_bytes` buffer to NULL and 0 size as the PLAIN SASL mechanism does not implement challenge/response.] */
/* Tests_SRS_SASL_PLAIN_01_013: [On success, `saslplain_challenge` shall return 0.] */
TEST_FUNCTION(saslplain_challenge_returns_a_NULL_response_bytes_buffer)
{
    // arrange
    int result;
    SASL_PLAIN_CONFIG sasl_plain_config = { "test_authcid", "test_pwd", NULL };
    CONCRETE_SASL_MECHANISM_HANDLE saslplain = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);
    SASL_MECHANISM_BYTES challenge_bytes;
    SASL_MECHANISM_BYTES response_bytes;
    umock_c_reset_all_calls();

    // act
    result = saslplain_get_interface()->concrete_sasl_mechanism_challenge(saslplain, &challenge_bytes, &response_bytes);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_IS_NULL(response_bytes.bytes);
    ASSERT_ARE_EQUAL(size_t, 0, response_bytes.length);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslplain_get_interface()->concrete_sasl_mechanism_destroy(saslplain);
}

/* Tests_SRS_SASL_PLAIN_01_013: [On success, `saslplain_challenge` shall return 0.] */
TEST_FUNCTION(saslplain_with_NULL_challenge_bytes_returns_a_NULL_response_bytes_buffer)
{
    // arrange
    int result;
    SASL_PLAIN_CONFIG sasl_plain_config = { "test_authcid", "test_pwd", NULL };
    CONCRETE_SASL_MECHANISM_HANDLE saslplain = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);
    SASL_MECHANISM_BYTES response_bytes;
    umock_c_reset_all_calls();

    // act
    result = saslplain_get_interface()->concrete_sasl_mechanism_challenge(saslplain, NULL, &response_bytes);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_IS_NULL(response_bytes.bytes);
    ASSERT_ARE_EQUAL(size_t, 0, response_bytes.length);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslplain_get_interface()->concrete_sasl_mechanism_destroy(saslplain);
}

/* Tests_SRS_SASL_PLAIN_01_014: [If the `concrete_sasl_mechanism` or `response_bytes` argument is NULL then `saslplain_challenge` shall fail and return a non-zero value.] */
TEST_FUNCTION(saslplain_challenge_with_NULL_handle_fails)
{
    // arrange
    SASL_MECHANISM_BYTES challenge_bytes;
    SASL_MECHANISM_BYTES response_bytes;

    // act
    int result = saslplain_get_interface()->concrete_sasl_mechanism_challenge(NULL, &challenge_bytes, &response_bytes);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_SASL_PLAIN_01_014: [If the `concrete_sasl_mechanism` or `response_bytes` argument is NULL then `saslplain_challenge` shall fail and return a non-zero value.] */
TEST_FUNCTION(saslplain_challenge_with_NULL_response_bytes_fails)
{
    // arrange
    int result;
    SASL_PLAIN_CONFIG sasl_plain_config = { "test_authcid", "test_pwd", NULL };
    CONCRETE_SASL_MECHANISM_HANDLE saslplain = saslplain_get_interface()->concrete_sasl_mechanism_create(&sasl_plain_config);
    SASL_MECHANISM_BYTES challenge_bytes;
    umock_c_reset_all_calls();

    // act
    result = saslplain_get_interface()->concrete_sasl_mechanism_challenge(saslplain, &challenge_bytes, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    saslplain_get_interface()->concrete_sasl_mechanism_destroy(saslplain);
}

/* saslplain_get_interface */

/* Tests_SRS_SASL_PLAIN_01_015: [**`saslplain_get_interface` shall return a pointer to a `SASL_MECHANISM_INTERFACE_DESCRIPTION` structure that contains pointers to the functions: `saslplain_create`, `saslplain_destroy`, `saslplain_get_init_bytes`, `saslplain_get_mechanism_name`, `saslplain_challenge`.] */
TEST_FUNCTION(saslplain_get_interface_returns_the_sasl_plain_mechanism_interface)
{
    // arrange

    // act
    const SASL_MECHANISM_INTERFACE_DESCRIPTION* result = saslplain_get_interface();

    // assert
    ASSERT_IS_NOT_NULL(result->concrete_sasl_mechanism_create);
    ASSERT_IS_NOT_NULL(result->concrete_sasl_mechanism_destroy);
    ASSERT_IS_NOT_NULL(result->concrete_sasl_mechanism_get_init_bytes);
    ASSERT_IS_NOT_NULL(result->concrete_sasl_mechanism_get_mechanism_name);
    ASSERT_IS_NOT_NULL(result->concrete_sasl_mechanism_challenge);
}

END_TEST_SUITE(sasl_plain_ut)
