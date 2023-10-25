// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_uamqp_c/uamqp.h"

#if _WIN32
#include "windows.h"
#endif

static const size_t msg_count = 1000;
static unsigned int sent_messages = 0;

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
        CONNECTION_HANDLE connection;
        SESSION_HANDLE session;
        LINK_HANDLE link;
        MESSAGE_SENDER_HANDLE message_sender;
        MESSAGE_HANDLE message;
        AMQP_VALUE source;
        AMQP_VALUE target;
        unsigned char hello[] = { 'H', 'e', 'l', 'l', 'o' };
        BINARY_DATA binary_data;

        size_t last_memory_used = 0;

        XIO_HANDLE socket_io;
        SOCKETIO_CONFIG socketio_config = { "localhost", 5672, NULL };

        gballoc_init();

        /* create socket IO */
        socket_io = xio_create(socketio_get_interface_description(), &socketio_config);

        /* create the connection, session and link */
        connection = connection_create(socket_io, "localhost", "some", NULL, NULL);
        session = session_create(connection, NULL, NULL);
        session_set_incoming_window(session, 2147483647);
        session_set_outgoing_window(session, 65536);

        source = messaging_create_source("ingress");
        target = messaging_create_target("localhost/ingress");
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
        }

        messagesender_destroy(message_sender);
        link_destroy(link);
        session_destroy(session);
        connection_destroy(connection);
        xio_destroy(socket_io);
        platform_deinit();

        (void)printf("Max memory usage:%lu\r\n", (unsigned long)gballoc_getCurrentMemoryUsed());
        (void)printf("Current memory usage:%lu\r\n", (unsigned long)gballoc_getMaximumMemoryUsed());

        result = 0;

        gballoc_deinit();
    }

    return result;
}
