// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdint.h>

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#include <stdbool.h>
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

#include "azure_macro_utils/macro_utils.h"

#include "dns_resolver.h"

/**
 * The gballoc.h will replace the malloc, free, and realloc by the my_gballoc functions, in this case,
 *    if you define these mock functions after include the gballoc.h, you will create an infinity recursion,
 *    so, places the my_gballoc functions before the #include "azure_c_shared_utility/gballoc.h"
 */
void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

void* my_gballoc_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#define ENABLE_MOCKS

#include "socket_async_os.h"
#include "azure_c_shared_utility/gballoc.h"
#ifdef __cplusplus
extern "C" {
#endif

MOCKABLE_FUNCTION(, int, getaddrinfo, const char*, node, const char*, service, const struct addrinfo*, hints, struct addrinfo**, res);

#ifdef __cplusplus
}
#endif

#undef ENABLE_MOCKS

void freeaddrinfo(struct addrinfo* ai)
{
    (void)ai;
}


#define GETADDRINFO_SUCCESS 0
#define GETADDRINFO_FAIL -1
#define FAKE_GOOD_IP_ADDR 444

struct sockaddr_in fake_good_addr;
struct addrinfo fake_addrinfo;

int my_getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res)
{
    (void)node;
    (void)service;
    (void)hints;
    fake_addrinfo.ai_next = NULL;
    fake_addrinfo.ai_family = AF_INET;
    fake_addrinfo.ai_addr = (struct sockaddr*)(&fake_good_addr);
    ((struct sockaddr_in *) fake_addrinfo.ai_addr)->sin_addr.s_addr = FAKE_GOOD_IP_ADDR;
    *res = &fake_addrinfo;
    return 0;
}

/**
 * Include the test tools.
 */
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_bool.h"
#include "umock_c/umocktypes_stdint.h"
#include "umock_c/umock_c_negative_tests.h"
#include "azure_macro_utils/macro_utils.h"



MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}


/**
 * This is necessary for the test suite, just keep as is.
 */
static TEST_MUTEX_HANDLE g_testByTest;

BEGIN_TEST_SUITE(dns_resolver_ut)

    /**
     * This is the place where we initialize the test system. Replace the test name to associate the test
     *   suite with your test cases.
     * It is called once, before start the tests.
     */
    TEST_SUITE_INITIALIZE(a)
    {
        int result;
        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        (void)umock_c_init(on_umock_c_error);

        result = umocktypes_charptr_register_types();
        ASSERT_ARE_EQUAL(int, 0, result);
        result = umocktypes_bool_register_types();
        ASSERT_ARE_EQUAL(int, 0, result);
        umocktypes_stdint_register_types();
        ASSERT_ARE_EQUAL(int, 0, result);

        REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);

        REGISTER_GLOBAL_MOCK_RETURNS(getaddrinfo, GETADDRINFO_SUCCESS, GETADDRINFO_FAIL);
        REGISTER_GLOBAL_MOCK_HOOK(getaddrinfo, my_getaddrinfo);
}

    /**
     * The test suite will call this function to cleanup your machine.
     * It is called only once, after all tests is done.
     */
    TEST_SUITE_CLEANUP(TestClassCleanup)
    {
        umock_c_deinit();

        TEST_MUTEX_DESTROY(g_testByTest);
    }

    /**
     * The test suite will call this function to prepare the machine for the new test.
     * It is called before execute each test.
     */
    TEST_FUNCTION_INITIALIZE(initialize)
    {
        if (TEST_MUTEX_ACQUIRE(g_testByTest))
        {
            ASSERT_FAIL("Could not acquire test serialization mutex.");
        }

        umock_c_reset_all_calls();
    }

    /**
     * The test suite will call this function to cleanup your machine for the next test.
     * It is called after execute each test.
     */
    TEST_FUNCTION_CLEANUP(cleans)
    {
        TEST_MUTEX_RELEASE(g_testByTest);
    }

    /* Tests_SRS_dns_resolver_30_022: [ If the DNS lookup process has completed, dns_resolver_is_create_complete shall return true. ]*/
    /* Tests_SRS_dns_resolver_30_032: [ If dns_resolver_is_create_complete has returned true and the lookup process has succeeded, dns_resolver_get_ipv4 shall return the discovered IPv4 address. ]*/
    /* Tests_SRS_dns_resolver_30_024: [ If dns_resolver_is_create_complete has previously returned true, dns_resolver_is_create_complete shall do nothing and return true. ]*/
    TEST_FUNCTION(dns_resolver__is_complete_repeated_call__succeeds)
    {
        ///arrange
        DNSRESOLVER_HANDLE dns = dns_resolver_create("fake.com", NULL);
        // We're calling this twice
        bool result = dns_resolver_is_lookup_complete(dns);
        uint32_t ipv4 = dns_resolver_get_ipv4(dns);
        ASSERT_ARE_EQUAL(uint32_t, FAKE_GOOD_IP_ADDR, ipv4, "Unexpected IP");
        ASSERT_IS_TRUE(result, "Unexpected non-completion");
        umock_c_reset_all_calls();

        ///act
        result = dns_resolver_is_lookup_complete(dns);
        ipv4 = dns_resolver_get_ipv4(dns);

        ///assert
        ASSERT_IS_TRUE(result, "Unexpected non-completion");
        ASSERT_ARE_EQUAL(uint32_t, FAKE_GOOD_IP_ADDR, ipv4, "Unexpected IP");
        // Verify it didn't do anything
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        dns_resolver_destroy(dns);
    }

    /* Tests_SRS_dns_resolver_30_023: [ If the DNS lookup process is not yet complete, dns_resolver_is_create_complete shall return false. ]*/
    TEST_FUNCTION(dns_resolver__is_complete_waiting__succeeds)
    {
        // This condition cannot be tested with the blocking immplementation of dns_resolver because the module never waits.
    }

    /* Tests_SRS_dns_resolver_30_022: [ If the DNS lookup process has completed, dns_resolver_is_create_complete shall return true. ]*/
    TEST_FUNCTION(dns_resolver__is_complete_yes__succeeds)
    {
        ///arrange
        bool result;
        DNSRESOLVER_HANDLE dns = dns_resolver_create("fake.com", NULL);
        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(getaddrinfo(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

        ///act
        result = dns_resolver_is_lookup_complete(dns);

        ///assert
        ASSERT_IS_TRUE(result, "Unexpected non-completion");

        ///cleanup
        dns_resolver_destroy(dns);
    }

    /* Tests_SRS_dns_resolver_30_032: [ If dns_resolver_is_create_complete has returned true and the lookup process has succeeded, dns_resolver_get_ipv4 shall return the discovered IPv4 address. ]*/
    TEST_FUNCTION(dns_resolver__dns_resolver_get_ipv4__succeeds)
    {
        ///arrange
        bool result;
        uint32_t ipv4;
        DNSRESOLVER_HANDLE dns = dns_resolver_create("fake.com", NULL);
        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(getaddrinfo(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        result = dns_resolver_is_lookup_complete(dns);
        ASSERT_IS_TRUE(result, "Unexpected non-completion");

        ///act
        ipv4 = dns_resolver_get_ipv4(dns);

        ///assert
        ASSERT_ARE_EQUAL(uint32_t, FAKE_GOOD_IP_ADDR, ipv4, "Unexpected IP");

        ///cleanup
        dns_resolver_destroy(dns);
    }

    /* Tests_SRS_dns_resolver_30_022: [ If the DNS lookup process has completed, dns_resolver_is_create_complete shall return true. ]*/
    TEST_FUNCTION(dns_resolver__is_complete_yes_after_failure__fails)
    {
        ///arrange
        bool result;
        DNSRESOLVER_HANDLE dns = dns_resolver_create("fake.com", NULL);
        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(getaddrinfo(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(GETADDRINFO_FAIL);

        ///act
        result = dns_resolver_is_lookup_complete(dns);

        ///assert
        ASSERT_IS_TRUE(result, "Unexpected non-completion");

        ///cleanup
        dns_resolver_destroy(dns);
    }

    /* Tests_SRS_dns_resolver_30_033: [ If dns_resolver_is_create_complete has returned true and the lookup process has failed, dns_resolver_get_ipv4 shall return 0. ]*/
    TEST_FUNCTION(dns_resolver__async_get_ipv4__fails)
    {
        ///arrange
        bool result;
        uint32_t ipv4;
        DNSRESOLVER_HANDLE dns = dns_resolver_create("fake.com", NULL);
        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(getaddrinfo(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(GETADDRINFO_FAIL);
        result = dns_resolver_is_lookup_complete(dns);
        ASSERT_IS_TRUE(result, "Unexpected non-completion");

        ///act
        ipv4 = dns_resolver_get_ipv4(dns);

        ///assert
        ASSERT_ARE_EQUAL(uint32_t, 0, ipv4, "Unexpected non-zero IP");

        ///cleanup
        dns_resolver_destroy(dns);
    }

    /* Tests_SRS_dns_resolver_30_020: [ If the dns parameter is NULL, dns_resolver_is_create_complete shall log an error and return false. ]*/
    TEST_FUNCTION(dns_resolver__is_complete_parameter_validation__fails)
    {
        ///arrange

        ///act
        bool result = dns_resolver_is_lookup_complete(NULL);

        ///assert
        ASSERT_IS_FALSE(result, "Unexpected non-zero IPv4");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_dns_resolver_30_031: [ If dns_resolver_is_create_complete has not yet returned true, dns_resolver_get_ipv4 shall log an error and return 0. ]*/
    TEST_FUNCTION(dns_resolver__get_ipv4_too_early__fails)
    {
        ///arrange
        DNSRESOLVER_HANDLE dns = dns_resolver_create("fake.com", NULL);

        ///act
        uint32_t result = dns_resolver_get_ipv4(dns);

        ///assert
        ASSERT_ARE_EQUAL(uint32_t, 0, result, "Unexpected non-zero IPv4");

        ///cleanup
        dns_resolver_destroy(dns);
    }

    /* Tests_SRS_dns_resolver_30_030: [ If the dns parameter is NULL, dns_resolver_get_ipv4 shall log an error and return 0. ]*/
    TEST_FUNCTION(dns_resolver__get_ipv4_parameter_validation__fails)
    {
        ///arrange

        ///act
        uint32_t result = dns_resolver_get_ipv4(NULL);

        ///assert
        ASSERT_ARE_EQUAL(uint32_t, 0, result, "Unexpected non-zero IPv4");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_dns_resolver_30_050: [ If the dns parameter is NULL, dns_resolver_destroy shall log an error and do nothing. ]*/
    TEST_FUNCTION(dns_resolver__destroy_parameter_validation__fails)
    {
        ///arrange

        ///act
        dns_resolver_destroy(NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_dns_resolver_30_051: [ dns_resolver_destroy shall delete all acquired resources and delete the DNSRESOLVER_HANDLE. ]*/
    TEST_FUNCTION(dns_resolver__destroy__success)
    {
        ///arrange
        DNSRESOLVER_HANDLE result = dns_resolver_create("fake.com", NULL);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));  // copy hostname
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));  // instance

        ///act
        dns_resolver_destroy(result);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_dns_resolver_30_014: [ On any failure, dns_resolver_create shall log an error and return NULL. ]*/
    TEST_FUNCTION(dns_resolver__create__success)
    {
        ///arrange
        DNSRESOLVER_HANDLE result;
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));  // copy hostname
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));  // instance

        ///act
        result = dns_resolver_create("fake.com", NULL);

        ///assert
        ASSERT_IS_NOT_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        dns_resolver_destroy(result);
    }

    /* Tests_SRS_dns_resolver_30_014: [ On any failure, dns_resolver_create shall log an error and return NULL. ]*/
    TEST_FUNCTION(dns_resolver__create_unhappy_paths__fails)
    {
        ///arrange
        unsigned int i;
        int negativeTestsInitResult = umock_c_negative_tests_init();
        ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));  // copy hostname
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));  // instance
        umock_c_negative_tests_snapshot();

        for (i = 0; i < umock_c_negative_tests_call_count(); i++)
        {
            DNSRESOLVER_HANDLE result;

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            ///act
            result = dns_resolver_create("fake.com", NULL);

            ///assert
            ASSERT_IS_NULL(result);
        }

        ///cleanup
        umock_c_negative_tests_deinit();
    }

    /* Tests_SRS_dns_resolver_30_011: [ If the hostname parameter is NULL, dns_resolver_create shall log an error and return NULL. ]*/
    TEST_FUNCTION(dns_resolver__create_parameter_validation__fails)
    {
        ///arrange

        ///act
        DNSRESOLVER_HANDLE result = dns_resolver_create(NULL, NULL);

        ///assert
        ASSERT_IS_NULL(result, "Unexpected success with NULL hostname");
    }


END_TEST_SUITE(dns_resolver_ut)
