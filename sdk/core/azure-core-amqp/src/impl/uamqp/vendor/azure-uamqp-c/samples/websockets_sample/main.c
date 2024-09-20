// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/sastoken.h"
#include "azure_c_shared_utility/wsio.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_uamqp_c/uamqp.h"
#include "iothub_certs.h"

/* Replace the below settings with your own.*/

#define IOT_HUB_HOST "<<<Replace with your own IoTHub host (like myiothub.azure-devices.net)>>>"
#define IOT_HUB_DEVICE_NAME "<<<Replace with your device Id (like test_Device)>>>"
#define IOT_HUB_DEVICE_KEY "<<<Replace with your own device key>>>"

static const size_t msg_count = 1000;
static unsigned int sent_messages = 0;
static bool auth = false;

static void on_cbs_open_complete(void* context, CBS_OPEN_COMPLETE_RESULT open_complete_result)
{
    (void)context;
    switch (open_complete_result)
    {
    default:
        (void)printf("CBS instance open failed.\r\n");
        break;

    case CBS_OPEN_OK:
        (void)printf("CBS instance open.\r\n");
        break;
    }
}

static void on_cbs_error(void* context)
{
    (void)context;
    (void)printf("CBS error.\r\n");
}

static void on_message_send_complete(void* context, MESSAGE_SEND_RESULT send_result, AMQP_VALUE delivery_state)
{
    (void)send_result;
    (void)context;
    (void)delivery_state;

    printf("Sent.\r\n");
    sent_messages++;
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

int main(int argc, char** argv)
{
    int result;

    (void)argc;
    (void)argv;

    if (platform_init() != 0)
    {
        result = -1;
    }
    else
    {
        XIO_HANDLE sasl_io;
        CONNECTION_HANDLE connection;
        SESSION_HANDLE session;
        LINK_HANDLE link;
        MESSAGE_SENDER_HANDLE message_sender;
        MESSAGE_HANDLE message;
        SASLCLIENTIO_CONFIG sasl_io_config;
        STRING_HANDLE keyname_string;

        size_t last_memory_used = 0;

        /* create SASL MSSBCBS handler */
        SASL_MECHANISM_HANDLE sasl_mechanism_handle = saslmechanism_create(saslmssbcbs_get_interface(), NULL);
        XIO_HANDLE ws_io;
        WSIO_CONFIG ws_io_config;
        TLSIO_CONFIG tls_io_config;
        const IO_INTERFACE_DESCRIPTION* tlsio_interface;
        STRING_HANDLE key_string;
        STRING_HANDLE scope_string;
        STRING_HANDLE sas_token;
        CBS_HANDLE cbs;
        AMQP_VALUE source;
        AMQP_VALUE target;
        unsigned char hello[5] = { 'h', 'e', 'l', 'l', 'o' };
        BINARY_DATA binary_data;

        gballoc_init();

        tls_io_config.hostname = IOT_HUB_HOST;
        tls_io_config.port = 443;
        tls_io_config.underlying_io_interface = NULL;
        tls_io_config.underlying_io_parameters = NULL;

        /* create the TLS IO */
        ws_io_config.hostname = IOT_HUB_HOST;
        ws_io_config.port = 443;
        ws_io_config.protocol = "AMQPWSB10";
        ws_io_config.resource_name = "/$iothub/websocket";
        ws_io_config.underlying_io_interface = platform_get_default_tlsio();
        ws_io_config.underlying_io_parameters = &tls_io_config;

        tlsio_interface = wsio_get_interface_description();
        ws_io = xio_create(tlsio_interface, &ws_io_config);

        /* the websockets library uses OpenSSL and on Windows the certs need to be pushed down */
        (void)xio_setoption(ws_io, "TrustedCerts", iothub_certs);

        /* create the SASL IO using the WS IO */
        sasl_io_config.underlying_io = ws_io;
        sasl_io_config.sasl_mechanism = sasl_mechanism_handle;

        sasl_io = xio_create(saslclientio_get_interface_description(), &sasl_io_config);

        /* create the connection, session and link */
        connection = connection_create(sasl_io, IOT_HUB_HOST, "some", NULL, NULL);
        connection_set_trace(connection, true);
        session = session_create(connection, NULL, NULL);
        session_set_incoming_window(session, 2147483647);
        session_set_outgoing_window(session, 65536);

        key_string = STRING_new();
        STRING_concat(key_string, IOT_HUB_DEVICE_KEY);
        scope_string = STRING_new();
        STRING_concat(scope_string, IOT_HUB_HOST "/devices/" IOT_HUB_DEVICE_NAME);
        keyname_string = STRING_new();

        sas_token = SASToken_Create(key_string, scope_string, keyname_string, (size_t)time(NULL) + 3600);

        STRING_delete(key_string);
        STRING_delete(scope_string);
        STRING_delete(keyname_string);

        cbs = cbs_create(session);
        if (cbs_open_async(cbs, on_cbs_open_complete, cbs, on_cbs_error, cbs) == 0)
        {
            (void)cbs_put_token_async(cbs, "servicebus.windows.net:sastoken", IOT_HUB_HOST "/devices/" IOT_HUB_DEVICE_NAME, STRING_c_str(sas_token), on_cbs_put_token_complete, cbs);

            while (!auth)
            {
                size_t current_memory_used;
                size_t maximum_memory_used;
                connection_dowork(connection);

                current_memory_used = gballoc_getCurrentMemoryUsed();
                maximum_memory_used = gballoc_getMaximumMemoryUsed();

                if (current_memory_used != last_memory_used)
                {
                    printf("Current memory usage:%lu (max:%lu)\r\n", (unsigned long)current_memory_used, (unsigned long)maximum_memory_used);
                    last_memory_used = current_memory_used;
                }
            }
        }

        source = messaging_create_source("ingress");
        target = messaging_create_target("amqps://" IOT_HUB_HOST "/devices/" IOT_HUB_DEVICE_NAME "/messages/events");
        link = link_create(session, "sender-link", role_sender, source, target);
        (void)link_set_max_message_size(link, 65536);

        amqpvalue_destroy(source);
        amqpvalue_destroy(target);

        message = message_create();

        binary_data.bytes = hello;
        binary_data.length = sizeof(hello);
        message_add_body_amqp_data(message, binary_data);

        /* create a message sender */
        message_sender = messagesender_create(link, NULL, NULL);
        if (messagesender_open(message_sender) == 0)
        {
            uint32_t i;
            bool keep_running = true;

            for (i = 0; i < msg_count; i++)
            {
                (void)messagesender_send_async(message_sender, message, on_message_send_complete, message, 0);
            }

            message_destroy(message);

            while (keep_running)
            {
                size_t current_memory_used;
                size_t maximum_memory_used;
                connection_dowork(connection);

                current_memory_used = gballoc_getCurrentMemoryUsed();
                maximum_memory_used = gballoc_getMaximumMemoryUsed();

                if (current_memory_used != last_memory_used)
                {
                    printf("Current memory usage:%lu (max:%lu)\r\n", (unsigned long)current_memory_used, (unsigned long)maximum_memory_used);
                    last_memory_used = current_memory_used;
                }

                if (sent_messages == msg_count)
                {
                    break;
                }
            }
        }
        else
        {
            message_destroy(message);
        }

        STRING_delete(sas_token);

        messagesender_destroy(message_sender);
        cbs_destroy(cbs);
        link_destroy(link);
        session_destroy(session);
        connection_destroy(connection);
        xio_destroy(sasl_io);
        xio_destroy(ws_io);
        saslmechanism_destroy(sasl_mechanism_handle);
        platform_deinit();

        printf("Max memory usage:%lu\r\n", (unsigned long)gballoc_getCurrentMemoryUsed());
        printf("Current memory usage:%lu\r\n", (unsigned long)gballoc_getMaximumMemoryUsed());

        gballoc_deinit();

        result = 0;
    }

    return result;
}
