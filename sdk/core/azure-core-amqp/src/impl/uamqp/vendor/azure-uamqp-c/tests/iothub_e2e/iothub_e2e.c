// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#else
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#endif
#include "testrunnerswitcher.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/sastoken.h"
#include "azure_uamqp_c/uamqp.h"

static TEST_MUTEX_HANDLE g_testByTest;
static char* iothub_name;
static char* hostname;
static char* key_name;
static char* shared_access_key;
static char* iothub_suffix;

#define TEST_TIMEOUT 30 // seconds

typedef struct SERVER_INSTANCE_TAG
{
    CONNECTION_HANDLE connection;
    SESSION_HANDLE session;
    LINK_HANDLE link;
    MESSAGE_RECEIVER_HANDLE message_receiver;
    size_t received_messages;
    XIO_HANDLE header_detect_io;
} SERVER_INSTANCE;

static bool auth;

static void on_cbs_open_complete(void* context, CBS_OPEN_COMPLETE_RESULT open_complete_result)
{
    (void)context;
    switch (open_complete_result)
    {
    default:
        ASSERT_FAIL("CBS instance open failed.");
        break;

    case CBS_OPEN_OK:
        (void)printf("CBS instance open.\r\n");
        break;
    }
}

static void on_cbs_error(void* context)
{
    (void)context;
}

void on_cbs_put_token_complete(void* context, CBS_OPERATION_RESULT cbs_operation_result, unsigned int status_code, const char* status_description)
{
    (void)context;
    (void)status_code;
    (void)status_description;

    if (cbs_operation_result == CBS_OPERATION_RESULT_OK)
    {
        auth = true;
    }
}

static void on_message_send_complete(void* context, MESSAGE_SEND_RESULT send_result, AMQP_VALUE delivery_state)
{
    size_t* sent_messages = (size_t*)context;

    (void)delivery_state;

    if (send_result == MESSAGE_SEND_OK)
    {
        (*sent_messages)++;
    }
    else
    {
        ASSERT_FAIL("Message send failed");
    }
}

BEGIN_TEST_SUITE(iothub_e2e)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;
    int beginName, endName, beginIothub, endIothub, beginHost, endHost, beginKey;
    size_t totalLen;
    const char* iothub_connection_string;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    result = platform_init();
    ASSERT_ARE_EQUAL(int, 0, result, "platform_init failed");

    iothub_connection_string = getenv("IOTHUB_CONNECTION_STRING");
    ASSERT_IS_NOT_NULL(iothub_connection_string, "Could not get IoTHub connection string");

    totalLen = strlen(iothub_connection_string);
    if (sscanf(iothub_connection_string, "HostName=%n%*[^.]%n.%n%*[^;];%nSharedAccessKeyName=%n%*[^;];%nSharedAccessKey=%n", &beginHost, &endHost, &beginIothub, &endIothub, &beginName, &endName, &beginKey) != 0)
    {
        ASSERT_FAIL("Failure determining the string length parameters.\r\n");
    }
    else
    {
        if ((iothub_name = (char*)malloc(endHost - beginHost + 1)) == NULL)
        {
            ASSERT_FAIL("Failure allocating iothubName.\r\n");
        }
        else if ((hostname = (char*)malloc(endIothub - beginHost + 1)) == NULL)
        {
            ASSERT_FAIL("Failure allocating hostname.\r\n");
        }
        else if ((key_name = (char*)malloc(endName - beginName + 1)) == NULL)
        {
            ASSERT_FAIL("Failure allocating hostName.\r\n");
        }
        else if ((shared_access_key = (char*)malloc(totalLen + 1 - beginKey + 1)) == NULL)
        {
            ASSERT_FAIL("Failure allocating hostName.\r\n");
        }
        else if (sscanf(iothub_connection_string, "HostName=%[^.].%[^;];SharedAccessKeyName=%[^;];SharedAccessKey=%s", iothub_name,
            hostname + endHost - beginHost + 1,
            key_name,
            shared_access_key) != 4)
        {
            ASSERT_FAIL("Failure determining the string values.\r\n");
        }
        else
        {
            (void)strcpy(hostname, iothub_name);
            hostname[endHost - beginHost] = '.';
            if (mallocAndStrcpy_s(&iothub_suffix, hostname + endHost - beginHost + 1) != 0)
            {
                ASSERT_FAIL("[IoTHubAccount] Failure constructing the iothubSuffix.");
            }
            else
            {
                result = 0;
            }
        }
    }
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    platform_deinit();

    free(iothub_suffix);
    free(iothub_name);
    free(hostname);
    free(key_name);
    free(shared_access_key);

    TEST_MUTEX_DESTROY(g_testByTest);
}

TEST_FUNCTION(send_1_message_to_iothub_unsettled_auth_with_cbs)
{
    // arrange
    int result;
    ASYNC_OPERATION_HANDLE async_operation;
    CONNECTION_HANDLE client_connection;
    SESSION_HANDLE client_session;
    LINK_HANDLE client_link;
    MESSAGE_HANDLE client_send_message;
    MESSAGE_SENDER_HANDLE client_message_sender;
    size_t sent_messages = 0;
    AMQP_VALUE source;
    AMQP_VALUE target;
    time_t now_time;
    time_t start_time;
    SASL_MECHANISM_HANDLE sasl_mechanism;
    XIO_HANDLE tls_io;
    XIO_HANDLE sasl_client_io;
    SASLCLIENTIO_CONFIG sasl_io_config;
    size_t target_length;
    char* target_string;
    STRING_HANDLE key_string;
    STRING_HANDLE scope_string;
    STRING_HANDLE keyname_string;
    STRING_HANDLE sas_token;
    CBS_HANDLE cbs;
    unsigned char hello[] = { 'H', 'e', 'l', 'l', 'o' };
    BINARY_DATA binary_data;
    ASYNC_OPERATION_HANDLE send_operation;

    // start the client
    TLSIO_CONFIG tlsio_config;
    tlsio_config.hostname = hostname;
    tlsio_config.port = 5671;
    tlsio_config.underlying_io_interface = NULL;
    tlsio_config.underlying_io_parameters = NULL;

    tls_io = xio_create(platform_get_default_tlsio(), &tlsio_config);
    ASSERT_IS_NOT_NULL(tls_io, "Could not create TLS IO");

    /* create SASL MSSBCBS handler */
    sasl_mechanism = saslmechanism_create(saslmssbcbs_get_interface(), NULL);
    ASSERT_IS_NOT_NULL(sasl_mechanism, "Could not create SASL mechanism handle");

    sasl_io_config.underlying_io = tls_io;
    sasl_io_config.sasl_mechanism = sasl_mechanism;
    sasl_client_io = xio_create(saslclientio_get_interface_description(), &sasl_io_config);
    ASSERT_IS_NOT_NULL(sasl_client_io, "Could not create SASL client IO");

    /* create the connection, session and link */
    client_connection = connection_create(sasl_client_io, hostname, "some", NULL, NULL);
    ASSERT_IS_NOT_NULL(client_connection, "Could not create client connection");

    (void)connection_set_trace(client_connection, true);
    client_session = session_create(client_connection, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_session, "Could not create client session");

    const char* eh_testdevice_device_key;
    eh_testdevice_device_key = getenv("UAMQP_E2E_DEVICE_KEY");
    ASSERT_IS_NOT_NULL(eh_testdevice_device_key, "Could not get eh_testdevice device key");

    key_string = STRING_new();
    ASSERT_IS_NOT_NULL(key_string, "Could not create key_string");
    result = STRING_concat(key_string, eh_testdevice_device_key);
    ASSERT_ARE_EQUAL(int, 0, result, "Could not create key_string");
    scope_string = STRING_new();
    ASSERT_IS_NOT_NULL(scope_string, "Could not create scope string");
    result = STRING_concat(scope_string, hostname);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot create scope for SAS token");
    result = STRING_concat(scope_string, "/devices/eh_testdevice");
    ASSERT_ARE_EQUAL(int, 0, result, "cannot create scope for SAS token");
    keyname_string = STRING_new();
    ASSERT_IS_NOT_NULL(keyname_string, "Could not create keyname_string");

    sas_token = SASToken_Create(key_string, scope_string, keyname_string, (size_t)time(NULL) + 3600);
    ASSERT_IS_NOT_NULL(sas_token, "Could not create sas_token");

    cbs = cbs_create(client_session);
    result = cbs_open_async(cbs, on_cbs_open_complete, cbs, on_cbs_error, cbs);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot open cbs client");

    auth = false;

    async_operation = cbs_put_token_async(cbs, "servicebus.windows.net:sastoken", STRING_c_str(scope_string), STRING_c_str(sas_token), on_cbs_put_token_complete, cbs);
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, async_operation, "cannot put cbs token");

    start_time = time(NULL);
    while ((now_time = time(NULL)),
        (difftime(now_time, start_time) < TEST_TIMEOUT))
    {
        // schedule work for all components
        connection_dowork(client_connection);

        ThreadAPI_Sleep(1);

        if (auth)
        {
            break;
        }
    }

    source = messaging_create_source("ingress");
    ASSERT_IS_NOT_NULL(source, "Could not create source");
    target_length = snprintf(NULL, 0, "amqps://%s/devices/%s/messages/events", hostname, "eh_testdevice");
    target_string = (char*)malloc(target_length + 1);
    ASSERT_IS_NOT_NULL(target_string, "Could not allocate memory for target string");
    target_length = snprintf(target_string, target_length + 1, "amqps://%s/devices/%s/messages/events", hostname, "eh_testdevice");
    target = messaging_create_target(target_string);
    ASSERT_IS_NOT_NULL(target, "Could not create target");
    client_link = link_create(client_session, "sender-link", role_sender, source, target);
    ASSERT_IS_NOT_NULL(client_link, "Could not create client link");
    result = link_set_snd_settle_mode(client_link, sender_settle_mode_settled);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set sender settle mode");

    amqpvalue_destroy(source);
    amqpvalue_destroy(target);

    client_send_message = message_create();
    ASSERT_IS_NOT_NULL(client_send_message, "Could not create message");
    binary_data.bytes = hello;
    binary_data.length = sizeof(hello);
    result = message_add_body_amqp_data(client_send_message, binary_data);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set message body");

    /* create a message sender */
    client_message_sender = messagesender_create(client_link, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_message_sender, "Could not create message sender");
    result = messagesender_open(client_message_sender);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot open message sender");
    send_operation = messagesender_send_async(client_message_sender, client_send_message, on_message_send_complete, &sent_messages, 0);
    ASSERT_IS_NOT_NULL(send_operation, "cannot send message");
    message_destroy(client_send_message);

    while ((now_time = time(NULL)),
        (difftime(now_time, start_time) < TEST_TIMEOUT))
    {
        // schedule work for all components
        connection_dowork(client_connection);

        // if we sent the message, break
        if (sent_messages >= 1)
        {
            break;
        }

        ThreadAPI_Sleep(1);
    }

    // assert
    ASSERT_ARE_EQUAL(size_t, 1, sent_messages, "Bad sent messages count");

    // cleanup
    STRING_delete(key_string);
    STRING_delete(keyname_string);
    STRING_delete(scope_string);
    STRING_delete(sas_token);

    cbs_destroy(cbs);
    messagesender_destroy(client_message_sender);
    link_destroy(client_link);
    session_destroy(client_session);
    connection_destroy(client_connection);
    xio_destroy(sasl_client_io);
    xio_destroy(tls_io);
    saslmechanism_destroy(sasl_mechanism);
    free(target_string);
}

END_TEST_SUITE(iothub_e2e)
