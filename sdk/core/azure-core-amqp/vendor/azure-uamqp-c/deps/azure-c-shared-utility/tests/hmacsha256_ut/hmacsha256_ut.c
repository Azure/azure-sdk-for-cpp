// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"

void* real_malloc(size_t size)
{
    return malloc(size);
}

void* real_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

void* real_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

void real_free(void* ptr)
{
    free(ptr);
}

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/strings.h"
#undef ENABLE_MOCKS
#include "azure_c_shared_utility/hmacsha256.h"

static TEST_MUTEX_HANDLE g_testByTest;

TEST_DEFINE_ENUM_TYPE(HMACSHA256_RESULT, HMACSHA256_RESULT_VALUES);

static BUFFER_HANDLE hash;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(HMACSHA256_UnitTests)

TEST_SUITE_INITIALIZE(TestClassInitialize)
{
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, real_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, real_calloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, real_free);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, real_realloc);
}

TEST_SUITE_CLEANUP(TestClassCleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }
    umock_c_reset_all_calls();
    hash = BUFFER_new();
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    BUFFER_delete(hash);
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* HMACSHA256_ComputeHash */

TEST_FUNCTION(HMACSHA256_ComputeHash_With_NULL_Key_Fails)
{
    // arrange
    static const unsigned char key[] = "key";
    static const unsigned char buffer[] = "testPayload";

    // act
    HMACSHA256_RESULT result = HMACSHA256_ComputeHash(NULL, sizeof(key) - 1, buffer, sizeof(buffer) - 1, hash);

    // assert
    ASSERT_ARE_EQUAL(HMACSHA256_RESULT, HMACSHA256_INVALID_ARG, result);
}

TEST_FUNCTION(HMACSHA256_ComputeHash_With_Zero_Key_Buffer_Size_Fails)
{
    // arrange
    static const unsigned char key[] = "key";
    static const unsigned char buffer[] = "testPayload";

    // act
    HMACSHA256_RESULT result = HMACSHA256_ComputeHash(key, 0, buffer, sizeof(buffer) - 1, hash);

    // assert
    ASSERT_ARE_EQUAL(HMACSHA256_RESULT, HMACSHA256_INVALID_ARG, result);
}

TEST_FUNCTION(HMACSHA256_ComputeHash_With_NULL_Payload_Fails)
{
    // arrange
    static const unsigned char key[] = "key";
    static const unsigned char buffer[] = "testPayload";

    // act
    HMACSHA256_RESULT result = HMACSHA256_ComputeHash(key, sizeof(key) - 1, NULL, sizeof(buffer) - 1, hash);

    // assert
    ASSERT_ARE_EQUAL(HMACSHA256_RESULT, HMACSHA256_INVALID_ARG, result);
}

TEST_FUNCTION(HMACSHA256_ComputeHash_With_Zero_Payload_Buffer_Size_Fails)
{
    // arrange
    static const unsigned char key[] = "key";
    static const unsigned char buffer[] = "testPayload";

    // act
    HMACSHA256_RESULT result = HMACSHA256_ComputeHash(key, sizeof(key) - 1, buffer, 0, hash);

    // assert
    ASSERT_ARE_EQUAL(HMACSHA256_RESULT, HMACSHA256_INVALID_ARG, result);
}

TEST_FUNCTION(HMACSHA256_ComputeHash_With_NULL_Hash_Fails)
{
    // arrange
    static const unsigned char key[] = "key";
    static const unsigned char buffer[] = "testPayload";

    // act
    HMACSHA256_RESULT result = HMACSHA256_ComputeHash(key, sizeof(key) - 1, buffer, sizeof(buffer) - 1, NULL);

    // assert
    ASSERT_ARE_EQUAL(HMACSHA256_RESULT, HMACSHA256_INVALID_ARG, result);
}

TEST_FUNCTION(HMACSHA256_ComputeHash_Succeeds)
{
    // arrange
    static const unsigned char key[] = "key";
    static const unsigned char buffer[] = "testPayload";
    unsigned char expectedHash[32] = { 108, 7, 130, 47, 104, 233, 39, 188, 126, 122, 134, 187, 63, 19, 52, 120, 172, 7, 43, 25, 133, 60, 92, 217, 59, 59, 69, 116, 85, 104, 55, 224 };

    // act
    HMACSHA256_RESULT result = HMACSHA256_ComputeHash(key, sizeof(key) - 1, buffer, sizeof(buffer) - 1, hash);

    // assert
    ASSERT_ARE_EQUAL(HMACSHA256_RESULT, HMACSHA256_OK, result);
    ASSERT_ARE_EQUAL(int, 0, memcmp(BUFFER_u_char(hash), expectedHash, 8));
}

END_TEST_SUITE(HMACSHA256_UnitTests)
