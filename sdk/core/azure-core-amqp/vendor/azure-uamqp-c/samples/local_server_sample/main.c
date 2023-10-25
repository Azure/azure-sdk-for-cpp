// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_uamqp_c/uamqp.h"

static unsigned int sent_messages = 0;
static const size_t msg_count = 1;
static CONNECTION_HANDLE connection;
static SESSION_HANDLE session;
static LINK_HANDLE link;
static MESSAGE_RECEIVER_HANDLE message_receiver;

static void on_message_receiver_state_changed(const void* context, MESSAGE_RECEIVER_STATE new_state, MESSAGE_RECEIVER_STATE previous_state)
{
    (void)context, new_state, previous_state;
}

static AMQP_VALUE on_message_received(const void* context, MESSAGE_HANDLE message)
{
    (void)context;
    (void)message;

    printf("Message received.\r\n");

    return messaging_delivery_accepted();
}

static bool on_new_link_attached(void* context, LINK_ENDPOINT_HANDLE new_link_endpoint, const char* name, role role, AMQP_VALUE source, AMQP_VALUE target, fields properties)
{
    (void)context;
    (void)properties;
    link = link_create_from_endpoint(session, new_link_endpoint, name, role, source, target);
    link_set_rcv_settle_mode(link, receiver_settle_mode_first);
    message_receiver = messagereceiver_create(link, on_message_receiver_state_changed, NULL);
    messagereceiver_open(message_receiver, on_message_received, NULL);
    return true;
}

static bool on_new_session_endpoint(void* context, ENDPOINT_HANDLE new_endpoint)
{
    (void)context;
    session = session_create_from_endpoint(connection, new_endpoint, on_new_link_attached, NULL);
    session_set_incoming_window(session, 10000);
    session_begin(session);
    return true;
}

static void on_socket_accepted(void* context, const IO_INTERFACE_DESCRIPTION* interface_description, void* io_parameters)
{
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    XIO_HANDLE underlying_io;
    XIO_HANDLE header_detect_io;

    (void)context;

    underlying_io = xio_create(interface_description, io_parameters);
    header_detect_io_config.underlying_io = underlying_io;
    header_detect_io = xio_create(header_detect_io_get_interface_description(), &header_detect_io_config);
    connection = connection_create(header_detect_io, NULL, "1", on_new_session_endpoint, NULL);
    connection_listen(connection);
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
        size_t last_memory_used = 0;
        SOCKET_LISTENER_HANDLE socket_listener;

        gballoc_init();

        socket_listener = socketlistener_create(5672);
        if (socketlistener_start(socket_listener, on_socket_accepted, NULL) != 0)
        {
            result = -1;
        }
        else
        {
            bool keep_running = true;
            while (keep_running)
            {
                size_t current_memory_used;
                size_t maximum_memory_used;
                socketlistener_dowork(socket_listener);

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

                if (connection != NULL)
                {
                    connection_dowork(connection);
                }
            }

            result = 0;
        }

        socketlistener_destroy(socket_listener);
        platform_deinit();

        printf("Max memory usage:%lu\r\n", (unsigned long)gballoc_getCurrentMemoryUsed());
        printf("Current memory usage:%lu\r\n", (unsigned long)gballoc_getMaximumMemoryUsed());

        gballoc_deinit();
    }

    return result;
}
