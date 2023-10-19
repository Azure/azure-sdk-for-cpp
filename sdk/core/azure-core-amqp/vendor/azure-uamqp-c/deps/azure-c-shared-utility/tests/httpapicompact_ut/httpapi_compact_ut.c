// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstddef>
#include <ctime>
#include <cstdlib>
#else
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"

static int currentmalloc_call = 0;
static int whenShallmalloc_fail = 0;

void* my_gballoc_malloc(size_t size)
{
    void* result;
    currentmalloc_call++;
    if (whenShallmalloc_fail > 0)
    {
        if (currentmalloc_call >= whenShallmalloc_fail)
        {
            currentmalloc_call--;
            result = NULL;
        }
        else
        {
            result = malloc(size);
        }
    }
    else
    {
        result = malloc(size);
    }
    return result;
}

void* my_gballoc_realloc(void* ptr, size_t size)
{
    void* newptr = realloc(ptr, size);

    if (ptr == NULL)
    {
        currentmalloc_call++;
    }

    return newptr;
}

void my_gballoc_free(void* ptr)
{
    currentmalloc_call--;
    free(ptr);
}

int my_mallocAndStrcpy_s(char** destination, const char* source)
{
    *destination = (char*)malloc(strlen(source) + 1);
    (void)strcpy(*destination, source);
    currentmalloc_call++;
    return 0;
}

#define MAX_RECEIVE_BUFFER_SIZES    3
#define HUGE_RELATIVE_PATH_SIZE        10000

#define TEST_CREATE_CONNECTION_HOST_NAME (const char*)"https://test.azure-devices.net"
#define TEST_EXECUTE_REQUEST_RELATIVE_PATH (const char*)"/devices/Huzzah_w_DHT22/messages/events?api-version=2016-11-14"
#define TEST_EXECUTE_REQUEST_CONTENT (const unsigned char*)"{\"ObjectType\":\"DeviceInfo\", \"Version\":\"1.0\", \"IsSimulatedDevice\":false, \"DeviceProperties\":{\"DeviceID\":\"Huzzah_w_DHT22\", \"HubEnabledState\":true}, \"Commands\":[{ \"Name\":\"SetHumidity\", \"Parameters\":[{\"Name\":\"humidity\",\"Type\":\"int\"}]},{ \"Name\":\"SetTemperature\", \"Parameters\":[{\"Name\":\"temperature\",\"Type\":\"int\"}]}]}"
#define TEST_EXECUTE_REQUEST_CONTENT_LENGTH (size_t)320
#define TEST_SETOPTIONS_CERTIFICATE    (const unsigned char*)"blah!blah!blah!"
#define TEST_SETOPTIONS_X509CLIENTCERT    (const unsigned char*)"ADMITONE"
#define TEST_SETOPTIONS_X509PRIVATEKEY    (const unsigned char*)"SPEAKFRIENDANDENTER"
#define TEST_GET_HEADER_HEAD_COUNT (size_t)2


#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/httpheaders.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/buffer_.h"
#include "azure_c_shared_utility/http_proxy_io.h"
#undef ENABLE_MOCKS
#include "azure_c_shared_utility/httpapi.h"
#include "azure_c_shared_utility/shared_util_options.h"

static bool current_xioCreate_must_fail = false;
XIO_HANDLE my_xio_create(const IO_INTERFACE_DESCRIPTION* io_interface_description, const void* xio_create_parameters)
{
    XIO_HANDLE result;
    (void)io_interface_description;
    (void)xio_create_parameters;
    if (current_xioCreate_must_fail)
    {
        result = NULL;
    }
    else
    {
        result = (XIO_HANDLE)malloc(sizeof(XIO_HANDLE));
    }
    return result;
}

void my_xio_destroy(XIO_HANDLE xio)
{
    if (xio != NULL)
    {
        free(xio);
    }
}

static int xio_setoption_shallReturn;
int my_xio_setoption(XIO_HANDLE xio, const char* optionName, const void* value)
{
    int result;
    if ((xio == NULL) || (optionName == NULL) || (value == NULL))
    {
        result = MU_FAILURE;
    }
    else
    {
        result = xio_setoption_shallReturn;
    }
    return result;
}

static int xio_open_shallReturn;
static int xio_close_shallReturn;
static const int* xio_send_shallReturn;
static int xio_send_shallReturn_counter;
static char xio_send_transmited_buffer[1024];
static int xio_send_transmited_buffer_target = 0;

typedef enum xio_dowork_job_tag
{
    XIO_DOWORK_JOB_NONE,
    XIO_DOWORK_JOB_OPEN,
    XIO_DOWORK_JOB_SEND,
    XIO_DOWORK_JOB_RECEIVED,
    XIO_DOWORK_JOB_CLOSE,
    XIO_DOWORK_JOB_ERROR,
    XIO_DOWORK_JOB_END
} xio_dowork_job;

static const int xio_send_0[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static const int xio_send_e[4] = { 123, 123, 123, 123 };
static const int xio_send_0_e[4] = { 0, 123, 0, 0 };
static const int xio_send_00_e[4] = { 0, 0, 123, 0 };
static const int xio_send_7x0[7] = { 0, 0, 0, 0, 0, 0, 0 };
static const int xio_send_6x0_e[7] = { 0, 0, 0, 0, 0, 0, 123 };
static const xio_dowork_job doworkjob_end[1] = { XIO_DOWORK_JOB_END };
static const xio_dowork_job doworkjob_oe[2] = { XIO_DOWORK_JOB_OPEN, XIO_DOWORK_JOB_END };
static const xio_dowork_job doworkjob_4none_oe[6] = { XIO_DOWORK_JOB_NONE, XIO_DOWORK_JOB_NONE, XIO_DOWORK_JOB_NONE, XIO_DOWORK_JOB_NONE, XIO_DOWORK_JOB_OPEN, XIO_DOWORK_JOB_END };
static const xio_dowork_job doworkjob_4none_ee[6] = { XIO_DOWORK_JOB_NONE, XIO_DOWORK_JOB_NONE, XIO_DOWORK_JOB_NONE, XIO_DOWORK_JOB_NONE, XIO_DOWORK_JOB_ERROR, XIO_DOWORK_JOB_END };
static const xio_dowork_job doworkjob_o_3none_ee[6] = { XIO_DOWORK_JOB_OPEN, XIO_DOWORK_JOB_NONE, XIO_DOWORK_JOB_NONE, XIO_DOWORK_JOB_NONE, XIO_DOWORK_JOB_ERROR, XIO_DOWORK_JOB_END };
static const xio_dowork_job doworkjob_oee[3] = { XIO_DOWORK_JOB_OPEN, XIO_DOWORK_JOB_ERROR, XIO_DOWORK_JOB_END };
static const xio_dowork_job doworkjob_ose[3] = { XIO_DOWORK_JOB_OPEN, XIO_DOWORK_JOB_SEND, XIO_DOWORK_JOB_END };
static const xio_dowork_job doworkjob_ee[2] = { XIO_DOWORK_JOB_ERROR, XIO_DOWORK_JOB_END };
static const xio_dowork_job doworkjob_o_re[3] = { XIO_DOWORK_JOB_OPEN, XIO_DOWORK_JOB_RECEIVED, XIO_DOWORK_JOB_END };
static const xio_dowork_job doworkjob_o_rce[8] = { XIO_DOWORK_JOB_OPEN, XIO_DOWORK_JOB_RECEIVED, XIO_DOWORK_JOB_RECEIVED, XIO_DOWORK_JOB_RECEIVED, XIO_DOWORK_JOB_RECEIVED, XIO_DOWORK_JOB_RECEIVED, XIO_DOWORK_JOB_CLOSE, XIO_DOWORK_JOB_END };
static const xio_dowork_job doworkjob_o_rc_error[9] = { XIO_DOWORK_JOB_OPEN, XIO_DOWORK_JOB_RECEIVED, XIO_DOWORK_JOB_RECEIVED, XIO_DOWORK_JOB_RECEIVED, XIO_DOWORK_JOB_RECEIVED, XIO_DOWORK_JOB_RECEIVED, XIO_DOWORK_JOB_CLOSE, XIO_DOWORK_JOB_ERROR, XIO_DOWORK_JOB_END };
static const xio_dowork_job doworkjob_o_rre[4] = { XIO_DOWORK_JOB_OPEN, XIO_DOWORK_JOB_RECEIVED, XIO_DOWORK_JOB_RECEIVED, XIO_DOWORK_JOB_END };
static const xio_dowork_job doworkjob_o_sre[15] = { XIO_DOWORK_JOB_OPEN,
    XIO_DOWORK_JOB_SEND, XIO_DOWORK_JOB_SEND, XIO_DOWORK_JOB_SEND, XIO_DOWORK_JOB_SEND, XIO_DOWORK_JOB_SEND, XIO_DOWORK_JOB_SEND, XIO_DOWORK_JOB_SEND,
    XIO_DOWORK_JOB_RECEIVED, XIO_DOWORK_JOB_RECEIVED, XIO_DOWORK_JOB_RECEIVED, XIO_DOWORK_JOB_RECEIVED, XIO_DOWORK_JOB_RECEIVED, XIO_DOWORK_JOB_CLOSE, XIO_DOWORK_JOB_END };

static const IO_OPEN_RESULT openresult_ok[1] = { IO_OPEN_OK };
static const IO_OPEN_RESULT openresult_error[1] = { IO_OPEN_ERROR };

static const IO_SEND_RESULT sendresult_error[1]  = { IO_SEND_ERROR };
static const IO_SEND_RESULT sendresult_o_3error[4] = { IO_SEND_OK, IO_SEND_ERROR, IO_SEND_ERROR, IO_SEND_ERROR };
static const IO_SEND_RESULT sendresult_7ok[7] = {
    IO_SEND_OK,
        IO_SEND_OK,
        IO_SEND_OK,
        IO_SEND_OK,
        IO_SEND_OK,
        IO_SEND_OK,
        IO_SEND_OK
};
static const IO_SEND_RESULT sendresult_6ok_error[7] = {
    IO_SEND_OK,
    IO_SEND_OK,
    IO_SEND_OK,
    IO_SEND_OK,
    IO_SEND_OK,
    IO_SEND_OK,
    IO_SEND_ERROR
};


static const xio_dowork_job* DoworkJobs = (const xio_dowork_job*)doworkjob_end;
static const IO_OPEN_RESULT* DoworkJobsOpenResult;
static int SkipDoworkJobsOpenResult;
static const IO_SEND_RESULT* DoworkJobsSendResult;
static int SkipDoworkJobsSendResult;

static bool DoworkJobsCloseSuccess;
static int SkipDoworkJobsCloseResult;
static bool call_on_io_close_complete_in_xio_close;

static ON_IO_OPEN_COMPLETE my_on_io_open_complete;
static void* my_on_io_open_complete_context;

static ON_IO_CLOSE_COMPLETE my_on_io_close_complete;
static void* my_on_io_close_complete_context;

static ON_SEND_COMPLETE my_on_send_complete;
static void* my_on_send_complete_context;
static bool call_on_send_complete_in_xio_send;

static ON_BYTES_RECEIVED my_on_bytes_received;
static void* my_on_bytes_received_context;
static const unsigned char* DoworkJobsReceivedBuffer;
static size_t DoworkJobsReceivedBuffer_size[MAX_RECEIVE_BUFFER_SIZES];
static int DoworkJobsReceivedBuffer_counter;

static ON_IO_ERROR my_on_io_error;
static void* my_on_io_error_context;

int my_xio_open(XIO_HANDLE xio,
    ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context,
    ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context,
    ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    int result;
    if ((xio == NULL) ||
        (on_io_open_complete == NULL) || (on_io_open_complete_context == NULL) ||
        (on_bytes_received == NULL) || (on_bytes_received_context == NULL) ||
        (on_io_error == NULL) || (on_io_error_context == NULL))
    {
        result = MU_FAILURE;
    }
    else
    {
        my_on_io_open_complete = on_io_open_complete;
        my_on_io_open_complete_context = on_io_open_complete_context;
        my_on_bytes_received = on_bytes_received;
        my_on_bytes_received_context = on_bytes_received_context;
        my_on_io_error = on_io_error;
        my_on_io_error_context = on_io_error_context;

        result = xio_open_shallReturn;
    }

    if (result == 0)
    {
        xio_dowork(xio);
    }
    return result;
}

int my_xio_close(XIO_HANDLE xio, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* on_io_close_complete_context)
{
    int result;
    if ((xio == NULL) ||
        (on_io_close_complete == NULL) || (on_io_close_complete_context == NULL))
    {
        result = MU_FAILURE;
    }
    else
    {
        my_on_io_close_complete = on_io_close_complete;
        my_on_io_close_complete_context = on_io_close_complete_context;
        result = xio_close_shallReturn;
        if (call_on_io_close_complete_in_xio_close)
        {
            my_on_io_close_complete(my_on_io_close_complete_context);
        }
    }
    return result;
}

int my_xio_send(XIO_HANDLE xio, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;
    if ((xio == NULL) ||
        (buffer == NULL) || (size == 0))
    {
        result = MU_FAILURE;

        if (call_on_send_complete_in_xio_send)
        {
            on_send_complete(callback_context, IO_SEND_ERROR);
        }
    }
    else
    {
        my_on_send_complete = on_send_complete;
        my_on_send_complete_context = callback_context;

        if (xio_send_transmited_buffer_target > 0)
        {
            xio_send_transmited_buffer_target--;
            if (xio_send_transmited_buffer_target == 0)
            {
                (void)memcpy(xio_send_transmited_buffer, buffer, size);
            }
        }
        result = xio_send_shallReturn[xio_send_shallReturn_counter];
        xio_send_shallReturn_counter++;

        if (call_on_send_complete_in_xio_send)
        {
            on_send_complete(callback_context, IO_SEND_OK);
        }
    }
    return result;
}

void my_xio_dowork(XIO_HANDLE xio)
{
    if (xio != NULL)
    {
        switch (*DoworkJobs)
        {
        case XIO_DOWORK_JOB_NONE:
            DoworkJobs++;
            break;
        case XIO_DOWORK_JOB_OPEN:
            if ((SkipDoworkJobsOpenResult--) <= 0)
            {
                if (my_on_io_open_complete != NULL)
                {
                    my_on_io_open_complete(my_on_io_open_complete_context, (*DoworkJobsOpenResult));
                }
                DoworkJobs++;
                DoworkJobsOpenResult++;
                SkipDoworkJobsOpenResult = 0;
            }
            break;
        case XIO_DOWORK_JOB_SEND:
            if ((SkipDoworkJobsSendResult--) <= 0)
            {
                if (my_on_send_complete != NULL)
                {
                    my_on_send_complete(my_on_send_complete_context, (*DoworkJobsSendResult));
                }
                DoworkJobs++;
                DoworkJobsSendResult++;
                SkipDoworkJobsSendResult = 0;
            }
            break;
        case XIO_DOWORK_JOB_RECEIVED:
            if (my_on_bytes_received != NULL)
            {
                my_on_bytes_received(my_on_bytes_received_context, DoworkJobsReceivedBuffer, DoworkJobsReceivedBuffer_size[DoworkJobsReceivedBuffer_counter]);
            }
            DoworkJobs++;
            if (DoworkJobsReceivedBuffer_counter < MAX_RECEIVE_BUFFER_SIZES-1)
            {
                DoworkJobsReceivedBuffer_counter++;
            }
            break;
        case XIO_DOWORK_JOB_CLOSE:
            if ((SkipDoworkJobsCloseResult--) <= 0)
            {
                if(DoworkJobsCloseSuccess)
                {
                    my_on_io_close_complete(my_on_io_close_complete_context);
                }
                DoworkJobs++;
                SkipDoworkJobsCloseResult = 0;
            }
            break;
        case XIO_DOWORK_JOB_ERROR:
            if (my_on_io_error != NULL)
            {
                my_on_io_error(my_on_io_error_context);
            }
            DoworkJobs++;
            break;
        default:
            break;
        }

    }
}

HTTP_HEADERS_HANDLE my_HTTPHeaders_Alloc(void)
{
    return (HTTP_HEADERS_HANDLE)malloc(1);
}

void my_HTTPHeaders_Free(HTTP_HEADERS_HANDLE handle)
{
    free(handle);
}

static BUFFER_HANDLE TestBufferHandle;

BUFFER_HANDLE my_BUFFER_new(void)
{
    return (BUFFER_HANDLE)malloc(1);
}

void my_BUFFER_delete(BUFFER_HANDLE handle)
{
    free(handle);
}

static HTTP_HEADERS_RESULT HTTPHeaders_GetHeaderCount_shallReturn;
HTTP_HEADERS_RESULT my_HTTPHeaders_GetHeaderCount(HTTP_HEADERS_HANDLE handle, size_t* headerCount)
{
    HTTP_HEADERS_RESULT result;

    if (handle == NULL)
    {
        result = HTTP_HEADERS_ERROR;
    }
    else
    {
        (*headerCount) = TEST_GET_HEADER_HEAD_COUNT;
        result = HTTPHeaders_GetHeaderCount_shallReturn;
    }

    return result;
}

static HTTP_HEADERS_RESULT HTTPHeaders_GetHeader_shallReturn;
HTTP_HEADERS_RESULT my_HTTPHeaders_GetHeader(HTTP_HEADERS_HANDLE handle, size_t index, char** destination)
{
    HTTP_HEADERS_RESULT result;

    if ((handle == NULL) || (destination == NULL) || (index > TEST_GET_HEADER_HEAD_COUNT))
    {
        result = HTTP_HEADERS_INVALID_ARG;
    }
    else
    {
        *destination = (char*)malloc(11 * sizeof(char));
        strcpy(*destination, "0123456789");
        result = HTTPHeaders_GetHeaderCount_shallReturn;
    }

    return result;
}

static const IO_INTERFACE_DESCRIPTION default_tlsio = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
const IO_INTERFACE_DESCRIPTION* my_platform_get_default_tlsio(void)
{
    return &default_tlsio;
}


static void createHttpObjects(HTTP_HEADERS_HANDLE* requestHttpHeaders, HTTP_HEADERS_HANDLE* responseHttpHeaders)
{
    STRICT_EXPECTED_CALL(HTTPHeaders_Alloc());
    STRICT_EXPECTED_CALL(gballoc_malloc(1));
    STRICT_EXPECTED_CALL(HTTPHeaders_Alloc());
    STRICT_EXPECTED_CALL(gballoc_malloc(1));

    /*assumed to never fail*/
    *requestHttpHeaders = HTTPHeaders_Alloc();
    *responseHttpHeaders = HTTPHeaders_Alloc();
    if (
        (*requestHttpHeaders == NULL) ||
        (*responseHttpHeaders == NULL)
        )
    {
        ASSERT_FAIL("unable to build test prerequisites");
    }
}

static void destroyHttpObjects(HTTP_HEADERS_HANDLE* requestHttpHeaders, HTTP_HEADERS_HANDLE* responseHttpHeaders)
{
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    HTTPHeaders_Free(*requestHttpHeaders);
    *requestHttpHeaders = NULL;
    HTTPHeaders_Free(*responseHttpHeaders);
    *responseHttpHeaders = NULL;
}

static HTTP_HANDLE createHttpConnection(void)
{
    xio_open_shallReturn = 0;
    xio_send_shallReturn_counter = 0;
    xio_send_shallReturn = (const int*)xio_send_0;

    my_on_io_open_complete = NULL;
    my_on_io_open_complete_context = NULL;

    my_on_io_close_complete = NULL;
    my_on_io_close_complete_context = NULL;

    my_on_send_complete = NULL;
    my_on_send_complete_context = NULL;

    my_on_bytes_received = NULL;
    my_on_bytes_received_context = NULL;

    my_on_io_error = NULL;
    my_on_io_error_context = NULL;

    HTTPHeaders_GetHeaderCount_shallReturn = HTTP_HEADERS_OK;
    xio_setoption_shallReturn = 0;

    current_xioCreate_must_fail = false;

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(platform_get_default_tlsio());
    STRICT_EXPECTED_CALL(xio_create(&default_tlsio, IGNORED_PTR_ARG)).IgnoreArgument(2);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);

    HTTPAPI_Init();
    return HTTPAPI_CreateConnection(TEST_CREATE_CONNECTION_HOST_NAME);    /* currentmalloc_call += 2 */
}

static void setHttpCertificate(HTTP_HANDLE httpHandle)
{
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    /*Tests_SRS_HTTPAPI_COMPACT_21_056: [ The HTTPAPI_SetOption shall change the HTTP options. ]*/
    /*Tests_SRS_HTTPAPI_COMPACT_21_057: [ The HTTPAPI_SetOption shall receive a handle that identiry the HTTP connection. ]*/
    /*Tests_SRS_HTTPAPI_COMPACT_21_058: [ The HTTPAPI_SetOption shall receive the option as a pair optionName/value. ]*/
    HTTPAPI_SetOption(httpHandle, "TrustedCerts", TEST_SETOPTIONS_CERTIFICATE);                /* currentmalloc_call += 1 */
}

static void setHttpx509ClientCertificateAndKey(HTTP_HANDLE httpHandle)
{
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    /*Tests_SRS_HTTPAPI_COMPACT_21_056: [ The HTTPAPI_SetOption shall change the HTTP options. ]*/
    /*Tests_SRS_HTTPAPI_COMPACT_21_057: [ The HTTPAPI_SetOption shall receive a handle that identiry the HTTP connection. ]*/
    /*Tests_SRS_HTTPAPI_COMPACT_21_058: [ The HTTPAPI_SetOption shall receive the option as a pair optionName/value. ]*/
    HTTPAPI_SetOption(httpHandle, SU_OPTION_X509_CERT, TEST_SETOPTIONS_X509CLIENTCERT);                /* currentmalloc_call += 1 */
    HTTPAPI_SetOption(httpHandle, SU_OPTION_X509_PRIVATE_KEY, TEST_SETOPTIONS_X509PRIVATEKEY);                /* currentmalloc_call += 1 */
}

static void setupAllCallBeforeOpenHTTPsequence(HTTP_HEADERS_HANDLE requestHttpHeaders, int numberOfDoWork, bool useClientCert)
{
    int i;

    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeaderCount(requestHttpHeaders, IGNORED_PTR_ARG))
        .IgnoreArgument(2);
    STRICT_EXPECTED_CALL(xio_setoption(IGNORED_PTR_ARG, "TrustedCerts", TEST_SETOPTIONS_CERTIFICATE))
        .IgnoreArgument(1)
        .IgnoreArgument(3);
    if (useClientCert == true)
    {
        STRICT_EXPECTED_CALL(xio_setoption(IGNORED_PTR_ARG, SU_OPTION_X509_CERT, TEST_SETOPTIONS_X509CLIENTCERT))
            .IgnoreArgument(1)
            .IgnoreArgument(3);
        STRICT_EXPECTED_CALL(xio_setoption(IGNORED_PTR_ARG, SU_OPTION_X509_PRIVATE_KEY, TEST_SETOPTIONS_X509PRIVATEKEY))
            .IgnoreArgument(1)
            .IgnoreArgument(3);
    }
    STRICT_EXPECTED_CALL(xio_open(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    for (i = 0; i < numberOfDoWork; i++)
    {
        STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
            .IgnoreArgument(1);
        if (i > 0)
        {
            STRICT_EXPECTED_CALL(ThreadAPI_Sleep(100));
        }
    }
}

#define TEST_RECEIVED_ANSWER (const unsigned char*)"HTTP/111.222 433 555\r\ncontent-length:10\r\ntransfer-encoding:\r\n\r\n0123456789\r\n\r\n"
static void setupAllCallBeforeReceiveHTTPsequenceWithSuccess()
{
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_NUM_ARG, DoworkJobsReceivedBuffer_size[0])).IgnoreArgument(1);

    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).IgnoreAllArguments();
    STRICT_EXPECTED_CALL(HTTPHeaders_AddHeaderNameValuePair(IGNORED_PTR_ARG, "content-length", "10")).IgnoreArgument(1);

    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).IgnoreAllArguments();
    STRICT_EXPECTED_CALL(HTTPHeaders_AddHeaderNameValuePair(IGNORED_PTR_ARG, "transfer-encoding", "")).IgnoreArgument(1);

    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).IgnoreAllArguments();
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
}

static void setupAllCallBeforeSendHTTPsequenceWithSuccess(HTTP_HEADERS_HANDLE requestHttpHeaders)
{
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeader(requestHttpHeaders, IGNORED_NUM_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeader(requestHttpHeaders, IGNORED_NUM_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();

    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
}

#define TEST_HEAD_RECEIVED_ANSWER (const unsigned char*)"HTTP/111.222 433 555\r\ncontent-length:10\r\ntransfer-encoding:\r\n\r\n"
static void setupAllCallBeforeReceiveHTTPHeadsequenceWithSuccess()
{
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG));

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_NUM_ARG, DoworkJobsReceivedBuffer_size[0]));

    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(HTTPHeaders_AddHeaderNameValuePair(IGNORED_PTR_ARG, "content-length", "10"));

    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(HTTPHeaders_AddHeaderNameValuePair(IGNORED_PTR_ARG, "transfer-encoding", ""));

    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
}

static const IO_OPEN_RESULT* DoworkJobsOpenResult_ReceiveHead = (const IO_OPEN_RESULT*)openresult_ok;
static const IO_SEND_RESULT* DoworkJobsSendResult_ReceiveHead = (const IO_SEND_RESULT*) sendresult_7ok;

static void PrepareReceiveHead(HTTP_HEADERS_HANDLE requestHttpHeaders, size_t bufferSize[], int doworkReduction[], int countSizes)
{
    int countBuffer;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);

    for (countBuffer = 0; countBuffer < countSizes; countBuffer++)
    {
        int countChar;
        if (countBuffer > 0)
        {
            STRICT_EXPECTED_CALL(ThreadAPI_Sleep(100));
        }
        STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
            .IgnoreArgument(1);
        STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_NUM_ARG, bufferSize[countBuffer])).IgnoreArgument(1);
        for (countChar = 0; countChar < doworkReduction[countBuffer]; countChar++)
        {
            STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
                .IgnoreArgument(1);
        }
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
            .IgnoreArgument(1);
    }

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;
}

IMPLEMENT_UMOCK_C_ENUM_TYPE(HTTP_HEADERS_RESULT, HTTP_HEADERS_RESULT_VALUES);

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(httpapicompact_ut)

TEST_SUITE_INITIALIZE(setsBufferTempSize)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    TestBufferHandle = BUFFER_new();
    ASSERT_IS_NULL(TestBufferHandle);

    REGISTER_TYPE(HTTP_HEADERS_RESULT, HTTP_HEADERS_RESULT);

    REGISTER_UMOCK_ALIAS_TYPE(HTTP_HEADERS_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(XIO_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_SEND_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_CLOSE_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_OPEN_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_RECEIVED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_ERROR, void*);
    REGISTER_UMOCK_ALIAS_TYPE(BUFFER_HANDLE, void*);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_HOOK(xio_create, my_xio_create);
    REGISTER_GLOBAL_MOCK_HOOK(xio_destroy, my_xio_destroy);
    REGISTER_GLOBAL_MOCK_HOOK(xio_setoption, my_xio_setoption);
    REGISTER_GLOBAL_MOCK_HOOK(xio_open, my_xio_open);
    REGISTER_GLOBAL_MOCK_HOOK(xio_close, my_xio_close);
    REGISTER_GLOBAL_MOCK_HOOK(xio_send, my_xio_send);
    REGISTER_GLOBAL_MOCK_HOOK(xio_dowork, my_xio_dowork);

    REGISTER_GLOBAL_MOCK_RETURN(HTTPHeaders_AddHeaderNameValuePair, HTTP_HEADERS_OK);
    REGISTER_GLOBAL_MOCK_HOOK(HTTPHeaders_Alloc, my_HTTPHeaders_Alloc);
    REGISTER_GLOBAL_MOCK_HOOK(HTTPHeaders_Free, my_HTTPHeaders_Free);
    REGISTER_GLOBAL_MOCK_HOOK(BUFFER_new, my_BUFFER_new);
    REGISTER_GLOBAL_MOCK_HOOK(BUFFER_delete, my_BUFFER_delete);
    REGISTER_GLOBAL_MOCK_HOOK(HTTPHeaders_GetHeaderCount, my_HTTPHeaders_GetHeaderCount);
    REGISTER_GLOBAL_MOCK_HOOK(HTTPHeaders_GetHeader, my_HTTPHeaders_GetHeader);

    REGISTER_GLOBAL_MOCK_HOOK(platform_get_default_tlsio, my_platform_get_default_tlsio);
    REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_s, my_mallocAndStrcpy_s);
}

TEST_SUITE_CLEANUP(TestClassCleanup)
{
    BUFFER_delete(TestBufferHandle);

    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
}

TEST_FUNCTION_INITIALIZE(f)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }

    umock_c_reset_all_calls();

    currentmalloc_call = 0;
    whenShallmalloc_fail = 0;

    xio_send_transmited_buffer[0] = '\0';

    call_on_send_complete_in_xio_send = true;
    SkipDoworkJobsOpenResult = 0;
    SkipDoworkJobsCloseResult = 0;
    SkipDoworkJobsSendResult = 0;

    xio_close_shallReturn = 0;
    DoworkJobsCloseSuccess = true;
    call_on_io_close_complete_in_xio_close = true;
}

TEST_FUNCTION_CLEANUP(cleans)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}


/* HTTPAPI_Init */

/*Tests_SRS_HTTPAPI_COMPACT_21_001: [ The httpapi_compact shall implement the methods defined by the httpapi.h. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_002: [ The httpapi_compact shall support the http requests. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_003: [ The httpapi_compact shall return error codes defined by HTTPAPI_RESULT. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_004: [ The HTTPAPI_Init shall allocate all memory to control the http protocol. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_007: [ If there is not enough memory to control the http protocol, the HTTPAPI_Init shall return HTTPAPI_ALLOC_FAILED. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_006: [ If HTTPAPI_Init succeed allocating all the needed memory, it shall return HTTPAPI_OK. ]*/
TEST_FUNCTION(HTTPAPI_Init__always_return_HTTPAPI_OK_Succeed)
{
    /// arrange
    int result;

    /// act
    result = HTTPAPI_Init();

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);

    /// cleanup
    //none
}


/* HTTPAPI_Deinit */

/*Tests_SRS_HTTPAPI_COMPACT_21_009: [ The HTTPAPI_Init shall release all memory allocated by the httpapi_compact. ]*/
TEST_FUNCTION(HTTPAPI_Deinit__just_call_Succeed)
{
    /// arrange
    HTTPAPI_Init();

    /// act
    HTTPAPI_Deinit();

    /// assert
    //none

    /// cleanup
    //none
}


/* HTTPAPI_CreateConnection */

/*Tests_SRS_HTTPAPI_COMPACT_21_014: [ If the hostName is NULL, the HTTPAPI_CreateConnection shall return NULL as the handle. ]*/
TEST_FUNCTION(HTTPAPI_CreateConnection__hostName_NULL_Failed)
{
    /// arrange
    HTTP_HANDLE httpHandle;
    HTTPAPI_Init();
    current_xioCreate_must_fail = false;

    /// act
    httpHandle = HTTPAPI_CreateConnection(NULL);    /* currentmalloc_call += 0 */

    /// assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);
    ASSERT_IS_NULL(httpHandle);

    /// cleanup
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_015: [ If the hostName is empty, the HTTPAPI_CreateConnection shall return NULL as the handle. ]*/
TEST_FUNCTION(HTTPAPI_CreateConnection__empty_hostName_Failed)
{
    /// arrange
    const char* hostName = "";
    HTTP_HANDLE httpHandle;
    HTTPAPI_Init();
    current_xioCreate_must_fail = false;

    /// act
    httpHandle = HTTPAPI_CreateConnection(hostName);    /* currentmalloc_call += 0 */

    /// assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);
    ASSERT_IS_NULL(httpHandle);

    /// cleanup
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_011: [ The HTTPAPI_CreateConnection shall create an http connection to the host specified by the hostName parameter. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_012: [ The HTTPAPI_CreateConnection shall return a non-NULL handle on success. ]*/
TEST_FUNCTION(HTTPAPI_CreateConnection__valid_hostName_Succeed)
{
    /// arrange
    HTTP_HANDLE httpHandle;
    HTTPAPI_Init();
    current_xioCreate_must_fail = false;
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(platform_get_default_tlsio());
    STRICT_EXPECTED_CALL(xio_create(&default_tlsio, IGNORED_PTR_ARG)).IgnoreArgument(2);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);

    /// act
    httpHandle = HTTPAPI_CreateConnection(TEST_CREATE_CONNECTION_HOST_NAME);    /* currentmalloc_call += 2 */

    /// assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 3, currentmalloc_call);
    ASSERT_IS_NOT_NULL(httpHandle);

    /// cleanup
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 2 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_013: [ If there is not enough memory to control the http connection, the HTTPAPI_CreateConnection shall return NULL as the handle. ]*/
TEST_FUNCTION(HTTPAPI_CreateConnection__no_enough_memory_failed)
{
    /// arrange
    HTTP_HANDLE httpHandle;
    current_xioCreate_must_fail = false;
    whenShallmalloc_fail = 1;
    HTTPAPI_Init();
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);

    /// act
    httpHandle = HTTPAPI_CreateConnection(TEST_CREATE_CONNECTION_HOST_NAME);    /* currentmalloc_call += 2 */

    /// assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);
    ASSERT_IS_NULL(httpHandle);

    /// cleanup
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_016: [ If the HTTPAPI_CreateConnection failed to create the connection, it shall return NULL as the handle. ]*/
TEST_FUNCTION(HTTPAPI_CreateConnection__create_xio_connection_failed)
{
    /// arrange
    HTTP_HANDLE httpHandle;
    current_xioCreate_must_fail = true;
    HTTPAPI_Init();

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(platform_get_default_tlsio());
    STRICT_EXPECTED_CALL(xio_create(&default_tlsio, IGNORED_PTR_ARG)).IgnoreArgument(2);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // hostname
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG)).IgnoreArgument(1);

    /// act
    httpHandle = HTTPAPI_CreateConnection(TEST_CREATE_CONNECTION_HOST_NAME);    /* currentmalloc_call += 2 */

    /// assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);
    ASSERT_IS_NULL(httpHandle);

    /// cleanup
    HTTPAPI_Deinit();
}

/* HTTPAPI_CloseConnection */

/*Tests_SRS_HTTPAPI_COMPACT_21_017: [ The HTTPAPI_CloseConnection shall close the connection previously created in HTTPAPI_ExecuteRequest. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_076: [ After close the connection, The HTTPAPI_CloseConnection shall destroy the connection previously created in HTTPAPI_CreateConnection. ]*/
TEST_FUNCTION(HTTPAPI_CloseConnection__valid_hostName_Succeed)
{
    /// arrange
    HTTP_HANDLE httpHandle = createHttpConnection();
    STRICT_EXPECTED_CALL(xio_close(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_destroy(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    /// act
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 2 */

    /// assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);

    /// cleanup
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_020: [ If the connection handle is NULL, the HTTPAPI_CloseConnection shall not do anything. ]*/
TEST_FUNCTION(HTTPAPI_CloseConnection__handle_NULL_Succeed)
{
    /// arrange
    HTTP_HANDLE httpHandle = NULL;
    HTTPAPI_Init();

    /// act
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 0 */

    /// assert
    ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);

    /// cleanup
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_018: [ If there is a certificate associated to this connection, the HTTPAPI_CloseConnection shall free all allocated memory for the certificate. ]*/
TEST_FUNCTION(HTTPAPI_CloseConnection__free_certificate_memory_succeed)
{
    /// arrange
    HTTP_HANDLE httpHandle = createHttpConnection();
    setHttpCertificate(httpHandle);
    STRICT_EXPECTED_CALL(xio_close(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_destroy(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // hostname

    /// act
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */

    /// assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);

    /// cleanup
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_06_001: [ If there is a x509 client certificate associated to this connection, the HTTAPI_CloseConnection shall free all allocated memory for the certificate. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_06_002: [ If there is a x509 client private key associated to this connection, then HTTP_CloseConnection shall free all the allocated memory for the private key. ]*/
TEST_FUNCTION(HTTPAPI_CloseConnection__free_x509client_memory_succeed)
{
    /// arrange
    HTTP_HANDLE httpHandle = createHttpConnection();
    setHttpx509ClientCertificateAndKey(httpHandle);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(xio_close(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_destroy(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)) // From the xio destroy
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)) // the cert
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)) // the key
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)) // the instance.
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // hostname

    /// act
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */

    /// assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);

    /// cleanup
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_087: [ If the xio return anything different than 0, the HTTPAPI_CloseConnection shall destroy the connection anyway. ]*/
TEST_FUNCTION(HTTPAPI_CloseConnection__return_LINE_failed)
{
    /// arrange
    HTTP_HANDLE httpHandle = createHttpConnection();

    xio_close_shallReturn = MU_FAILURE;
    DoworkJobsCloseSuccess = true;
    SkipDoworkJobsCloseResult = 0;
    call_on_io_close_complete_in_xio_close = true;

    STRICT_EXPECTED_CALL(xio_close(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_destroy(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // hostname

    /// act
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 2 */

                                            /// assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);

    /// cleanup
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_084: [ The HTTPAPI_CloseConnection shall wait, at least, 10 seconds for the SSL close process. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_086: [ The HTTPAPI_CloseConnection shall wait, at least, 100 milliseconds between retries. ]*/
TEST_FUNCTION(HTTPAPI_CloseConnection__close_on_dowork_succeed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rce;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);
    setupAllCallBeforeReceiveHTTPsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    xio_close_shallReturn = 0;
    DoworkJobsCloseSuccess = true;
    SkipDoworkJobsCloseResult = 0;
    call_on_io_close_complete_in_xio_close = false;

    STRICT_EXPECTED_CALL(xio_close(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_destroy(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // hostname

    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);

    /// act
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */

    /// assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 2, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_084: [ The HTTPAPI_CloseConnection shall wait, at least, 10 seconds for the SSL close process. ]*/
TEST_FUNCTION(HTTPAPI_CloseConnection__close_on_dowork_retry_n_succeed)
{
    /// arrange
    int i;
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rce;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);
    setupAllCallBeforeReceiveHTTPsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    xio_close_shallReturn = 0;
    DoworkJobsCloseSuccess = true;
    SkipDoworkJobsCloseResult = 90;
    call_on_io_close_complete_in_xio_close = false;

    STRICT_EXPECTED_CALL(xio_close(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    for (i = 0; i < SkipDoworkJobsCloseResult; i++)
    {
        STRICT_EXPECTED_CALL(xio_dowork(IGNORED_PTR_ARG))
            .IgnoreArgument(1);
        STRICT_EXPECTED_CALL(ThreadAPI_Sleep(100));
    }
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_destroy(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // hostname

    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);

    /// act
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */

                                            /// assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 2, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_084: [ The HTTPAPI_CloseConnection shall wait, at least, 10 seconds for the SSL close process. ]*/
TEST_FUNCTION(HTTPAPI_CloseConnection__close_on_dowork_retry_n_failed)
{
    /// arrange
    int i;
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rc_error;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);
    setupAllCallBeforeReceiveHTTPsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    xio_close_shallReturn = 0;
    DoworkJobsCloseSuccess = false;
    SkipDoworkJobsCloseResult = 90;
    call_on_io_close_complete_in_xio_close = false;

    STRICT_EXPECTED_CALL(xio_close(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    for (i = 0; i < SkipDoworkJobsCloseResult + 1; i++)
    {
        STRICT_EXPECTED_CALL(xio_dowork(IGNORED_PTR_ARG))
            .IgnoreArgument(1);
        STRICT_EXPECTED_CALL(ThreadAPI_Sleep(100));
    }
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_destroy(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);

    /// act
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */

                                            /// assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 2, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_085: [ If the HTTPAPI_CloseConnection retries 10 seconds to close the connection without success, it shall destroy the connection anyway. ]*/
TEST_FUNCTION(HTTPAPI_CloseConnection__close_timeout_failed)
{
    /// arrange
    int i;
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rce;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);
    setupAllCallBeforeReceiveHTTPsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    xio_close_shallReturn = 0;
    DoworkJobsCloseSuccess = true;
    SkipDoworkJobsCloseResult = 101;
    call_on_io_close_complete_in_xio_close = false;

    STRICT_EXPECTED_CALL(xio_close(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    for (i = 0; i < SkipDoworkJobsCloseResult; i++)
    {
        STRICT_EXPECTED_CALL(xio_dowork(IGNORED_PTR_ARG))
            .IgnoreArgument(1);
        STRICT_EXPECTED_CALL(ThreadAPI_Sleep(100));
    }
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_destroy(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); // hostname

    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);

    /// act
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */

                                            /// assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 2, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_Deinit();
}

/* HTTPAPI_ExecuteRequest */

/*Tests_SRS_HTTPAPI_COMPACT_21_034: [ If there is no previous connection, the HTTPAPI_ExecuteRequest shall return HTTPAPI_INVALID_ARG. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__NULL_handle_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);

    /// act
    result = HTTPAPI_ExecuteRequest(
        (HTTP_HANDLE)NULL,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_INVALID_ARG, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 2, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_037: [ If the request type is unknown, the HTTPAPI_ExecuteRequest shall return HTTPAPI_INVALID_ARG. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__invalid_request_type_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        (HTTPAPI_REQUEST_TYPE)MU_ENUM_VALUE_COUNT(HTTPAPI_REQUEST_TYPE_VALUES),
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_INVALID_ARG, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 5, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 2 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_039: [ If the relativePath is NULL or invalid, the HTTPAPI_ExecuteRequest shall return HTTPAPI_INVALID_ARG. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__NULL_relative_path_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        NULL,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_INVALID_ARG, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 5, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 2 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_041: [ If the httpHeadersHandle is NULL or invalid, the HTTPAPI_ExecuteRequest shall return HTTPAPI_INVALID_ARG. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__NULL_http_headers_handle_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        (HTTP_HEADERS_HANDLE)NULL,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_INVALID_ARG, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 5, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 2 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_041: [ If the httpHeadersHandle is NULL or invalid, the HTTPAPI_ExecuteRequest shall return HTTPAPI_INVALID_ARG. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__invalid_http_headers_handle_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    HTTPHeaders_GetHeaderCount_shallReturn = HTTP_HEADERS_INVALID_ARG;

    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeaderCount(requestHttpHeaders, IGNORED_PTR_ARG))
        .IgnoreArgument(2);

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_INVALID_ARG, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 5, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 2 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_054: [ If Http header maker cannot provide the number of headers, the HTTPAPI_ExecuteRequest shall return HTTPAPI_INVALID_ARG. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__http_headers_handle_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    HTTPHeaders_GetHeaderCount_shallReturn = HTTP_HEADERS_ERROR;

    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeaderCount(requestHttpHeaders, IGNORED_PTR_ARG))
        .IgnoreArgument(2);

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_INVALID_ARG, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 5, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 2 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_023: [ If the transport failed setting the Certificate, the HTTPAPI_ExecuteRequest shall not send any request and return HTTPAPI_SET_OPTION_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__certificate_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();

    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);
    xio_setoption_shallReturn = MU_FAILURE;

    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeaderCount(requestHttpHeaders, IGNORED_PTR_ARG))
        .IgnoreArgument(2);
    STRICT_EXPECTED_CALL(xio_setoption(IGNORED_PTR_ARG, "TrustedCerts", TEST_SETOPTIONS_CERTIFICATE))
        .IgnoreArgument(1)
        .IgnoreArgument(3);

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_SET_OPTION_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_06_005: [ If the transport failed setting the client certificate, the HTTPAPI_ExecuteRequest shall not send any request and return HTTPAPI_SET_OPTION_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__x509client_certificate_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();

    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    HTTPAPI_SetOption(httpHandle, SU_OPTION_X509_CERT, TEST_SETOPTIONS_X509CLIENTCERT);                /* currentmalloc_call += 1 */

    xio_setoption_shallReturn = MU_FAILURE;

    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeaderCount(requestHttpHeaders, IGNORED_PTR_ARG))
        .IgnoreArgument(2);
    STRICT_EXPECTED_CALL(xio_setoption(IGNORED_PTR_ARG, SU_OPTION_X509_CERT, TEST_SETOPTIONS_X509CLIENTCERT))
        .IgnoreArgument(1)
        .IgnoreArgument(3);

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_SET_OPTION_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_06_006: [ If the transport failed setting the client certificate private key, the HTTPAPI_ExecuteRequest shall not send any request and return HTTPAPI_SET_OPTION_FAILED. ] */
TEST_FUNCTION(HTTPAPI_ExecuteRequest__x509client_privatekey_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();

    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    HTTPAPI_SetOption(httpHandle, SU_OPTION_X509_PRIVATE_KEY, TEST_SETOPTIONS_X509PRIVATEKEY);                /* currentmalloc_call += 1 */

    xio_setoption_shallReturn = MU_FAILURE;

    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeaderCount(requestHttpHeaders, IGNORED_PTR_ARG))
        .IgnoreArgument(2);
    STRICT_EXPECTED_CALL(xio_setoption(IGNORED_PTR_ARG, SU_OPTION_X509_PRIVATE_KEY, TEST_SETOPTIONS_X509PRIVATEKEY))
        .IgnoreArgument(1)
        .IgnoreArgument(3);

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_SET_OPTION_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_052: [ If any memory allocation get fail, the HTTPAPI_ExecuteRequest shall return HTTPAPI_ALLOC_FAILED. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_062: [ If any memory allocation get fail, the HTTPAPI_SetOption shall return HTTPAPI_ALLOC_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__certificate_out_of_memory_failed)
{
    /// arrange
    HTTPAPI_RESULT result;
    HTTP_HANDLE httpHandle = createHttpConnection();
    xio_setoption_shallReturn = 0;
    whenShallmalloc_fail = 1;

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .IgnoreArgument(1);

    /// act
    result = HTTPAPI_SetOption(httpHandle, "TrustedCerts", TEST_SETOPTIONS_CERTIFICATE);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_ALLOC_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 3, currentmalloc_call);

    /// cleanup
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 2 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_064: [ If the HTTPAPI_SetOption get success setting the option, it shall return HTTPAPI_OK. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__certificate_succeed)
{
    /// arrange
    HTTPAPI_RESULT result;
    HTTP_HANDLE httpHandle = createHttpConnection();
    xio_setoption_shallReturn = 0;

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .IgnoreArgument(1);

    /// act
    result = HTTPAPI_SetOption(httpHandle, "TrustedCerts", TEST_SETOPTIONS_CERTIFICATE);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 4, currentmalloc_call);

    /// cleanup
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}



/*Tests_SRS_HTTPAPI_COMPACT_21_059: [ If the handle is NULL, the HTTPAPI_SetOption shall return HTTPAPI_INVALID_ARG. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__certificate_NULL_handle_failed)
{
    /// arrange
    HTTPAPI_RESULT result;

    /// act
    result = HTTPAPI_SetOption(NULL, "TrustedCerts", TEST_SETOPTIONS_CERTIFICATE);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_INVALID_ARG, result);

    /// cleanup
}

/*Tests_SRS_HTTPAPI_COMPACT_21_060: [ If the optionName is NULL, the HTTPAPI_SetOption shall return HTTPAPI_INVALID_ARG. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__certificate_NULL_optionName_failed)
{
    /// arrange
    HTTPAPI_RESULT result;
    HTTP_HANDLE httpHandle = createHttpConnection();
    xio_setoption_shallReturn = 0;

    /// act
    result = HTTPAPI_SetOption(httpHandle, NULL, TEST_SETOPTIONS_CERTIFICATE);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_INVALID_ARG, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 3, currentmalloc_call);

    /// cleanup
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 2 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_063: [ If the HTTP do not support the optionName, the HTTPAPI_SetOption shall return HTTPAPI_INVALID_ARG. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__certificate_invalid_optionName_failed)
{
    /// arrange
    HTTPAPI_RESULT result;
    HTTP_HANDLE httpHandle = createHttpConnection();
    xio_setoption_shallReturn = 0;

    /// act
    result = HTTPAPI_SetOption(httpHandle, "InvalidOptionName", TEST_SETOPTIONS_CERTIFICATE);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_INVALID_ARG, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 3, currentmalloc_call);

    /// cleanup
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 2 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_061: [ If the value is NULL, the HTTPAPI_SetOption shall return HTTPAPI_INVALID_ARG. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__certificate_NULL_value_failed)
{
    /// arrange
    HTTPAPI_RESULT result;
    HTTP_HANDLE httpHandle = createHttpConnection();
    xio_setoption_shallReturn = 0;

    /// act
    result = HTTPAPI_SetOption(httpHandle, "TrustedCerts", NULL);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_INVALID_ARG, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 3, currentmalloc_call);

    /// cleanup
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 2 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_052: [ If any memory allocation get fail, the HTTPAPI_ExecuteRequest shall return HTTPAPI_ALLOC_FAILED. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_070: [ If any memory allocation get fail, the HTTPAPI_CloneOption shall return HTTPAPI_ALLOC_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__clone_certificate_out_of_memory_failed)
{
    /// arrange
    HTTPAPI_RESULT result;
    unsigned char* cloneCertificate;
    whenShallmalloc_fail = 1;

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .IgnoreArgument(1);

    /// act
    result = HTTPAPI_CloneOption("TrustedCerts", TEST_SETOPTIONS_CERTIFICATE, (const void**)&cloneCertificate);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_ALLOC_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);

    /// cleanup
}

/*Tests_SRS_HTTPAPI_COMPACT_21_065: [ The HTTPAPI_CloneOption shall provide the means to clone the HTTP option. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_066: [ The HTTPAPI_CloneOption shall return a clone of the value identified by the optionName. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_072: [ If the HTTPAPI_CloneOption get success setting the option, it shall return HTTPAPI_OK. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__clone_certificate_succeed)
{
    /// arrange
    HTTPAPI_RESULT result;
    unsigned char* cloneCertificate;

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .IgnoreArgument(1);

    /// act
    result = HTTPAPI_CloneOption("TrustedCerts", TEST_SETOPTIONS_CERTIFICATE, (const void**)&cloneCertificate);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, (char*)TEST_SETOPTIONS_CERTIFICATE, (char*)cloneCertificate);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 1, currentmalloc_call);

    /// cleanup
    free((void*)cloneCertificate);
}

/*Tests_SRS_HTTPAPI_COMPACT_21_067: [ If the optionName is NULL, the HTTPAPI_CloneOption shall return HTTPAPI_INVALID_ARG. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__clone_certificate_NULL_optionName_failed)
{
    /// arrange
    HTTPAPI_RESULT result;
    unsigned char* cloneCertificate;

    /// act
    result = HTTPAPI_CloneOption(NULL, TEST_SETOPTIONS_CERTIFICATE, (const void**)&cloneCertificate);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_INVALID_ARG, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);

    /// cleanup
}

/*Tests_SRS_HTTPAPI_COMPACT_21_068: [ If the value is NULL, the HTTPAPI_CloneOption shall return HTTPAPI_INVALID_ARG. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__clone_certificate_NULL_value_failed)
{
    /// arrange
    HTTPAPI_RESULT result;
    unsigned char* cloneCertificate;

    /// act
    result = HTTPAPI_CloneOption("TrustedCerts", NULL, (const void**)&cloneCertificate);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_INVALID_ARG, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);

    /// cleanup
}

/*Tests_SRS_HTTPAPI_COMPACT_21_069: [ If the savedValue is NULL, the HTTPAPI_CloneOption shall return HTTPAPI_INVALID_ARG. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__clone_certificate_NULL_savadValue_failed)
{
    /// arrange
    HTTPAPI_RESULT result;

    /// act
    result = HTTPAPI_CloneOption("TrustedCerts", TEST_SETOPTIONS_CERTIFICATE, NULL);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_INVALID_ARG, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);

    /// cleanup
}

/*Tests_SRS_HTTPAPI_COMPACT_21_071: [ If the HTTP do not support the optionName, the HTTPAPI_CloneOption shall return HTTPAPI_INVALID_ARG. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__clone_certificate_invalid_optionName_failed)
{
    /// arrange
    HTTPAPI_RESULT result;
    unsigned char* cloneCertificate;

    /// act
    result = HTTPAPI_CloneOption("InvalidOptionName", TEST_SETOPTIONS_CERTIFICATE, (const void**)&cloneCertificate);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_INVALID_ARG, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, currentmalloc_call);

    /// cleanup
}

/*Tests_SRS_HTTPAPI_COMPACT_21_025: [ If the open process failed, the HTTPAPI_ExecuteRequest shall not send any request and return HTTPAPI_OPEN_REQUEST_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__xoi_open_returns_LINE_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);
    xio_open_shallReturn = MU_FAILURE;
    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 0, false);

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OPEN_REQUEST_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_025: [ If the open process failed, the HTTPAPI_ExecuteRequest shall not send any request and return HTTPAPI_OPEN_REQUEST_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__on_io_open_complete_with_error_on_openning_failed)
{
    /// arrange
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle;
    unsigned int statusCode;
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    httpHandle = createHttpConnection();
    setHttpCertificate(httpHandle);

    DoworkJobs = (const xio_dowork_job*)doworkjob_oe;
    DoworkJobsOpenResult = (const IO_OPEN_RESULT*)openresult_error;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OPEN_REQUEST_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_025: [ If the open process failed, the HTTPAPI_ExecuteRequest shall not send any request and return HTTPAPI_OPEN_REQUEST_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__on_io_open_complete_with_error_on_working_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobs = (const xio_dowork_job*) doworkjob_4none_oe;
    DoworkJobsOpenResult = (const IO_OPEN_RESULT*)openresult_error;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 5, false);

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OPEN_REQUEST_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_077: [ The HTTPAPI_ExecuteRequest shall wait, at least, 10 seconds for the SSL open process. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_078: [ If the HTTPAPI_ExecuteRequest cannot open the connection in 10 seconds, it shall fail and return HTTPAPI_OPEN_REQUEST_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__on_io_open_complete_with_error_after_n_retry_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobs = (const xio_dowork_job*)doworkjob_4none_oe;
    DoworkJobsOpenResult = (const IO_OPEN_RESULT*)openresult_error;

    SkipDoworkJobsOpenResult = 5;
    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, SkipDoworkJobsOpenResult+5, false);

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OPEN_REQUEST_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_077: [ The HTTPAPI_ExecuteRequest shall wait, at least, 10 seconds for the SSL open process. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_078: [ If the HTTPAPI_ExecuteRequest cannot open the connection in 10 seconds, it shall fail and return HTTPAPI_OPEN_REQUEST_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__on_io_open_complete_with_timeout_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobs = (const xio_dowork_job*)doworkjob_4none_oe;
    DoworkJobsOpenResult = (const IO_OPEN_RESULT*)openresult_ok;

    SkipDoworkJobsOpenResult = 98;
    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, SkipDoworkJobsOpenResult + 4, false);
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OPEN_REQUEST_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_025: [ If the open process failed, the HTTPAPI_ExecuteRequest shall not send any request and return HTTPAPI_OPEN_REQUEST_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__on_io_error_on_openning_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobs = (const xio_dowork_job*)doworkjob_ee;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OPEN_REQUEST_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_025: [ If the open process failed, the HTTPAPI_ExecuteRequest shall not send any request and return HTTPAPI_OPEN_REQUEST_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__on_io_error_on_working_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobs = (const xio_dowork_job*)doworkjob_4none_ee;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 5, false);

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OPEN_REQUEST_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders);    /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_027: [ If the HTTPAPI_ExecuteRequest cannot create a buffer to send the request, it shall not send any request and return HTTPAPI_STRING_PROCESSING_ERROR. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__huge_relative_path_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    size_t i;
    char hugeRelativePath[HUGE_RELATIVE_PATH_SIZE];
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);

    DoworkJobs = (const xio_dowork_job*)doworkjob_oe;
    DoworkJobsOpenResult = (const IO_OPEN_RESULT*)openresult_ok;

    setHttpCertificate(httpHandle);
    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);

    for (i = 0; i < HUGE_RELATIVE_PATH_SIZE; i++)
    {
        hugeRelativePath[i] = 'a';
    }
    hugeRelativePath[HUGE_RELATIVE_PATH_SIZE - 1] = '\0';

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        hugeRelativePath,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_STRING_PROCESSING_ERROR, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_026: [ If the open process succeed, the HTTPAPI_ExecuteRequest shall send the request message to the host. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_024: [ The HTTPAPI_ExecuteRequest shall open the transport connection with the host to send the request. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_022: [ If a Certificate was provided, the HTTPAPI_ExecuteRequest shall set this option on the transport layer. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_06_003: [ If the x509 client certificate is provided, the HTTPAPI_ExecuteRequest shall set this option on the transport layer. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_06_004: [ If the x509 client certificate private key is provided, the HTTPAPI_ExecuteRequest shall set this optionon the transport layer. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_028: [ If the HTTPAPI_ExecuteRequest cannot send the request header, it shall return HTTPAPI_SEND_REQUEST_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__io_send_header_return_error_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);

    DoworkJobs = (const xio_dowork_job*)doworkjob_oe;
    DoworkJobsOpenResult = (const IO_OPEN_RESULT*)openresult_ok;

    setHttpCertificate(httpHandle);
    setHttpx509ClientCertificateAndKey(httpHandle);
    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, true);
    xio_send_shallReturn = (const int*)xio_send_e;
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_SEND_REQUEST_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 8, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_028: [ If the HTTPAPI_ExecuteRequest cannot send the request header, it shall return HTTPAPI_SEND_REQUEST_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__on_send_header_complete_with_success_before_error_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobs = (const xio_dowork_job*)doworkjob_oe;
    DoworkJobsOpenResult = (const IO_OPEN_RESULT*)openresult_ok;
    DoworkJobsSendResult = (const IO_SEND_RESULT*)sendresult_o_3error;
    xio_send_shallReturn = (const int*)xio_send_0_e;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeader(requestHttpHeaders, IGNORED_NUM_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_SEND_REQUEST_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_028: [ If the HTTPAPI_ExecuteRequest cannot send the request header, it shall return HTTPAPI_SEND_REQUEST_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__on_send_header_complete_with_2_success_before_error_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobs = (const xio_dowork_job*)doworkjob_oe;
    DoworkJobsOpenResult = (const IO_OPEN_RESULT*)openresult_ok;
    DoworkJobsSendResult = (const IO_SEND_RESULT*)sendresult_o_3error;
    xio_send_shallReturn = (const int*)xio_send_00_e;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeader(requestHttpHeaders, IGNORED_NUM_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_SEND_REQUEST_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_079: [ The HTTPAPI_ExecuteRequest shall wait, at least, 20 seconds to send a buffer using the SSL connection. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_080: [ If the HTTPAPI_ExecuteRequest retries to send the message for 20 seconds without success, it shall fail and return HTTPAPI_SEND_REQUEST_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__on_send_header_complete_timeout_failed)
{
    /// arrange
    int i;
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobs = (const xio_dowork_job*)doworkjob_ose;
    DoworkJobsOpenResult = (const IO_OPEN_RESULT*)openresult_ok;
    DoworkJobsSendResult = (const IO_SEND_RESULT*)sendresult_o_3error;
    xio_send_shallReturn = (const int*)xio_send_00_e;
    call_on_send_complete_in_xio_send = false;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    SkipDoworkJobsSendResult = 200;
    for (i = 0; i < SkipDoworkJobsSendResult; i++)
    {
        STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
            .IgnoreArgument(1);
        STRICT_EXPECTED_CALL(ThreadAPI_Sleep(100));
    }
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_SEND_REQUEST_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_079: [ The HTTPAPI_ExecuteRequest shall wait, at least, 20 seconds to send a buffer using the SSL connection. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_080: [ If the HTTPAPI_ExecuteRequest retries to send the message for 20 seconds without success, it shall fail and return HTTPAPI_SEND_REQUEST_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__on_send_header_complete_retry_n_and_failed)
{
    /// arrange
    int i;
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobs = (const xio_dowork_job*)doworkjob_ose;
    DoworkJobsOpenResult = (const IO_OPEN_RESULT*)openresult_ok;
    DoworkJobsSendResult = (const IO_SEND_RESULT*)sendresult_error;
    xio_send_shallReturn = (const int*)xio_send_00_e;
    call_on_send_complete_in_xio_send = false;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    SkipDoworkJobsSendResult = 10;
    for (i = 0; i < SkipDoworkJobsSendResult; i++)
    {
        STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
            .IgnoreArgument(1);
        STRICT_EXPECTED_CALL(ThreadAPI_Sleep(100));
    }
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_SEND_REQUEST_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_029: [ If the HTTPAPI_ExecuteRequest cannot send the buffer with the request, it shall return HTTPAPI_SEND_REQUEST_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__on_send_buffer_complete_with_error_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobs = (const xio_dowork_job*)doworkjob_oe;
    DoworkJobsOpenResult = (const IO_OPEN_RESULT*)openresult_ok;
    DoworkJobsSendResult = (const IO_SEND_RESULT*)sendresult_6ok_error;
    xio_send_shallReturn = (const int*)xio_send_6x0_e;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);

    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeader(requestHttpHeaders, IGNORED_NUM_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeader(requestHttpHeaders, IGNORED_NUM_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();

    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_SEND_REQUEST_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_030: [ At the end of the transmission, the HTTPAPI_ExecuteRequest shall receive the response from the host. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_032: [ If the HTTPAPI_ExecuteRequest cannot read the message with the request result, it shall return HTTPAPI_READ_DATA_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__on_read_header_failed_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobs = (const xio_dowork_job*)doworkjob_oee;
    DoworkJobsOpenResult = (const IO_OPEN_RESULT*)openresult_ok;
    DoworkJobsSendResult = (const IO_SEND_RESULT*)sendresult_7ok;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);

    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_READ_DATA_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_032: [ If the HTTPAPI_ExecuteRequest cannot read the message with the request result, it shall return HTTPAPI_READ_DATA_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__on_read_NULL_header_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobsReceivedBuffer = NULL;
    DoworkJobsReceivedBuffer_size[0] = 10;
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_re;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);

    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_READ_DATA_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_055: [ If the HTTPAPI_ExecuteRequest cannot parser the received message, it shall return HTTPAPI_RECEIVE_RESPONSE_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__on_read_not_HTTP_header_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobsReceivedBuffer = (const unsigned char*)"HTTPS/111.222 433 555\r\n";
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_re;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);

    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_NUM_ARG, DoworkJobsReceivedBuffer_size[0])).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_RECEIVE_RESPONSE_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_055: [ If the HTTPAPI_ExecuteRequest cannot parser the received message, it shall return HTTPAPI_RECEIVE_RESPONSE_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__on_read_wrong_URL_header_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    int doworkReduction[1] = { 0 };
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobsReceivedBuffer = (const unsigned char*)"HTTP/111222 433 555\r\n";
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    PrepareReceiveHead(requestHttpHeaders, DoworkJobsReceivedBuffer_size, doworkReduction, 1);
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_re;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_RECEIVE_RESPONSE_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_055: [ If the HTTPAPI_ExecuteRequest cannot parser the received message, it shall return HTTPAPI_RECEIVE_RESPONSE_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__on_read_header_with_no_statusCode_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    int doworkReduction[1] = { 0 };
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobsReceivedBuffer = (const unsigned char*)"HTTP/111.222\r\n";
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    PrepareReceiveHead(requestHttpHeaders, DoworkJobsReceivedBuffer_size, doworkReduction, 1);
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_re;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_RECEIVE_RESPONSE_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_055: [ If the HTTPAPI_ExecuteRequest cannot parser the received message, it shall return HTTPAPI_RECEIVE_RESPONSE_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__on_read_header_incomplete_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    int doworkReduction[1] = { 0 };
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobsReceivedBuffer = (const unsigned char*)"HTTP/111\r\n";
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    PrepareReceiveHead(requestHttpHeaders, DoworkJobsReceivedBuffer_size, doworkReduction, 1);
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_re;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_RECEIVE_RESPONSE_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_055: [ If the HTTPAPI_ExecuteRequest cannot parser the received message, it shall return HTTPAPI_RECEIVE_RESPONSE_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__on_read_multi_header_with_size_0_and_error_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    int doworkReduction[2] = { 0, 0 };
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobsReceivedBuffer = (const unsigned char*)"HTTP/111\r\n";
    DoworkJobsReceivedBuffer_size[0] = 0;
    DoworkJobsReceivedBuffer_size[1] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    PrepareReceiveHead(requestHttpHeaders, DoworkJobsReceivedBuffer_size, doworkReduction, 2);
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rre;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_RECEIVE_RESPONSE_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_032: [ If the HTTPAPI_ExecuteRequest cannot read the message with the request result, it shall return HTTPAPI_READ_DATA_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__read_huge_header_failed)
{
    /// arrange
    HTTP_HANDLE httpHandle;
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    int doworkReduction[1] = { 0 };
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    size_t i;
    unsigned char hugeBuffer[3000] = "HTTP/111.";
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    for (i = strlen((const char*)hugeBuffer); i < 3000; i++)
    {
        hugeBuffer[i] = 'a';
    }
    hugeBuffer[2995] = '\r';
    hugeBuffer[2996] = '\n';
    hugeBuffer[2997] = '\r';
    hugeBuffer[2998] = '\n';
    hugeBuffer[2999] = '\0';

    httpHandle = createHttpConnection();
    setHttpCertificate(httpHandle);

    DoworkJobsReceivedBuffer = (const unsigned char*)hugeBuffer;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    PrepareReceiveHead(requestHttpHeaders, DoworkJobsReceivedBuffer_size, doworkReduction, 1);
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_re;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_READ_DATA_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_032: [ If the HTTPAPI_ExecuteRequest cannot read the message with the request result, it shall return HTTPAPI_READ_DATA_FAILED. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__content_length_without_value_failed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    int doworkReduction[1] = { 1 };
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobsReceivedBuffer = (const unsigned char*)"HTTP/111.222 433 555\r\ncontent-length:\r\n\r\n";
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    PrepareReceiveHead(requestHttpHeaders, DoworkJobsReceivedBuffer_size, doworkReduction, 1);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_re;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_READ_DATA_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_031: [ After receive the response, the HTTPAPI_ExecuteRequest shall close the transport connection with the host. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_033: [ If the whole process succeed, the HTTPAPI_ExecuteRequest shall retur HTTPAPI_OK. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_021: [ The HTTPAPI_ExecuteRequest shall execute the http communtication with the provided host, sending a request and reciving the response. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_040: [ The request shall contain the http header provided in httpHeadersHandle parameter. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_042: [ The request can contain the a content message, provided in content parameter. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_046: [ The HTTPAPI_ExecuteRequest shall return the http status reported by the host in the received response. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_047: [ The HTTPAPI_ExecuteRequest shall report the status in the statusCode parameter. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_050: [ If there is a content in the response, the HTTPAPI_ExecuteRequest shall copy it in the responseContent buffer. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_073: [ The message received by the HTTPAPI_ExecuteRequest shall starts with a valid header. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_074: [ After the header, the message received by the HTTPAPI_ExecuteRequest can contain addition information about the content. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_053: [ The HTTPAPI_ExecuteRequest shall produce a set of http header to send to the host. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__Execute_request_succeed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rce;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);
    setupAllCallBeforeReceiveHTTPsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);
    ASSERT_ARE_EQUAL(int, 433, statusCode);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_077: [ The HTTPAPI_ExecuteRequest shall wait, at least, 10 seconds for the SSL open process. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__Execute_request_retry_open_succeed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rce;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    SkipDoworkJobsOpenResult = 97;
    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, SkipDoworkJobsOpenResult + 1, false);

    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);
    setupAllCallBeforeReceiveHTTPsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);
    ASSERT_ARE_EQUAL(int, 433, statusCode);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_079: [ The HTTPAPI_ExecuteRequest shall wait, at least, 20 seconds to send a buffer using the SSL connection. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__Execute_request_retry_send_succeed)
{
    /// arrange
    unsigned int statusCode;
    int i;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_sre;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;
    call_on_send_complete_in_xio_send = false;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);

    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    SkipDoworkJobsSendResult = 199;
    for (i = 0; i < SkipDoworkJobsSendResult+1; i++)
    {
        STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
            .IgnoreArgument(1);
        STRICT_EXPECTED_CALL(ThreadAPI_Sleep(100));
    }
    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeader(requestHttpHeaders, IGNORED_NUM_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(ThreadAPI_Sleep(100));
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(ThreadAPI_Sleep(100));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeader(requestHttpHeaders, IGNORED_NUM_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(ThreadAPI_Sleep(100));
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(ThreadAPI_Sleep(100));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(ThreadAPI_Sleep(100));

    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(ThreadAPI_Sleep(100));

    setupAllCallBeforeReceiveHTTPsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);
    ASSERT_ARE_EQUAL(int, 433, statusCode);
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_035: [ The HTTPAPI_ExecuteRequest shall execute resquest for types GET, POST, PUT, DELETE, PATCH, HEAD. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_036: [ The request type shall be provided in the parameter requestType. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__request_get_succeed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rce;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);
    setupAllCallBeforeReceiveHTTPsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;
    xio_send_transmited_buffer_target = 1;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);
    ASSERT_ARE_EQUAL(int, 433, statusCode);
    xio_send_transmited_buffer[3] = '\0';
    ASSERT_ARE_EQUAL(char_ptr, "GET", xio_send_transmited_buffer);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_035: [ The HTTPAPI_ExecuteRequest shall execute resquest for types GET, POST, PUT, DELETE, PATCH, HEAD. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__request_post_succeed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);

    setHttpCertificate(httpHandle);
    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rce;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);
    setupAllCallBeforeReceiveHTTPsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;
    xio_send_transmited_buffer_target = 1;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_POST,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);
    ASSERT_ARE_EQUAL(int, 433, statusCode);
    xio_send_transmited_buffer[4] = '\0';
    ASSERT_ARE_EQUAL(char_ptr, "POST", xio_send_transmited_buffer);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_035: [ The HTTPAPI_ExecuteRequest shall execute resquest for types GET, POST, PUT, DELETE, PATCH, HEAD. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__request_put_succeed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);

    setHttpCertificate(httpHandle);
    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rce;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);
    setupAllCallBeforeReceiveHTTPsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;
    xio_send_transmited_buffer_target = 1;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_PUT,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);
    ASSERT_ARE_EQUAL(int, 433, statusCode);
    xio_send_transmited_buffer[3] = '\0';
    ASSERT_ARE_EQUAL(char_ptr, "PUT", xio_send_transmited_buffer);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_035: [ The HTTPAPI_ExecuteRequest shall execute resquest for types GET, POST, PUT, DELETE, PATCH, HEAD. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__request_delete_succeed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);

    setHttpCertificate(httpHandle);
    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rce;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);
    setupAllCallBeforeReceiveHTTPsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;
    xio_send_transmited_buffer_target = 1;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_DELETE,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);
    ASSERT_ARE_EQUAL(int, 433, statusCode);
    xio_send_transmited_buffer[6] = '\0';
    ASSERT_ARE_EQUAL(char_ptr, "DELETE", xio_send_transmited_buffer);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_035: [ The HTTPAPI_ExecuteRequest shall execute resquest for types GET, POST, PUT, DELETE, PATCH, HEAD. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__request_patch_succeed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);

    setHttpCertificate(httpHandle);
    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rce;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);
    setupAllCallBeforeReceiveHTTPsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;
    xio_send_transmited_buffer_target = 1;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_PATCH,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);
    ASSERT_ARE_EQUAL(int, 433, statusCode);
    xio_send_transmited_buffer[5] = '\0';
    ASSERT_ARE_EQUAL(char_ptr, "PATCH", xio_send_transmited_buffer);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_035: [ The HTTPAPI_ExecuteRequest shall execute resquest for types GET, POST, PUT, DELETE, PATCH, HEAD. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__request_head_succeed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);

    setHttpCertificate(httpHandle);
    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rce;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);
    setupAllCallBeforeReceiveHTTPHeadsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;
    xio_send_transmited_buffer_target = 1;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_HEAD,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);
    ASSERT_ARE_EQUAL(int, 433, statusCode);
    xio_send_transmited_buffer[4] = '\0';
    ASSERT_ARE_EQUAL(char_ptr, "HEAD", xio_send_transmited_buffer);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_038: [ The HTTPAPI_ExecuteRequest shall execute the resquest for the path in relativePath parameter. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__request_relative_path_succeed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);

    setHttpCertificate(httpHandle);
    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rce;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);
    setupAllCallBeforeReceiveHTTPsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;
    xio_send_transmited_buffer_target = 1;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);
    ASSERT_ARE_EQUAL(int, 433, statusCode);
    xio_send_transmited_buffer[66] = '\0';
    ASSERT_ARE_EQUAL(char_ptr, TEST_EXECUTE_REQUEST_RELATIVE_PATH, &(xio_send_transmited_buffer[4]));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_043: [ If the content is NULL, the HTTPAPI_ExecuteRequest shall send the request without content. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_044: [ If the content is not NULL, the number of bytes in the content shall be provided in contentLength parameter. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_075: [ The message received by the HTTPAPI_ExecuteRequest can contain a body with the message content. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__request_with_content_succeed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);

    setHttpCertificate(httpHandle);
    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rce;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);
    setupAllCallBeforeReceiveHTTPsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;
    xio_send_transmited_buffer_target = 7;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);
    ASSERT_ARE_EQUAL(int, 433, statusCode);
    ASSERT_ARE_EQUAL(char_ptr, (const char*)TEST_EXECUTE_REQUEST_CONTENT, xio_send_transmited_buffer);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_043: [ If the content is NULL, the HTTPAPI_ExecuteRequest shall send the request without content. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__request_NULL_content_succeed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);

    setHttpCertificate(httpHandle);
    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rce;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeader(requestHttpHeaders, IGNORED_NUM_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeader(requestHttpHeaders, IGNORED_NUM_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    setupAllCallBeforeReceiveHTTPsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;
    xio_send_transmited_buffer_target = 7;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        NULL,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);
    ASSERT_ARE_EQUAL(int, 433, statusCode);
    ASSERT_ARE_EQUAL(char_ptr, (const char*)"", xio_send_transmited_buffer);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_045: [ If the contentLength is lower than one, the HTTPAPI_ExecuteRequest shall send the request without content. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__request_content_size_0_succeed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);

    setHttpCertificate(httpHandle);
    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rce;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);

    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeader(requestHttpHeaders, IGNORED_NUM_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeader(requestHttpHeaders, IGNORED_NUM_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    setupAllCallBeforeReceiveHTTPsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;
    xio_send_transmited_buffer_target = 7;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        0,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);
    ASSERT_ARE_EQUAL(int, 433, statusCode);
    ASSERT_ARE_EQUAL(char_ptr, (const char*)"", xio_send_transmited_buffer);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_048: [ If the statusCode is NULL, the HTTPAPI_ExecuteRequest shall report not report any status. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__Execute_request_no_statusCode_succeed)
{
    /// arrange
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);

    setHttpCertificate(httpHandle);
    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rce;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);
    setupAllCallBeforeReceiveHTTPsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        NULL,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_049: [ If responseHeadersHandle is provide, the HTTPAPI_ExecuteRequest shall prepare a Response Header usign the HTTPHeaders_AddHeaderNameValuePair. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__Execute_request_no_responseHeadersHandle_succeed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);

    setHttpCertificate(httpHandle);
    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_re;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);

    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeader(requestHttpHeaders, IGNORED_NUM_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(HTTPHeaders_GetHeader(requestHttpHeaders, IGNORED_NUM_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreAllArguments();
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_NUM_ARG, DoworkJobsReceivedBuffer_size[0])).IgnoreArgument(1);

    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        NULL,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);
    ASSERT_ARE_EQUAL(int, 433, statusCode);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_051: [ If the responseContent is NULL, the HTTPAPI_ExecuteRequest shall ignore any content in the response. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__Execute_request_responseContent_NULL_succeed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);

    setHttpCertificate(httpHandle);
    DoworkJobsReceivedBuffer = TEST_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rce;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);
    setupAllCallBeforeReceiveHTTPsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        NULL);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);
    ASSERT_ARE_EQUAL(int, 433, statusCode);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_081: [ The HTTPAPI_ExecuteRequest shall try to read the message with the response up to 20 seconds. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_082: [ If the HTTPAPI_ExecuteRequest retries 20 seconds to receive the message without success, it shall fail and return HTTPAPI_READ_DATA_FAILED. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_083: [ The HTTPAPI_ExecuteRequest shall wait, at least, 100 milliseconds between retries. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__Execute_request_with_truncated_content_failed)
{
    /// arrange
    int i;
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobsReceivedBuffer = (const unsigned char*)"HTTP/111.222 433 555\r\ncontent-length:10\r\ntransfer-encoding:\r\n\r\n0123";
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_re;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);

    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_NUM_ARG, DoworkJobsReceivedBuffer_size[0])).IgnoreArgument(1);

    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(HTTPHeaders_AddHeaderNameValuePair(IGNORED_PTR_ARG, "content-length", "10")).IgnoreArgument(1);

    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(HTTPHeaders_AddHeaderNameValuePair(IGNORED_PTR_ARG, "transfer-encoding", "")).IgnoreArgument(1);

    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);

    for (i = 0; i < 200; i++)
    {
        STRICT_EXPECTED_CALL(ThreadAPI_Sleep(100));
        STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
            .IgnoreArgument(1);
    }

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_READ_DATA_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_081: [ The HTTPAPI_ExecuteRequest shall try to read the message with the response up to 20 seconds. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_082: [ If the HTTPAPI_ExecuteRequest retries 20 seconds to receive the message without success, it shall fail and return HTTPAPI_READ_DATA_FAILED. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_083: [ The HTTPAPI_ExecuteRequest shall wait, at least, 100 milliseconds between retries. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__Execute_request_with_truncated_parameter_failed)
{
    /// arrange
    int i;
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobsReceivedBuffer = (const unsigned char*)"HTTP/111.222 433 555\r\ncontent-length:10\r\ntransfer-enc";
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_re;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);

    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_NUM_ARG, DoworkJobsReceivedBuffer_size[0])).IgnoreArgument(1);

    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(HTTPHeaders_AddHeaderNameValuePair(IGNORED_PTR_ARG, "content-length", "10")).IgnoreArgument(1);

    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);

    for (i = 0; i < 200; i++)
    {
        STRICT_EXPECTED_CALL(ThreadAPI_Sleep(100));
        STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
            .IgnoreArgument(1);
    }


    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_READ_DATA_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_21_081: [ The HTTPAPI_ExecuteRequest shall try to read the message with the response up to 20 seconds. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_082: [ If the HTTPAPI_ExecuteRequest retries 20 seconds to receive the message without success, it shall fail and return HTTPAPI_READ_DATA_FAILED. ]*/
/*Tests_SRS_HTTPAPI_COMPACT_21_083: [ The HTTPAPI_ExecuteRequest shall wait, at least, 100 milliseconds between retries. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__Execute_request_with_truncated_header_failed)
{
    /// arrange
    int i;
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);
    setHttpCertificate(httpHandle);

    DoworkJobsReceivedBuffer = (const unsigned char*)"HTTP/111.222 ";
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_re;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);

    STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_NUM_ARG, DoworkJobsReceivedBuffer_size[0])).IgnoreArgument(1);

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);

    for (i = 0; i < 200; i++)
    {
        STRICT_EXPECTED_CALL(ThreadAPI_Sleep(100));
        STRICT_EXPECTED_CALL(xio_dowork(IGNORED_NUM_ARG))
            .IgnoreArgument(1);
    }


    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_GET,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_READ_DATA_FAILED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

/*Tests_SRS_HTTPAPI_COMPACT_42_088: [ The message received by the HTTPAPI_ExecuteRequest should not contain http body. ]*/
TEST_FUNCTION(HTTPAPI_ExecuteRequest__request_with_no_content_succeed)
{
    /// arrange
    unsigned int statusCode;
    HTTPAPI_RESULT result;
    HTTP_HEADERS_HANDLE requestHttpHeaders;
    HTTP_HEADERS_HANDLE responseHttpHeaders;
    HTTP_HANDLE httpHandle = createHttpConnection();
    createHttpObjects(&requestHttpHeaders, &responseHttpHeaders);

    setHttpCertificate(httpHandle);
    DoworkJobsReceivedBuffer = TEST_HEAD_RECEIVED_ANSWER;
    DoworkJobsReceivedBuffer_size[0] = strlen((const char*)DoworkJobsReceivedBuffer);
    DoworkJobsReceivedBuffer_counter = 0;
    DoworkJobs = (const xio_dowork_job*)doworkjob_o_rce;
    DoworkJobsOpenResult = DoworkJobsOpenResult_ReceiveHead;
    DoworkJobsSendResult = DoworkJobsSendResult_ReceiveHead;

    setupAllCallBeforeOpenHTTPsequence(requestHttpHeaders, 1, false);
    setupAllCallBeforeSendHTTPsequenceWithSuccess(requestHttpHeaders);
    setupAllCallBeforeReceiveHTTPHeadsequenceWithSuccess();

    HTTPHeaders_GetHeader_shallReturn = HTTP_HEADERS_OK;
    xio_send_transmited_buffer_target = 7;

    /// act
    result = HTTPAPI_ExecuteRequest(
        httpHandle,
        HTTPAPI_REQUEST_HEAD,
        TEST_EXECUTE_REQUEST_RELATIVE_PATH,
        requestHttpHeaders,
        TEST_EXECUTE_REQUEST_CONTENT,
        TEST_EXECUTE_REQUEST_CONTENT_LENGTH,
        &statusCode,
        responseHttpHeaders,
        TestBufferHandle);

    /// assert
    ASSERT_ARE_EQUAL(int, HTTPAPI_OK, result);
    ASSERT_ARE_EQUAL(int, 433, statusCode);
    ASSERT_ARE_EQUAL(char_ptr, (const char*)TEST_EXECUTE_REQUEST_CONTENT, xio_send_transmited_buffer);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 6, currentmalloc_call);

    /// cleanup
    destroyHttpObjects(&requestHttpHeaders, &responseHttpHeaders); /* currentmalloc_call -= 2 */
    HTTPAPI_CloseConnection(httpHandle);    /* currentmalloc_call -= 3 */
    HTTPAPI_Deinit();
}

END_TEST_SUITE(httpapicompact_ut)
