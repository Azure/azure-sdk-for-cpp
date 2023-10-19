// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void my_gballoc_free(void* s)
{
    free(s);
}

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umock_c_negative_tests.h"

#define ENABLE_MOCKS
#define GBALLOC_H
#include "azure_c_shared_utility/vector.h"
#include "../src/vector.c"
#undef GBALLOC_H
#include "azure_c_shared_utility/gballoc.h"
#include "umock_c/umock_c_prod.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/vector.h"

MOCKABLE_FUNCTION(, void*, aCloneOption, const char*, name, const void*, value);
MOCKABLE_FUNCTION(, void, aDestroyOption, const char*, name, const void*, value);
MOCKABLE_FUNCTION(, int, aSetOption, void*, handle, const char*, name, const void*, value);

#undef ENABLE_MOCKS

#include "azure_c_shared_utility/optionhandler.h"

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

TEST_DEFINE_ENUM_TYPE(OPTIONHANDLER_RESULT, OPTIONHANDLER_RESULT_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(OPTIONHANDLER_RESULT, OPTIONHANDLER_RESULT_VALUES);

static int my_mallocAndStrcpy_s(char** destination, const char* source)
{

    size_t l = strlen(source);
    char* temp = (char*)my_gballoc_malloc(l + 1);
    (void)memcpy(temp, source, l+1);
    *destination=temp;
    return 0;
}

static void* my_aCloneOption(const char* name, const void* value)
{
    (void)name, (void)value;
    return my_gballoc_malloc(2);
}

static void my_aDestroyOption(const char* name, const void* value)
{
    (void)(name);
    my_gballoc_free((void*)value);
}


BEGIN_TEST_SUITE(optionhandler_unittests)

    TEST_SUITE_INITIALIZE(a)
    {
        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        (void)umock_c_init(on_umock_c_error);

        (void)umocktypes_charptr_register_types();

        REGISTER_UMOCK_ALIAS_TYPE(const VECTOR_HANDLE, void*);
        REGISTER_UMOCK_ALIAS_TYPE(VECTOR_HANDLE, void*);

        REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);

        REGISTER_GLOBAL_INTERFACE_HOOKS(vector);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(VECTOR_create, NULL);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(VECTOR_push_back, MU_FAILURE);

        REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_s, my_mallocAndStrcpy_s);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(mallocAndStrcpy_s, MU_FAILURE);

        REGISTER_GLOBAL_MOCK_HOOK(aCloneOption, my_aCloneOption);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(aCloneOption, NULL);

        REGISTER_GLOBAL_MOCK_FAIL_RETURN(aSetOption, MU_FAILURE);

        REGISTER_GLOBAL_MOCK_HOOK(aDestroyOption, my_aDestroyOption);
    }

    TEST_SUITE_CLEANUP(TestClassCleanup)
    {
        umock_c_deinit();

        TEST_MUTEX_DESTROY(g_testByTest);
    }

    TEST_FUNCTION_INITIALIZE(initialize)
    {
        umock_c_reset_all_calls();
    }

    TEST_FUNCTION_CLEANUP(cleans)
    {
    }

    /* OptionHandler_Create */

    /*Tests_SRS_OPTIONHANDLER_02_001: [ OptionHandler_Create shall fail and retun NULL if any parameters are NULL. ]*/
    TEST_FUNCTION(OptionHandler_Create_fails_with_NULL_cloneOption_parameter)
    {
        ///arrange

        ///act
        OPTIONHANDLER_HANDLE h = OptionHandler_Create(NULL, aDestroyOption, aSetOption);

        ///assert
        ASSERT_IS_NULL(h);

        ///cleanup
    }

    /*Tests_SRS_OPTIONHANDLER_02_001: [ OptionHandler_Create shall fail and retun NULL if any parameters are NULL. ]*/
    TEST_FUNCTION(OptionHandler_Create_fails_with_NULL_destroyOption_parameter)
    {
        ///arrange

        ///act
        OPTIONHANDLER_HANDLE h = OptionHandler_Create(aCloneOption, NULL, aSetOption);

        ///assert
        ASSERT_IS_NULL(h);

        ///cleanup
    }

    /*Tests_SRS_OPTIONHANDLER_02_001: [ OptionHandler_Create shall fail and retun NULL if any parameters are NULL. ]*/
    TEST_FUNCTION(OptionHandler_Create_fails_with_NULL_setOption_parameter)
    {
        ///arrange

        ///act
        OPTIONHANDLER_HANDLE h = OptionHandler_Create(aCloneOption, aDestroyOption, NULL);

        ///assert
        ASSERT_IS_NULL(h);

        ///cleanup
    }

    /*the following function, called "inert" - allegedly half happy and half unhappy is hared between the positive and negative test cases*/
    static void OptionHandler_Create_inert_path(void)
    {
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)) /*this is creating the handle*/
            .IgnoreArgument_size();

        STRICT_EXPECTED_CALL(VECTOR_create(IGNORED_NUM_ARG)) /*this is creating the vector that stores the options*/
            .IgnoreArgument_elementSize();
    }

    /*Tests_SRS_OPTIONHANDLER_02_002: [ OptionHandler_Create shall create an empty VECTOR that will hold pairs of const char* and void*. ]*/
    /*Tests_SRS_OPTIONHANDLER_02_003: [ If all the operations succeed then OptionHandler_Create shall succeed and return a non-NULL handle. ]*/
    TEST_FUNCTION(OptionHandler_Create_happy_path)
    {
        ///arrange
        OPTIONHANDLER_HANDLE h;
        OptionHandler_Create_inert_path(); /*in this case, it is happy*/

        ///act
        h = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);

        ///assert

        ASSERT_IS_NOT_NULL(h);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        OptionHandler_Destroy(h);
    }

    /*Tests_SRS_OPTIONHANDLER_02_004: [ Otherwise, OptionHandler_Create shall fail and return NULL. ]*/
    TEST_FUNCTION(OptionHandler_Create_unhappy_paths)
    {
        ///arrange
        size_t i;
        int negativeTestsInitResult = umock_c_negative_tests_init();
        ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

        OptionHandler_Create_inert_path(); /*in this case, it is unhappy*/

        umock_c_negative_tests_snapshot();

        for (i = 0; i < umock_c_negative_tests_call_count(); i++)
        {
            char temp_str[128];
            OPTIONHANDLER_HANDLE h;

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            ///act
            (void)sprintf(temp_str, "On failed call %lu", (unsigned long)i);

            ///act
            h = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);

            ///assert
            ASSERT_IS_NULL(h, temp_str);
        }

        ///cleanup
        umock_c_negative_tests_deinit();
    }

    /* OptionHandler_Clone */

    /* Tests_SRS_OPTIONHANDLER_01_010: [ If handler is NULL, OptionHandler_Clone shall fail and return NULL. ]*/
    TEST_FUNCTION(OptionHandler_Clone_with_NULL_handler_fails)
    {
        ///arrange
        OPTIONHANDLER_HANDLE result;

        ///act
        result = OptionHandler_Clone(NULL);

        ///assert
        ASSERT_IS_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_OPTIONHANDLER_01_001: [ OptionHandler_Clone shall clone an existing option handler instance. ]*/
    /* Tests_SRS_OPTIONHANDLER_01_002: [ On success it shall return a non-NULL handle. ]*/
    /* Tests_SRS_OPTIONHANDLER_01_003: [ OptionHandler_Clone shall allocate memory for the new option handler instance. ]*/
    /* Tests_SRS_OPTIONHANDLER_01_005: [ OptionHandler_Clone shall iterate through all the options stored by the option handler to be cloned by using VECTOR's iteration mechanism. ]*/
    TEST_FUNCTION(OptionHandler_Clone_clones_an_instance_with_no_options)
    {
        ///arrange
        OPTIONHANDLER_HANDLE source;
        OPTIONHANDLER_HANDLE result;

        source = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument_size();
        STRICT_EXPECTED_CALL(VECTOR_create(IGNORED_NUM_ARG))
            .IgnoreArgument_elementSize();
        STRICT_EXPECTED_CALL(VECTOR_size(IGNORED_PTR_ARG))
            .IgnoreArgument_handle();

        ///act
        result = OptionHandler_Clone(source);

        ///assert
        ASSERT_IS_NOT_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        OptionHandler_Destroy(source);
        OptionHandler_Destroy(result);
    }

    /* Tests_SRS_OPTIONHANDLER_01_001: [ OptionHandler_Clone shall clone an existing option handler instance. ]*/
    /* Tests_SRS_OPTIONHANDLER_01_002: [ On success it shall return a non-NULL handle. ]*/
    /* Tests_SRS_OPTIONHANDLER_01_003: [ OptionHandler_Clone shall allocate memory for the new option handler instance. ]*/
    /* Tests_SRS_OPTIONHANDLER_01_005: [ OptionHandler_Clone shall iterate through all the options stored by the option handler to be cloned by using VECTOR's iteration mechanism. ]*/
    /* Tests_SRS_OPTIONHANDLER_01_006: [ For each option the option name shall be cloned by calling mallocAndStrcpy_s. ]*/
    /* Tests_SRS_OPTIONHANDLER_01_007: [ For each option the value shall be cloned by using the cloning function associated with the source option handler handler. ]*/
    TEST_FUNCTION(OptionHandler_Clone_clones_an_instance_with_one_option)
    {
        ///arrange
        OPTIONHANDLER_HANDLE source;
        OPTIONHANDLER_HANDLE result;

        source = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);
        (void)OptionHandler_AddOption(source, "TrustedCerts", "xxx");
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument_size();
        STRICT_EXPECTED_CALL(VECTOR_create(IGNORED_NUM_ARG))
            .IgnoreArgument_elementSize();
        STRICT_EXPECTED_CALL(VECTOR_size(IGNORED_PTR_ARG))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 0))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "TrustedCerts"))
            .IgnoreArgument_destination();
        STRICT_EXPECTED_CALL(aCloneOption("TrustedCerts", IGNORED_PTR_ARG))
            .IgnoreArgument_value();
        STRICT_EXPECTED_CALL(VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
            .IgnoreArgument_handle()
            .IgnoreArgument_elements();

        ///act
        result = OptionHandler_Clone(source);

        ///assert
        ASSERT_IS_NOT_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        OptionHandler_Destroy(source);
        OptionHandler_Destroy(result);
    }

    /* Tests_SRS_OPTIONHANDLER_01_001: [ OptionHandler_Clone shall clone an existing option handler instance. ]*/
    /* Tests_SRS_OPTIONHANDLER_01_002: [ On success it shall return a non-NULL handle. ]*/
    /* Tests_SRS_OPTIONHANDLER_01_003: [ OptionHandler_Clone shall allocate memory for the new option handler instance. ]*/
    /* Tests_SRS_OPTIONHANDLER_01_005: [ OptionHandler_Clone shall iterate through all the options stored by the option handler to be cloned by using VECTOR's iteration mechanism. ]*/
    /* Tests_SRS_OPTIONHANDLER_01_006: [ For each option the option name shall be cloned by calling mallocAndStrcpy_s. ]*/
    /* Tests_SRS_OPTIONHANDLER_01_007: [ For each option the value shall be cloned by using the cloning function associated with the source option handler handler. ]*/
    TEST_FUNCTION(OptionHandler_Clone_clones_an_instance_with_2_options)
    {
        ///arrange
        OPTIONHANDLER_HANDLE source;
        OPTIONHANDLER_HANDLE result;

        source = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);
        (void)OptionHandler_AddOption(source, "TrustedCerts", "xxx");
        (void)OptionHandler_AddOption(source, "option_2", "y");
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument_size();
        STRICT_EXPECTED_CALL(VECTOR_create(IGNORED_NUM_ARG))
            .IgnoreArgument_elementSize();
        STRICT_EXPECTED_CALL(VECTOR_size(IGNORED_PTR_ARG))
            .IgnoreArgument_handle();

        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 0))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "TrustedCerts"))
            .IgnoreArgument_destination();
        STRICT_EXPECTED_CALL(aCloneOption("TrustedCerts", IGNORED_PTR_ARG))
            .IgnoreArgument_value();
        STRICT_EXPECTED_CALL(VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
            .IgnoreArgument_handle()
            .IgnoreArgument_elements();

        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 1))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "option_2"))
            .IgnoreArgument_destination();
        STRICT_EXPECTED_CALL(aCloneOption("option_2", IGNORED_PTR_ARG))
            .IgnoreArgument_value();
        STRICT_EXPECTED_CALL(VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
            .IgnoreArgument_handle()
            .IgnoreArgument_elements();

        ///act
        result = OptionHandler_Clone(source);

        ///assert
        ASSERT_IS_NOT_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        OptionHandler_Destroy(source);
        OptionHandler_Destroy(result);
    }

    /* Tests_SRS_OPTIONHANDLER_01_004: [ If allocating memory fails, OptionHandler_Clone shall return NULL. ]*/
    TEST_FUNCTION(when_allocating_memory_for_the_cloned_option_handler_fails_OptionHandler_Clone_fails)
    {
        ///arrange
        OPTIONHANDLER_HANDLE source;
        OPTIONHANDLER_HANDLE result;

        source = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);
        (void)OptionHandler_AddOption(source, "TrustedCerts", "xxx");
        (void)OptionHandler_AddOption(source, "option_2", "y");
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument_size()
            .SetReturn(NULL);

        ///act
        result = OptionHandler_Clone(source);

        ///assert
        ASSERT_IS_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        OptionHandler_Destroy(source);
    }

    /* Tests_SRS_OPTIONHANDLER_01_004: [ If allocating memory fails, OptionHandler_Clone shall return NULL. ]*/
    TEST_FUNCTION(when_creating_the_vector_fails_OptionHandler_Clone_fails)
    {
        ///arrange
        OPTIONHANDLER_HANDLE source;
        OPTIONHANDLER_HANDLE result;

        source = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);
        (void)OptionHandler_AddOption(source, "TrustedCerts", "xxx");
        (void)OptionHandler_AddOption(source, "option_2", "y");
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument_size();
        STRICT_EXPECTED_CALL(VECTOR_create(IGNORED_NUM_ARG))
            .IgnoreArgument_elementSize()
            .SetReturn(NULL);
        EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        ///act
        result = OptionHandler_Clone(source);

        ///assert
        ASSERT_IS_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        OptionHandler_Destroy(source);
    }

    /* Tests_SRS_OPTIONHANDLER_01_008: [ If cloning one of the option names fails, OptionHandler_Clone shall return NULL. ]*/
    TEST_FUNCTION(when_cloning_the_first_option_name_fails_OptionHandler_Clone_fails)
    {
        ///arrange
        OPTIONHANDLER_HANDLE source;
        OPTIONHANDLER_HANDLE result;

        source = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);
        (void)OptionHandler_AddOption(source, "TrustedCerts", "xxx");
        (void)OptionHandler_AddOption(source, "option_2", "y");
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument_size();
        STRICT_EXPECTED_CALL(VECTOR_create(IGNORED_NUM_ARG))
            .IgnoreArgument_elementSize();
        STRICT_EXPECTED_CALL(VECTOR_size(IGNORED_PTR_ARG))
            .IgnoreArgument_handle();

        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 0))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "TrustedCerts"))
            .IgnoreArgument_destination()
            .SetReturn(1);

        STRICT_EXPECTED_CALL(VECTOR_size(IGNORED_PTR_ARG))
            .IgnoreArgument_handle();
        EXPECTED_CALL(VECTOR_destroy(IGNORED_PTR_ARG));
        EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        ///act
        result = OptionHandler_Clone(source);

        ///assert
        ASSERT_IS_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        OptionHandler_Destroy(source);
    }

    /* Tests_SRS_OPTIONHANDLER_01_009: [ If cloning one of the option values fails, OptionHandler_Clone shall return NULL. ]*/
    TEST_FUNCTION(when_cloning_the_first_option_value_fails_OptionHandler_Clone_fails)
    {
        ///arrange
        OPTIONHANDLER_HANDLE source;
        OPTIONHANDLER_HANDLE result;

        source = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);
        (void)OptionHandler_AddOption(source, "TrustedCerts", "xxx");
        (void)OptionHandler_AddOption(source, "option_2", "y");
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument_size();
        STRICT_EXPECTED_CALL(VECTOR_create(IGNORED_NUM_ARG))
            .IgnoreArgument_elementSize();
        STRICT_EXPECTED_CALL(VECTOR_size(IGNORED_PTR_ARG))
            .IgnoreArgument_handle();

        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 0))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "TrustedCerts"))
            .IgnoreArgument_destination();
        STRICT_EXPECTED_CALL(aCloneOption("TrustedCerts", IGNORED_PTR_ARG))
            .IgnoreArgument_value()
            .SetReturn(NULL);

        EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(VECTOR_size(IGNORED_PTR_ARG))
            .IgnoreArgument_handle();
        EXPECTED_CALL(VECTOR_destroy(IGNORED_PTR_ARG));
        EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        ///act
        result = OptionHandler_Clone(source);

        ///assert
        ASSERT_IS_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        OptionHandler_Destroy(source);
    }

    /* Tests_SRS_OPTIONHANDLER_01_011: [ When adding one of the newly cloned options to the option handler storage vector fails, OptionHandler_Clone shall return NULL. ]*/
    TEST_FUNCTION(when_adding_the_first_cloned_option_fails_OptionHandler_Clone_fails)
    {
        ///arrange
        OPTIONHANDLER_HANDLE source;
        OPTIONHANDLER_HANDLE result;

        source = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);
        (void)OptionHandler_AddOption(source, "TrustedCerts", "xxx");
        (void)OptionHandler_AddOption(source, "option_2", "y");
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument_size();
        STRICT_EXPECTED_CALL(VECTOR_create(IGNORED_NUM_ARG))
            .IgnoreArgument_elementSize();
        STRICT_EXPECTED_CALL(VECTOR_size(IGNORED_PTR_ARG))
            .IgnoreArgument_handle();

        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 0))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "TrustedCerts"))
            .IgnoreArgument_destination();
        STRICT_EXPECTED_CALL(aCloneOption("TrustedCerts", IGNORED_PTR_ARG))
            .IgnoreArgument_value();
        STRICT_EXPECTED_CALL(VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
            .IgnoreArgument_handle()
            .IgnoreArgument_elements()
            .SetReturn(1);

        EXPECTED_CALL(aDestroyOption("TrustedCerts", IGNORED_PTR_ARG));
        EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(VECTOR_size(IGNORED_PTR_ARG))
            .IgnoreArgument_handle();
        EXPECTED_CALL(VECTOR_destroy(IGNORED_PTR_ARG));
        EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        ///act
        result = OptionHandler_Clone(source);

        ///assert
        ASSERT_IS_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        OptionHandler_Destroy(source);
    }

    /* Tests_SRS_OPTIONHANDLER_01_008: [ If cloning one of the option names fails, OptionHandler_Clone shall return NULL. ]*/
    TEST_FUNCTION(when_cloning_the_2nd_option_name_fails_OptionHandler_Clone_fails)
    {
        ///arrange
        OPTIONHANDLER_HANDLE source;
        OPTIONHANDLER_HANDLE result;

        source = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);
        (void)OptionHandler_AddOption(source, "TrustedCerts", "xxx");
        (void)OptionHandler_AddOption(source, "option_2", "y");
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument_size();
        STRICT_EXPECTED_CALL(VECTOR_create(IGNORED_NUM_ARG))
            .IgnoreArgument_elementSize();
        STRICT_EXPECTED_CALL(VECTOR_size(IGNORED_PTR_ARG))
            .IgnoreArgument_handle();

        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 0))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "TrustedCerts"))
            .IgnoreArgument_destination();
        STRICT_EXPECTED_CALL(aCloneOption("TrustedCerts", IGNORED_PTR_ARG))
            .IgnoreArgument_value();
        STRICT_EXPECTED_CALL(VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
            .IgnoreArgument_handle()
            .IgnoreArgument_elements();

        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 1))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "option_2"))
            .IgnoreArgument_destination()
            .SetReturn(1);

        STRICT_EXPECTED_CALL(VECTOR_size(IGNORED_PTR_ARG))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 0))
            .IgnoreArgument_handle();
        EXPECTED_CALL(aDestroyOption("TrustedCerts", IGNORED_PTR_ARG));
        EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
        EXPECTED_CALL(VECTOR_destroy(IGNORED_PTR_ARG));
        EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        ///act
        result = OptionHandler_Clone(source);

        ///assert
        ASSERT_IS_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        OptionHandler_Destroy(source);
    }

    /* Tests_SRS_OPTIONHANDLER_01_009: [ If cloning one of the option values fails, OptionHandler_Clone shall return NULL. ]*/
    TEST_FUNCTION(when_cloning_the_2nd_option_value_fails_OptionHandler_Clone_fails)
    {
        ///arrange
        OPTIONHANDLER_HANDLE source;
        OPTIONHANDLER_HANDLE result;

        source = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);
        (void)OptionHandler_AddOption(source, "TrustedCerts", "xxx");
        (void)OptionHandler_AddOption(source, "option_2", "y");
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument_size();
        STRICT_EXPECTED_CALL(VECTOR_create(IGNORED_NUM_ARG))
            .IgnoreArgument_elementSize();
        STRICT_EXPECTED_CALL(VECTOR_size(IGNORED_PTR_ARG))
            .IgnoreArgument_handle();

        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 0))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "TrustedCerts"))
            .IgnoreArgument_destination();
        STRICT_EXPECTED_CALL(aCloneOption("TrustedCerts", IGNORED_PTR_ARG))
            .IgnoreArgument_value();
        STRICT_EXPECTED_CALL(VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
            .IgnoreArgument_handle()
            .IgnoreArgument_elements();

        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 1))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "option_2"))
            .IgnoreArgument_destination();
        STRICT_EXPECTED_CALL(aCloneOption("option_2", IGNORED_PTR_ARG))
            .IgnoreArgument_value()
            .SetReturn(NULL);

        EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(VECTOR_size(IGNORED_PTR_ARG))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 0))
            .IgnoreArgument_handle();
        EXPECTED_CALL(aDestroyOption("TrustedCerts", IGNORED_PTR_ARG));
        EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
        EXPECTED_CALL(VECTOR_destroy(IGNORED_PTR_ARG));
        EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        ///act
        result = OptionHandler_Clone(source);

        ///assert
        ASSERT_IS_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        OptionHandler_Destroy(source);
    }

    /* Tests_SRS_OPTIONHANDLER_01_011: [ When adding one of the newly cloned options to the option handler storage vector fails, OptionHandler_Clone shall return NULL. ]*/
    TEST_FUNCTION(when_adding_the_2nd_cloned_option_fails_OptionHandler_Clone_fails)
    {
        ///arrange
        OPTIONHANDLER_HANDLE source;
        OPTIONHANDLER_HANDLE result;

        source = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);
        (void)OptionHandler_AddOption(source, "TrustedCerts", "xxx");
        (void)OptionHandler_AddOption(source, "option_2", "y");
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
            .IgnoreArgument_size();
        STRICT_EXPECTED_CALL(VECTOR_create(IGNORED_NUM_ARG))
            .IgnoreArgument_elementSize();
        STRICT_EXPECTED_CALL(VECTOR_size(IGNORED_PTR_ARG))
            .IgnoreArgument_handle();

        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 0))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "TrustedCerts"))
            .IgnoreArgument_destination();
        STRICT_EXPECTED_CALL(aCloneOption("TrustedCerts", IGNORED_PTR_ARG))
            .IgnoreArgument_value();
        STRICT_EXPECTED_CALL(VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
            .IgnoreArgument_handle()
            .IgnoreArgument_elements();

        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 1))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "option_2"))
            .IgnoreArgument_destination();
        STRICT_EXPECTED_CALL(aCloneOption("option_2", IGNORED_PTR_ARG))
            .IgnoreArgument_value();
        STRICT_EXPECTED_CALL(VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
            .IgnoreArgument_handle()
            .IgnoreArgument_elements()
            .SetReturn(1);

        EXPECTED_CALL(aDestroyOption("option_2", IGNORED_PTR_ARG));
        EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(VECTOR_size(IGNORED_PTR_ARG))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 0))
            .IgnoreArgument_handle();
        EXPECTED_CALL(aDestroyOption("TrustedCerts", IGNORED_PTR_ARG));
        EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
        EXPECTED_CALL(VECTOR_destroy(IGNORED_PTR_ARG));
        EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

        ///act
        result = OptionHandler_Clone(source);

        ///assert
        ASSERT_IS_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        OptionHandler_Destroy(source);
    }

    /* OptionHandler_AddOption */

    /*Tests_SRS_OPTIONHANDLER_02_005: [ OptionHandler_AddOption shall fail and return OPTIONHANDLER_INVALIDARG if any parameter is NULL. ]*/
    TEST_FUNCTION(OptionHandler_AddOption_with_NULL_handle_fails)
    {
        ///arrange

        ///act
        OPTIONHANDLER_RESULT result = OptionHandler_AddOption(NULL, "name", "some value");

        ///assert
        ASSERT_ARE_EQUAL(OPTIONHANDLER_RESULT, OPTIONHANDLER_INVALIDARG, result);

        ///cleanup
    }

    /*Tests_SRS_OPTIONHANDLER_02_005: [ OptionHandler_AddOption shall fail and return OPTIONHANDLER_INVALIDARG if any parameter is NULL. ]*/
    TEST_FUNCTION(OptionHandler_AddOption_with_NULL_name_fails)
    {
        ///arrange
        OPTIONHANDLER_HANDLE handle = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);

        ///act
        OPTIONHANDLER_RESULT result = OptionHandler_AddOption(handle, NULL, "some value");

        ///assert
        ASSERT_ARE_EQUAL(OPTIONHANDLER_RESULT, OPTIONHANDLER_INVALIDARG, result);

        ///cleanup
        OptionHandler_Destroy(handle);
    }

    /*Tests_SRS_OPTIONHANDLER_02_005: [ OptionHandler_AddOption shall fail and return OPTIONHANDLER_INVALIDARG if any parameter is NULL. ]*/
    TEST_FUNCTION(OptionHandler_AddOption_with_NULL_value_fails)
    {
        ///arrange
        OPTIONHANDLER_HANDLE handle = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);

        ///act
        OPTIONHANDLER_RESULT result = OptionHandler_AddOption(handle, "name", NULL);

        ///assert
        ASSERT_ARE_EQUAL(OPTIONHANDLER_RESULT, OPTIONHANDLER_INVALIDARG, result);

        ///cleanup
        OptionHandler_Destroy(handle);
    }

    void OptionHandler_AddOption_inert_path(void* value)
    {
        STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, "name"))
            .IgnoreArgument_destination();
        STRICT_EXPECTED_CALL(aCloneOption("name", value))
            .IgnoreAllArguments();
        STRICT_EXPECTED_CALL(VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
            .IgnoreArgument_handle()
            .IgnoreArgument_elements();
    }

    /*Tests_SRS_OPTIONHANDLER_02_006: [ OptionHandler_AddOption shall call pfCloneOption passing name and value. ]*/
    /*Tests_SRS_OPTIONHANDLER_02_007: [ OptionHandler_AddOption shall use VECTOR APIs to save the name and the newly created clone of value. ]*/
    /*Tests_SRS_OPTIONHANDLER_02_008: [ If all the operations succed then OptionHandler_AddOption shall succeed and return OPTIONHANDLER_OK. ]*/
    TEST_FUNCTION(OptionHandler_AddOption_happy_path)
    {
        ///arrange
        OPTIONHANDLER_RESULT result;
        OPTIONHANDLER_HANDLE handle = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);

        void* value = "value";
        OptionHandler_AddOption_inert_path(value);

        ///act
        result = OptionHandler_AddOption(handle, "name", value);

        ///assert
        ASSERT_ARE_EQUAL(OPTIONHANDLER_RESULT, OPTIONHANDLER_OK, result);

        ///cleanup
        OptionHandler_Destroy(handle);
    }

    /*Tests_SRS_OPTIONHANDLER_02_009: [ Otherwise, OptionHandler_AddOption shall succeed and return OPTIONHANDLER_ERROR. ]*/
    /*Tests_SRS_OPTIONHANDLER_02_008: [ If all the operations succed then OptionHandler_AddOption shall succeed and return OPTIONHANDLER_OK. ]*/
    TEST_FUNCTION(OptionHandler_AddOption_unhappy_path)
    {
        ///arrange
        void* value = "value";
        size_t i;
        OPTIONHANDLER_HANDLE handle;
        int negativeTestsInitResult = umock_c_negative_tests_init();
        ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

        handle = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);
        umock_c_reset_all_calls();

        OptionHandler_AddOption_inert_path(value);

        umock_c_negative_tests_snapshot();

        for (i = 0; i < umock_c_negative_tests_call_count(); i++)
        {
            char temp_str[128];
            OPTIONHANDLER_RESULT result;

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            ///act
            (void)sprintf(temp_str, "On failed call %lu", (unsigned long)i);

            ///act
            result = OptionHandler_AddOption(handle, "name", value);

            ///assert
            ASSERT_ARE_EQUAL(OPTIONHANDLER_RESULT, OPTIONHANDLER_ERROR, result, temp_str);
        }

        ///cleanup
        OptionHandler_Destroy(handle);
        umock_c_negative_tests_deinit();
    }

    /* OptionHandler_FeedOptions */

    /*Tests_SRS_OPTIONHANDLER_02_010: [ OptionHandler_FeedOptions shall fail and return OPTIONHANDLER_INVALIDARG if any argument is NULL. ]*/
    TEST_FUNCTION(OptionHandler_FeedOptions_with_NULL_handle_fails)
    {
        ///arrange

        ///act
        OPTIONHANDLER_RESULT result = OptionHandler_FeedOptions(NULL, "something non NULL");

        ///assert
        ASSERT_ARE_EQUAL(OPTIONHANDLER_RESULT, OPTIONHANDLER_INVALIDARG, result);

        ///cleanup
    }

    /*Tests_SRS_OPTIONHANDLER_02_010: [ OptionHandler_FeedOptions shall fail and return OPTIONHANDLER_INVALIDARG if any argument is NULL. ]*/
    TEST_FUNCTION(OptionHandler_FeedOptions_with_NULL_destinationhandle_fails)
    {
        ///arrange
        OPTIONHANDLER_HANDLE handle = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);

        ///act
        OPTIONHANDLER_RESULT result = OptionHandler_FeedOptions(handle, NULL);

        ///assert
        ASSERT_ARE_EQUAL(OPTIONHANDLER_RESULT, OPTIONHANDLER_INVALIDARG, result);

        ///cleanup
        OptionHandler_Destroy(handle);
    }

    /*Tests_SRS_OPTIONHANDLER_02_011: [ Otherwise, OptionHandler_FeedOptions shall use VECTOR's iteration mechanisms to retrieve pairs of name, value (const char* and void*). ]*/
    /*Tests_SRS_OPTIONHANDLER_02_012: [ OptionHandler_FeedOptions shall call for every pair of name,value setOption passing destinationHandle, name and value. ]*/
    /*Tests_SRS_OPTIONHANDLER_02_013: [ If all the operations succeed then OptionHandler_FeedOptions shall succeed and return OPTIONHANDLER_OK. ]*/
    TEST_FUNCTION(OptionHandler_FeedOptions_with_0_saved_options_feeds_0_succeeds)
    {
        ///arrange
        OPTIONHANDLER_RESULT result;
        OPTIONHANDLER_HANDLE handle = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(VECTOR_size(IGNORED_PTR_ARG)) /*will return 0, so nothing else happens*/
            .IgnoreArgument_handle();

        ///act
        result = OptionHandler_FeedOptions(handle, (void*)42);

        ///assert
        ASSERT_ARE_EQUAL(OPTIONHANDLER_RESULT, OPTIONHANDLER_OK, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        OptionHandler_Destroy(handle);
    }

    static void OptionHandler_FeedOptions_with_1_saved_options_feeds_1_inert_path(void)
    {
        STRICT_EXPECTED_CALL(VECTOR_size(IGNORED_PTR_ARG)) /*will return 0, so nothing else happens*/
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 0))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(aSetOption((void*)42, "a", IGNORED_PTR_ARG))
            .IgnoreArgument_value();
    }

    /*Tests_SRS_OPTIONHANDLER_02_011: [ Otherwise, OptionHandler_FeedOptions shall use VECTOR's iteration mechanisms to retrieve pairs of name, value (const char* and void*). ]*/
    /*Tests_SRS_OPTIONHANDLER_02_012: [ OptionHandler_FeedOptions shall call for every pair of name,value setOption passing destinationHandle, name and value. ]*/
    /*Tests_SRS_OPTIONHANDLER_02_013: [ If all the operations succeed then OptionHandler_FeedOptions shall succeed and return OPTIONHANDLER_OK. ]*/
    TEST_FUNCTION(OptionHandler_FeedOptions_with_1_saved_options_feeds_1_happypath)
    {
        ///arrange
        OPTIONHANDLER_RESULT result;
        OPTIONHANDLER_HANDLE handle = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);
        (void)OptionHandler_AddOption(handle, "a", "b");
        umock_c_reset_all_calls();

        OptionHandler_FeedOptions_with_1_saved_options_feeds_1_inert_path();

        ///act
        result = OptionHandler_FeedOptions(handle, (void*)42);

        ///assert
        ASSERT_ARE_EQUAL(OPTIONHANDLER_RESULT, OPTIONHANDLER_OK, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        OptionHandler_Destroy(handle);
    }

    /*Tests_SRS_OPTIONHANDLER_02_014: [ Otherwise, OptionHandler_FeedOptions shall fail and return OPTIONHANDLER_ERROR. ]*/
    TEST_FUNCTION(OptionHandler_FeedOptions_with_1_saved_options_feeds_1_unhappypaths)
    {
        ///arrange
        size_t i;
        size_t calls_that_cannot_fail[] =
        {
            0,
            1,
        };
        OPTIONHANDLER_HANDLE handle;

        int negativeTestsInitResult = umock_c_negative_tests_init();
        ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

        handle = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);
        (void)OptionHandler_AddOption(handle, "a", "b");
        umock_c_reset_all_calls();

        OptionHandler_FeedOptions_with_1_saved_options_feeds_1_inert_path();

        umock_c_negative_tests_snapshot();

        for (i = 0; i < umock_c_negative_tests_call_count(); i++)
        {
            char temp_str[128];
            size_t j;
            OPTIONHANDLER_RESULT result;

            for (j = 0;j < sizeof(calls_that_cannot_fail) / sizeof(calls_that_cannot_fail[0]);j++)
            {
                if (calls_that_cannot_fail[j] == i)
                {
                    break;
                }
            }

            if (j != sizeof(calls_that_cannot_fail) / sizeof(calls_that_cannot_fail[0]))
            {
                continue; //just go to the next call that might be failed.
            }

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            ///act
            (void)sprintf(temp_str, "On failed call %lu", (unsigned long)i);

            ///act
            result = OptionHandler_FeedOptions(handle, (void*)42);

            ///assert
            ASSERT_ARE_EQUAL(OPTIONHANDLER_RESULT, OPTIONHANDLER_ERROR, result, temp_str);

        }

        ///cleanup
        OptionHandler_Destroy(handle);
        umock_c_negative_tests_deinit();
    }

    static void OptionHandler_FeedOptions_with_2_saved_options_feeds_2_inert_path(void)
    {
        STRICT_EXPECTED_CALL(VECTOR_size(IGNORED_PTR_ARG)) /*will return 0, so nothing else happens*/
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 0))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(aSetOption((void*)42, "a", IGNORED_PTR_ARG))
            .IgnoreArgument_value();

        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 1))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(aSetOption((void*)42, "c", IGNORED_PTR_ARG))
            .IgnoreArgument_value();
    }

    /*Tests_SRS_OPTIONHANDLER_02_011: [ Otherwise, OptionHandler_FeedOptions shall use VECTOR's iteration mechanisms to retrieve pairs of name, value (const char* and void*). ]*/
    /*Tests_SRS_OPTIONHANDLER_02_012: [ OptionHandler_FeedOptions shall call for every pair of name,value setOption passing destinationHandle, name and value. ]*/
    /*Tests_SRS_OPTIONHANDLER_02_013: [ If all the operations succeed then OptionHandler_FeedOptions shall succeed and return OPTIONHANDLER_OK. ]*/
    TEST_FUNCTION(OptionHandler_FeedOptions_with_2_saved_options_feeds_2_happypath)
    {
        ///arrange
        OPTIONHANDLER_HANDLE handle = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);
        OPTIONHANDLER_RESULT result;

        (void)OptionHandler_AddOption(handle, "a", "b");
        (void)OptionHandler_AddOption(handle, "c", "b2");
        umock_c_reset_all_calls();

        OptionHandler_FeedOptions_with_2_saved_options_feeds_2_inert_path();

        ///act
        result = OptionHandler_FeedOptions(handle, (void*)42);

        ///assert
        ASSERT_ARE_EQUAL(OPTIONHANDLER_RESULT, OPTIONHANDLER_OK, result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        OptionHandler_Destroy(handle);
    }

    /*Tests_SRS_OPTIONHANDLER_02_014: [ Otherwise, OptionHandler_FeedOptions shall fail and return OPTIONHANDLER_ERROR. ]*/
    TEST_FUNCTION(OptionHandler_FeedOptions_with_2_saved_options_feeds_2_unhappypaths)
    {
        ///arrange
        OPTIONHANDLER_HANDLE handle;
        size_t calls_that_cannot_fail[] =
        {
            0,
            1,
            3,
        };
        size_t i;
        int negativeTestsInitResult = umock_c_negative_tests_init();
        ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

        handle = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);
        (void)OptionHandler_AddOption(handle, "a", "b");
        (void)OptionHandler_AddOption(handle, "c", "b2");
        umock_c_reset_all_calls();

        OptionHandler_FeedOptions_with_2_saved_options_feeds_2_inert_path();

        umock_c_negative_tests_snapshot();

        for (i = 0; i < umock_c_negative_tests_call_count(); i++)
        {
            char temp_str[128];
            size_t j;
            OPTIONHANDLER_RESULT result;

            for (j = 0;j < sizeof(calls_that_cannot_fail) / sizeof(calls_that_cannot_fail[0]);j++)
            {
                if (calls_that_cannot_fail[j] == i)
                {
                    break;
                }
            }

            if (j != sizeof(calls_that_cannot_fail) / sizeof(calls_that_cannot_fail[0]))
            {
                continue; //just go to the next call that might be failed.
            }

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            ///act
            (void)sprintf(temp_str, "On failed call %lu", (unsigned long)i);

            ///act
            result = OptionHandler_FeedOptions(handle, (void*)42);

            ///assert
            ASSERT_ARE_EQUAL(OPTIONHANDLER_RESULT, OPTIONHANDLER_ERROR, result, temp_str);
        }

        ///cleanup
        OptionHandler_Destroy(handle);
        umock_c_negative_tests_deinit();
    }

    /* OptionHandler_Destroy */

    /*Tests_SRS_OPTIONHANDLER_02_015: [ OptionHandler_Destroy shall do nothing if parameter handle is NULL. ]*/
    TEST_FUNCTION(OptionHandler_Destroy_with_NULL_does_nothing)
    {
        ///arrange

        ///act
        OptionHandler_Destroy(NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
    }

    /*Tests_SRS_OPTIONHANDLER_02_016: [ Otherwise, OptionHandler_Destroy shall free all used resources. ]*/
    TEST_FUNCTION(OptionHandler_Destroy_with_non_NULL_handle_succeeds)
    {
        ///arrange
        OPTIONHANDLER_HANDLE handle = OptionHandler_Create(aCloneOption, aDestroyOption, aSetOption);
        (void)OptionHandler_AddOption(handle, "a", "b");
        (void)OptionHandler_AddOption(handle, "c", "b2");
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(VECTOR_size(IGNORED_PTR_ARG))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 0))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(aDestroyOption("a", IGNORED_PTR_ARG))
            .IgnoreArgument_value();
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument_ptr();

        STRICT_EXPECTED_CALL(VECTOR_element(IGNORED_PTR_ARG, 1))
            .IgnoreArgument_handle();
        STRICT_EXPECTED_CALL(aDestroyOption("c", IGNORED_PTR_ARG))
            .IgnoreArgument_value();
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument_ptr();

        STRICT_EXPECTED_CALL(VECTOR_destroy(IGNORED_PTR_ARG))
            .IgnoreArgument_handle();

        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument_ptr();

        ///act
        OptionHandler_Destroy(handle);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
    }

END_TEST_SUITE(optionhandler_unittests)


