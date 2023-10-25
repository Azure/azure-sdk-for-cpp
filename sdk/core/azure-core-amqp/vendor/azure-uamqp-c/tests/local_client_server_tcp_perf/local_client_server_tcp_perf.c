// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdbool.h>
#include <stdlib.h>
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_uamqp_c/uamqp.h"

#define CLIENT_COUNT 1
#define OUTSTANDING_MESSAGE_COUNT 1
#define TEST_RUNTIME 5000 // ms

static SINGLYLINKEDLIST_HANDLE server_connected_clients;
static size_t total_messages_received;

typedef struct SERVER_CONNECTED_CLIENT_TAG
{
    CONNECTION_HANDLE connection;
    SESSION_HANDLE session;
    LINK_HANDLE link;
    MESSAGE_RECEIVER_HANDLE message_receiver;
    XIO_HANDLE io;
} SERVER_CONNECTED_CLIENT;

static void on_message_receiver_state_changed(const void* context, MESSAGE_RECEIVER_STATE new_state, MESSAGE_RECEIVER_STATE previous_state)
{
    (void)context;
    (void)new_state;
    (void)previous_state;
}

static AMQP_VALUE on_message_received(const void* context, MESSAGE_HANDLE message)
{
    (void)context;
    (void)message;

    total_messages_received++;

    return messaging_delivery_accepted();
}

static bool on_new_link_attached(void* context, LINK_ENDPOINT_HANDLE new_link_endpoint, const char* name, role role, AMQP_VALUE source, AMQP_VALUE target, fields properties)
{
    SERVER_CONNECTED_CLIENT* server_connected_client = (SERVER_CONNECTED_CLIENT*)context;
    bool result;
    (void)properties;

    LogInfo("Link attached");

    server_connected_client->link = link_create_from_endpoint(server_connected_client->session, new_link_endpoint, name, role, source, target);
    if (server_connected_client->link == NULL)
    {
        LogError("Cannot create link");
        result = false;
    }
    else
    {
        if (link_set_rcv_settle_mode(server_connected_client->link, receiver_settle_mode_first) != 0)
        {
            link_destroy(server_connected_client->link);
            server_connected_client->link = NULL;
            LogError("Cannot set receiver settle mode");
            result = false;
        }
        else
        {
            server_connected_client->message_receiver = messagereceiver_create(server_connected_client->link, on_message_receiver_state_changed, NULL);
            if (server_connected_client->message_receiver == NULL)
            {
                link_destroy(server_connected_client->link);
                server_connected_client->link = NULL;
                LogError("Cannot create message receiver");
                result = false;
            }
            else
            {
                if (messagereceiver_open(server_connected_client->message_receiver, on_message_received, NULL) != 0)
                {
                    messagereceiver_destroy(server_connected_client->message_receiver);
                    server_connected_client->message_receiver = NULL;
                    link_destroy(server_connected_client->link);
                    server_connected_client->link = NULL;
                    LogError("Cannot create message receiver");
                    result = false;
                }
                else
                {
                    /* all OK */
                    result = true;
                }
            }
        }
    }

    return result;
}

static bool on_new_session_endpoint(void* context, ENDPOINT_HANDLE new_endpoint)
{
    SERVER_CONNECTED_CLIENT* server_connected_client = (SERVER_CONNECTED_CLIENT*)context;
    bool result;

    LogInfo("Session begun");

    server_connected_client->session = session_create_from_endpoint(server_connected_client->connection, new_endpoint, on_new_link_attached, server_connected_client);
    if (server_connected_client->session == NULL)
    {
        LogError("Cannot create session");
        result = false;
    }
    else
    {
        if (session_set_incoming_window(server_connected_client->session, 100) != 0)
        {
            session_destroy(server_connected_client->session);
            server_connected_client->session = NULL;
            LogError("Cannot set incoming window");
            result = false;
        }
        else
        {
            if (session_begin(server_connected_client->session) != 0)
            {
                session_destroy(server_connected_client->session);
                server_connected_client->session = NULL;
                LogError("Cannot begin session");
                result = false;
            }
            else
            {
                /* all OK */
                result = true;
            }
        }
    }

    return result;
}

static void on_socket_accepted(void* context, const IO_INTERFACE_DESCRIPTION* interface_description, void* io_parameters)
{
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    XIO_HANDLE underlying_io;
    XIO_HANDLE header_detect_io;

    (void)context;

    LogInfo("Socket accepted");

    underlying_io = xio_create(interface_description, io_parameters);
    if (underlying_io == NULL)
    {
        LogError("Cannot create accepted socket IO");
    }
    else
    {
        HEADER_DETECT_ENTRY header_detect_entries[1];

        header_detect_entries[0].header = header_detect_io_get_amqp_header();
        header_detect_entries[0].io_interface_description = NULL;

        header_detect_io_config.underlying_io = underlying_io;
        header_detect_io_config.header_detect_entry_count = 1;
        header_detect_io_config.header_detect_entries = header_detect_entries;
        header_detect_io = xio_create(header_detect_io_get_interface_description(), &header_detect_io_config);
        if (header_detect_io == NULL)
        {
            xio_destroy(underlying_io);
            LogError("Cannot create header detect IO");
        }
        else
        {
            SERVER_CONNECTED_CLIENT* server_connected_client = (SERVER_CONNECTED_CLIENT*)malloc(sizeof(SERVER_CONNECTED_CLIENT));
            if (server_connected_client == NULL)
            {
                xio_destroy(underlying_io);
                xio_destroy(header_detect_io);
                LogError("Cannot create header detect IO");
            }
            else
            {
                if (singlylinkedlist_add(server_connected_clients, server_connected_client) == NULL)
                {
                    free(server_connected_client);
                    xio_destroy(underlying_io);
                    xio_destroy(header_detect_io);
                    LogError("Cannot create header detect IO");
                }
                else
                {
                    server_connected_client->link = NULL;
                    server_connected_client->message_receiver = NULL;
                    server_connected_client->session = NULL;
                    server_connected_client->connection = connection_create(header_detect_io, NULL, "1", on_new_session_endpoint, server_connected_client);
                    if (server_connected_client->connection == NULL)
                    {
                        free(server_connected_client);
                        xio_destroy(underlying_io);
                        xio_destroy(header_detect_io);
                        LogError("Cannot create header detect IO");
                    }
                    else
                    {
                        if (connection_listen(server_connected_client->connection) != 0)
                        {
                            connection_destroy(server_connected_client->connection);
                            free(server_connected_client);
                            xio_destroy(underlying_io);
                            xio_destroy(header_detect_io);
                            LogError("Cannot create header detect IO");
                        }
                        else
                        {
                            /* All OK */
                            server_connected_client->io = header_detect_io;
                        }
                    }
                }
            }
        }
    }
}

typedef struct CLIENT_TAG
{
    CONNECTION_HANDLE connection;
    SESSION_HANDLE session;
    LINK_HANDLE link;
    MESSAGE_SENDER_HANDLE message_sender;
    XIO_HANDLE io;
    size_t outstanding_message_count;
} CLIENT;

static void on_message_send_complete(void* context, MESSAGE_SEND_RESULT send_result, AMQP_VALUE delivery_state)
{
    CLIENT* client = (CLIENT*)context;
    client->outstanding_message_count--;
    (void)send_result;
    (void)context;
    (void)delivery_state;
}

int main(void)
{
    int result;
    if (platform_init() != 0)
    {
        LogError("platform_init failed");
        result = -1;
    }
    else
    {
        server_connected_clients = singlylinkedlist_create();
        if (server_connected_clients == NULL)
        {
            LogError("Cannot create server connected clients list");
            result = -1;
        }
        else
        {
            LIST_ITEM_HANDLE current_item;
            SOCKET_LISTENER_HANDLE socket_listener;

            socket_listener = socketlistener_create(5672);
            if (socket_listener == NULL)
            {
                LogError("Cannot create socket listener");
                result = -1;
            }
            else
            {
                if (socketlistener_start(socket_listener, on_socket_accepted, NULL) != 0)
                {
                    LogError("socketlistener_start failed");
                    result = -1;
                }
                else
                {
                    size_t i;
                    CLIENT clients[CLIENT_COUNT];
                    TICK_COUNTER_HANDLE tick_counter;

                    for (i = 0; i < CLIENT_COUNT; i++)
                    {
                        AMQP_VALUE source;
                        AMQP_VALUE target;

                        SOCKETIO_CONFIG socketio_config = { "localhost", 5672, NULL };

                        /* create socket IO */
                        clients[i].io = xio_create(socketio_get_interface_description(), &socketio_config);
                        if (clients[i].io == NULL)
                        {
                            LogError("Cannot create client IO");
                            break;
                        }

                        /* create the connection, session and link */
                        clients[i].connection = connection_create(clients[i].io, "localhost", "some", NULL, NULL);
                        if (clients[i].connection == NULL)
                        {
                            LogError("Cannot create client connection");
                            xio_destroy(clients[i].io);
                            clients[i].io = NULL;
                            break;
                        }

                        clients[i].session = session_create(clients[i].connection, NULL, NULL);
                        if (clients[i].session == NULL)
                        {
                            LogError("Cannot create client session");
                            connection_destroy(clients[i].connection);
                            xio_destroy(clients[i].io);
                            clients[i].io = NULL;
                            break;
                        }

                        source = messaging_create_source("ingress");
                        target = messaging_create_target("localhost/ingress");

                        clients[i].link = link_create(clients[i].session, "sender-link", role_sender, source, target);
                        if (clients[i].session == NULL)
                        {
                            LogError("Cannot create client link");
                            session_destroy(clients[i].session);
                            connection_destroy(clients[i].connection);
                            xio_destroy(clients[i].io);
                            clients[i].io = NULL;
                            break;
                        }

                        amqpvalue_destroy(source);
                        amqpvalue_destroy(target);

                        if ((link_set_snd_settle_mode(clients[i].link, sender_settle_mode_settled) != 0) ||
                            (link_set_max_message_size(clients[i].link, 65536) != 0))
                        {
                            LogError("Cannot set link properties");
                            link_destroy(clients[i].link);
                            session_destroy(clients[i].session);
                            connection_destroy(clients[i].connection);
                            xio_destroy(clients[i].io);
                            clients[i].io = NULL;
                            break;
                        }

                        /* create a message sender */
                        clients[i].message_sender = messagesender_create(clients[i].link, NULL, NULL);
                        if (messagesender_open(clients[i].message_sender) != 0)
                        {
                            LogError("Cannot create client message sender");
                            link_destroy(clients[i].link);
                            session_destroy(clients[i].session);
                            connection_destroy(clients[i].connection);
                            xio_destroy(clients[i].io);
                            clients[i].io = NULL;
                            break;
                        }

                        clients[i].outstanding_message_count = 0;
                    }

                    tick_counter = tickcounter_create();
                    if (tick_counter == NULL)
                    {
                        LogError("Cannot create tick counter");
                    }
                    else
                    {
                        bool keep_running = true;
                        tickcounter_ms_t current_ms = 0;
                        tickcounter_ms_t start_ms = 0;

                        if (tickcounter_get_current_ms(tick_counter, &start_ms) != 0)
                        {
                            LogError("Caanot get tick counter value");
                        }
                        else
                        {
                            while (keep_running)
                            {
                                SERVER_CONNECTED_CLIENT* server_connected_client;

                                socketlistener_dowork(socket_listener);

                                // schedule client work
                                for (i = 0; i < CLIENT_COUNT; i++)
                                {
                                    bool is_error = false;
                                    connection_dowork(clients[i].connection);

                                    while (clients[i].outstanding_message_count < OUTSTANDING_MESSAGE_COUNT)
                                    {
                                        MESSAGE_HANDLE message = message_create();
                                        unsigned char hello[] = { 'H', 'e', 'l', 'l', 'o' };
                                        BINARY_DATA binary_data;

                                        if (message == NULL)
                                        {
                                            LogError("Error creating message");
                                            is_error = true;
                                            break;
                                        }

                                        binary_data.bytes = hello;
                                        binary_data.length = sizeof(hello);
                                        message_add_body_amqp_data(message, binary_data);

                                        if (messagesender_send_async(clients[i].message_sender, message, on_message_send_complete, &clients[i], 0) == NULL)
                                        {
                                            message_destroy(message);
                                            LogError("Error sending message");
                                            is_error = true;
                                            break;
                                        }

                                        message_destroy(message);

                                        clients[i].outstanding_message_count++;
                                    }

                                    if (is_error)
                                    {
                                        break;
                                    }
                                }

                                if (i < CLIENT_COUNT)
                                {
                                    LogError("Error processing clients");
                                    break;
                                }

                                current_item = singlylinkedlist_get_head_item(server_connected_clients);
                                while (current_item != NULL)
                                {
                                    server_connected_client = (SERVER_CONNECTED_CLIENT*)singlylinkedlist_item_get_value(current_item);
                                    connection_dowork(server_connected_client->connection);
                                    current_item = singlylinkedlist_get_next_item(current_item);
                                }

                                if (tickcounter_get_current_ms(tick_counter, &current_ms) != 0)
                                {
                                    LogError("Caanot get tick counter value");
                                    break;
                                }

                                if (current_ms - start_ms > TEST_RUNTIME)
                                {
                                    break;
                                }
                            }

                            LogInfo("Received %lu messages in %02f seconds, %02f messages/s",
                                (unsigned long)total_messages_received,
                                ((double)current_ms - start_ms) / 1000,
                                total_messages_received / (((double)current_ms - start_ms) / 1000));

                            for (i = 0; i < CLIENT_COUNT; i++)
                            {
                                messagesender_destroy(clients[i].message_sender);
                                link_destroy(clients[i].link);
                                session_destroy(clients[i].session);
                                connection_destroy(clients[i].connection);
                                xio_destroy(clients[i].io);
                            }
                        }

                        tickcounter_destroy(tick_counter);
                    }

                    (void)socketlistener_stop(socket_listener);
                }

                socketlistener_destroy(socket_listener);
            }

            current_item = singlylinkedlist_get_head_item(server_connected_clients);
            while (current_item != NULL)
            {
                SERVER_CONNECTED_CLIENT* server_connected_client = (SERVER_CONNECTED_CLIENT*)singlylinkedlist_item_get_value(current_item);
                messagereceiver_destroy(server_connected_client->message_receiver);
                link_destroy(server_connected_client->link);
                session_destroy(server_connected_client->session);
                connection_destroy(server_connected_client->connection);
                xio_destroy(server_connected_client->io);

                singlylinkedlist_remove(server_connected_clients, current_item);
                current_item = singlylinkedlist_get_head_item(server_connected_clients);
                free(server_connected_client);
            }

            singlylinkedlist_destroy(server_connected_clients);
        }

        platform_deinit();
    }
}
