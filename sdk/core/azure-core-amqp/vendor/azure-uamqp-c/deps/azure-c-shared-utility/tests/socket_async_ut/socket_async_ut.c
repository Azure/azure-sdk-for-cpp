// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#include <stdint.h>
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

#include "socket_async.h"

// This file is OS-specific, and is identified by setting include directories
// in the project
#include "socket_async_os.h"


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

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"

#ifdef __cplusplus
extern "C" {
#endif
    MOCKABLE_FUNCTION(, int, socket, int, af, int, type, int, protocol);
    MOCKABLE_FUNCTION(, int, bind, int, sockfd, const struct sockaddr*, addr, socklen_t, addrlen);
    MOCKABLE_FUNCTION(, int, setsockopt, int, sockfd, int, level, int, optname, const void*, optval, socklen_t, optlen);
    MOCKABLE_FUNCTION(, int, getsockopt, int, sockfd, int, level, int, optname, void*, optval, socklen_t*, optlen);
    MOCKABLE_FUNCTION(, int, connect, int, sockfd, const struct sockaddr*, addr, socklen_t, addrlen);
    MOCKABLE_FUNCTION(, int, select, int, nfds, fd_set*, readfds, fd_set*, writefds, fd_set*, exceptfds, struct timeval*, timeout);
    MOCKABLE_FUNCTION(, ssize_t, send, int, sockfd, const void*, buf, size_t, len, int, flags);
    MOCKABLE_FUNCTION(, ssize_t, recv, int, sockfd, void*, buf, size_t, len, int, flags);
    MOCKABLE_FUNCTION(, int, close, int, sockfd);
#ifdef __cplusplus
}
#endif

#undef ENABLE_MOCKS

#include "test_defines.h"
#include "keep_alive.h"

// A non-tested function from socket.h
int fcntl(int fd, int cmd, ... /* arg */) { (void)fd; (void)cmd; return 0; }

typedef enum
{
    SELECT_TCP_IS_COMPLETE_ERRSET_FAIL,
    SELECT_TCP_IS_COMPLETE_READY_OK,
    SELECT_TCP_IS_COMPLETE_NOT_READY_OK,
} SELECT_BEHAVIOR;

// The mocked select() function uses FD_SET, etc. macros, so it needs to be specially implemented
static SELECT_BEHAVIOR select_behavior;

int my_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    (void)timeout;
    (void)nfds;
    (void)readfds;
    // TP_TCP_IS_COMPLETE_ERRSET_FAIL,     // a non-empty error set
    // TP_TCP_IS_COMPLETE_READY_OK,        //
    // TTP_TCP_IS_COMPLETE_NOT_READY_OK,    //

    // This arguably odd sequence of FD_SET, etc. was necessary
    // to make the linux_c-ubuntu-clang build succeed. FD_CLR
    // did not work as expected on that system, but this does the job.
    switch (select_behavior)
    {
    case SELECT_TCP_IS_COMPLETE_ERRSET_FAIL:
        FD_SET(nfds, exceptfds);
        break;
    case SELECT_TCP_IS_COMPLETE_READY_OK:
        FD_ZERO(exceptfds);
        FD_SET(nfds, writefds);
        break;
    case SELECT_TCP_IS_COMPLETE_NOT_READY_OK:
        FD_ZERO(exceptfds);
        FD_ZERO(writefds);
        break;
    default:
        ASSERT_FAIL("program bug");
    }
    return 0;
}

/**
* Umock error will helps you to identify errors in the test suite or in the way that you are
*    using it, just keep it as is.
*/
MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

/**
* This is necessary for the test suite, just keep as is.
*/
static TEST_MUTEX_HANDLE g_testByTest;

BEGIN_TEST_SUITE(socket_async_ut)

/**
* This is the place where we initialize the test system. Replace the test name to associate the test
*   suite with your test cases.
* It is called once, before start the tests.
*/
TEST_SUITE_INITIALIZE(a)
{
    int result;
    size_t type_size;
    ssize_t sr_error;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    (void)umock_c_init(on_umock_c_error);

    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_bool_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);
    umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    // Unnatural type_size variable exists to avoid "conditional expression is constant" warning
    type_size = sizeof(ssize_t);
    if (type_size == sizeof(int32_t))
    {
        REGISTER_UMOCK_ALIAS_TYPE(ssize_t, int32_t);
    }
    else if (type_size == sizeof(int64_t))
    {
        REGISTER_UMOCK_ALIAS_TYPE(ssize_t, int64_t);
    }
    else
    {
        ASSERT_FAIL("bad ssize_t");
    }

    type_size = sizeof(socklen_t);
    if (type_size == sizeof(int32_t))
    {
        REGISTER_UMOCK_ALIAS_TYPE(socklen_t, uint32_t);
    }
    else if (type_size == sizeof(int64_t))
    {
        REGISTER_UMOCK_ALIAS_TYPE(socklen_t, uint64_t);
    }
    else
    {
        ASSERT_FAIL("bad socklen_t");
    }

    sr_error = (ssize_t)(-1);
    REGISTER_GLOBAL_MOCK_RETURNS(socket, test_socket, -1);
    REGISTER_GLOBAL_MOCK_RETURNS(bind, 0, -1);
    REGISTER_GLOBAL_MOCK_RETURNS(connect, 0, -1);
    REGISTER_GLOBAL_MOCK_RETURNS(setsockopt, 0, -1);
    REGISTER_GLOBAL_MOCK_RETURNS(getsockopt, EAGAIN, EXTENDED_ERROR_FAIL);
    REGISTER_GLOBAL_MOCK_RETURNS(select, 0, -1);
    REGISTER_GLOBAL_MOCK_RETURNS(send, sizeof(test_msg), sr_error);
    REGISTER_GLOBAL_MOCK_RETURNS(recv, sizeof(test_msg), sr_error);

    REGISTER_GLOBAL_MOCK_HOOK(setsockopt, my_setsockopt);
    REGISTER_GLOBAL_MOCK_HOOK(select, my_select);
}


    /**
     * The test suite will call this function to cleanup your machine.
     * It is called only once, after all tests is done.
     */
    TEST_SUITE_CLEANUP(TestClassCleanup)
    {
        //free(g_GenericPointer);

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

    /* Tests_SRS_SOCKET_ASYNC_30_071: [ socket_async_destroy shall call the underlying close method on the supplied socket. ]*/
    TEST_FUNCTION(socket_async_destroy__succeeds)
    {
        ///arrange
        STRICT_EXPECTED_CALL(close(test_socket));

        ///act
        socket_async_destroy(test_socket);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        // no cleanup necessary
    }

    /* Tests_SRS_SOCKET_ASYNC_30_052: [ If the buffer parameter is NULL, socket_async_receive shall log the error and return FAILURE. ]*/
    /* Tests_SRS_SOCKET_ASYNC_30_053: [ If the received_count parameter is NULL, socket_async_receive shall log the error and return FAILURE. ]*/
    /* Tests_SRS_SOCKET_ASYNC_30_072: [ If the size parameter is 0, socket_async_receive shall log an error and return FAILURE. ]*/
    TEST_FUNCTION(socket_async_receive__parameter_validation__fails)
    {
        ///arrange
        // no calls expected
        size_t i;
        size_t received_count_receptor = BAD_BUFFER_COUNT;
        send_receive_parameters_t parameters[3];

        //                                     buffer       size                received_count              fail_msg
        populate_s_r_parameters(parameters + 0, NULL,       sizeof(test_msg),   &received_count_receptor,   "Unexpected receive_result success when buffer is NULL");
        populate_s_r_parameters(parameters + 1, test_msg,   sizeof(test_msg),   NULL,                       "Unexpected receive_result success when received_count is NULL");
        populate_s_r_parameters(parameters + 2, test_msg,   0,                  &received_count_receptor,   "Unexpected receive_result success when size parameter is 0");

        // Cycle through each failing combo of parameters
        for (i = 0; i < sizeof(parameters) / sizeof(send_receive_parameters_t); i++)
        {
            ///act
            int receive_result = socket_async_receive(test_socket, parameters[i].buffer, parameters[i].size, parameters[i].returned_count);

            ///assert
            ASSERT_ARE_EQUAL(size_t, received_count_receptor, BAD_BUFFER_COUNT, "Unexpected received_count_receptor");
            ASSERT_ARE_NOT_EQUAL(int, receive_result, 0, parameters[i].fail_msg);
        }


        ///cleanup
        // no cleanup necessary
    }

    /* Codes_SRS_SOCKET_ASYNC_30_056: [ If the underlying socket fails unexpectedly, socket_async_receive shall log the error and return FAILURE. ]*/
    TEST_FUNCTION(socket_async_receive__recv_fail__fails)
    {
        ///arrange
        char *buffer = test_msg;
        int receive_result;
        size_t size = sizeof(test_msg);
        size_t received_count_receptor = BAD_BUFFER_COUNT;
        size_t *received_count = &received_count_receptor;
        // getsockopt is used to get the extended error information after a socket failure
        int getsockopt_extended_error_return_value = EXTENDED_ERROR_FAIL;

        STRICT_EXPECTED_CALL(recv(test_socket, buffer, size, RECV_ZERO_FLAGS)).SetReturn(RECV_FAIL_RETURN);
        // getsockopt is used to get the extended error information after a socket failure
        STRICT_EXPECTED_CALL(getsockopt(test_socket, SOL_SOCKET, SO_ERROR, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .CopyOutArgumentBuffer_optval(&getsockopt_extended_error_return_value, sizeof_int);

        ///act
        receive_result = socket_async_receive(test_socket, buffer, size, received_count);

        ///assert
        ASSERT_ARE_EQUAL(size_t, received_count_receptor, BAD_BUFFER_COUNT, "Unexpected received_count_receptor");
        ASSERT_ARE_NOT_EQUAL(int, receive_result, 0, "Unexpected receive_result success");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        // no cleanup necessary
    }

    /* Tests_SRS_SOCKET_ASYNC_30_055: [ If the underlying socket has no received bytes available, socket_async_receive shall return 0 and the received_count parameter shall receive the value 0. ]*/
    TEST_FUNCTION(socket_async_receive__recv_waiting__succeeds)
    {
        ///arrange
        char *buffer = test_msg;
        int receive_result;
        size_t size = sizeof(test_msg);
        size_t received_count_receptor = BAD_BUFFER_COUNT;
        size_t *received_count = &received_count_receptor;
        // getsockopt is used to get the extended error information after a socket failure
        int getsockopt_extended_error_return_value = EXTENDED_ERROR_WAITING;

        STRICT_EXPECTED_CALL(recv(test_socket, buffer, size, RECV_ZERO_FLAGS)).SetReturn(RECV_FAIL_RETURN);
        // getsockopt is used to get the extended error information after a socket failure
        STRICT_EXPECTED_CALL(getsockopt(test_socket, SOL_SOCKET, SO_ERROR, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .CopyOutArgumentBuffer_optval(&getsockopt_extended_error_return_value, sizeof_int);

        ///act
        receive_result = socket_async_receive(test_socket, buffer, size, received_count);

        ///assert
        ASSERT_ARE_EQUAL(size_t, received_count_receptor, 0, "Unexpected received_count_receptor");
        ASSERT_ARE_EQUAL(int, receive_result, 0, "Unexpected receive_result failure");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        // no cleanup necessary
    }

    /* Tests_SRS_SOCKET_ASYNC_30_054: [ On success, the underlying socket shall set one or more received bytes into buffer, socket_async_receive shall return 0, and the received_count parameter shall receive the number of bytes received into buffer. ]*/
    TEST_FUNCTION(socket_async_receive__recv__succeeds)
    {
        ///arrange
        char *buffer = test_msg;
        int receive_result;
        size_t size = sizeof(test_msg);
        size_t received_count_receptor;
        size_t *received_count = &received_count_receptor;

        STRICT_EXPECTED_CALL(recv(test_socket, buffer, size, RECV_ZERO_FLAGS));

        ///act
        receive_result = socket_async_receive(test_socket, buffer, size, received_count);

        ///assert
        ASSERT_ARE_EQUAL(size_t, received_count_receptor, sizeof(test_msg), "Unexpected received_count_receptor");
        ASSERT_ARE_EQUAL(int, receive_result, 0, "Unexpected receive_result failure");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        // no cleanup necessary
    }

    /* Tests_SRS_SOCKET_ASYNC_30_033: [ If the buffer parameter is NULL, socket_async_send shall log the error return FAILURE. ]*/
    /* Tests_SRS_SOCKET_ASYNC_30_034: [ If the sent_count parameter is NULL, socket_async_send shall log the error return FAILURE. ]*/
    TEST_FUNCTION(socket_async_send__parameter_validation__fails)
    {
        ///arrange
        // no calls expected
        size_t i;
        size_t sent_count_receptor = BAD_BUFFER_COUNT;
        send_receive_parameters_t parameters[2];
        //                                     buffer       size            received_count              fail_msg
        populate_s_r_parameters(parameters + 0, NULL,     sizeof(test_msg), &sent_count_receptor, "Unexpected receive_result success when buffer is NULL");
        populate_s_r_parameters(parameters + 1, test_msg, sizeof(test_msg), NULL,                 "Unexpected receive_result success when received_count is NULL");

        // Cycle through each failing combo of parameters
        for (i = 0; i < sizeof(parameters) / sizeof(send_receive_parameters_t); i++)
        {
            ///act
            int send_result = socket_async_send(test_socket, parameters[i].buffer, parameters[i].size, parameters[i].returned_count);

            ///assert
            ASSERT_ARE_NOT_EQUAL(int, send_result, 0, parameters[i].fail_msg);
        }

        ///cleanup
        // no cleanup necessary
    }

    /* Tests_SRS_SOCKET_ASYNC_30_037: [ If socket_async_send fails unexpectedly, socket_async_send shall log the error return FAILURE. ]*/
    TEST_FUNCTION(socket_async_send__send_fail__fails)
    {
        ///arrange
        int send_result;
        char *buffer = test_msg;
        size_t size = sizeof(test_msg);
        size_t sent_count_receptor = BAD_BUFFER_COUNT;
        size_t *sent_count = &sent_count_receptor;
        // getsockopt is used to get the extended error information after a socket failure
        int getsockopt_extended_error_return_value = EXTENDED_ERROR_FAIL;

        STRICT_EXPECTED_CALL(send(test_socket, buffer, size, SEND_ZERO_FLAGS)).SetReturn(SEND_FAIL_RETURN);
        // getsockopt is used to get the extended error information after a socket failure
        STRICT_EXPECTED_CALL(getsockopt(test_socket, SOL_SOCKET, SO_ERROR, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .CopyOutArgumentBuffer_optval(&getsockopt_extended_error_return_value, sizeof_int);

        ///act
        send_result = socket_async_send(test_socket, buffer, size, sent_count);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, send_result, 0, "Unexpected send_result success");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        // no cleanup necessary
    }

    /* Tests_SRS_SOCKET_ASYNC_30_036: [ If the underlying socket is unable to accept any bytes for transmission because its buffer is full, socket_async_send shall return 0 and the sent_count parameter shall receive the value 0. ]*/
    TEST_FUNCTION(socket_async_send__send_waiting__succeeds)
    {
        ///arrange
        int send_result;
        char *buffer = test_msg;
        size_t size = sizeof(test_msg);
        size_t sent_count_receptor = BAD_BUFFER_COUNT;
        size_t *sent_count = &sent_count_receptor;
        // getsockopt is used to get the extended error information after a socket failure
        int getsockopt_extended_error_return_value = EXTENDED_ERROR_WAITING;

        STRICT_EXPECTED_CALL(send(test_socket, buffer, size, SEND_ZERO_FLAGS)).SetReturn(SEND_FAIL_RETURN);
        // getsockopt is used to get the extended error information after a socket failure
        STRICT_EXPECTED_CALL(getsockopt(test_socket, SOL_SOCKET, SO_ERROR, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .CopyOutArgumentBuffer_optval(&getsockopt_extended_error_return_value, sizeof_int);

        ///act
        send_result = socket_async_send(test_socket, buffer, size, sent_count);

        ///assert
        ASSERT_ARE_EQUAL(size_t, sent_count_receptor, 0, "Unexpected sent_count_receptor");
        ASSERT_ARE_EQUAL(int, send_result, 0, "Unexpected send_result failure");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        // no cleanup necessary
    }

    /* Tests_SRS_SOCKET_ASYNC_30_073: [ If the size parameter is 0, socket_async_send shall set sent_count to 0 and return 0. ]*/
    TEST_FUNCTION(socket_async_send__send_0_bytes__succeeds)
    {
        ///arrange
        char *buffer = test_msg;
        size_t size = 0;
        size_t sent_count_receptor = BAD_BUFFER_COUNT;
        size_t *sent_count = &sent_count_receptor;


        ///act
        int send_result = socket_async_send(test_socket, buffer, size, sent_count);

        ///assert
        ASSERT_ARE_EQUAL(size_t, sent_count_receptor, 0, "Unexpected sent_count_receptor");
        ASSERT_ARE_EQUAL(int, send_result, 0, "Unexpected send_result failure");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        // no cleanup necessary
    }

    /* Tests_SRS_SOCKET_ASYNC_30_035: [ If the underlying socket accepts one or more bytes for transmission, socket_async_send shall return 0 and the sent_count parameter shall receive the number of bytes accepted for transmission. ]*/
    TEST_FUNCTION(socket_async_send__succeeds)
    {
        ///arrange
        int send_result;
        char *buffer = test_msg;
        size_t size = sizeof(test_msg);
        size_t sent_count_receptor = BAD_BUFFER_COUNT;
        size_t *sent_count = &sent_count_receptor;

        STRICT_EXPECTED_CALL(send(test_socket, buffer, size, SEND_ZERO_FLAGS));

        ///act
        send_result = socket_async_send(test_socket, buffer, size, sent_count);

        ///assert
        ASSERT_ARE_EQUAL(size_t, sent_count_receptor, sizeof(test_msg), "Unexpected sent_count_receptor");
        ASSERT_ARE_EQUAL(int, send_result, 0, "Unexpected send_result failure");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        // no cleanup necessary
    }

    /* Tests_SRS_SOCKET_ASYNC_30_026: [ If the is_complete parameter is NULL, socket_async_is_create_complete shall log an error and return FAILURE. ]*/
    TEST_FUNCTION(socket_async_is_create_complete__parameter_validation__fails)
    {
        ///arrange
        //bool is_complete = true;
        //bool* is_complete_param = &is_complete;
        bool* is_complete_param = NULL;

        ///act
        int create_complete_result = socket_async_is_create_complete(test_socket, is_complete_param);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, create_complete_result, 0, "Unexpected send_result success");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_SOCKET_ASYNC_30_028: [ On failure, the is_complete value shall be set to false and socket_async_create shall return FAILURE. ]*/
    TEST_FUNCTION(socket_async_is_create_complete__select_fail__fails)
    {
        ///arrange
        int create_complete_result;
        bool is_complete;
        bool* is_complete_param = &is_complete;
        // getsockopt is used to get the extended error information after a socket failure
        int getsockopt_extended_error_return_value = EXTENDED_ERROR_FAIL;

        STRICT_EXPECTED_CALL(select(test_socket + 1, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(SELECT_FAIL_RETURN);
        // getsockopt is used to get the extended error information after a socket failure
        STRICT_EXPECTED_CALL(getsockopt(test_socket, SOL_SOCKET, SO_ERROR, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .CopyOutArgumentBuffer_optval(&getsockopt_extended_error_return_value, sizeof_int);

        ///act
        create_complete_result = socket_async_is_create_complete(test_socket, is_complete_param);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, create_complete_result, 0, "Unexpected create_complete_result success");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_SOCKET_ASYNC_30_028: [ On failure, the is_complete value shall be set to false and socket_async_create shall return FAILURE. ]*/
    TEST_FUNCTION(socket_async_is_create_complete__errset_set__fails)
    {
        ///arrange
        int getsockopt_extended_error_return_value;
        int create_complete_result;
        bool is_complete;
        bool* is_complete_param = &is_complete;
        // Define how the FD_ISSET etc. macros behave
        // Cause the FD_ISSET macro to detect a failure even though select() succeeded
        select_behavior = SELECT_TCP_IS_COMPLETE_ERRSET_FAIL;
        // getsockopt is used to get the extended error information after a socket failure
        getsockopt_extended_error_return_value = EXTENDED_ERROR_FAIL;

        STRICT_EXPECTED_CALL(select(test_socket + 1, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        // getsockopt is used to get the extended error information after a socket failure
        STRICT_EXPECTED_CALL(getsockopt(test_socket, SOL_SOCKET, SO_ERROR, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .CopyOutArgumentBuffer_optval(&getsockopt_extended_error_return_value, sizeof_int);

        ///act
        create_complete_result = socket_async_is_create_complete(test_socket, is_complete_param);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, create_complete_result, 0, "Unexpected create_complete_result success");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Codes_SRS_SOCKET_ASYNC_30_027: [ On success, the is_complete value shall be set to the completion state and socket_async_create shall return 0. ]*/
    TEST_FUNCTION(socket_async_is_create_complete__waiting__succeeds)
    {
        ///arrange
        int create_complete_result;
        bool is_complete = true; // unexpected so change can be detected
        bool* is_complete_param = &is_complete;
        // Define how the FD_ISET etc. macros behave
        select_behavior = SELECT_TCP_IS_COMPLETE_NOT_READY_OK;

        STRICT_EXPECTED_CALL(select(test_socket + 1, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

        ///act
        create_complete_result = socket_async_is_create_complete(test_socket, is_complete_param);

        ///assert
        ASSERT_IS_FALSE(is_complete, "Unexpected is_complete failure");
        ASSERT_ARE_EQUAL(int, create_complete_result, 0, "Unexpected create_complete_result failure");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Codes_SRS_SOCKET_ASYNC_30_027: [ On success, the is_complete value shall be set to the completion state and socket_async_create shall return 0. ]*/
    TEST_FUNCTION(socket_async_is_create_complete__succeeds)
    {
        ///arrange
        int create_complete_result;
        bool is_complete = false; // unexpected so change can be detected
        bool* is_complete_param = &is_complete;
        // Define how the FD_ISET etc. macros behave
        select_behavior = SELECT_TCP_IS_COMPLETE_READY_OK;

        STRICT_EXPECTED_CALL(select(test_socket + 1, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

        ///act
        create_complete_result = socket_async_is_create_complete(test_socket, is_complete_param);

        ///assert
        ASSERT_IS_TRUE(is_complete, "Unexpected is_complete failure");
        ASSERT_ARE_EQUAL(int, create_complete_result, 0, "Unexpected create_complete_result failure");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_SOCKET_ASYNC_30_010: [ If socket option creation fails, socket_async_create shall log an error and return SOCKET_ASYNC_INVALID_SOCKET. ]*/
    TEST_FUNCTION(socket_async_create__create_fail__fails)
    {
        ///arrange
        SOCKET_ASYNC_HANDLE create_result;
        SOCKET_ASYNC_OPTIONS* options = NULL;
        bool is_udp = false;

        STRICT_EXPECTED_CALL(socket(AF_INET, SOCK_STREAM /* the TCP value, doesn't matter */, 0)).SetReturn(SOCKET_FAIL_RETURN);

        ///act
        create_result = socket_async_create(test_ipv4, test_port, is_udp, options);

        ///assert
        ASSERT_ARE_EQUAL(int, create_result, SOCKET_ASYNC_INVALID_SOCKET, "Unexpected create_result success");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_SOCKET_ASYNC_30_020: [ If socket option setting fails, the sock value shall be set to SOCKET_ASYNC_INVALID_SOCKET and socket_async_create shall log an error and return FAILURE. ]*/
    TEST_FUNCTION(socket_async_create__opt_default_fail__fails)
    {
        ///arrange
        SOCKET_ASYNC_HANDLE create_result;
        SOCKET_ASYNC_OPTIONS* options = NULL;
        bool is_udp = false;


        STRICT_EXPECTED_CALL(socket(AF_INET, SOCK_STREAM, 0));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(SETSOCKOPT_FAIL_RETURN);

        ///act
        create_result = socket_async_create(test_ipv4, test_port, is_udp, options);

        ///assert
        ASSERT_ARE_EQUAL(int, create_result, SOCKET_ASYNC_INVALID_SOCKET, "Unexpected create_result success");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_SOCKET_ASYNC_30_020: [ If socket option setting fails, the sock value shall be set to SOCKET_ASYNC_INVALID_SOCKET and socket_async_create shall log an error and return FAILURE. ]*/
    TEST_FUNCTION(socket_async_create__set_all_options_fail__fails)
    {
        ///arrange
        SOCKET_ASYNC_OPTIONS options_value = { test_keep_alive , test_keep_idle , test_keep_interval, test_keep_count };
        SOCKET_ASYNC_OPTIONS* options = &options_value;
        bool is_udp = false;
        int negativeTestsInitResult = umock_c_negative_tests_init();
        size_t i;

        ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

        STRICT_EXPECTED_CALL(socket(AF_INET, SOCK_STREAM, 0));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        umock_c_negative_tests_snapshot();

        for (i = 1; i < umock_c_negative_tests_call_count(); i++)
        {
            SOCKET_ASYNC_HANDLE create_result;

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            ///act
            create_result = socket_async_create(test_ipv4, test_port, is_udp, options);

            ///assert
            ASSERT_ARE_EQUAL(int, create_result, SOCKET_ASYNC_INVALID_SOCKET, "Unexpected create_result success");
        }

        ///cleanup
        umock_c_negative_tests_deinit();
    }

    /* Tests_SRS_SOCKET_ASYNC_30_021: [ If socket binding fails, socket_async_create shall log an error and return SOCKET_ASYNC_INVALID_SOCKET. ]*/
    TEST_FUNCTION(socket_async_create__bind_fail__fails)
    {
        ///arrange
        SOCKET_ASYNC_OPTIONS options_value = { test_keep_alive , test_keep_idle , test_keep_interval, test_keep_count };
        SOCKET_ASYNC_OPTIONS* options = &options_value;
        bool is_udp = false;
        // getsockopt is used to get the extended error information after a socket failure
        int getsockopt_extended_error_return_value = EXTENDED_ERROR_FAIL;
        SOCKET_ASYNC_HANDLE create_result;

        STRICT_EXPECTED_CALL(socket(AF_INET, SOCK_STREAM, 0));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(bind(test_socket, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(BIND_FAIL_RETURN);
        STRICT_EXPECTED_CALL(getsockopt(test_socket, SOL_SOCKET, SO_ERROR, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .CopyOutArgumentBuffer_optval(&getsockopt_extended_error_return_value, sizeof_int);

        ///act
        create_result = socket_async_create(test_ipv4, test_port, is_udp, options);

        ///assert
        ASSERT_ARE_EQUAL(int, create_result, SOCKET_ASYNC_INVALID_SOCKET, "Unexpected create_result success");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_SOCKET_ASYNC_30_022: [ If socket connection fails, socket_async_create shall log an error and return SOCKET_ASYNC_INVALID_SOCKET. ]*/
    TEST_FUNCTION(socket_async_create__connect_fail__fails)
    {
        ///arrange
        SOCKET_ASYNC_OPTIONS options_value = { test_keep_alive , test_keep_idle , test_keep_interval, test_keep_count };
        SOCKET_ASYNC_OPTIONS* options = &options_value;
        bool is_udp = false;
        SOCKET_ASYNC_HANDLE create_result;
        // getsockopt is used to get the extended error information after a socket failure
        int getsockopt_extended_error_return_value = EXTENDED_ERROR_FAIL;


        STRICT_EXPECTED_CALL(socket(AF_INET, SOCK_STREAM, 0));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(bind(test_socket, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(connect(test_socket, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(BIND_FAIL_RETURN);
        STRICT_EXPECTED_CALL(getsockopt(test_socket, SOL_SOCKET, SO_ERROR, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .CopyOutArgumentBuffer_optval(&getsockopt_extended_error_return_value, sizeof_int);

        ///act
        create_result = socket_async_create(test_ipv4, test_port, is_udp, options);

        ///assert
        ASSERT_ARE_EQUAL(int, create_result, SOCKET_ASYNC_INVALID_SOCKET, "Unexpected create_result success");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_SOCKET_ASYNC_30_018: [ On success, socket_async_create shall return the created and configured SOCKET_ASYNC_HANDLE. ]*/
    /* Tests_SRS_SOCKET_ASYNC_30_013: [ The is_UDP parameter shall be true for a UDP connection, and false for TCP. ]*/
    /* Tests_SRS_SOCKET_ASYNC_30_014: [ If the optional options parameter is non-NULL and is_UDP is false, socket_async_create shall set the socket options to the provided values. ]*/
    TEST_FUNCTION(socket_async_create__tcp_connect_waiting__succeeds)
    {
        ///arrange
        SOCKET_ASYNC_OPTIONS options_value = { test_keep_alive , test_keep_idle , test_keep_interval, test_keep_count };
        SOCKET_ASYNC_OPTIONS* options = &options_value;
        bool is_udp = false;
        SOCKET_ASYNC_HANDLE create_result;
        // getsockopt is used to get the extended error information after a socket failure
        int getsockopt_extended_error_return_value = EXTENDED_ERROR_CONNECT_WAITING;
        init_keep_alive_values();

        STRICT_EXPECTED_CALL(socket(AF_INET, SOCK_STREAM, 0));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(bind(test_socket, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(connect(test_socket, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(BIND_FAIL_RETURN);
        STRICT_EXPECTED_CALL(getsockopt(test_socket, SOL_SOCKET, SO_ERROR, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .CopyOutArgumentBuffer_optval(&getsockopt_extended_error_return_value, sizeof_int);

        ///act
        create_result = socket_async_create(test_ipv4, test_port, is_udp, options);

        ///assert
        ASSERT_KEEP_ALIVE_SET();
        ASSERT_ARE_EQUAL(int, create_result, test_socket, "Unexpected create_result failure");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_SOCKET_ASYNC_30_018: [ On success, socket_async_create shall return the created and configured SOCKET_ASYNC_HANDLE. ]*/
    /* Tests_SRS_SOCKET_ASYNC_30_013: [ The is_UDP parameter shall be true for a UDP connection, and false for TCP. ]*/
    /* Tests_SRS_SOCKET_ASYNC_30_014: [ If the optional options parameter is non-NULL and is_UDP is false, socket_async_create shall set the socket options to the provided values. ]*/
    TEST_FUNCTION(socket_async_create__tcp_succeeds)
    {
        ///arrange
        SOCKET_ASYNC_OPTIONS options_value = { test_keep_alive , test_keep_idle , test_keep_interval, test_keep_count };
        SOCKET_ASYNC_OPTIONS* options = &options_value;
        bool is_udp = false;
        SOCKET_ASYNC_HANDLE create_result;
        init_keep_alive_values();

        STRICT_EXPECTED_CALL(socket(AF_INET, SOCK_STREAM /* the TCP value */, 0));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(bind(test_socket, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(connect(test_socket, IGNORED_PTR_ARG, IGNORED_NUM_ARG));

        ///act
        create_result = socket_async_create(test_ipv4, test_port, is_udp, options);

        ///assert
        ASSERT_KEEP_ALIVE_SET();
        ASSERT_ARE_EQUAL(int, create_result, test_socket, "Unexpected create_result failure");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_SOCKET_ASYNC_30_018: [ On success, socket_async_create shall return the created and configured SOCKET_ASYNC_HANDLE. ]*/
    /* Tests_SRS_SOCKET_ASYNC_30_013: [ The is_UDP parameter shall be true for a UDP connection, and false for TCP. ]*/
    /* Tests_SRS_SOCKET_ASYNC_30_015: [ If the optional options parameter is non-NULL and is_UDP is false, and options->keep_alive is negative, socket_async_create not set the socket keep-alive options. ]*/
    TEST_FUNCTION(socket_async_create__tcp_default_sys_keep_alive__succeeds)
    {
        ///arrange
        SOCKET_ASYNC_OPTIONS options_value = { test_keep_alive_sys_default , test_keep_idle , test_keep_interval, test_keep_count };
        SOCKET_ASYNC_OPTIONS* options = &options_value;
        bool is_udp = false;
        SOCKET_ASYNC_HANDLE create_result;
        init_keep_alive_values();

        STRICT_EXPECTED_CALL(socket(AF_INET, SOCK_STREAM /* the TCP value */, 0));
        STRICT_EXPECTED_CALL(bind(test_socket, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(connect(test_socket, IGNORED_PTR_ARG, IGNORED_NUM_ARG));

        ///act
        create_result = socket_async_create(test_ipv4, test_port, is_udp, options);

        ///assert
        ASSERT_KEEP_ALIVE_UNTOUCHED();
        ASSERT_ARE_EQUAL(int, create_result, test_socket, "Unexpected create_result failure");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_SOCKET_ASYNC_30_018: [ On success, socket_async_create shall return the created and configured SOCKET_ASYNC_HANDLE. ]*/
    /* Tests_SRS_SOCKET_ASYNC_30_013: [ The is_UDP parameter shall be true for a UDP connection, and false for TCP. ]*/
    /* Tests_SRS_SOCKET_ASYNC_30_017: [ If the optional options parameter is NULL and is_UDP is false, socket_async_create shall disable TCP keep-alive. ]*/
    TEST_FUNCTION(socket_async_create__tcp_keep_alive_off_by_default__succeeds)
    {
        ///arrange
        SOCKET_ASYNC_OPTIONS* options = NULL;
        bool is_udp = false;
        SOCKET_ASYNC_HANDLE create_result;
        init_keep_alive_values();

        STRICT_EXPECTED_CALL(socket(AF_INET, SOCK_STREAM /* the TCP value */, 0));
        STRICT_EXPECTED_CALL(setsockopt(test_socket, IGNORED_NUM_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(bind(test_socket, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(connect(test_socket, IGNORED_PTR_ARG, IGNORED_NUM_ARG));

        ///act
        create_result = socket_async_create(test_ipv4, test_port, is_udp, options);

        ///assert
        ASSERT_KEEP_ALIVE_FALSE();
        ASSERT_ARE_EQUAL(int, create_result, test_socket, "Unexpected create_result failure");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

    /* Tests_SRS_SOCKET_ASYNC_30_018: [ On success, socket_async_create shall return the created and configured SOCKET_ASYNC_HANDLE. ]*/
    /* Tests_SRS_SOCKET_ASYNC_30_013: [ The is_UDP parameter shall be true for a UDP connection, and false for TCP. ]*/
    // Keep alive does not apply to udp connections
    TEST_FUNCTION(socket_async_create__udp__succeeds)
    {
        ///arrange
        SOCKET_ASYNC_OPTIONS options_value = { test_keep_alive_sys_default , test_keep_idle , test_keep_interval, test_keep_count };
        SOCKET_ASYNC_OPTIONS* options = &options_value;
        bool is_udp = true;
        SOCKET_ASYNC_HANDLE create_result;
        init_keep_alive_values();

        STRICT_EXPECTED_CALL(socket(AF_INET, SOCK_DGRAM /* the UDP value */, 0));
        STRICT_EXPECTED_CALL(bind(test_socket, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(connect(test_socket, IGNORED_PTR_ARG, IGNORED_NUM_ARG));

        ///act
        create_result = socket_async_create(test_ipv4, test_port, is_udp, options);

        ///assert
        ASSERT_KEEP_ALIVE_UNTOUCHED();
        ASSERT_ARE_EQUAL(int, create_result, test_socket, "Unexpected create_result failure");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

END_TEST_SUITE(socket_async_ut)
