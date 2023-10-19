// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "stdio.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/tlsio.h"
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
        XIO_HANDLE tlsio = (XIO_HANDLE)context;
        const char to_send[] = "GET / HTTP/1.1\r\n"
            "Host: www.google.com\r\n"
            "\r\n";
        (void)printf("Sending bytes ...\r\n");
        if (xio_send(tlsio, to_send, sizeof(to_send), on_send_complete, NULL) != 0)
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
        const IO_INTERFACE_DESCRIPTION* tlsio_interface = platform_get_default_tlsio();
        if (tlsio_interface == NULL)
        {
            (void)printf("Error getting tlsio interface description.");
            result = MU_FAILURE;
        }
        else
        {
            TLSIO_CONFIG tlsio_config = { "www.google.com", 443, NULL, NULL};
            XIO_HANDLE tlsio;

            tlsio = xio_create(tlsio_interface, &tlsio_config);
            if (tlsio == NULL)
            {
                (void)printf("Error creating TLS IO.");
                result = MU_FAILURE;
            }
            else
            {
                if (xio_open(tlsio, on_io_open_complete, tlsio, on_io_bytes_received, tlsio, on_io_error, tlsio) != 0)
                {
                    (void)printf("Error opening TLS IO.");
                    result = MU_FAILURE;
                }
                else
                {
                    unsigned char done = 0;
                    while (!done)
                    {
                        xio_dowork(tlsio);
                    }

                    result = 0;
                }

                xio_destroy(tlsio);
            }
        }

        platform_deinit();
    }

    return result;
}
