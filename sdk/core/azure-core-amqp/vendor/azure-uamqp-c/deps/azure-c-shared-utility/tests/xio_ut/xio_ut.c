// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"

static unsigned int g_fail_alloc_calls;

static void* my_gballoc_malloc(size_t size)
{
    void* result = NULL;
    if (g_fail_alloc_calls == 0)
    {
        result = malloc(size);
    }
    return result;
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umock_c_negative_tests.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/optionhandler.h"
#undef ENABLE_MOCKS

#include "azure_c_shared_utility/xio.h"
static CONCRETE_IO_HANDLE TEST_CONCRETE_IO_HANDLE = (CONCRETE_IO_HANDLE)0x4242;

#define ENABLE_MOCKS
MOCK_FUNCTION_WITH_CODE(, CONCRETE_IO_HANDLE, test_xio_create, void*, xio_create_parameters)
MOCK_FUNCTION_END(TEST_CONCRETE_IO_HANDLE)
MOCK_FUNCTION_WITH_CODE(, void, test_xio_destroy, CONCRETE_IO_HANDLE, handle)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, int, test_xio_open, CONCRETE_IO_HANDLE, handle, ON_IO_OPEN_COMPLETE, on_io_open_complete, void*, on_io_open_complete_context, ON_BYTES_RECEIVED, on_bytes_received, void*, on_bytes_received_context, ON_IO_ERROR, on_io_error, void*, on_io_error_context)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(, int, test_xio_close, CONCRETE_IO_HANDLE, handle, ON_IO_CLOSE_COMPLETE, on_io_close_complete, void*, callback_context)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(, int, test_xio_send, CONCRETE_IO_HANDLE, handle, const void*, buffer, size_t, size, ON_SEND_COMPLETE, on_send_complete, void*, callback_context)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(, void, test_xio_dowork, CONCRETE_IO_HANDLE, handle)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(, int, test_xio_setoption, CONCRETE_IO_HANDLE, handle, const char*, optionName, const void*, value)
MOCK_FUNCTION_END(0)

#include "umock_c/umock_c_prod.h"
/*this function will clone an option given by name and value*/
MOCKABLE_FUNCTION(,void*, test_xio_CloneOption, const char*, name, const void*, value);

MOCKABLE_FUNCTION(, void, test_xio_DestroyOption, const char*, name, const void*, value);

MOCKABLE_FUNCTION(, OPTIONHANDLER_HANDLE, test_xio_retrieveoptions, CONCRETE_IO_HANDLE, handle);

#undef ENABLE_MOCKS

#ifdef __cplusplus
extern "C" {
#endif
    void test_on_bytes_received(void* context, const unsigned char* buffer, size_t size)
    {
        (void)context;
        (void)buffer;
        (void)size;
    }

    void test_on_io_open_complete(void* context, IO_OPEN_RESULT open_result)
    {
        (void)context;
        (void)open_result;
    }

    void test_on_io_close_complete(void* context)
    {
        (void)context;
    }

    void test_on_io_error(void* context)
    {
        (void)context;
    }

    void test_on_send_complete(void* context, IO_SEND_RESULT send_result)
    {
        (void)context;
        (void)send_result;
    }
#ifdef __cplusplus
}
#endif



const IO_INTERFACE_DESCRIPTION test_io_description =
{
    test_xio_retrieveoptions,
    test_xio_create,
    test_xio_destroy,
    test_xio_open,
    test_xio_close,
    test_xio_send,
    test_xio_dowork,
    test_xio_setoption
};

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

static OPTIONHANDLER_HANDLE my_OptionHandler_Create(pfCloneOption cloneOption, pfDestroyOption destroyOption, pfSetOption setOption)
{
    (void)cloneOption, (void)destroyOption, (void)setOption;
    return (OPTIONHANDLER_HANDLE)my_gballoc_malloc(1);
}

static OPTIONHANDLER_HANDLE my_test_xio_retrieveoptions(CONCRETE_IO_HANDLE handle)
{
    (void)(handle);
    return (OPTIONHANDLER_HANDLE)my_gballoc_malloc(2);
}

static OPTIONHANDLER_RESULT my_OptionHandler_AddOption(OPTIONHANDLER_HANDLE handle, const char* name, const void* value)
{
    (void)name, (void)handle, (void)value;
    return OPTIONHANDLER_OK;
}

static void my_OptionHandler_Destroy(OPTIONHANDLER_HANDLE handle)
{
    my_gballoc_free((void*)handle);
}


BEGIN_TEST_SUITE(xio_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_UMOCK_ALIAS_TYPE(CONCRETE_IO_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(XIO_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_SEND_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_CLOSE_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_OPEN_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_RECEIVED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_ERROR, void*);

    REGISTER_UMOCK_ALIAS_TYPE(pfCloneOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfDestroyOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfSetOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(OPTIONHANDLER_HANDLE, void*);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);

    REGISTER_GLOBAL_MOCK_HOOK(OptionHandler_Create, my_OptionHandler_Create);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(OptionHandler_Create, (OPTIONHANDLER_HANDLE)NULL);

    REGISTER_GLOBAL_MOCK_HOOK(test_xio_retrieveoptions, my_test_xio_retrieveoptions);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(test_xio_retrieveoptions, (OPTIONHANDLER_HANDLE)NULL);

    REGISTER_GLOBAL_MOCK_HOOK(OptionHandler_AddOption, my_OptionHandler_AddOption);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(OptionHandler_AddOption, OPTIONHANDLER_ERROR);

    REGISTER_GLOBAL_MOCK_HOOK(OptionHandler_Destroy, my_OptionHandler_Destroy);
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
        ASSERT_FAIL("Could not acquire test serialization mutex.");
    }
    g_fail_alloc_calls = 0;

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* xio_create */

/* Tests_SRS_XIO_01_001: [xio_create shall return on success a non-NULL handle to a new IO interface.] */
/* Tests_SRS_XIO_01_002: [In order to instantiate the concrete IO implementation the function concrete_io_create from the io_interface_description shall be called, passing the xio_create_parameters argument.] */
TEST_FUNCTION(xio_create_with_all_args_except_interface_description_NULL_succeeds)
{
    // arrange
    XIO_HANDLE result;
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_xio_create(NULL));

    // act
    result = xio_create(&test_io_description, NULL);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    xio_destroy(result);
}

/* Tests_SRS_XIO_01_002: [In order to instantiate the concrete IO implementation the function concrete_io_create from the io_interface_description shall be called, passing the xio_create_parameters argument.] */
TEST_FUNCTION(xio_create_passes_the_args_to_the_concrete_io_implementation)
{
    // arrange
    XIO_HANDLE result;
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(test_xio_create((void*)0x4243));

    // act
    result = xio_create(&test_io_description, (void*)0x4243);

    // assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    xio_destroy(result);
}

/* Tests_SRS_XIO_01_016: [If the underlying concrete_xxio_create call fails, xxio_create shall return NULL.] */
TEST_FUNCTION(when_concrete_xio_create_fails_then_xio_create_fails)
{
    // arrange
    XIO_HANDLE result;
    STRICT_EXPECTED_CALL(test_xio_create(NULL))
        .SetReturn((CONCRETE_IO_HANDLE)NULL);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    result = xio_create(&test_io_description, NULL);

    // assert
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_XIO_01_003: [If the argument io_interface_description is NULL, xio_create shall return NULL.] */
TEST_FUNCTION(when_io_interface_description_is_NULL_then_xio_create_fails)
{
    // arrange

    // act
    XIO_HANDLE result = xio_create(NULL, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_XIO_01_004: [If any io_interface_description member is NULL, xio_create shall return NULL.] */
TEST_FUNCTION(when_concrete_xio_retrieveoptions_is_NULL_then_xio_create_fails)
{
    // arrange
    const IO_INTERFACE_DESCRIPTION io_description_null =
    {
        NULL,
        test_xio_create,
        test_xio_destroy,
        test_xio_open,
        test_xio_close,
        test_xio_send,
        test_xio_dowork,
        test_xio_setoption
    };

    // act
    XIO_HANDLE result = xio_create(&io_description_null, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_XIO_01_004: [If any io_interface_description member is NULL, xio_create shall return NULL.] */
TEST_FUNCTION(when_concrete_xio_create_is_NULL_then_xio_create_fails)
{
    // arrange
    const IO_INTERFACE_DESCRIPTION io_description_null =
    {
        test_xio_retrieveoptions,
        NULL,
        test_xio_destroy,
        test_xio_open,
        test_xio_close,
        test_xio_send,
        test_xio_dowork,
        test_xio_setoption
    };

    // act
    XIO_HANDLE result = xio_create(&io_description_null, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_XIO_01_004: [If any io_interface_description member is NULL, xio_create shall return NULL.] */
TEST_FUNCTION(when_concrete_xio_destroy_is_NULL_then_xio_create_fails)
{
    // arrange
    const IO_INTERFACE_DESCRIPTION io_description_null =
    {
        test_xio_retrieveoptions,
        test_xio_create,
        NULL,
        test_xio_open,
        test_xio_close,
        test_xio_send,
        test_xio_dowork,
        test_xio_setoption
    };

    // act
    XIO_HANDLE result = xio_create(&io_description_null, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_XIO_01_004: [If any io_interface_description member is NULL, xio_create shall return NULL.] */
TEST_FUNCTION(when_concrete_xio_open_is_NULL_then_xio_create_fails)
{
    // arrange
    const IO_INTERFACE_DESCRIPTION io_description_null =
    {
        test_xio_retrieveoptions,
        test_xio_create,
        test_xio_destroy,
        NULL,
        test_xio_close,
        test_xio_send,
        test_xio_dowork,
        test_xio_setoption
    };

    // act
    XIO_HANDLE result = xio_create(&io_description_null, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_XIO_01_004: [If any io_interface_description member is NULL, xio_create shall return NULL.] */
TEST_FUNCTION(when_concrete_xio_close_is_NULL_then_xio_create_fails)
{
    // arrange
    const IO_INTERFACE_DESCRIPTION io_description_null =
    {
        test_xio_retrieveoptions,
        test_xio_create,
        test_xio_destroy,
        test_xio_open,
        NULL,
        test_xio_send,
        test_xio_dowork,
        test_xio_setoption
    };

    // act
    XIO_HANDLE result = xio_create(&io_description_null, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_XIO_01_004: [If any io_interface_description member is NULL, xio_create shall return NULL.] */
TEST_FUNCTION(when_concrete_xio_send_is_NULL_then_xio_create_fails)
{
    // arrange
    const IO_INTERFACE_DESCRIPTION io_description_null =
    {
        test_xio_retrieveoptions,
        test_xio_create,
        test_xio_destroy,
        test_xio_open,
        test_xio_close,
        NULL,
        test_xio_dowork,
        test_xio_setoption
    };

    // act
    XIO_HANDLE result = xio_create(&io_description_null, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_XIO_01_004: [If any io_interface_description member is NULL, xio_create shall return NULL.] */
TEST_FUNCTION(when_concrete_xio_dowork_is_NULL_then_xio_create_fails)
{
    // arrange
    const IO_INTERFACE_DESCRIPTION io_description_null =
    {
        test_xio_retrieveoptions,
        test_xio_create,
        test_xio_destroy,
        test_xio_open,
        test_xio_close,
        test_xio_send,
        NULL,
        test_xio_setoption
    };

    // act
    XIO_HANDLE result = xio_create(&io_description_null, NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_XIO_01_004: [If any io_interface_description member is NULL, xio_create shall return NULL.] */
TEST_FUNCTION(when_concrete_xio_setoption_is_NULL_then_xio_create_fails)
{
    // arrange
    const IO_INTERFACE_DESCRIPTION io_description_null =
    {
        test_xio_retrieveoptions,
        test_xio_create,
        test_xio_destroy,
        test_xio_open,
        test_xio_close,
        test_xio_send,
        test_xio_dowork,
        NULL
    };

    // act
    XIO_HANDLE result = xio_create(&io_description_null, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* Tests_SRS_XIO_01_017: [If allocating the memory needed for the IO interface fails then xio_create shall return NULL.] */
TEST_FUNCTION(when_allocating_memory_Fails_then_xio_create_fails)
{
    // arrange
    XIO_HANDLE result;
    g_fail_alloc_calls = 1;

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn((void*)NULL);

    // act
    result = xio_create(&test_io_description, NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);
}

/* xio_destroy */

/* Tests_SRS_XIO_01_005: [xio_destroy shall free all resources associated with the IO handle.] */
/* Tests_SRS_XIO_01_006: [xio_destroy shall also call the concrete_xio_destroy function that is member of the io_interface_description argument passed to xio_create, while passing as argument to concrete_xio_destroy the result of the underlying concrete_xio_create handle that was called as part of the xio_create call.] */
TEST_FUNCTION(xio_destroy_calls_concrete_xio_destroy_and_frees_memory)
{
    // arrange
    XIO_HANDLE handle = xio_create(&test_io_description, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_xio_destroy(TEST_CONCRETE_IO_HANDLE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    xio_destroy(handle);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_XIO_01_007: [If handle is NULL, xio_destroy shall do nothing.] */
TEST_FUNCTION(xio_destroy_with_null_handle_does_nothing)
{
    // arrange

    // act
    xio_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* xio_open */

/* Tests_SRS_XIO_01_019: SRS_XIO_01_019: [xio_open shall call the specific concrete_xio_open function specified in xio_create, passing callback function and context arguments for three events: open completed, bytes received, and IO error.] */
/* Tests_SRS_XIO_01_020: [On success, xio_open shall return 0.] */
TEST_FUNCTION(xio_open_calls_the_underlying_concrete_xio_open_and_succeeds)
{
    // arrange
    int result;
    XIO_HANDLE handle = xio_create(&test_io_description, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_xio_open(TEST_CONCRETE_IO_HANDLE, test_on_io_open_complete, (void*)1, test_on_bytes_received, (void*)2, test_on_io_error, (void*)3));

    // act
    result = xio_open(handle, test_on_io_open_complete, (void*)1, test_on_bytes_received, (void*)2, test_on_io_error, (void*)3);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    xio_destroy(handle);
}

/* Tests_SRS_XIO_01_021: [If handle is NULL, xio_open shall return a non-zero value.] */
TEST_FUNCTION(xio_open_with_NULL_handle_fails)
{
    // arrange

    // act
    int result = xio_open(NULL, test_on_io_open_complete, (void*)1, test_on_bytes_received, (void*)2, test_on_io_error, (void*)3);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_XIO_01_022: [If the underlying concrete_xio_open fails, xio_open shall return a non-zero value.] */
TEST_FUNCTION(when_the_concrete_xio_open_fails_then_xio_open_fails)
{
    // arrange
    int result;
    XIO_HANDLE handle = xio_create(&test_io_description, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_xio_open(TEST_CONCRETE_IO_HANDLE, test_on_io_open_complete, (void*)1, test_on_bytes_received, (void*)2, test_on_io_error, (void*)3))
        .SetReturn(1);

    // act
    result = xio_open(handle, test_on_io_open_complete, (void*)1, test_on_bytes_received, (void*)2, test_on_io_error, (void*)3);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    xio_destroy(handle);
}

/* xio_close */

/* Tests_SRS_XIO_01_023: [xio_close shall call the specific concrete_xio_close function specified in xio_create.] */
/* Tests_SRS_XIO_01_024: [On success, xio_close shall return 0.] */
TEST_FUNCTION(xio_close_calls_the_underlying_concrete_xio_close_and_succeeds)
{
    // arrange
    int result;
    XIO_HANDLE handle = xio_create(&test_io_description, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_xio_close(TEST_CONCRETE_IO_HANDLE, test_on_io_close_complete, (void*)0x4242));

    // act
    result = xio_close(handle, test_on_io_close_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    xio_destroy(handle);
}

/* Tests_SRS_XIO_01_025: [If handle is NULL, xio_close shall return a non-zero value.] */
TEST_FUNCTION(xio_close_with_NULL_handle_fails)
{
    // arrange

    // act
    int result = xio_close(NULL, test_on_io_close_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_XIO_01_026: [If the underlying concrete_xio_close fails, xio_close shall return a non-zero value.] */
TEST_FUNCTION(when_the_concrete_xio_close_fails_then_xio_close_fails)
{
    // arrange
    int result;
    XIO_HANDLE handle = xio_create(&test_io_description, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_xio_close(TEST_CONCRETE_IO_HANDLE, test_on_io_close_complete, (void*)0x4242))
        .SetReturn(1);

    // act
    result = xio_close(handle, test_on_io_close_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    xio_destroy(handle);
}

/* xio_send */

/* Tests_SRS_XIO_01_008: [xio_send shall pass the sequence of bytes pointed to by buffer to the concrete IO implementation specified in xio_create, by calling the concrete_xio_send function while passing down the buffer and size arguments to it.] */
/* Tests_SRS_XIO_01_009: [On success, xio_send shall return 0.] */
TEST_FUNCTION(xio_send_calls_the_underlying_concrete_xio_send_and_succeeds)
{
    // arrange
    int result;
    unsigned char send_data[] = { 0x42, 43 };
    XIO_HANDLE handle = xio_create(&test_io_description, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_xio_send(TEST_CONCRETE_IO_HANDLE, send_data, sizeof(send_data), test_on_send_complete, (void*)0x4242));

    // act
    result = xio_send(handle, send_data, sizeof(send_data), test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    xio_destroy(handle);
}

/* Tests_SRS_XIO_01_010: [If handle is NULL, xio_send shall return a non-zero value.] */
TEST_FUNCTION(xio_send_with_NULL_handle_fails)
{
    // arrange
    int result;
    unsigned char send_data[] = { 0x42, 43 };
    umock_c_reset_all_calls();

    // act
    result = xio_send(NULL, send_data, sizeof(send_data), test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_XIO_01_015: [If the underlying concrete_xio_send fails, xio_send shall return a non-zero value.] */
TEST_FUNCTION(when_the_concrete_xio_send_fails_then_xio_send_fails)
{
    // arrange
    int result;
    unsigned char send_data[] = { 0x42, 43 };
    XIO_HANDLE handle = xio_create(&test_io_description, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_xio_send(TEST_CONCRETE_IO_HANDLE, send_data, sizeof(send_data), test_on_send_complete, (void*)0x4242))
        .SetReturn(42);

    // act
    result = xio_send(handle, send_data, sizeof(send_data), test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    xio_destroy(handle);
}

/* Tests_SRS_XIO_01_011: [No error check shall be performed on buffer and size.] */
TEST_FUNCTION(xio_send_with_NULL_buffer_and_nonzero_length_passes_the_args_down_and_succeeds)
{
    // arrange
    int result;
    XIO_HANDLE handle = xio_create(&test_io_description, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_xio_send(TEST_CONCRETE_IO_HANDLE, NULL, 1, test_on_send_complete, (void*)0x4242));

    // act
    result = xio_send(handle, NULL, 1, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    xio_destroy(handle);
}

/* Tests_SRS_XIO_01_011: [No error check shall be performed on buffer and size.] */
TEST_FUNCTION(xio_send_with_NULL_buffer_and_zero_length_passes_the_args_down_and_succeeds)
{
    // arrange
    int result;
    XIO_HANDLE handle = xio_create(&test_io_description, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_xio_send(TEST_CONCRETE_IO_HANDLE, NULL, 0, test_on_send_complete, (void*)0x4242));

    // act
    result = xio_send(handle, NULL, 0, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    xio_destroy(handle);
}

/* Tests_SRS_XIO_01_011: [No error check shall be performed on buffer and size.] */
TEST_FUNCTION(xio_send_with_non_NULL_buffer_and_zero_length_passes_the_args_down_and_succeeds)
{
    // arrange
    int result;
    unsigned char send_data[] = { 0x42, 43 };
    XIO_HANDLE handle = xio_create(&test_io_description, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_xio_send(TEST_CONCRETE_IO_HANDLE, send_data, 0, test_on_send_complete, (void*)0x4242));

    // act
    result = xio_send(handle, send_data, 0, test_on_send_complete, (void*)0x4242);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    xio_destroy(handle);
}

/* xio_dowork */

/* Tests_SRS_XIO_01_012: [xio_dowork shall call the concrete IO implementation specified in xio_create, by calling the concrete_xio_dowork function.] */
TEST_FUNCTION(xio_dowork_calls_the_concrete_dowork_and_succeeds)
{
    // arrange
    XIO_HANDLE handle = xio_create(&test_io_description, NULL);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_xio_dowork(TEST_CONCRETE_IO_HANDLE));

    // act
    xio_dowork(handle);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    xio_destroy(handle);
}

/* Tests_SRS_XIO_01_018: [When the handle argument is NULL, xio_dowork shall do nothing.] */
TEST_FUNCTION(xio_dowork_with_NULL_handle_does_nothing)
{
    // arrange

    // act
    xio_dowork(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_XIO_03_030: [If the xio argument or the optionName argument is NULL, xio_setoption shall return a non-zero value.] */
TEST_FUNCTION(xio_setoption_with_NULL_handle_fails)
{
    // arrange
    int result;
    const char* optionName = "TheOptionName";
    const void* optionValue = (void*)1;

    umock_c_reset_all_calls();

    // act
    result = xio_setoption(NULL, optionName, optionValue);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

/* Tests_SRS_XIO_03_030: [If the xio argument or the optionName argument is NULL, xio_setoption shall return a non-zero value.] */
TEST_FUNCTION(xio_setoption_with_NULL_optionName_fails)
{
    // arrange
    const void* optionValue = (void*)1;
    int result;
    XIO_HANDLE handle = xio_create(&test_io_description, NULL);

    umock_c_reset_all_calls();

    // act
    result = xio_setoption(handle, NULL, optionValue);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    xio_destroy(handle);
}

/* Tests_SRS_XIO_003_028: [xio_setoption shall pass the optionName and value to the concrete IO implementation specified in xio_create by invoking the concrete_xio_setoption function.] */
/* Tests_SRS_XIO_03_029: [xio_setoption shall return 0 upon success.] */
TEST_FUNCTION(xio_setoption_with_valid_args_passes_the_args_down_and_succeeds)
{
    // arrange
    int result;
    const char* optionName = "TheOptionName";
    const void* optionValue = (void*)1;
    XIO_HANDLE handle = xio_create(&test_io_description, NULL);

    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_xio_setoption(TEST_CONCRETE_IO_HANDLE, optionName, optionValue));

    // act
    result = xio_setoption(handle, optionName, optionValue);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    xio_destroy(handle);
}

/* Tests_SRS_XIO_03_031: [If the underlying concrete_xio_setoption fails, xio_setOption shall return a non-zero value.] */
TEST_FUNCTION(xio_setoption_fails_when_concrete_xio_setoption_fails)
{
    // arrange
    int result;
    const char* optionName = "TheOptionName";
    const void* optionValue = (void*)1;
    XIO_HANDLE handle = xio_create(&test_io_description, NULL);

    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(test_xio_setoption(TEST_CONCRETE_IO_HANDLE, optionName, optionValue))
        .SetReturn(42);

    // act
    result = xio_setoption(handle, optionName, optionValue);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    xio_destroy(handle);
}

/*Tests_SRS_XIO_02_001: [ If argument xio is NULL then xio_retrieveoptions shall fail and return NULL. ]*/
TEST_FUNCTION(xio_retrieveoptions_with_NULL_xio_fails)
{
    ///arrange

    ///act
    OPTIONHANDLER_HANDLE h = xio_retrieveoptions(NULL);

    ///assert
    ASSERT_IS_NULL(h);

    ///cleanup
}

/*this function exists for the purpose of sharing code between happy and unhappy paths*/
static void xio_retrieveoptions_inert_path(void)
{
    STRICT_EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, (pfSetOption)xio_setoption));

    STRICT_EXPECTED_CALL(test_xio_retrieveoptions(IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(OptionHandler_AddOption(IGNORED_PTR_ARG, "concreteOptions", IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(OptionHandler_Destroy(IGNORED_PTR_ARG));

}

/*Tests_SRS_XIO_02_002: [ xio_retrieveoptions shall create a OPTIONHANDLER_HANDLE by calling OptionHandler_Create passing xio_setoption as setOption argument and xio_CloneOption and xio_DestroyOption for cloneOption and destroyOption. ]*/
/*Tests_SRS_XIO_02_003: [ xio_retrieveoptions shall retrieve the concrete handle's options by a call to concrete_io_retrieveoptions. ]*/
/*Tests_SRS_XIO_02_004: [ xio_retrieveoptions shall add a hardcoded option named concreteOptions having the same content as the concrete handle's options. ]*/
/*Tests_SRS_XIO_02_006: [ Otherwise, xio_retrieveoptions shall succeed and return a non-NULL handle. ]*/
TEST_FUNCTION(xio_retrieveoptions_happypath)
{
    ///arrange
    OPTIONHANDLER_HANDLE h;
    XIO_HANDLE x = xio_create(&test_io_description, NULL);
    umock_c_reset_all_calls();

    xio_retrieveoptions_inert_path();

    ///act
    h = xio_retrieveoptions(x);

    ///assert
    ASSERT_IS_NOT_NULL(h);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
    OptionHandler_Destroy(h);
    xio_destroy(x);
}

/*Tests_SRS_XIO_02_005: [ If any operation fails, then xio_retrieveoptions shall fail and return NULL. ]*/

TEST_FUNCTION(xio_retrieveoptions_unhappypaths)
{
    ///arrange
    XIO_HANDLE x;
    size_t i;
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    x = xio_create(&test_io_description, NULL);
    umock_c_reset_all_calls();

    xio_retrieveoptions_inert_path();

    umock_c_negative_tests_snapshot();

    ///act
    for (i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        if (!umock_c_negative_tests_can_call_fail(i)) continue;

        char temp_str[128];
        OPTIONHANDLER_HANDLE h;

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        ///act
        (void)sprintf(temp_str, "On failed call %lu", (unsigned long)i);

        ///act
        h = xio_retrieveoptions(x);

        ///assert
        ASSERT_IS_NULL(h, temp_str);
    }

    ///cleanup
    xio_destroy(x);
    umock_c_negative_tests_deinit();
}


END_TEST_SUITE(xio_unittests)
