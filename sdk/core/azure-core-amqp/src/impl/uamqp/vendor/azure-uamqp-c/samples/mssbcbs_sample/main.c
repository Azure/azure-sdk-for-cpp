// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_uamqp_c/uamqp.h"

/* This sample connects to an IoTHub, authenticates using a CBS token and sends one message */
/* Replace the below settings with your own.*/

#define IOT_HUB_HOST "<<<Replace with your own IoTHub host (like myiothub.azure-devices.net)>>>"
#define IOT_HUB_DEVICE_NAME "<<<Replace with your device Id (like test_Device)>>>"
#define IOT_HUB_DEVICE_SAS_TOKEN "<<<Replace with your own device SAS token (needs to be generated)>>>"

static unsigned int sent_messages = 0;
static const size_t msg_count = 1;
static bool auth = false;

static void on_cbs_open_complete(void* context, CBS_OPEN_COMPLETE_RESULT open_complete_result)
{
    (void)context;
    (void)open_complete_result;
    (void)printf("CBS instance open.\r\n");
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

static void on_cbs_operation_complete(void* context, CBS_OPERATION_RESULT cbs_operation_result, unsigned int status_code, const char* status_description)
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
        SASL_MECHANISM_HANDLE sasl_mechanism_handle;
        size_t last_memory_used = 0;
        XIO_HANDLE tls_io;
        TLSIO_CONFIG tls_io_config = { IOT_HUB_HOST, 5671 };
        const IO_INTERFACE_DESCRIPTION* tlsio_interface;
        SASLCLIENTIO_CONFIG sasl_io_config;
        CBS_HANDLE cbs;
        AMQP_VALUE source;
        AMQP_VALUE target;
        unsigned char hello[] = { 'H', 'e', 'l', 'l', 'o' };
        BINARY_DATA binary_data;
        fields attach_properties;
        AMQP_VALUE test_attach_property_key;
        AMQP_VALUE test_attach_property_value;

        gballoc_init();

        /* create SASL MSSBCBS handler */
        sasl_mechanism_handle = saslmechanism_create(saslmssbcbs_get_interface(), NULL);

        /* create the TLS IO */
        tlsio_interface = platform_get_default_tlsio();
        tls_io = xio_create(tlsio_interface, &tls_io_config);

        /* create the SASL client IO using the TLS IO */
        sasl_io_config.underlying_io = tls_io;
        sasl_io_config.sasl_mechanism = sasl_mechanism_handle;
        sasl_io = xio_create(saslclientio_get_interface_description(), &sasl_io_config);

        /* create the connection, session and link */
        connection = connection_create(sasl_io, IOT_HUB_HOST, "some", NULL, NULL);
        session = session_create(connection, NULL, NULL);
        session_set_incoming_window(session, 2147483647);
        session_set_outgoing_window(session, 2);

        cbs = cbs_create(session);
        if (cbs_open_async(cbs, on_cbs_open_complete, cbs, on_cbs_error, cbs) == 0)
        {
            (void)cbs_put_token_async(cbs, "servicebus.windows.net:sastoken", IOT_HUB_HOST "/devices/" IOT_HUB_DEVICE_NAME, IOT_HUB_DEVICE_SAS_TOKEN, on_cbs_operation_complete, cbs);

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

        attach_properties = amqpvalue_create_map();
        test_attach_property_key = amqpvalue_create_string("test_attach_property_key");
        test_attach_property_value = amqpvalue_create_string("a_test_property");
        (void)amqpvalue_set_map_value(attach_properties, test_attach_property_key, test_attach_property_value);

        link_set_attach_properties(link, attach_properties);

        amqpvalue_destroy(test_attach_property_key);
        amqpvalue_destroy(test_attach_property_value);
        amqpvalue_destroy(attach_properties);

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

        cbs_destroy(cbs);
        messagesender_destroy(message_sender);
        link_destroy(link);
        session_destroy(session);
        connection_destroy(connection);
        xio_destroy(sasl_io);
        xio_destroy(tls_io);
        saslmechanism_destroy(sasl_mechanism_handle);
        platform_deinit();

        printf("Max memory usage:%lu\r\n", (unsigned long)gballoc_getCurrentMemoryUsed());
        printf("Current memory usage:%lu\r\n", (unsigned long)gballoc_getMaximumMemoryUsed());

        gballoc_deinit();
    }

    return 0;
}
