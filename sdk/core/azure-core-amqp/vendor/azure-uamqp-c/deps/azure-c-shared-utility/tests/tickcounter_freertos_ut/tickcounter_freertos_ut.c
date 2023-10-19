// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#else
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "azure_c_shared_utility/threadapi.h"
#include "umock_c/umocktypes_stdint.h"

static TEST_MUTEX_HANDLE g_testByTest;

#define FAKE_TICK_NO_OVERFLOW 333
#define FAKE_TICK_INTERVAL 120
#define FAKE_TICK_SCALED_INTERVAL FAKE_TICK_INTERVAL * 1000  / CONFIG_FREERTOS_HZ
#define FAKE_TICK_OVERFLOW_OFFSET 40
#define FAKE_TICK_BEFORE_OVERFLOW (UINT32_MAX - FAKE_TICK_OVERFLOW_OFFSET)
#define FAKE_TICK_AFTER_OVERFLOW (FAKE_TICK_INTERVAL - FAKE_TICK_OVERFLOW_OFFSET - 1)


static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#include "azure_c_shared_utility/tickcounter.h"

#define ENABLE_MOCKS

#include "freertos/FreeRTOS.h"

#include "azure_c_shared_utility/gballoc.h"

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static int tickcounter_ms_t_Compare(tickcounter_ms_t left, tickcounter_ms_t right)
{
    return left != right;
}

static void tickcounter_ms_t_ToString(char* string, size_t bufferSize, tickcounter_ms_t val)
{
    (void)snprintf(string, bufferSize, "%llu", (unsigned long long)val);
}

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(tickcounter_freertos_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    REGISTER_UMOCK_ALIAS_TYPE(TICK_COUNTER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);
    REGISTER_UMOCK_ALIAS_TYPE(tickcounter_ms_t, unsigned long long);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
}

TEST_SUITE_CLEANUP(suite_cleanup)
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
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* Tests_SRS_TICKCOUNTER_FREERTOS_30_004: [ If allocation of the internally-defined TICK_COUNTER_INSTANCE structure fails, tickcounter_create shall return NULL. (Initialization failure is not possible for FreeRTOS.) ] */
TEST_FUNCTION(tickcounter_freertos_create_fails)
{
    ///arrange
    TICK_COUNTER_HANDLE tickHandle;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .IgnoreArgument(1)
        .SetReturn((void*)NULL);

    ///act
    tickHandle = tickcounter_create();

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(tickHandle);
}

#define FAKE_TICK_NO_OVERFLOW 333

/* Tests_SRS_TICKCOUNTER_FREERTOS_30_003: [ tickcounter_create shall allocate and initialize an internally-defined TICK_COUNTER_INSTANCE structure and return its pointer on success. ] */
TEST_FUNCTION(tickcounter_freertos_create_succeed)
{
    ///arrange
    TICK_COUNTER_HANDLE tickHandle;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xTaskGetTickCount())
        .SetReturn(FAKE_TICK_NO_OVERFLOW);

    ///act
    tickHandle = tickcounter_create();

    ///assert
    ASSERT_IS_NOT_NULL(tickHandle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    /// cleanup
    tickcounter_destroy(tickHandle);
}

/* Tests_SRS_TICKCOUNTER_FREERTOS_30_006: [ If the tick_counter parameter is NULL, tickcounter_destroy shall do nothing. ] */
TEST_FUNCTION(tickcounter_freertos_destroy_tick_counter_NULL_succeed)
{
    ///arrange

    ///act
    tickcounter_destroy(NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_TICKCOUNTER_FREERTOS_30_005: [ tickcounter_destroy shall delete the internally-defined TICK_COUNTER_INSTANCE structure specified by the tick_counter parameter. (This call has no failure case.) ] */
TEST_FUNCTION(tickcounter_freertos_destroy_succeed)
{
    ///arrange
    TICK_COUNTER_HANDLE tickHandle = tickcounter_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);

    ///act
    tickcounter_destroy(tickHandle);

    ///assert
    ASSERT_IS_NOT_NULL(tickHandle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_TICKCOUNTER_FREERTOS_30_007: [ If the tick_counter parameter is NULL, tickcounter_get_current_ms shall return a non-zero value to indicate error. ] */
TEST_FUNCTION(tickcounter_freertos_get_current_ms_tick_counter_NULL_fail)
{
    ///arrange
    tickcounter_ms_t current_ms = 0;

    ///act
    int result = tickcounter_get_current_ms(NULL, &current_ms);

    ///assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_TICKCOUNTER_FREERTOS_30_008: [ If the current_ms parameter is NULL, tickcounter_get_current_ms shall return a non-zero value to indicate error. ] */
TEST_FUNCTION(tickcounter_freertos_get_current_ms_current_ms_NULL_fail)
{
    ///arrange
    int result;
    TICK_COUNTER_HANDLE tickHandle = tickcounter_create();
    umock_c_reset_all_calls();

    ///act
    result = tickcounter_get_current_ms(tickHandle, NULL);

    ///assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    /// cleanup
    tickcounter_destroy(tickHandle);
}

/* Tests_SRS_TICKCOUNTER_FREERTOS_30_009: [ tickcounter_get_current_ms shall set *current_ms to the number of milliseconds elapsed since the tickcounter_create call for the specified tick_counter and return 0 to indicate success (In FreeRTOS this call has no failure case.) ] */
TEST_FUNCTION(tickcounter_freertos_get_current_ms_succeed)
{
    ///arrange
    tickcounter_ms_t current_ms = 0;
    int result;
    TICK_COUNTER_HANDLE tickHandle;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xTaskGetTickCount())
        .SetReturn(FAKE_TICK_NO_OVERFLOW);
    STRICT_EXPECTED_CALL(xTaskGetTickCount())
        .SetReturn((FAKE_TICK_NO_OVERFLOW + FAKE_TICK_INTERVAL));

    ///act
    tickHandle = tickcounter_create();
    result = tickcounter_get_current_ms(tickHandle, &current_ms);

    ///assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(tickcounter_ms_t, FAKE_TICK_SCALED_INTERVAL, current_ms);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    /// clean
    tickcounter_destroy(tickHandle);
}

/* Tests_SRS_TICKCOUNTER_FREERTOS_30_010: [ If the FreeRTOS call xTaskGetTickCount experiences a single overflow between the calls to tickcounter_create and tickcounter_get_current_ms, the tickcounter_get_current_ms call shall still return the correct interval. ] */
TEST_FUNCTION(tickcounter_freertos_get_current_ms_succeed_despite_overflow)
{
    ///arrange
    TICK_COUNTER_HANDLE tickHandle;
    tickcounter_ms_t current_ms = 0;
    int result;

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xTaskGetTickCount())
        .SetReturn(FAKE_TICK_BEFORE_OVERFLOW);
    STRICT_EXPECTED_CALL(xTaskGetTickCount())
        .SetReturn((FAKE_TICK_AFTER_OVERFLOW));

    tickHandle = tickcounter_create();

    ///act
    result = tickcounter_get_current_ms(tickHandle, &current_ms);

    ///assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(tickcounter_ms_t, FAKE_TICK_SCALED_INTERVAL, current_ms);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    /// clean
    tickcounter_destroy(tickHandle);
}

END_TEST_SUITE(tickcounter_freertos_unittests)
