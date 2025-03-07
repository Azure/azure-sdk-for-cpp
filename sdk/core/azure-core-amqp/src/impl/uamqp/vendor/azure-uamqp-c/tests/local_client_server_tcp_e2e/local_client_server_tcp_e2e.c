// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#else
#include <stdlib.h>
#include <stddef.h>
#endif
#include "testrunnerswitcher.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_uamqp_c/uamqp.h"

static TEST_MUTEX_HANDLE g_testByTest;

static const char* test_redirect_hostname = "blahblah";
static const char* test_redirect_network_host = "1.2.3.4";
static const char* test_redirect_address = "blahblah/hagauaga";
static uint16_t test_redirect_port = 4242;

#define TEST_TIMEOUT 30 // seconds

static int generate_port_number(void)
{
    int port_number;

    port_number = 5672 + (int)(5000 * (double)rand() / RAND_MAX); // pseudo random number.

    LogInfo("Generated port number: %d", port_number);

    return port_number;
}

typedef struct SERVER_SESSION_TAG
{
    SESSION_HANDLE session;
    struct SERVER_INSTANCE_TAG* server;
} SERVER_SESSION;

typedef struct SERVER_INSTANCE_TAG
{
    CONNECTION_HANDLE connection;
    size_t session_count;
    SERVER_SESSION sessions[2];
    size_t link_count;
    LINK_HANDLE links[2];
    MESSAGE_RECEIVER_HANDLE message_receivers[2];
    size_t received_messages;
    XIO_HANDLE header_detect_io;
    XIO_HANDLE underlying_io;
} SERVER_INSTANCE;

BEGIN_TEST_SUITE(local_client_server_tcp_e2e)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    result = platform_init();
    ASSERT_ARE_EQUAL(int, 0, result, "platform_init failed");

    srand((unsigned int)clock());
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    platform_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
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

static void on_message_send_cancelled(void* context, MESSAGE_SEND_RESULT send_result, AMQP_VALUE delivery_state)
{
    size_t* cancelled_messages = (size_t*)context;

    (void)delivery_state;

    if (send_result == MESSAGE_SEND_CANCELLED)
    {
        (*cancelled_messages)++;
    }
    else
    {
        ASSERT_FAIL("Unexpected message send result");
    }
}

static void on_message_receivers_state_changed(const void* context, MESSAGE_RECEIVER_STATE new_state, MESSAGE_RECEIVER_STATE previous_state)
{
    (void)context;
    (void)new_state;
    (void)previous_state;
}

static AMQP_VALUE on_message_received(const void* context, MESSAGE_HANDLE message)
{
    BINARY_DATA binary_data;
    int result;
    SERVER_INSTANCE* server;
    const unsigned char expected_payload[] = { 'H', 'e', 'l', 'l', 'o' };

    (void)message;

    server = (SERVER_INSTANCE*)context;
    server->received_messages++;

    result = message_get_body_amqp_data_in_place(message, 0, &binary_data);
    ASSERT_ARE_EQUAL(int, 0, result, "message receiver open failed");

    ASSERT_ARE_EQUAL(size_t, sizeof(expected_payload), binary_data.length, "received message length mismatch");
    ASSERT_ARE_EQUAL(int, 0, memcmp(expected_payload, binary_data.bytes, sizeof(expected_payload)), "received message payload mismatch");

    return messaging_delivery_accepted();
}

static bool on_new_link_attached(void* context, LINK_ENDPOINT_HANDLE new_link_endpoint, const char* name, role role, AMQP_VALUE source, AMQP_VALUE target, fields properties)
{
    SERVER_SESSION* server_session = (SERVER_SESSION*)context;
	struct SERVER_INSTANCE_TAG* server = (struct SERVER_INSTANCE_TAG*)server_session->server;
    int result;
    (void)properties;

    server->links[server->link_count] = link_create_from_endpoint(server_session->session, new_link_endpoint, name, role, source, target);
    ASSERT_IS_NOT_NULL(server->links[server->link_count], "Could not create link");
    server->message_receivers[server->link_count] = messagereceiver_create(server->links[server->link_count], on_message_receivers_state_changed, server);
    ASSERT_IS_NOT_NULL(server->message_receivers[server->link_count], "Could not create message receiver");
    result = messagereceiver_open(server->message_receivers[server->link_count], on_message_received, server);
    ASSERT_ARE_EQUAL(int, 0, result, "message receiver open failed");
    server->link_count++;

    return true;
}

static bool on_new_session_endpoint(void* context, ENDPOINT_HANDLE new_endpoint)
{
    struct SERVER_INSTANCE_TAG* server = (struct SERVER_INSTANCE_TAG*)context;
    int result;

    SESSION_HANDLE session = session_create_from_endpoint(server->connection, new_endpoint, on_new_link_attached, &server->sessions[server->session_count]);
    server->sessions[server->session_count].server = server;
    server->sessions[server->session_count].session = session;
    server->session_count++;
    ASSERT_IS_NOT_NULL(session, "Could not create server session");
    result = session_begin(session);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot begin server session");

    return true;
}

static void on_socket_accepted(void* context, const IO_INTERFACE_DESCRIPTION* interface_description, void* io_parameters)
{
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    HEADER_DETECT_ENTRY header_detect_entries[1];
    SERVER_INSTANCE* server = (SERVER_INSTANCE*)context;
    int result;
    AMQP_HEADER amqp_header;

    server->underlying_io = xio_create(interface_description, io_parameters);
    ASSERT_IS_NOT_NULL(server->underlying_io, "Could not create underlying IO");

    amqp_header = header_detect_io_get_amqp_header();
    header_detect_entries[0].header.header_bytes = amqp_header.header_bytes;
    header_detect_entries[0].header.header_size = amqp_header.header_size;
    header_detect_entries[0].io_interface_description = NULL;

    header_detect_io_config.underlying_io = server->underlying_io;
    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;

    server->header_detect_io = xio_create(header_detect_io_get_interface_description(), &header_detect_io_config);
    ASSERT_IS_NOT_NULL(server->header_detect_io, "Could not create header detect IO");
    server->connection = connection_create(server->header_detect_io, NULL, "1", on_new_session_endpoint, server);
    ASSERT_IS_NOT_NULL(server->connection, "Could not create server connection");
    (void)connection_set_trace(server->connection, true);
    result = connection_listen(server->connection);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot start listening");
}

TEST_FUNCTION(client_and_server_connect_and_send_one_message_settled)
{
    // arrange
    int port_number = generate_port_number();
    SERVER_INSTANCE server_instance;
    SOCKET_LISTENER_HANDLE socket_listener = socketlistener_create(port_number);
    int result;
    XIO_HANDLE socket_io;
    CONNECTION_HANDLE client_connection;
    SESSION_HANDLE client_session;
    LINK_HANDLE client_link;
    MESSAGE_HANDLE client_send_message;
    MESSAGE_SENDER_HANDLE client_message_sender;
    size_t sent_messages;
    AMQP_VALUE source;
    AMQP_VALUE target;
    time_t now_time;
    time_t start_time;
    SOCKETIO_CONFIG socketio_config = { "localhost", 0, NULL };
    unsigned char hello[] = { 'H', 'e', 'l', 'l', 'o' };
    BINARY_DATA binary_data;
    ASYNC_OPERATION_HANDLE send_async_operation;

    server_instance.connection = NULL;
    server_instance.session_count = 0;
    server_instance.sessions[0].session = NULL;
    server_instance.link_count = 0;
    server_instance.links[0] = NULL;
    server_instance.message_receivers[0] = NULL;
    server_instance.received_messages = 0;

    sent_messages = 0;

    result = socketlistener_start(socket_listener, on_socket_accepted, &server_instance);
    ASSERT_ARE_EQUAL(int, 0, result, "socketlistener_start failed");

    // start the client
    socketio_config.port = port_number;
    socket_io = xio_create(socketio_get_interface_description(), &socketio_config);
    ASSERT_IS_NOT_NULL(socket_io, "Could not create socket IO");

    /* create the connection, session and link */
    client_connection = connection_create(socket_io, "localhost", "some", NULL, NULL);
    ASSERT_IS_NOT_NULL(client_connection, "Could not create client connection");

    (void)connection_set_trace(client_connection, true);
    client_session = session_create(client_connection, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_session, "Could not create client session");

    source = messaging_create_source("ingress");
    ASSERT_IS_NOT_NULL(source, "Could not create source");
    target = messaging_create_target("localhost/ingress");
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
    send_async_operation = messagesender_send_async(client_message_sender, client_send_message, on_message_send_complete, &sent_messages, 0);
    ASSERT_IS_NOT_NULL(send_async_operation, "cannot send message");
    message_destroy(client_send_message);

    start_time = time(NULL);
    while ((now_time = time(NULL)),
        (difftime(now_time, start_time) < TEST_TIMEOUT))
    {
        // schedule work for all components
        socketlistener_dowork(socket_listener);
        connection_dowork(client_connection);
        connection_dowork(server_instance.connection);

        // if we received the message, break
        if (server_instance.received_messages >= 1)
        {
            break;
        }

        ThreadAPI_Sleep(1);
    }

    // assert
    ASSERT_ARE_EQUAL(size_t, 1, sent_messages, "Bad sent messages count");
    ASSERT_ARE_EQUAL(size_t, 1, server_instance.received_messages, "Bad received messages count");

    // cleanup
    socketlistener_stop(socket_listener);
    messagesender_destroy(client_message_sender);
    link_destroy(client_link);
    session_destroy(client_session);
    connection_destroy(client_connection);
    xio_destroy(socket_io);

    messagereceiver_destroy(server_instance.message_receivers[0]);
    link_destroy(server_instance.links[0]);
    session_destroy(server_instance.sessions[0].session);
    connection_destroy(server_instance.connection);
    xio_destroy(server_instance.header_detect_io);
    xio_destroy(server_instance.underlying_io);
    socketlistener_destroy(socket_listener);
}

TEST_FUNCTION(client_and_server_connect_and_send_one_message_unsettled)
{
    // arrange
    int port_number = generate_port_number();
    SERVER_INSTANCE server_instance;
    SOCKET_LISTENER_HANDLE socket_listener = socketlistener_create(port_number);
    int result;
    XIO_HANDLE socket_io;
    CONNECTION_HANDLE client_connection;
    SESSION_HANDLE client_session;
    LINK_HANDLE client_link;
    MESSAGE_HANDLE client_send_message;
    MESSAGE_SENDER_HANDLE client_message_sender;
    size_t sent_messages;
    AMQP_VALUE source;
    AMQP_VALUE target;
    time_t now_time;
    time_t start_time;
    SOCKETIO_CONFIG socketio_config = { "localhost", 0, NULL };
    unsigned char hello[] = { 'H', 'e', 'l', 'l', 'o' };
    BINARY_DATA binary_data;
    ASYNC_OPERATION_HANDLE send_async_operation;

    server_instance.connection = NULL;
    server_instance.session_count = 0;
    server_instance.sessions[0].session = NULL;
    server_instance.link_count = 0;
    server_instance.links[0] = NULL;
    server_instance.message_receivers[0] = NULL;
    server_instance.received_messages = 0;

    sent_messages = 0;

    result = socketlistener_start(socket_listener, on_socket_accepted, &server_instance);
    ASSERT_ARE_EQUAL(int, 0, result, "socketlistener_start failed");

    // start the client
    socketio_config.port = port_number;
    socket_io = xio_create(socketio_get_interface_description(), &socketio_config);
    ASSERT_IS_NOT_NULL(socket_io, "Could not create socket IO");

    /* create the connection, session and link */
    client_connection = connection_create(socket_io, "localhost", "some", NULL, NULL);
    ASSERT_IS_NOT_NULL(client_connection, "Could not create client connection");

    (void)connection_set_trace(client_connection, true);
    client_session = session_create(client_connection, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_session, "Could not create client session");

    source = messaging_create_source("ingress");
    ASSERT_IS_NOT_NULL(source, "Could not create source");
    target = messaging_create_target("localhost/ingress");
    ASSERT_IS_NOT_NULL(target, "Could not create target");
    client_link = link_create(client_session, "sender-link", role_sender, source, target);
    ASSERT_IS_NOT_NULL(client_link, "Could not create client link");
    result = link_set_snd_settle_mode(client_link, sender_settle_mode_unsettled);
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
    send_async_operation = messagesender_send_async(client_message_sender, client_send_message, on_message_send_complete, &sent_messages, 0);
    ASSERT_IS_NOT_NULL(send_async_operation, "cannot send message");
    message_destroy(client_send_message);

    start_time = time(NULL);
    while ((now_time = time(NULL)),
        (difftime(now_time, start_time) < TEST_TIMEOUT))
    {
        // schedule work for all components
        socketlistener_dowork(socket_listener);
        connection_dowork(client_connection);
        connection_dowork(server_instance.connection);

        // if we received the message, break
        if ((server_instance.received_messages >= 1) &&
            (sent_messages >= 1))
        {
            break;
        }

        ThreadAPI_Sleep(1);
    }

    // assert
    ASSERT_ARE_EQUAL(size_t, 1, sent_messages, "Bad sent messages count");
    ASSERT_ARE_EQUAL(size_t, 1, server_instance.received_messages, "Bad received messages count");

    // cleanup
    socketlistener_stop(socket_listener);
    messagesender_destroy(client_message_sender);
    link_destroy(client_link);
    session_destroy(client_session);
    connection_destroy(client_connection);
    xio_destroy(socket_io);

    messagereceiver_destroy(server_instance.message_receivers[0]);
    link_destroy(server_instance.links[0]);
    session_destroy(server_instance.sessions[0].session);
    connection_destroy(server_instance.connection);
    xio_destroy(server_instance.header_detect_io);
    xio_destroy(server_instance.underlying_io);
    socketlistener_destroy(socket_listener);
}

TEST_FUNCTION(cancelling_a_send_works)
{
    // arrange
    int port_number = generate_port_number();
    SERVER_INSTANCE server_instance;
    SOCKET_LISTENER_HANDLE socket_listener = socketlistener_create(port_number);
    int result;
    XIO_HANDLE socket_io;
    CONNECTION_HANDLE client_connection;
    SESSION_HANDLE client_session;
    LINK_HANDLE client_link;
    MESSAGE_HANDLE client_send_message;
    MESSAGE_SENDER_HANDLE client_message_sender;
    size_t cancelled_messages;
    AMQP_VALUE source;
    AMQP_VALUE target;
    time_t now_time;
    time_t start_time;
    SOCKETIO_CONFIG socketio_config = { "localhost", 0, NULL };
    unsigned char hello[] = { 'H', 'e', 'l', 'l', 'o' };
    BINARY_DATA binary_data;
    ASYNC_OPERATION_HANDLE send_async_operation;

    server_instance.connection = NULL;
    server_instance.session_count = 0;
    server_instance.sessions[0].session = NULL;
    server_instance.link_count = 0;
    server_instance.links[0] = NULL;
    server_instance.message_receivers[0] = NULL;
    server_instance.received_messages = 0;

    cancelled_messages = 0;

    result = socketlistener_start(socket_listener, on_socket_accepted, &server_instance);
    ASSERT_ARE_EQUAL(int, 0, result, "socketlistener_start failed");

    // start the client
    socketio_config.port = port_number;
    socket_io = xio_create(socketio_get_interface_description(), &socketio_config);
    ASSERT_IS_NOT_NULL(socket_io, "Could not create socket IO");

    /* create the connection, session and link */
    client_connection = connection_create(socket_io, "localhost", "some", NULL, NULL);
    ASSERT_IS_NOT_NULL(client_connection, "Could not create client connection");

    (void)connection_set_trace(client_connection, true);
    client_session = session_create(client_connection, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_session, "Could not create client session");

    source = messaging_create_source("ingress");
    ASSERT_IS_NOT_NULL(source, "Could not create source");
    target = messaging_create_target("localhost/ingress");
    ASSERT_IS_NOT_NULL(target, "Could not create target");
    client_link = link_create(client_session, "sender-link", role_sender, source, target);
    ASSERT_IS_NOT_NULL(client_link, "Could not create client link");
    result = link_set_snd_settle_mode(client_link, sender_settle_mode_unsettled);
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
    send_async_operation = messagesender_send_async(client_message_sender, client_send_message, on_message_send_cancelled, &cancelled_messages, 0);
    ASSERT_IS_NOT_NULL(send_async_operation, "cannot send message");
    result = async_operation_cancel(send_async_operation);
    ASSERT_ARE_EQUAL(int, 0, result, "async operation cancel failed");
    message_destroy(client_send_message);

    start_time = time(NULL);
    while ((now_time = time(NULL)),
        (difftime(now_time, start_time) < TEST_TIMEOUT))
    {
        // schedule work for all components
        socketlistener_dowork(socket_listener);
        connection_dowork(client_connection);
        connection_dowork(server_instance.connection);

        // if we received the message, break
        if (cancelled_messages == 1)
        {
            break;
        }

        ThreadAPI_Sleep(1);
    }

    // assert
    ASSERT_ARE_EQUAL(size_t, 1, cancelled_messages, "Bad cancelled messages count");
    ASSERT_ARE_EQUAL(size_t, 0, server_instance.received_messages, "Bad received messages count");

    // cleanup
    socketlistener_stop(socket_listener);
    messagesender_destroy(client_message_sender);
    link_destroy(client_link);
    session_destroy(client_session);
    connection_destroy(client_connection);
    xio_destroy(socket_io);

    messagereceiver_destroy(server_instance.message_receivers[0]);
    link_destroy(server_instance.links[0]);
    session_destroy(server_instance.sessions[0].session);
    connection_destroy(server_instance.connection);
    xio_destroy(server_instance.header_detect_io);
    xio_destroy(server_instance.underlying_io);
    socketlistener_destroy(socket_listener);
}

TEST_FUNCTION(destroying_one_out_of_2_senders_works)
{
    // arrange
    int port_number = generate_port_number();
    SERVER_INSTANCE server_instance;
    SOCKET_LISTENER_HANDLE socket_listener = socketlistener_create(port_number);
    int result;
    XIO_HANDLE socket_io;
    CONNECTION_HANDLE client_connection;
    SESSION_HANDLE client_session;
    LINK_HANDLE client_link_1;
    LINK_HANDLE client_link_2;
    MESSAGE_HANDLE client_send_message;
    MESSAGE_SENDER_HANDLE client_message_sender_1;
    MESSAGE_SENDER_HANDLE client_message_sender_2;
    size_t sent_messages;
    AMQP_VALUE source;
    AMQP_VALUE target;
    time_t now_time;
    time_t start_time;
    SOCKETIO_CONFIG socketio_config = { "localhost", 0, NULL };
    unsigned char hello[] = { 'H', 'e', 'l', 'l', 'o' };
    BINARY_DATA binary_data;
    ASYNC_OPERATION_HANDLE send_async_operation;

    server_instance.connection = NULL;
    server_instance.session_count = 0;
    server_instance.sessions[0].session = NULL;
    server_instance.link_count = 0;
    server_instance.links[0] = NULL;
    server_instance.links[1] = NULL;
    server_instance.message_receivers[0] = NULL;
    server_instance.message_receivers[1] = NULL;
    server_instance.received_messages = 0;

    sent_messages = 0;

    result = socketlistener_start(socket_listener, on_socket_accepted, &server_instance);
    ASSERT_ARE_EQUAL(int, 0, result, "socketlistener_start failed");

    // start the client
    socketio_config.port = port_number;
    socket_io = xio_create(socketio_get_interface_description(), &socketio_config);
    ASSERT_IS_NOT_NULL(socket_io, "Could not create socket IO");

    /* create the connection, session and link */
    client_connection = connection_create(socket_io, "localhost", "some", NULL, NULL);
    ASSERT_IS_NOT_NULL(client_connection, "Could not create client connection");

    (void)connection_set_trace(client_connection, true);
    client_session = session_create(client_connection, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_session, "Could not create client session");

    source = messaging_create_source("ingress");
    ASSERT_IS_NOT_NULL(source, "Could not create source");
    target = messaging_create_target("localhost/ingress");
    ASSERT_IS_NOT_NULL(target, "Could not create target");

    // 1st sender link
    client_link_1 = link_create(client_session, "sender-link-1", role_sender, source, target);
    ASSERT_IS_NOT_NULL(client_link_1, "Could not create client link 1");
    result = link_set_snd_settle_mode(client_link_1, sender_settle_mode_unsettled);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set sender settle mode on link 1");

    // 2ndt sender link
    client_link_2 = link_create(client_session, "sender-link-2", role_sender, source, target);
    ASSERT_IS_NOT_NULL(client_link_2, "Could not create client link 2");
    result = link_set_snd_settle_mode(client_link_2, sender_settle_mode_unsettled);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set sender settle mode on link 2");

    amqpvalue_destroy(source);
    amqpvalue_destroy(target);

    client_send_message = message_create();
    ASSERT_IS_NOT_NULL(client_send_message, "Could not create message");
    binary_data.bytes = hello;
    binary_data.length = sizeof(hello);
    result = message_add_body_amqp_data(client_send_message, binary_data);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set message body");

    /* create the 1st message sender */
    client_message_sender_1 = messagesender_create(client_link_1, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_message_sender_1, "Could not create message sender 1");
    result = messagesender_open(client_message_sender_1);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot open message sender 1");

    /* create the 2nd message sender */
    client_message_sender_2 = messagesender_create(client_link_2, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_message_sender_2, "Could not create message sender 2");
    result = messagesender_open(client_message_sender_2);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot open message sender 2");

    // send message
    send_async_operation = messagesender_send_async(client_message_sender_1, client_send_message, on_message_send_complete, &sent_messages, 0);
    ASSERT_IS_NOT_NULL(send_async_operation, "cannot send message");

    // wait for either time elapsed or message received
    start_time = time(NULL);
    while ((now_time = time(NULL)),
        (difftime(now_time, start_time) < TEST_TIMEOUT))
    {
        // schedule work for all components
        socketlistener_dowork(socket_listener);
        connection_dowork(client_connection);
        connection_dowork(server_instance.connection);

        // if we received the message, break
        if (sent_messages == 1)
        {
            break;
        }

        ThreadAPI_Sleep(1);
    }

    ASSERT_ARE_EQUAL(size_t, 1, sent_messages, "Could not send one message");

    // detach link
    messagesender_destroy(client_message_sender_2);
    link_destroy(client_link_2);

    // send 2nd message
    send_async_operation = messagesender_send_async(client_message_sender_1, client_send_message, on_message_send_complete, &sent_messages, 0);
    ASSERT_IS_NOT_NULL(send_async_operation, "cannot send message");
    message_destroy(client_send_message);

    // wait for
    start_time = time(NULL);
    while ((now_time = time(NULL)),
        (difftime(now_time, start_time) < TEST_TIMEOUT))
    {
        // schedule work for all components
        socketlistener_dowork(socket_listener);
        connection_dowork(client_connection);
        connection_dowork(server_instance.connection);

        // if we received the message, break
        if (sent_messages == 2)
        {
            break;
        }

        ThreadAPI_Sleep(1);
    }

    // assert
    ASSERT_ARE_EQUAL(size_t, 2, sent_messages, "Bad sent messages count");
    ASSERT_ARE_EQUAL(size_t, 2, server_instance.received_messages, "Bad received messages count");

    // cleanup
    socketlistener_stop(socket_listener);
    messagesender_destroy(client_message_sender_1);
    link_destroy(client_link_1);
    session_destroy(client_session);
    connection_destroy(client_connection);
    xio_destroy(socket_io);

    messagereceiver_destroy(server_instance.message_receivers[0]);
    messagereceiver_destroy(server_instance.message_receivers[1]);
    link_destroy(server_instance.links[0]);
    link_destroy(server_instance.links[1]);
    session_destroy(server_instance.sessions[0].session);
    connection_destroy(server_instance.connection);
    xio_destroy(server_instance.header_detect_io);
    xio_destroy(server_instance.underlying_io);
    socketlistener_destroy(socket_listener);
}

static void on_connection_redirect_received(void* context, ERROR_HANDLE error)
{
    bool* redirect_received = (bool*)context;
    fields info = NULL;
    AMQP_VALUE hostname_key = amqpvalue_create_string("hostname");
    AMQP_VALUE network_host_key = amqpvalue_create_string("network-host");
    AMQP_VALUE port_key = amqpvalue_create_string("port");
    AMQP_VALUE hostname_value;
    AMQP_VALUE network_host_value;
    AMQP_VALUE port_value;
    const char* condition_string;
    const char* hostname_string;
    const char* network_host_string;
    uint16_t port_number;

    ASSERT_IS_NOT_NULL(error, "NULL error information");
    (void)error_get_condition(error, &condition_string);
    ASSERT_ARE_EQUAL(char_ptr, "amqp:connection:redirect", condition_string);
    (void)error_get_info(error, &info);
    ASSERT_IS_NOT_NULL(info, "NULL info in error");

    hostname_value = amqpvalue_get_map_value(info, hostname_key);
    ASSERT_IS_NOT_NULL(hostname_value, "NULL hostname_value");
    network_host_value = amqpvalue_get_map_value(info, network_host_key);
    ASSERT_IS_NOT_NULL(network_host_value, "NULL network_host_value");
    port_value = amqpvalue_get_map_value(info, port_key);
    ASSERT_IS_NOT_NULL(port_value, "NULL port_value");

    (void)amqpvalue_get_string(hostname_value, &hostname_string);
    ASSERT_ARE_EQUAL(char_ptr, test_redirect_hostname, hostname_string);
    (void)amqpvalue_get_string(network_host_value, &network_host_string);
    ASSERT_ARE_EQUAL(char_ptr, test_redirect_network_host, network_host_string);
    (void)amqpvalue_get_ushort(port_value, &port_number);
    ASSERT_ARE_EQUAL(uint16_t, test_redirect_port, port_number);

    amqpvalue_destroy(hostname_key);
    amqpvalue_destroy(network_host_key);
    amqpvalue_destroy(port_key);
    amqpvalue_destroy(hostname_value);
    amqpvalue_destroy(network_host_value);
    amqpvalue_destroy(port_value);

    *redirect_received = true;
}

static bool on_new_session_endpoint_connection_redirect(void* context, ENDPOINT_HANDLE new_endpoint)
{
    SERVER_INSTANCE* server = (SERVER_INSTANCE*)context;
    AMQP_VALUE redirect_map = amqpvalue_create_map();
    AMQP_VALUE hostname_key = amqpvalue_create_string("hostname");
    AMQP_VALUE network_host_key = amqpvalue_create_string("network-host");
    AMQP_VALUE port_key = amqpvalue_create_string("port");
    AMQP_VALUE hostname_value = amqpvalue_create_string(test_redirect_hostname);
    AMQP_VALUE network_host_value = amqpvalue_create_string(test_redirect_network_host);
    AMQP_VALUE port_value = amqpvalue_create_ushort(test_redirect_port);

    (void)new_endpoint;

    (void)amqpvalue_set_map_value(redirect_map, hostname_key, hostname_value);
    (void)amqpvalue_set_map_value(redirect_map, network_host_key, network_host_value);
    (void)amqpvalue_set_map_value(redirect_map, port_key, port_value);

    amqpvalue_destroy(hostname_key);
    amqpvalue_destroy(hostname_value);
    amqpvalue_destroy(network_host_key);
    amqpvalue_destroy(network_host_value);
    amqpvalue_destroy(port_key);
    amqpvalue_destroy(port_value);

    (void)connection_close(server->connection, connection_error_redirect, "Redirect", redirect_map);
    amqpvalue_destroy(redirect_map);

    return false;
}

static void on_socket_accepted_connection_redirect(void* context, const IO_INTERFACE_DESCRIPTION* interface_description, void* io_parameters)
{
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    HEADER_DETECT_ENTRY header_detect_entries[1];
    SERVER_INSTANCE* server = (SERVER_INSTANCE*)context;
    int result;
    AMQP_HEADER amqp_header;

    server->underlying_io = xio_create(interface_description, io_parameters);
    ASSERT_IS_NOT_NULL(server->underlying_io, "Could not create underlying IO");

    amqp_header = header_detect_io_get_amqp_header();
    header_detect_entries[0].header.header_bytes = amqp_header.header_bytes;
    header_detect_entries[0].header.header_size = amqp_header.header_size;
    header_detect_entries[0].io_interface_description = NULL;

    header_detect_io_config.underlying_io = server->underlying_io;
    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;

    server->header_detect_io = xio_create(header_detect_io_get_interface_description(), &header_detect_io_config);
    ASSERT_IS_NOT_NULL(server->header_detect_io, "Could not create header detect IO");
    server->connection = connection_create(server->header_detect_io, NULL, "1", on_new_session_endpoint_connection_redirect, server);
    ASSERT_IS_NOT_NULL(server->connection, "Could not create server connection");
    (void)connection_set_trace(server->connection, true);
    result = connection_listen(server->connection);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot start listening");
}

TEST_FUNCTION(connection_redirect_notifies_the_user_of_the_event)
{
    // arrange
    int port_number = generate_port_number();
    SERVER_INSTANCE server_instance;
    SOCKET_LISTENER_HANDLE socket_listener = socketlistener_create(port_number);
    int result;
    XIO_HANDLE socket_io;
    CONNECTION_HANDLE client_connection;
    SESSION_HANDLE client_session;
    LINK_HANDLE client_link;
    MESSAGE_SENDER_HANDLE client_message_sender;
    bool redirect_received;
    AMQP_VALUE source;
    AMQP_VALUE target;
    time_t now_time;
    time_t start_time;
    SOCKETIO_CONFIG socketio_config = { "localhost", 0, NULL };

    server_instance.connection = NULL;
    server_instance.session_count = 0;
    server_instance.sessions[0].session = NULL;
    server_instance.link_count = 0;
    server_instance.links[0] = NULL;
    server_instance.links[1] = NULL;
    server_instance.message_receivers[0] = NULL;
    server_instance.message_receivers[1] = NULL;
    server_instance.received_messages = 0;

    redirect_received = false;

    result = socketlistener_start(socket_listener, on_socket_accepted_connection_redirect, &server_instance);
    ASSERT_ARE_EQUAL(int, 0, result, "socketlistener_start failed");

    // start the client
    socketio_config.port = port_number;
    socket_io = xio_create(socketio_get_interface_description(), &socketio_config);
    ASSERT_IS_NOT_NULL(socket_io, "Could not create socket IO");

    /* create the connection, session and link */
    client_connection = connection_create(socket_io, "localhost", "some", NULL, NULL);
    ASSERT_IS_NOT_NULL(client_connection, "Could not create client connection");

    (void)connection_set_trace(client_connection, true);
    client_session = session_create(client_connection, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_session, "Could not create client session");

    (void)connection_subscribe_on_connection_close_received(client_connection, on_connection_redirect_received, &redirect_received);

    source = messaging_create_source("ingress");
    ASSERT_IS_NOT_NULL(source, "Could not create source");
    target = messaging_create_target("localhost/ingress");
    ASSERT_IS_NOT_NULL(target, "Could not create target");

    // link
    client_link = link_create(client_session, "sender-link-1", role_sender, source, target);
    ASSERT_IS_NOT_NULL(client_link, "Could not create client link");
    result = link_set_snd_settle_mode(client_link, sender_settle_mode_unsettled);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set sender settle mode on link");

    amqpvalue_destroy(source);
    amqpvalue_destroy(target);

    /* create the message sender */
    client_message_sender = messagesender_create(client_link, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_message_sender, "Could not create message sender");
    result = messagesender_open(client_message_sender);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot open message sender");

    // wait for either time elapsed or message received
    start_time = time(NULL);
    while ((now_time = time(NULL)),
        (difftime(now_time, start_time) < TEST_TIMEOUT))
    {
        // schedule work for all components
        socketlistener_dowork(socket_listener);
        connection_dowork(client_connection);
        connection_dowork(server_instance.connection);

        // if we received the message, break
        if (redirect_received)
        {
            break;
        }

        ThreadAPI_Sleep(1);
    }

    // assert
    ASSERT_IS_TRUE(redirect_received, "Redirect information not received");

    // cleanup
    socketlistener_stop(socket_listener);
    messagesender_destroy(client_message_sender);
    link_destroy(client_link);
    session_destroy(client_session);
    connection_destroy(client_connection);
    xio_destroy(socket_io);

    messagereceiver_destroy(server_instance.message_receivers[0]);
    messagereceiver_destroy(server_instance.message_receivers[1]);
    link_destroy(server_instance.links[0]);
    link_destroy(server_instance.links[1]);
    session_destroy(server_instance.sessions[0].session);
    connection_destroy(server_instance.connection);
    xio_destroy(server_instance.header_detect_io);
    xio_destroy(server_instance.underlying_io);
    socketlistener_destroy(socket_listener);
}

static void on_link_redirect_received(void* context, ERROR_HANDLE error)
{
    bool* redirect_received = (bool*)context;
    fields info = NULL;
    AMQP_VALUE hostname_key = amqpvalue_create_string("hostname");
    AMQP_VALUE network_host_key = amqpvalue_create_string("network-host");
    AMQP_VALUE port_key = amqpvalue_create_string("port");
    AMQP_VALUE address_key = amqpvalue_create_string("address");
    AMQP_VALUE hostname_value;
    AMQP_VALUE network_host_value;
    AMQP_VALUE port_value;
    AMQP_VALUE address_value;
    const char* condition_string;
    const char* hostname_string;
    const char* network_host_string;
    const char* address_string;
    uint16_t port_number;

    ASSERT_IS_NOT_NULL(error, "NULL error information");
    (void)error_get_condition(error, &condition_string);
    ASSERT_ARE_EQUAL(char_ptr, "amqp:link:redirect", condition_string);
    (void)error_get_info(error, &info);
    ASSERT_IS_NOT_NULL(info, "NULL info in error");

    hostname_value = amqpvalue_get_map_value(info, hostname_key);
    ASSERT_IS_NOT_NULL(hostname_value, "NULL hostname_value");
    network_host_value = amqpvalue_get_map_value(info, network_host_key);
    ASSERT_IS_NOT_NULL(network_host_value, "NULL network_host_value");
    port_value = amqpvalue_get_map_value(info, port_key);
    ASSERT_IS_NOT_NULL(port_value, "NULL port_value");
    address_value = amqpvalue_get_map_value(info, address_key);
    ASSERT_IS_NOT_NULL(address_value, "NULL address_value");

    (void)amqpvalue_get_string(hostname_value, &hostname_string);
    ASSERT_ARE_EQUAL(char_ptr, test_redirect_hostname, hostname_string);
    (void)amqpvalue_get_string(network_host_value, &network_host_string);
    ASSERT_ARE_EQUAL(char_ptr, test_redirect_network_host, network_host_string);
    (void)amqpvalue_get_ushort(port_value, &port_number);
    ASSERT_ARE_EQUAL(uint16_t, test_redirect_port, port_number);
    (void)amqpvalue_get_string(address_value, &address_string);
    ASSERT_ARE_EQUAL(char_ptr, test_redirect_address, address_string);

    amqpvalue_destroy(hostname_key);
    amqpvalue_destroy(network_host_key);
    amqpvalue_destroy(port_key);
    amqpvalue_destroy(address_key);
    amqpvalue_destroy(hostname_value);
    amqpvalue_destroy(network_host_value);
    amqpvalue_destroy(port_value);
    amqpvalue_destroy(address_value);

    *redirect_received = true;
}

static bool on_new_link_attached_link_redirect(void* context, LINK_ENDPOINT_HANDLE new_link_endpoint, const char* name, role role, AMQP_VALUE source, AMQP_VALUE target, fields properties)
{
    SERVER_SESSION* server_session = (SERVER_SESSION*)context;
	struct SERVER_INSTANCE_TAG* server = (struct SERVER_INSTANCE_TAG*)server_session->server;
    int result;
    AMQP_VALUE redirect_map = amqpvalue_create_map();
    AMQP_VALUE hostname_key = amqpvalue_create_string("hostname");
    AMQP_VALUE network_host_key = amqpvalue_create_string("network-host");
    AMQP_VALUE port_key = amqpvalue_create_string("port");
    AMQP_VALUE address_key = amqpvalue_create_string("address");
    AMQP_VALUE hostname_value = amqpvalue_create_string(test_redirect_hostname);
    AMQP_VALUE network_host_value = amqpvalue_create_string(test_redirect_network_host);
    AMQP_VALUE port_value = amqpvalue_create_ushort(test_redirect_port);
    AMQP_VALUE address_value = amqpvalue_create_string(test_redirect_address);

    (void)properties;
    (void)amqpvalue_set_map_value(redirect_map, hostname_key, hostname_value);
    (void)amqpvalue_set_map_value(redirect_map, network_host_key, network_host_value);
    (void)amqpvalue_set_map_value(redirect_map, port_key, port_value);
    (void)amqpvalue_set_map_value(redirect_map, address_key, address_value);

    amqpvalue_destroy(hostname_key);
    amqpvalue_destroy(hostname_value);
    amqpvalue_destroy(network_host_key);
    amqpvalue_destroy(network_host_value);
    amqpvalue_destroy(port_key);
    amqpvalue_destroy(port_value);
    amqpvalue_destroy(address_key);
    amqpvalue_destroy(address_value);

    server->links[server->link_count] = link_create_from_endpoint(server_session->session, new_link_endpoint, name, role, source, target);
    ASSERT_IS_NOT_NULL(server->links[server->link_count], "Could not create link");
    server->message_receivers[server->link_count] = messagereceiver_create(server->links[server->link_count], on_message_receivers_state_changed, server);
    ASSERT_IS_NOT_NULL(server->message_receivers[server->link_count], "Could not create message receiver");
    result = messagereceiver_open(server->message_receivers[server->link_count], on_message_received, server);
    ASSERT_ARE_EQUAL(int, 0, result, "message receiver open failed");
    (void)link_detach(server->links[server->link_count], true, "amqp:link:redirect", "Redirect", redirect_map);
    amqpvalue_destroy(redirect_map);
    server->link_count++;

    return true;
}

static bool on_new_session_endpoint_link_redirect(void* context, ENDPOINT_HANDLE new_endpoint)
{
    SERVER_INSTANCE* server = (SERVER_INSTANCE*)context;
    int result;

    SESSION_HANDLE session = session_create_from_endpoint(server->connection, new_endpoint, on_new_link_attached_link_redirect, &server->sessions[server->session_count]);
    server->sessions[server->session_count].server = server;
    server->sessions[server->session_count].session = session;
    server->session_count++;
    ASSERT_IS_NOT_NULL(session, "Could not create server session");
    result = session_begin(session);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot begin server session");

    return true;
}

static void on_socket_accepted_link_redirect(void* context, const IO_INTERFACE_DESCRIPTION* interface_description, void* io_parameters)
{
    HEADER_DETECT_IO_CONFIG header_detect_io_config;
    HEADER_DETECT_ENTRY header_detect_entries[1];
    SERVER_INSTANCE* server = (SERVER_INSTANCE*)context;
    int result;
    AMQP_HEADER amqp_header;

    server->underlying_io = xio_create(interface_description, io_parameters);
    ASSERT_IS_NOT_NULL(server->underlying_io, "Could not create underlying IO");

    amqp_header = header_detect_io_get_amqp_header();
    header_detect_entries[0].header.header_bytes = amqp_header.header_bytes;
    header_detect_entries[0].header.header_size = amqp_header.header_size;
    header_detect_entries[0].io_interface_description = NULL;

    header_detect_io_config.underlying_io = server->underlying_io;
    header_detect_io_config.header_detect_entry_count = 1;
    header_detect_io_config.header_detect_entries = header_detect_entries;

    server->header_detect_io = xio_create(header_detect_io_get_interface_description(), &header_detect_io_config);
    ASSERT_IS_NOT_NULL(server->header_detect_io, "Could not create header detect IO");
    server->connection = connection_create(server->header_detect_io, NULL, "1", on_new_session_endpoint_link_redirect, server);
    ASSERT_IS_NOT_NULL(server->connection, "Could not create server connection");
    (void)connection_set_trace(server->connection, true);
    result = connection_listen(server->connection);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot start listening");
}

TEST_FUNCTION(link_redirect_notifies_the_user_of_the_event)
{
    // arrange
    int port_number = generate_port_number();
    SERVER_INSTANCE server_instance;
    SOCKET_LISTENER_HANDLE socket_listener = socketlistener_create(port_number);
    int result;
    XIO_HANDLE socket_io;
    CONNECTION_HANDLE client_connection;
    SESSION_HANDLE client_session;
    LINK_HANDLE client_link;
    MESSAGE_SENDER_HANDLE client_message_sender;
    bool redirect_received;
    AMQP_VALUE source;
    AMQP_VALUE target;
    time_t now_time;
    time_t start_time;
    SOCKETIO_CONFIG socketio_config = { "localhost", 0, NULL };

    server_instance.connection = NULL;
    server_instance.session_count = 0;
    server_instance.sessions[0].session = NULL;
    server_instance.link_count = 0;
    server_instance.links[0] = NULL;
    server_instance.links[1] = NULL;
    server_instance.message_receivers[0] = NULL;
    server_instance.message_receivers[1] = NULL;
    server_instance.received_messages = 0;

    redirect_received = false;

    result = socketlistener_start(socket_listener, on_socket_accepted_link_redirect, &server_instance);
    ASSERT_ARE_EQUAL(int, 0, result, "socketlistener_start failed");

    // start the client
    socketio_config.port = port_number;
    socket_io = xio_create(socketio_get_interface_description(), &socketio_config);
    ASSERT_IS_NOT_NULL(socket_io, "Could not create socket IO");

    /* create the connection, session and link */
    client_connection = connection_create(socket_io, "localhost", "some", NULL, NULL);
    ASSERT_IS_NOT_NULL(client_connection, "Could not create client connection");

    (void)connection_set_trace(client_connection, true);
    client_session = session_create(client_connection, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_session, "Could not create client session");

    source = messaging_create_source("ingress");
    ASSERT_IS_NOT_NULL(source, "Could not create source");
    target = messaging_create_target("localhost/ingress");
    ASSERT_IS_NOT_NULL(target, "Could not create target");

    // link
    client_link = link_create(client_session, "sender-link-1", role_sender, source, target);
    ASSERT_IS_NOT_NULL(client_link, "Could not create client link");
    result = link_set_snd_settle_mode(client_link, sender_settle_mode_unsettled);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set sender settle mode on link");

    (void)link_subscribe_on_link_detach_received(client_link, on_link_redirect_received, &redirect_received);

    amqpvalue_destroy(source);
    amqpvalue_destroy(target);

    /* create the message sender */
    client_message_sender = messagesender_create(client_link, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_message_sender, "Could not create message sender");
    result = messagesender_open(client_message_sender);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot open message sender");

    // wait for either time elapsed or message received
    start_time = time(NULL);
    while ((now_time = time(NULL)),
        (difftime(now_time, start_time) < TEST_TIMEOUT))
    {
        // schedule work for all components
        socketlistener_dowork(socket_listener);
        connection_dowork(client_connection);
        connection_dowork(server_instance.connection);

        // if we received the message, break
        if (redirect_received)
        {
            break;
        }

        ThreadAPI_Sleep(1);
    }

    // assert
    ASSERT_IS_TRUE(redirect_received, "Redirect information not received");

    // cleanup
    socketlistener_stop(socket_listener);
    messagesender_destroy(client_message_sender);
    link_destroy(client_link);
    session_destroy(client_session);
    connection_destroy(client_connection);
    xio_destroy(socket_io);

    messagereceiver_destroy(server_instance.message_receivers[0]);
    messagereceiver_destroy(server_instance.message_receivers[1]);
    link_destroy(server_instance.links[0]);
    link_destroy(server_instance.links[1]);
    session_destroy(server_instance.sessions[0].session);
    connection_destroy(server_instance.connection);
    xio_destroy(server_instance.header_detect_io);
    xio_destroy(server_instance.underlying_io);
    socketlistener_destroy(socket_listener);
}

TEST_FUNCTION(link_redirects_for_2_links_on_1_session_work)
{
    // arrange
    int port_number = generate_port_number();
    SERVER_INSTANCE server_instance;
    SOCKET_LISTENER_HANDLE socket_listener = socketlistener_create(port_number);
    int result;
    XIO_HANDLE socket_io;
    CONNECTION_HANDLE client_connection;
    SESSION_HANDLE client_session_1;
    LINK_HANDLE client_link_1;
    LINK_HANDLE client_link_2;
    MESSAGE_SENDER_HANDLE client_message_sender_1;
    MESSAGE_SENDER_HANDLE client_message_sender_2;
    bool redirect_received_1;
    bool redirect_received_2;
    AMQP_VALUE source;
    AMQP_VALUE target;
    time_t now_time;
    time_t start_time;
    SOCKETIO_CONFIG socketio_config = { "localhost", 0, NULL };

    server_instance.connection = NULL;
    server_instance.session_count = 0;
    server_instance.sessions[0].session = NULL;
    server_instance.link_count = 0;
    server_instance.links[0] = NULL;
    server_instance.links[1] = NULL;
    server_instance.message_receivers[0] = NULL;
    server_instance.message_receivers[1] = NULL;
    server_instance.received_messages = 0;

    redirect_received_1 = false;
    redirect_received_2 = false;

    result = socketlistener_start(socket_listener, on_socket_accepted_link_redirect, &server_instance);
    ASSERT_ARE_EQUAL(int, 0, result, "socketlistener_start failed");

    // start the client
    socketio_config.port = port_number;
    socket_io = xio_create(socketio_get_interface_description(), &socketio_config);
    ASSERT_IS_NOT_NULL(socket_io, "Could not create socket IO");

    /* create the connection and sessions */
    client_connection = connection_create(socket_io, "localhost", "some", NULL, NULL);
    ASSERT_IS_NOT_NULL(client_connection, "Could not create client connection");

    (void)connection_set_trace(client_connection, true);
    client_session_1 = session_create(client_connection, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_session_1, "Could not create client session 1");

    source = messaging_create_source("ingress");
    ASSERT_IS_NOT_NULL(source, "Could not create source");
    target = messaging_create_target("localhost/ingress");
    ASSERT_IS_NOT_NULL(target, "Could not create target");

    // links
    client_link_1 = link_create(client_session_1, "sender-link-1", role_sender, source, target);
    ASSERT_IS_NOT_NULL(client_link_1, "Could not create client link 1");
    result = link_set_snd_settle_mode(client_link_1, sender_settle_mode_unsettled);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set sender settle mode on link 1");

    client_link_2 = link_create(client_session_1, "sender-link-2", role_sender, source, target);
    ASSERT_IS_NOT_NULL(client_link_2, "Could not create client link 2");
    result = link_set_snd_settle_mode(client_link_2, sender_settle_mode_unsettled);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set sender settle mode on link 2");

    (void)link_subscribe_on_link_detach_received(client_link_1, on_link_redirect_received, &redirect_received_1);
    (void)link_subscribe_on_link_detach_received(client_link_2, on_link_redirect_received, &redirect_received_2);

    amqpvalue_destroy(source);
    amqpvalue_destroy(target);

    /* create the message senders */
    client_message_sender_1 = messagesender_create(client_link_1, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_message_sender_1, "Could not create message sender 1");
    result = messagesender_open(client_message_sender_1);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot open message sender 1");

    client_message_sender_2 = messagesender_create(client_link_2, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_message_sender_2, "Could not create message sender 2");
    result = messagesender_open(client_message_sender_2);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot open message sender 2");

    // wait for either time elapsed or message received
    start_time = time(NULL);
    while ((now_time = time(NULL)),
        (difftime(now_time, start_time) < TEST_TIMEOUT))
    {
        // schedule work for all components
        socketlistener_dowork(socket_listener);
        connection_dowork(client_connection);
        connection_dowork(server_instance.connection);

        // if we received the message, break
        if (redirect_received_1 && redirect_received_2)
        {
            break;
        }

        ThreadAPI_Sleep(1);
    }

    // assert
    ASSERT_IS_TRUE(redirect_received_1, "Redirect information not received for link 1");
    ASSERT_IS_TRUE(redirect_received_2, "Redirect information not received for link 2");

    // cleanup
    socketlistener_stop(socket_listener);
    messagesender_destroy(client_message_sender_1);
    messagesender_destroy(client_message_sender_2);
    link_destroy(client_link_1);
    link_destroy(client_link_2);
    session_destroy(client_session_1);
    connection_destroy(client_connection);
    xio_destroy(socket_io);

    messagereceiver_destroy(server_instance.message_receivers[0]);
    messagereceiver_destroy(server_instance.message_receivers[1]);
    link_destroy(server_instance.links[0]);
    link_destroy(server_instance.links[1]);
    session_destroy(server_instance.sessions[0].session);
    connection_destroy(server_instance.connection);
    xio_destroy(server_instance.header_detect_io);
    xio_destroy(server_instance.underlying_io);
    socketlistener_destroy(socket_listener);
}

TEST_FUNCTION(link_redirects_for_2_links_on_2_different_sessions_work)
{
    // arrange
    int port_number = generate_port_number();
    SERVER_INSTANCE server_instance;
    SOCKET_LISTENER_HANDLE socket_listener = socketlistener_create(port_number);
    int result;
    XIO_HANDLE socket_io;
    CONNECTION_HANDLE client_connection;
    SESSION_HANDLE client_session_1;
    SESSION_HANDLE client_session_2;
    LINK_HANDLE client_link_1;
    LINK_HANDLE client_link_2;
    MESSAGE_SENDER_HANDLE client_message_sender_1;
    MESSAGE_SENDER_HANDLE client_message_sender_2;
    bool redirect_received_1;
    bool redirect_received_2;
    AMQP_VALUE source;
    AMQP_VALUE target;
    time_t now_time;
    time_t start_time;
    SOCKETIO_CONFIG socketio_config = { "localhost", 0, NULL };

    server_instance.connection = NULL;
    server_instance.session_count = 0;
    server_instance.sessions[0].session = NULL;
    server_instance.sessions[1].session = NULL;
    server_instance.link_count = 0;
    server_instance.links[0] = NULL;
    server_instance.links[1] = NULL;
    server_instance.message_receivers[0] = NULL;
    server_instance.message_receivers[1] = NULL;
    server_instance.received_messages = 0;

    redirect_received_1 = false;
    redirect_received_2 = false;

    result = socketlistener_start(socket_listener, on_socket_accepted_link_redirect, &server_instance);
    ASSERT_ARE_EQUAL(int, 0, result, "socketlistener_start failed");

    // start the client
    socketio_config.port = port_number;
    socket_io = xio_create(socketio_get_interface_description(), &socketio_config);
    ASSERT_IS_NOT_NULL(socket_io, "Could not create socket IO");

    /* create the connection and sessions */
    client_connection = connection_create(socket_io, "localhost", "some", NULL, NULL);
    ASSERT_IS_NOT_NULL(client_connection, "Could not create client connection");

    (void)connection_set_trace(client_connection, true);
    client_session_1 = session_create(client_connection, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_session_1, "Could not create client session 1");

    client_session_2 = session_create(client_connection, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_session_2, "Could not create client session 2");

    source = messaging_create_source("ingress");
    ASSERT_IS_NOT_NULL(source, "Could not create source");
    target = messaging_create_target("localhost/ingress");
    ASSERT_IS_NOT_NULL(target, "Could not create target");

    // links
    client_link_1 = link_create(client_session_1, "sender-link-1", role_sender, source, target);
    ASSERT_IS_NOT_NULL(client_link_1, "Could not create client link 1");
    result = link_set_snd_settle_mode(client_link_1, sender_settle_mode_unsettled);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set sender settle mode on link 1");

    client_link_2 = link_create(client_session_2, "sender-link-2", role_sender, source, target);
    ASSERT_IS_NOT_NULL(client_link_2, "Could not create client link 2");
    result = link_set_snd_settle_mode(client_link_2, sender_settle_mode_unsettled);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set sender settle mode on link 2");

    (void)link_subscribe_on_link_detach_received(client_link_1, on_link_redirect_received, &redirect_received_1);
    (void)link_subscribe_on_link_detach_received(client_link_2, on_link_redirect_received, &redirect_received_2);

    amqpvalue_destroy(source);
    amqpvalue_destroy(target);

    /* create the message senders */
    client_message_sender_1 = messagesender_create(client_link_1, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_message_sender_1, "Could not create message sender 1");
    result = messagesender_open(client_message_sender_1);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot open message sender 1");

    client_message_sender_2 = messagesender_create(client_link_2, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_message_sender_2, "Could not create message sender 2");
    result = messagesender_open(client_message_sender_2);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot open message sender 2");

    // wait for either time elapsed or message received
    start_time = time(NULL);
    while ((now_time = time(NULL)),
        (difftime(now_time, start_time) < TEST_TIMEOUT))
    {
        // schedule work for all components
        socketlistener_dowork(socket_listener);
        connection_dowork(client_connection);
        connection_dowork(server_instance.connection);

        // if we received the message, break
        if (redirect_received_1 && redirect_received_2)
        {
            break;
        }

        ThreadAPI_Sleep(1);
    }

    // assert
    ASSERT_IS_TRUE(redirect_received_1, "Redirect information not received for link 1");
    ASSERT_IS_TRUE(redirect_received_2, "Redirect information not received for link 2");

    // cleanup
    socketlistener_stop(socket_listener);
    messagesender_destroy(client_message_sender_1);
    messagesender_destroy(client_message_sender_2);
    link_destroy(client_link_1);
    link_destroy(client_link_2);
    session_destroy(client_session_1);
    session_destroy(client_session_2);
    connection_destroy(client_connection);
    xio_destroy(socket_io);

    messagereceiver_destroy(server_instance.message_receivers[0]);
    messagereceiver_destroy(server_instance.message_receivers[1]);
    link_destroy(server_instance.links[0]);
    link_destroy(server_instance.links[1]);
    session_destroy(server_instance.sessions[0].session);
    session_destroy(server_instance.sessions[1].session);
    connection_destroy(server_instance.connection);
    xio_destroy(server_instance.header_detect_io);
    xio_destroy(server_instance.underlying_io);
    socketlistener_destroy(socket_listener);
}

TEST_FUNCTION(client_and_server_connect_and_send_one_message_with_all_message_parts)
{
    // arrange
    int port_number = generate_port_number();
    SERVER_INSTANCE server_instance;
    SOCKET_LISTENER_HANDLE socket_listener = socketlistener_create(port_number);
    int result;
    XIO_HANDLE socket_io;
    CONNECTION_HANDLE client_connection;
    SESSION_HANDLE client_session;
    LINK_HANDLE client_link;
    MESSAGE_HANDLE client_send_message;
    MESSAGE_SENDER_HANDLE client_message_sender;
    size_t sent_messages;
    AMQP_VALUE source;
    AMQP_VALUE target;
    time_t now_time;
    time_t start_time;
    SOCKETIO_CONFIG socketio_config = { "localhost", 0, NULL };
    unsigned char hello[] = { 'H', 'e', 'l', 'l', 'o' };
    BINARY_DATA binary_data;
    ASYNC_OPERATION_HANDLE send_async_operation;
    HEADER_HANDLE message_header;
    AMQP_VALUE message_footer;
    AMQP_VALUE footer_key_1;
    AMQP_VALUE footer_value_1;
    delivery_annotations delivery_annotations_map;
    AMQP_VALUE delivery_annotations_key_1;
    AMQP_VALUE delivery_annotations_value_1;
    AMQP_VALUE message_annotations_map;
    message_annotations message_annotations_instance;
    AMQP_VALUE message_annotations_key_1;
    AMQP_VALUE message_annotations_value_1;
    PROPERTIES_HANDLE message_properties;
    AMQP_VALUE message_id;
    AMQP_VALUE to_value;
    AMQP_VALUE reply_to_value;
    AMQP_VALUE correlation_id;
    amqp_binary user_id_binary;

    server_instance.connection = NULL;
    server_instance.session_count = 0;
    server_instance.sessions[0].session = NULL;
    server_instance.link_count = 0;
    server_instance.links[0] = NULL;
    server_instance.message_receivers[0] = NULL;
    server_instance.received_messages = 0;

    sent_messages = 0;

    result = socketlistener_start(socket_listener, on_socket_accepted, &server_instance);
    ASSERT_ARE_EQUAL(int, 0, result, "socketlistener_start failed");

    // start the client
    socketio_config.port = port_number;
    socket_io = xio_create(socketio_get_interface_description(), &socketio_config);
    ASSERT_IS_NOT_NULL(socket_io, "Could not create socket IO");

    /* create the connection, session and link */
    client_connection = connection_create(socket_io, "localhost", "some", NULL, NULL);
    ASSERT_IS_NOT_NULL(client_connection, "Could not create client connection");

    (void)connection_set_trace(client_connection, true);
    client_session = session_create(client_connection, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_session, "Could not create client session");

    source = messaging_create_source("ingress");
    ASSERT_IS_NOT_NULL(source, "Could not create source");
    target = messaging_create_target("localhost/ingress");
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

    // add a message header
    message_header = header_create();
    ASSERT_IS_NOT_NULL(message_header, "Could not create message header");
    result = header_set_durable(message_header, true);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set durable on message header");
    result = header_set_priority(message_header, 1);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set priority on message header");
    result = header_set_ttl(message_header, 42);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set ttl on message header");
    result = header_set_first_acquirer(message_header, true);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set first-acquirer on message header");
    result = header_set_delivery_count(message_header, 45);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set delivery-count on message header");
    result = message_set_header(client_send_message, message_header);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set message header");
    header_destroy(message_header);

    // add delivery annotations
    delivery_annotations_map = amqpvalue_create_map();
    ASSERT_IS_NOT_NULL(delivery_annotations_map, "Could not create delivery annotation map");
    delivery_annotations_key_1 = amqpvalue_create_string("teststring_42");
    ASSERT_IS_NOT_NULL(delivery_annotations_key_1, "Could not create delivery annotations key 1");
    delivery_annotations_value_1 = amqpvalue_create_string("hagauaga");
    ASSERT_IS_NOT_NULL(delivery_annotations_value_1, "Could not create delivery annotations value 1");
    result = amqpvalue_set_map_value(delivery_annotations_map, delivery_annotations_key_1, delivery_annotations_value_1);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set value in delivery annotations map");
    result = message_set_delivery_annotations(client_send_message, delivery_annotations_map);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set message delivery annotations");
    amqpvalue_destroy(delivery_annotations_map);
    amqpvalue_destroy(delivery_annotations_key_1);
    amqpvalue_destroy(delivery_annotations_value_1);

    // add message annotations
    message_annotations_map = amqpvalue_create_map();
    ASSERT_IS_NOT_NULL(message_annotations_map, "Could not create message annotation map");
    message_annotations_key_1 = amqpvalue_create_string("teststring_42");
    ASSERT_IS_NOT_NULL(message_annotations_key_1, "Could not create message annotations key 1");
    message_annotations_value_1 = amqpvalue_create_string("hagauaga");
    ASSERT_IS_NOT_NULL(message_annotations_value_1, "Could not create message annotations value 1");
    result = amqpvalue_set_map_value(message_annotations_map, message_annotations_key_1, message_annotations_value_1);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set value in message annotations map");
    message_annotations_instance = (message_annotations)amqpvalue_create_message_annotations(message_annotations_map);
    ASSERT_IS_NOT_NULL(message_annotations_instance, "Could not create message annotations");
    result = message_set_message_annotations(client_send_message, message_annotations_instance);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set message annotations");
    annotations_destroy(message_annotations_instance);
    amqpvalue_destroy(message_annotations_map);
    amqpvalue_destroy(message_annotations_key_1);
    amqpvalue_destroy(message_annotations_value_1);

    // add message properties
    message_properties = properties_create();
    ASSERT_IS_NOT_NULL(message_properties, "Could not create message properties");
    message_id = amqpvalue_create_string("msg-X");
    ASSERT_IS_NOT_NULL(message_id, "Could not create message id");
    result = properties_set_message_id(message_properties, message_id);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set message-id on message properties");
    amqp_value_destroy(message_id);
    user_id_binary.bytes = "\0x42";
    user_id_binary.length = 1;
    result = properties_set_user_id(message_properties, user_id_binary);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set user-id on message properties");
    to_value = messaging_create_source("blahblah");
    ASSERT_IS_NOT_NULL(to_value, "Could not create to value");
    result = properties_set_to(message_properties, to_value);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set to on message properties");
    amqpvalue_destroy(to_value);
    result = properties_set_subject(message_properties, "123");
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set subject on message properties");
    reply_to_value = messaging_create_source("blahblah-reply-to");
    ASSERT_IS_NOT_NULL(reply_to_value, "Could not create reply-to value");
    result = properties_set_reply_to(message_properties, reply_to_value);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set reply-to on message properties");
    amqpvalue_destroy(reply_to_value);
    correlation_id = amqpvalue_create_string("msg-Y");
    ASSERT_IS_NOT_NULL(correlation_id, "Could not create correlation-id value");
    result = properties_set_reply_to(message_properties, correlation_id);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set correlation-id on message properties");
    amqpvalue_destroy(correlation_id);
    result = properties_set_content_type(message_properties, "text");
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set content-type on message properties");
    result = properties_set_content_encoding(message_properties, "to_json_or_not_to_json");
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set content-encoding on message properties");
    result = properties_set_absolute_expiry_time(message_properties, 42);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set absolute-expiry-time on message properties");
    result = properties_set_creation_time(message_properties, 43);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set creation-time on message properties");
    result = properties_set_group_id(message_properties, "argh");
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set group-id on message properties");
    result = properties_set_group_sequence(message_properties, 0x4242);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set group-sequence on message properties");
    result = properties_set_reply_to_group_id(message_properties, "I am a pirate");
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set reply-to-group-id on message properties");
    result = message_set_properties(client_send_message, message_properties);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set message properties");
    properties_destroy(message_properties);

    // add message footer
    message_footer = amqpvalue_create_map();
    ASSERT_IS_NOT_NULL(message_footer, "Could not create message footer");
    footer_key_1 = amqpvalue_create_string("teststring_42");
    ASSERT_IS_NOT_NULL(footer_key_1, "Could not create footer key 1");
    footer_value_1 = amqpvalue_create_string("hagauaga");
    ASSERT_IS_NOT_NULL(footer_value_1, "Could not create footer value 1");
    result = amqpvalue_set_map_value(message_footer, footer_key_1, footer_value_1);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set value in footer map");
    result = message_set_footer(client_send_message, message_footer);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot set message footer");
    amqpvalue_destroy(message_footer);
    amqpvalue_destroy(footer_key_1);
    amqpvalue_destroy(footer_value_1);

    /* create a message sender */
    client_message_sender = messagesender_create(client_link, NULL, NULL);
    ASSERT_IS_NOT_NULL(client_message_sender, "Could not create message sender");

    // enable tracing of message payloads to expose any potential leaks
    messagesender_set_trace(client_message_sender, true);

    result = messagesender_open(client_message_sender);
    ASSERT_ARE_EQUAL(int, 0, result, "cannot open message sender");
    send_async_operation = messagesender_send_async(client_message_sender, client_send_message, on_message_send_complete, &sent_messages, 0);
    ASSERT_IS_NOT_NULL(send_async_operation, "cannot send message");
    message_destroy(client_send_message);

    start_time = time(NULL);
    while ((now_time = time(NULL)),
        (difftime(now_time, start_time) < TEST_TIMEOUT))
    {
        // schedule work for all components
        socketlistener_dowork(socket_listener);
        connection_dowork(client_connection);
        connection_dowork(server_instance.connection);

        // if we received the message, break
        if (server_instance.received_messages >= 1)
        {
            break;
        }

        ThreadAPI_Sleep(1);
    }

    // assert
    ASSERT_ARE_EQUAL(size_t, 1, sent_messages, "Bad sent messages count");
    ASSERT_ARE_EQUAL(size_t, 1, server_instance.received_messages, "Bad received messages count");

    // cleanup
    socketlistener_stop(socket_listener);
    messagesender_destroy(client_message_sender);
    link_destroy(client_link);
    session_destroy(client_session);
    connection_destroy(client_connection);
    xio_destroy(socket_io);

    messagereceiver_destroy(server_instance.message_receivers[0]);
    link_destroy(server_instance.links[0]);
    session_destroy(server_instance.sessions[0].session);
    connection_destroy(server_instance.connection);
    xio_destroy(server_instance.header_detect_io);
    xio_destroy(server_instance.underlying_io);
    socketlistener_destroy(socket_listener);
}

END_TEST_SUITE(local_client_server_tcp_e2e)
