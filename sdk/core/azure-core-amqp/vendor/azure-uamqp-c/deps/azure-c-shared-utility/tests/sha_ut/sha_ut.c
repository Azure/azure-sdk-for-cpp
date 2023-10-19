// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

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
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umock_c_negative_tests.h"
#include "azure_macro_utils/macro_utils.h"

#define ENABLE_MOCKS
#undef ENABLE_MOCKS

#include "azure_c_shared_utility/sha.h"

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

/**
 * This is necessary for the test suite, just keep as is.
 */
static TEST_MUTEX_HANDLE g_testByTest;

BEGIN_TEST_SUITE(sha_ut)

    TEST_SUITE_INITIALIZE(TestClassInitialize)
    {
        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        (void)umock_c_init(on_umock_c_error);
    }
    TEST_SUITE_CLEANUP(TestClassCleanup)
    {
        umock_c_deinit();

        TEST_MUTEX_DESTROY(g_testByTest);
    }

    TEST_FUNCTION_INITIALIZE(initialize)
    {
        if (TEST_MUTEX_ACQUIRE(g_testByTest))
        {
            ASSERT_FAIL("Could not acquire test serialization mutex.");
        }

        umock_c_reset_all_calls();
    }

    TEST_FUNCTION_CLEANUP(cleans)
    {
        TEST_MUTEX_RELEASE(g_testByTest);
    }

    TEST_FUNCTION(SHA224Reset_succeeds)
    {
        //arrange
        int result;
        SHA224Context sha_ctx;

        //act
        result = SHA224Reset(&sha_ctx);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }
    TEST_FUNCTION(SHA256Reset_succeeds)
    {
        //arrange
        int result;
        SHA256Context sha_ctx;

        //act
        result = SHA256Reset(&sha_ctx);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(SHA256Reset_ctx_NULL_fail)
    {
        //arrange
        int result;

        //act
        result = SHA256Reset(NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(SHA256Input_2nd_call_fail)
    {
        //arrange
        int result;
        SHA256Context sha_ctx;
        uint8_t bytes[8];
        unsigned int count = 8;
        uint8_t msg_bits = 111;

        //act
        result = SHA256Reset(&sha_ctx);
        result = SHA256Input(&sha_ctx, bytes, count);
        result = SHA256FinalBits(&sha_ctx, msg_bits, 1);
        result = SHA256Input(&sha_ctx, bytes, count);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(SHA224Input_succeeds)
    {
        //arrange
        int result;
        SHA224Context sha_ctx;
        uint8_t bytes[32];
        unsigned int count = 32;

        //act
        result = SHA224Reset(&sha_ctx);
        result = SHA224Input(&sha_ctx, bytes, count);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(SHA256Input_succeeds)
    {
        //arrange
        int result;
        SHA256Context sha_ctx;
        uint8_t bytes[32];
        unsigned int count = 32;

        //act
        result = SHA256Reset(&sha_ctx);
        result = SHA256Input(&sha_ctx, bytes, count);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(SHA256Input_large_bytes_succeeds)
    {
        //arrange
        int result;
        SHA256Context sha_ctx;
        uint8_t bytes[128];
        unsigned int count = 128;

        //act
        result = SHA256Reset(&sha_ctx);
        result = SHA256Input(&sha_ctx, bytes, count);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(SHA256Input_ctx_NULL_fail)
    {
        //arrange
        int result;
        uint8_t bytes[32];
        unsigned int count = 32;

        //act
        result = SHA256Input(NULL, bytes, count);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(SHA256Input_bytes_NULL_count_invalid_fail)
    {
        //arrange
        int result;
        SHA256Context sha_ctx;
        unsigned int count = 10;

        //act
        result = SHA256Reset(&sha_ctx);
        result = SHA256Input(&sha_ctx, NULL, count);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(SHA256Input_bytes_NULL_succeeds)
    {
        //arrange
        int result;
        SHA256Context sha_ctx;
        unsigned int count = 0;

        //act
        result = SHA256Reset(&sha_ctx);
        result = SHA256Input(&sha_ctx, NULL, count);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(SHA256FinalBits_succeeds)
    {
        //arrange
        int result;
        SHA256Context sha_ctx;
        uint8_t bytes[1024];
        unsigned int count = 1024;
        uint8_t msg_bits = 87;

        //act
        result = SHA256Reset(&sha_ctx);
        result = SHA256Input(&sha_ctx, bytes, count);
        result = SHA256FinalBits(&sha_ctx, msg_bits, 2);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(SHA224FinalBits_succeeds)
    {
        //arrange
        int result;
        SHA224Context sha_ctx;
        uint8_t bytes[1024];
        unsigned int count = 1024;
        uint8_t msg_bits = 87;

        //act
        result = SHA224Reset(&sha_ctx);
        result = SHA224Input(&sha_ctx, bytes, count);
        result = SHA224FinalBits(&sha_ctx, msg_bits, 2);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(SHA256FinalBits_ctx_NULL_fail)
    {
        //arrange
        int result;
        uint8_t msg_bits = 234;

        //act
        result = SHA256FinalBits(NULL, msg_bits, 2);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(SHA256FinalBits_msg_bit_0_success)
    {
        //arrange
        int result;
        SHA256Context sha_ctx;
        uint8_t bytes[32];
        unsigned int count = 32;
        uint8_t msg_bits = 0;

        //act
        result = SHA256Reset(&sha_ctx);
        result = SHA256Input(&sha_ctx, bytes, count);
        result = SHA256FinalBits(&sha_ctx, msg_bits, 0);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(SHA256FinalBits_2nd_call_fail)
    {
        //arrange
        int result;
        SHA256Context sha_ctx;
        uint8_t bytes[32];
        unsigned int count = 32;
        uint8_t msg_bits = 234;

        //act
        result = SHA256Reset(&sha_ctx);
        result = SHA256Input(&sha_ctx, bytes, count);
        result = SHA256FinalBits(&sha_ctx, msg_bits, 2);
        result = SHA256FinalBits(&sha_ctx, msg_bits, 2);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(SHA256Result_succeeds)
    {
        //arrange
        int result;
        SHA256Context sha_ctx;
        uint8_t bytes[256];
        unsigned int count = 32;
        uint8_t msg_bits = 234;
        uint8_t Message_Digest[SHA256HashSize];

        //act
        result = SHA256Reset(&sha_ctx);
        result = SHA256Input(&sha_ctx, bytes, count);
        result = SHA256FinalBits(&sha_ctx, msg_bits, 2);
        result = SHA256Result(&sha_ctx, Message_Digest);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(SHA224Result_succeeds)
    {
        //arrange
        int result;
        SHA224Context sha_ctx;
        uint8_t bytes[32];
        unsigned int count = 32;
        uint8_t msg_bits = 87;
        uint8_t Message_Digest[SHA224HashSize];

        //act
        result = SHA224Reset(&sha_ctx);
        result = SHA224Input(&sha_ctx, bytes, count);
        result = SHA224FinalBits(&sha_ctx, msg_bits, 2);
        result = SHA224Result(&sha_ctx, Message_Digest);

        //assert
        ASSERT_ARE_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(SHA256Result_ctx_NULL_fail)
    {
        //arrange
        int result;
        uint8_t Message_Digest[SHA256HashSize];

        //act
        result = SHA256Result(NULL, Message_Digest);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

    TEST_FUNCTION(SHA256Result_msg_digest_NULL_succeeds)
    {
        //arrange
        int result;
        SHA256Context sha_ctx;

        //act
        result = SHA256Reset(&sha_ctx);
        result = SHA256Result(&sha_ctx, NULL);

        //assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        //cleanup
    }

END_TEST_SUITE(sha_ut)
