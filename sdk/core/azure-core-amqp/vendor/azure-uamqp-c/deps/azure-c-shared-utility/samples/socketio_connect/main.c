// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "stdio.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/platform.h"

// A simple sample callback.
static void on_send_complete(void* context, IO_SEND_RESULT send_result)
{
    (void)context;
    (void)send_result;
}

static void on_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    (void)context, (void)open_result;
    (void)printf("Open complete called\r\n");

    if (open_result == IO_OPEN_OK)
    {
        XIO_HANDLE socketio = (XIO_HANDLE)context;
        const char to_send[] = "GET / HTTP/1.1\r\n"
            "Host: www.google.com\r\n"
            "\r\n";
        (void)printf("Sending bytes ...\r\n");
        if (xio_send(socketio, to_send, sizeof(to_send), on_send_complete, NULL) != 0)
        {
            (void)printf("Send failed\r\n");
        }
    }
    else
    {
        (void)printf("Open error\r\n");
    }
}

static void on_io_bytes_received(void* context, const unsigned char* buffer, size_t size)
{
    (void)context, (void)buffer;
    (void)printf("Received %lu bytes\r\n", (unsigned long)size);
}

static void on_io_error(void* context)
{
    (void)context;
    (void)printf("IO reported an error\r\n");
}

int main(int argc, char** argv)
{
    int result;

    (void)argc, (void)argv;

    if (platform_init() != 0)
    {
        (void)printf("Cannot initialize platform.");
        result = MU_FAILURE;
    }
    else
    {
        const IO_INTERFACE_DESCRIPTION* socketio_interface = socketio_get_interface_description();
        if (socketio_interface == NULL)
        {
            (void)printf("Error getting socketio interface description.");
            result = MU_FAILURE;
        }
        else
        {
            SOCKETIO_CONFIG socketio_config;
            XIO_HANDLE socketio;

            socketio_config.hostname = "www.google.com";
            socketio_config.port = 80;
            socketio = xio_create(socketio_interface, &socketio_config);
            if (socketio == NULL)
            {
                (void)printf("Error creating socket IO.");
                result = MU_FAILURE;
            }
            else
            {
                if (xio_open(socketio, on_io_open_complete, socketio, on_io_bytes_received, socketio, on_io_error, socketio) != 0)
                {
                    (void)printf("Error opening socket IO.");
                    result = MU_FAILURE;
                }
                else
                {
                    unsigned char done = 0;
                    while (!done)
                    {
                        xio_dowork(socketio);
                    }

                    result = 0;
                }

                xio_destroy(socketio);
            }
        }

        platform_deinit();
    }

    return result;
}
