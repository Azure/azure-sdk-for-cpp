// Copyright(C) Microsoft Corporation.All rights reserved.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#include "azure_macro_utils/macro_utils.h"

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void my_gballoc_free(void* s)
{
    free(s);
}

#include "testrunnerswitcher.h"
#include "windows.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_stdint.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes.h"
#include "umock_c/umock_c_negative_tests.h"
#include "umock_c/umocktypes_bool.h"
#include "azure_c_shared_utility/timer.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#undef ENABLE_MOCKS

#ifdef __cplusplus
extern "C" {
#endif
   MOCKABLE_FUNCTION(, BOOLEAN, mocked_QueryPerformanceCounter, LARGE_INTEGER*, lpPerformanceCount)
   MOCKABLE_FUNCTION(, BOOLEAN, mocked_QueryPerformanceFrequency, LARGE_INTEGER*, lpFrequency)
#ifdef __cplusplus
}
#endif
static TEST_MUTEX_HANDLE test_serialize_mutex;


MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(timer_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    result = umock_c_init(on_umock_c_error);
    ASSERT_ARE_EQUAL(int, 0, result, "umock_c_init");

    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result, "umocktypes_stdint_register_types failed");

    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result, "umocktypes_charptr_register_types failed");

    result = umocktypes_bool_register_types();
    ASSERT_ARE_EQUAL(int, 0, result, "umocktypes_bool_register_types failed");

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    umock_c_negative_tests_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
}

TEST_FUNCTION_INITIALIZE(init)
{
    if (TEST_MUTEX_ACQUIRE(test_serialize_mutex))
    {
        ASSERT_FAIL("Could not acquire test serialization mutex.");
    }

    umock_c_reset_all_calls();
}

/* timer_create */
TEST_FUNCTION(timer_create_malloc_fails)
{
    //arrange
    STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    //act
    TIMER_HANDLE timer = timer_create();

    //assert
    ASSERT_IS_NULL(timer, "timer_create failed.");
}

static void test_timer_create_success_expectations(void)
{
    LARGE_INTEGER frequency, start_time;
    frequency.QuadPart = 10;
    start_time.QuadPart = 1;
    STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mocked_QueryPerformanceFrequency(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_lpFrequency(&frequency, sizeof(frequency));
    STRICT_EXPECTED_CALL(mocked_QueryPerformanceCounter(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_lpPerformanceCount(&start_time, sizeof(start_time));
}

TEST_FUNCTION(timer_create_succeeds)
{
    //arrange
    test_timer_create_success_expectations();

    //act
    TIMER_HANDLE timer = timer_create();

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(timer, "timer_create failed.");

    //cleanup
    timer_destroy(timer);
}

/* timer_start*/
TEST_FUNCTION(timer_start_returns_if_timer_is_null)
{
    //arrange
    //act
     timer_start(NULL);

    //assert
     ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(timer_start_succeeds)
{
    //arrange
    LARGE_INTEGER stop_time;
    stop_time.QuadPart = 100;
    test_timer_create_success_expectations();
    TIMER_HANDLE timer = timer_create();
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(mocked_QueryPerformanceCounter(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_lpPerformanceCount(&stop_time, sizeof(stop_time));

    //act
    timer_start(timer);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    timer_destroy(timer);
}

/*timer_get_elapsed*/
TEST_FUNCTION(timer_get_elapsed_fails_if_timer_is_null)
{
    //arrange
    //act
    double start = timer_get_elapsed(NULL);

    //assert
    ASSERT_ARE_EQUAL(double, -1, start);
}

TEST_FUNCTION(timer_get_elapsed_success)
{
    //arrange
    LARGE_INTEGER stop_time;
    stop_time.QuadPart = 100;
    test_timer_create_success_expectations();
    TIMER_HANDLE timer = timer_create();
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(mocked_QueryPerformanceCounter(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_lpPerformanceCount(&stop_time, sizeof(stop_time));
    
    //act
    double elapsed_time = timer_get_elapsed(timer);

    //assert
    ASSERT_ARE_EQUAL(double, 9.9, elapsed_time);

    //cleanup
    timer_destroy(timer);
}

/*timer_get_elapsed_ms*/
TEST_FUNCTION(timer_get_elapsed_ms_fails_if_timer_is_null)
{
    //arrange
    //act
    double elapsed_time_ms = timer_get_elapsed_ms(NULL);

    //assert
    ASSERT_ARE_EQUAL(double, -1, elapsed_time_ms);
}

TEST_FUNCTION(timer_get_elapsed_ms_success)
{
    //arrange
    LARGE_INTEGER stop_time;
    stop_time.QuadPart = 100;
    test_timer_create_success_expectations();
    TIMER_HANDLE timer = timer_create();
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(mocked_QueryPerformanceCounter(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_lpPerformanceCount(&stop_time, sizeof(stop_time));

    //act
    double elapsed_time_ms = timer_get_elapsed_ms(timer);

    //assert
    ASSERT_ARE_EQUAL(double, 9900, elapsed_time_ms);

    //cleanup
    timer_destroy(timer);
}

/*timer_destroy*/
TEST_FUNCTION(timer_destroy_returns_if_timer_is_NULL)
{
    //arrange
    //act
    timer_destroy(NULL);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(timer_destroy_frees_handle)
{
    //arrange
    test_timer_create_success_expectations();
    TIMER_HANDLE timer = timer_create();
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

    //act
    timer_destroy(timer);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION_CLEANUP(cleanup)
{
    TEST_MUTEX_RELEASE(test_serialize_mutex);
}

END_TEST_SUITE(timer_unittests)
