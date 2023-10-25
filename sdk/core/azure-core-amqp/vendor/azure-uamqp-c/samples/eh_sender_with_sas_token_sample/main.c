// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/buffer_.h"
#include "azure_c_shared_utility/azure_base64.h"
#include "azure_c_shared_utility/urlencode.h"
#include "azure_c_shared_utility/sastoken.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_uamqp_c/uamqp.h"

#if _WIN32
#include "windows.h"
#endif

/* This sample connects to an Event Hub, authenticates using SASL MSSBCBS (SAS token given by a put-token) and sends 1 message to the EH specifying a publisher ID */
/* The SAS token is generated based on the policy name/key */
/* Replace the below settings with your own.*/

#define EH_HOST "<<<Replace with your own EH host (like myeventhub.servicebus.windows.net)>>>"
#define EH_KEY_NAME "<<<Replace with your own key name>>>"
#define EH_KEY "<<<Replace with your own key>>>"
#define EH_NAME "<<<Replace with your own EH name (like ingress_eh)>>>"

#define EH_PUBLISHER "test_publisher"

static const size_t msg_count = 1;
static unsigned int sent_messages = 0;
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

static void on_cbs_put_token_complete(void* context, CBS_OPERATION_RESULT cbs_operation_result, unsigned int status_code, const char* status_description)
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
    (void)send_result;
    (void)context;
    (void)delivery_state;

    sent_messages++;
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

        size_t last_memory_used = 0;

        /* create SASL PLAIN handler */
        SASL_MECHANISM_HANDLE sasl_mechanism_handle = saslmechanism_create(saslmssbcbs_get_interface(), NULL);
        XIO_HANDLE tls_io;
        STRING_HANDLE sas_key_name;
        STRING_HANDLE sas_key_value;
        STRING_HANDLE resource_uri;
        STRING_HANDLE encoded_resource_uri;
        STRING_HANDLE sas_token;
        BUFFER_HANDLE buffer;
        TLSIO_CONFIG tls_io_config = { EH_HOST, 5671 };
        const IO_INTERFACE_DESCRIPTION* tlsio_interface;
        SASLCLIENTIO_CONFIG sasl_io_config;
        time_t currentTime;
        size_t expiry_time;
        CBS_HANDLE cbs;
        AMQP_VALUE source;
        AMQP_VALUE target;
        unsigned char hello[] = { 'H', 'e', 'l', 'l', 'o' };
        BINARY_DATA binary_data;

        gballoc_init();

        /* create the TLS IO */
        tlsio_interface = platform_get_default_tlsio();
        tls_io = xio_create(tlsio_interface, &tls_io_config);

        /* create the SASL client IO using the TLS IO */
        sasl_io_config.underlying_io = tls_io;
        sasl_io_config.sasl_mechanism = sasl_mechanism_handle;
        sasl_io = xio_create(saslclientio_get_interface_description(), &sasl_io_config);

        /* create the connection, session and link */
        connection = connection_create(sasl_io, EH_HOST, "some", NULL, NULL);
        session = session_create(connection, NULL, NULL);
        session_set_incoming_window(session, 2147483647);
        session_set_outgoing_window(session, 65536);

        /* Construct a SAS token */
        sas_key_name = STRING_construct(EH_KEY_NAME);

        /* unfortunately SASToken wants an encoded key - this should be fixed at a later time */
        buffer = BUFFER_create((unsigned char*)EH_KEY, strlen(EH_KEY));
        sas_key_value = Azure_Base64_Encode(buffer);
        BUFFER_delete(buffer);
        resource_uri = STRING_construct("sb://" EH_HOST "/" EH_NAME "/publishers/" EH_PUBLISHER);
        encoded_resource_uri = URL_EncodeString(STRING_c_str(resource_uri));

        /* Make a token that expires in one hour */
        currentTime = time(NULL);
        expiry_time = (size_t)(difftime(currentTime, 0) + 3600);

        sas_token = SASToken_Create(sas_key_value, encoded_resource_uri, sas_key_name, expiry_time);

        cbs = cbs_create(session);
        if (cbs_open_async(cbs, on_cbs_open_complete, cbs, on_cbs_error, cbs) == 0)
        {
            (void)cbs_put_token_async(cbs, "servicebus.windows.net:sastoken", "sb://" EH_HOST "/" EH_NAME "/publishers/" EH_PUBLISHER, STRING_c_str(sas_token), on_cbs_put_token_complete, cbs);

            while (!auth)
            {
                size_t current_memory_used;
                size_t maximum_memory_used;
                connection_dowork(connection);

                current_memory_used = gballoc_getCurrentMemoryUsed();
                maximum_memory_used = gballoc_getMaximumMemoryUsed();

                if (current_memory_used != last_memory_used)
                {
                    (void)printf("Current memory usage:%lu (max:%lu)\r\n", (unsigned long)current_memory_used, (unsigned long)maximum_memory_used);
                    last_memory_used = current_memory_used;
                }
            }
        }

        STRING_delete(sas_token);
        STRING_delete(sas_key_name);
        STRING_delete(sas_key_value);
        STRING_delete(resource_uri);
        STRING_delete(encoded_resource_uri);

        source = messaging_create_source("ingress");
        target = messaging_create_target("amqps://" EH_HOST "/" EH_NAME "/publishers/" EH_PUBLISHER);
        link = link_create(session, "sender-link", role_sender, source, target);
        link_set_snd_settle_mode(link, sender_settle_mode_settled);
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
            tickcounter_ms_t start_time;
            TICK_COUNTER_HANDLE tick_counter = tickcounter_create();

            if (tickcounter_get_current_ms(tick_counter, &start_time) != 0)
            {
                (void)printf("Error getting start time\r\n");
            }
            else
            {
                for (i = 0; i < msg_count; i++)
                {
                    /* timeout if it takes longer than 10s */
                    (void)messagesender_send_async(message_sender, message, on_message_send_complete, message, 10000);
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
                        (void)printf("Current memory usage:%lu (max:%lu)\r\n", (unsigned long)current_memory_used, (unsigned long)maximum_memory_used);
                        last_memory_used = current_memory_used;
                    }

                    if (sent_messages == msg_count)
                    {
                        break;
                    }
                }

                {
                    tickcounter_ms_t end_time;
                    if (tickcounter_get_current_ms(tick_counter, &end_time) != 0)
                    {
                        (void)printf("Error getting end time\r\n");
                    }
                    else
                    {
                        (void)printf("Send %u messages in %lu ms: %.02f msgs/sec\r\n", (unsigned int)msg_count, (unsigned long)(end_time - start_time), (float)msg_count / ((float)(end_time - start_time) / 1000));
                    }
                }
            }

            tickcounter_destroy(tick_counter);
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

        (void)printf("Max memory usage:%lu\r\n", (unsigned long)gballoc_getCurrentMemoryUsed());
        (void)printf("Current memory usage:%lu\r\n", (unsigned long)gballoc_getMaximumMemoryUsed());

        gballoc_deinit();

        result = 0;
    }

    return result;
}
